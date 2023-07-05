# -*- coding: utf-8 -*-
#
# test_iaf_ps_psp_accuracy.py
#
# This file is part of NEST.
#
# Copyright (C) 2004 The NEST Initiative
#
# NEST is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

"""
test_iaf_ps_psp_accuracy.sli checks the voltage response of the precise versions of the iaf_psc_alpha
model neurons to a single incoming spike. The voltage excursion is
called postsynaptic potential (PSP). In the model neurons
the postsynaptic current is described by an alpha-function
(see [1] and references therein). The resulting PSP has a finite
rise-time, with voltage and current beeing zero in the initial
condition (see [1]).

The dynamics is tested by connecting a device that emits spikes
at individually configurable times (see test_spike_generator) to
a model neuron.

The weight of the connection specifies the peak value (amplitude)
of the postsynaptic current (PSC) in pA.

The subthreshold dynamics of the model neurons is integrated exactly.
Therefore, it is suitable to check whether the simulation kernel
produces results independent of the computation step size
(resolution).

In order to obtain identical results for different computation
step sizes h, the SLI script needs to be independent of h.
This is achieved by specifying all time parameters in milliseconds
(ms). In particular the time of spike emission and the synaptic
delay need to be integer multiples of the computation step sizes
to be tested. test_iaf_dc_aligned_delay demonstrates the strategy
for the case of DC current input.

References:
  [1] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
      systems with applications to neuronal modeling. Biologial Cybernetics
      81:381-402.

Author:  May 2005, February 2008, Diesmann
SeeAlso: testsuite::test_iaf_psp, testsuite::test_iaf_ps_dc_accuracy
"""

import nest
import pytest
import math
from math import exp

# Global parameters
T = 6.
tau_syn = 0.3
tau_m = 10.
C_m = 250.
delay = 1.
weight = 500.

neuron_params = {"E_L": 0.,
                 "V_m": 0.,
                 "V_th": 15.,
                 "I_e": 0.,
                 "tau_m": tau_m,
                 "tau_syn_ex": tau_syn,
                 "tau_syn_in": tau_syn,
                 "C_m": C_m}

spike_emission = [2.]


def alpha_fn(t):
    prefactor = analytical_u = weight * math.e / (tau_syn * C_m)
    t1 = (exp(-t / tau_m) - exp(-t / tau_syn)) / (1 / tau_syn - 1 / tau_m)**2
    t2 = t * exp(-t / tau_syn) / (1 / tau_syn - 1 / tau_m)
    return prefactor * (t1 - t2)


def spiketrain_response(spiketrain):
    response = 0.
    for sp in spiketrain:
        t = T - delay - sp
        response += alpha_fn(t)
    return response


@pytest.fixture(scope="module")
def reference_potential():
    return spiketrain_response(spike_emission)


@pytest.mark.parametrize("h", [(i) for i in range(-12, 1, 2)])
def test_single_spike_different_stepsizes(h, reference_potential):
    nest.ResetKernel()
    print(h)
    nest.set(tics_per_ms=2**14, resolution=2**h)

    sg = nest.Create("spike_generator")
    sg.set(precise_times=False,
           origin=0.,
           spike_times=spike_emission,
           start=0.,
           stop=5.
           )

    neuron = nest.Create("iaf_psc_alpha_ps")
    neuron.set(neuron_params)

    nest.Connect(sg, neuron, syn_spec={"weight": weight, "delay": delay})

    nest.Simulate(T)
    u = neuron.get("V_m")
    assert abs(reference_potential - u) < 1e-12
