# -*- coding: utf-8 -*-
#
# test_iaf_psc_alpha_1to2.py
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


import nest
import numpy as np
import pandas as pd
import pytest


def Vm_theory(t):
    """
    Returns the value of the membrane potential at time t, assuming
    alpha-shaped post-synaptic currents and an incoming spike at t=0.
    """

    assert (t >= 0).all()

    d = nest.GetDefaults("iaf_psc_alpha")
    C_m = d["C_m"]
    tau_m = d["tau_m"]
    tau_syn = d["tau_syn_ex"]

    prefactor = np.exp(1) / (tau_syn * C_m)
    term1 = (np.exp(-t / tau_m) - np.exp(-t / tau_syn)) / (1 / tau_syn - 1 / tau_m) ** 2
    term2 = t * np.exp(-t / tau_syn) / (1 / tau_syn - 1 / tau_m)
    return prefactor * (term1 - term2)


@pytest.fixture
def sim_results(request):
    resolution, min_delay = request.param

    yield resolution, spikes, voltages


@pytest.mark.parametrize(
    "resolution, min_delay", [(0.1, 1.0), (0.2, 1.0), (0.5, 1.0), (1.0, 1.0), (0.1, 0.1), (0.1, 0.2), (0.1, 0.5)]
)
def test_1to2(resolution, min_delay):
    """
    Checks the spike interaction of two iaf_psc_alpha model neurons.

    In order to obtain identical results for different computation step
    sizes h, the SLI script needs to be independent of h.  This is
    achieved by specifying all time parameters in milliseconds (ms). In
    particular the time of spike emission and the synaptic delay need to
    be integer multiples of the computation step sizes to be
    tested. test_iaf_dc_aligned_delay demonstrates the strategy for the
    case of DC current input.

    A DC current in the pre-synaptic neuron is adjusted to cause a spike
    at a grid position (t=3.0 ms) joined by all computation step sizes to
    be tested.

    Note that in a neuron model where synaptic events are modeled by a
    truncated exponential the effect of the incoming spike would be
    visible at the time of impact (here, t=4.0 ms). This is because the
    initial condition for the postsynaptic potential (PSP) has a
    non-zero voltage component. For PSPs with finite rise time the
    situation is different. In this case the voltage component of the
    initial condition is zero (see documentation of
    test_iaf_psp). Therefore, at the time of impact the PSP is only
    visible in other components of the state vector.
    """

    nest.ResetKernel()
    nest.set(resolution=resolution, min_delay=min_delay, max_delay=1)

    # Create neurons
    n1, n2 = nest.Create("iaf_psc_alpha", 2)
    n1.I_e = 1450.0

    # Spike recorder for neuron 1
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    nest.Connect(n1, sr)

    # Voltmeter for n2
    vm = nest.Create("voltmeter", params={"interval": resolution, "time_in_steps": True})
    nest.Connect(vm, n2)

    nest.Connect(n1, n2, syn_spec={"weight": 100.0, "delay": 1.0})

    # Run the simulation
    nest.Simulate(8.0)

    spikes = sr.events["times"]
    voltages = pd.DataFrame.from_records(vm.events)

    # Consistency check on output of first neuron
    assert list(spikes) == [round(t / resolution) for t in [3, 8]]

    # Membrane potential before effect of incoming spike must be -70, i.e. up to 4 ms
    np.testing.assert_array_equal(voltages.loc[voltages.times <= round(4.0 / resolution)].V_m, -70)

    # Explicitly calculate membrane potential for times from 4 ms onward
    v_test = voltages.loc[voltages.times >= round(4.0 / resolution)]

    Vm_expected = -70 + 100 * Vm_theory(v_test.times * resolution - 4.0)

    np.testing.assert_allclose(v_test.V_m, Vm_expected, rtol=1e-12)
