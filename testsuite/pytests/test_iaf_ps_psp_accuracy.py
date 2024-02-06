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
Test that neuron with precise timing has correct post-synaptic potential for off-grid input.

Compares the voltage response of iaf_psc_alpha_ps from a single incoming
excitatory spike at an off-grid time to the analytical solution.

test_iaf_ps_psp_accuracy.sli checks the voltage response of the precise
versions of the iaf_psc_alpha model neurons to a single incoming spike.
The voltage excursion is called postsynaptic potential (PSP). In the model
neurons the postsynaptic current is described by an alpha-function
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

import math
from math import exp

import nest
import pytest

# Global parameters
T1 = 3.0
T2 = 6.0
tau_syn = 0.3
tau_m = 10.0
C_m = 250.0
delay = 1.0
weight = 500.0

neuron_params = {
    "E_L": 0.0,
    "V_m": 0.0,
    "V_th": 15.0,
    "I_e": 0.0,
    "tau_m": tau_m,
    "tau_syn_ex": tau_syn,
    "tau_syn_in": tau_syn,
    "C_m": C_m,
}

spike_emission = [1.132123512]


def V_m_response_fn(t):
    """
    Returns the value of the membrane potential at time t, assuming
    alpha-shaped post-synaptic currents and an incoming spike at t=0.
    The weight and neuron parameters are taken from outer scope.
    """
    if t < 0.0:
        return 0.0
    prefactor = weight * math.e / (tau_syn * C_m)
    term1 = (exp(-t / tau_m) - exp(-t / tau_syn)) / (1 / tau_syn - 1 / tau_m) ** 2
    term2 = t * exp(-t / tau_syn) / (1 / tau_syn - 1 / tau_m)
    return prefactor * (term1 - term2)


@pytest.mark.parametrize("h", range(-12, 1, 2))
def test_single_spike_different_stepsizes(h):
    nest.ResetKernel()
    res = 2**h
    nest.set(tics_per_ms=2**14, resolution=res)

    sg = nest.Create("spike_generator")
    sg.set(precise_times=True, origin=0.0, spike_times=spike_emission, start=0.0, stop=5.0)

    neuron = nest.Create("iaf_psc_alpha_ps", params=neuron_params)

    nest.Connect(sg, neuron, syn_spec={"weight": weight, "delay": delay})

    nest.Simulate(T1)
    V_m1 = neuron.get("V_m")

    nest.Simulate(T2 - T1)
    V_m2 = neuron.get("V_m")

    # first checkpoint
    t = T1 - delay - spike_emission[0]
    reference_potential1 = V_m_response_fn(t)

    # second checkpoint
    t = T2 - delay - spike_emission[0]
    reference_potential2 = V_m_response_fn(t)

    assert reference_potential1 == pytest.approx(V_m1)
    assert reference_potential2 == pytest.approx(V_m2)
