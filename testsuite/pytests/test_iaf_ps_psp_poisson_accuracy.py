# -*- coding: utf-8 -*-
#
# test_iaf_ps_psp_poisson_accuracy.py
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
Tests for correct voltage of precise timing neuron receiving input from precise timing poisson_generator

The test probes the interaction of a spike generator implementing a
poisson process in continuous time with a neuron model capable of
handling off-grid spike times. The result is verified by comparing the
superposition of postsynaptic potentials in the neuron model to the
the corresponding analytical solution. To achieve this, spike
generation of the neuron mode is prevented by setting the spike
threshold to a very high value. The test employs the parrot neuron for
precise spike times to provide the neuron model and the spike recorder
with an identical sequence of spike times. The independence of the
result from the computations step size is ensured by comparing the
results for a range of temporal resolutions. Due to this setup the
test requires that several critical timing relations between network
nodes of different types operate correctly. If the test fails go back
to simpler tests verifying individual node types.

Author:  May 2005, February 2008, March 2009; Diesmann
References:
 [1] Morrison A, Straube S, Plesser H E, & Diesmann M (2007) Exact Subthreshold
     Integration with Continuous Spike Times in Discrete Time Neural Network
     Simulations. Neural Computation 19:47--79
SeeAlso: testsuite::test_iaf_ps_psp_accuracy, testsuite::test_iaf_ps_dc_accuracy
"""


import math
from math import exp

import nest
import pytest

DEBUG = False

# Global parameters
T = 6.0
tau_syn = 0.3
tau_m = 10.0
C_m = 250.0
weight = 65.0
delay = 1.0

neuron_params = {
    "E_L": 0.0,
    "V_m": 0.0,
    "V_th": 1500.0,
    "I_e": 0.0,
    "tau_m": tau_m,
    "tau_syn_ex": tau_syn,
    "tau_syn_in": tau_syn,
    "C_m": C_m,
}


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


def spiketrain_response(spiketrain):
    """
    Compute the value of the membrane potential at time T
    given a spiketrain. Assumes all synaptic variables
    and membrane potential to have values 0 at time t=0.
    """
    response = 0.0
    for sp in spiketrain:
        t = T - delay - sp
        response += V_m_response_fn(t)
    return response


@pytest.mark.parametrize("h", range(-10, 1, 2))
def test_poisson_spikes_different_stepsizes(h):
    nest.ResetKernel()

    nest.set(tics_per_ms=2**10, resolution=2**h)

    pg = nest.Create("poisson_generator_ps", params={"rate": 16000.0})

    parrot = nest.Create("parrot_neuron")
    neuron = nest.Create("iaf_psc_alpha_ps", params=neuron_params)

    sr = nest.Create("spike_recorder")

    if DEBUG:
        mm = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": 2**h})
        nest.Connect(mm, neuron)

    nest.Connect(pg, parrot)
    nest.Connect(parrot, neuron, syn_spec={"weight": weight, "delay": delay})
    nest.Connect(parrot, sr)

    nest.Simulate(T)

    spiketrain = sr.get("events", "times")

    reference_potential = spiketrain_response(spiketrain)
    if DEBUG:
        u = neuron.get("V_m")
        nest.Simulate(1.0)  # to get V_m recording until time T
        times = mm.get("events", "times")
        V_m = mm.get("events", "V_m")
        import matplotlib.pyplot as plt

        plt.plot(times, V_m)
        plt.scatter([T], [u], s=20.0)
        plt.scatter([T], [reference_potential], s=20, marker="X")
        plt.show()
        neuron.set(V_m=u)  # reset to value before extra 1s simulation

    assert neuron.get("V_m") == pytest.approx(reference_potential)
