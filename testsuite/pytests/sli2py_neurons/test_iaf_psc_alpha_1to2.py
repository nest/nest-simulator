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
import pytest
import testutil
import numpy as np


@pytest.mark.parametrize("resolution", [0.1, 0.2, 0.5, 1.0])
@pytest.mark.parametrize("delay", [1.0])
def test_1to2(resolution, delay):
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
    nest.SetKernelStatus({"resolution": resolution})

    # Create neurons
    n1, n2 = nest.Create("iaf_psc_alpha", 2)
    n1.I_e = 1450.0

    # Create and connect voltmeter to n2
    vm = nest.Create("voltmeter", params={"interval": resolution})
    nest.Connect(vm, n2)

    # Connect neuron n1 to n2
    nest.Connect(n1, n2, syn_spec={"weight": 100.0, "delay": delay})

    # Run the simulation
    nest.Simulate(100.0)

    # Extract voltmeter data
    events = nest.GetStatus(vm, "events")[0]
    times = np.array(events["times"]).reshape(-1, 1)
    V_m = np.array(events["V_m"]).reshape(-1, 1)
    results = np.hstack((times, V_m))

    actual, expected = testutil.get_comparable_timesamples(
        results,
        np.array(
            [
                [2.5, -70],
                [2.6, -70],
                [2.7, -70],
                [2.8, -70],
                [2.9, -70],
                [3.0, -70],
                [3.1, -70],
                [3.2, -70],
                [3.3, -70],
                [3.4, -70],
                [3.5, -70],
                [3.6, -70],
                [3.7, -70],
                [3.8, -70],
                [3.9, -70],
                [4.0, -70],
                [4.1, -69.9974],
                [4.2, -69.9899],
                [4.3, -69.9781],
                [4.4, -69.9624],
                [4.5, -69.9434],
                [4.6, -69.9213],
                [4.7, -69.8967],
                [4.8, -69.8699],
                [4.9, -69.8411],
                [5.0, -69.8108],
                [5.1, -69.779],
                [5.2, -69.7463],
                [5.3, -69.7126],
                [5.4, -69.6783],
                [5.5, -69.6435],
                [5.6, -69.6084],
                [5.7, -69.5732],
            ]
        ),
    )
    assert actual == expected


@pytest.mark.parametrize("resolution", [0.1, 0.2, 0.5, 1.0])
def test_1to2_default_delay(resolution):
    """
    Same test but with the delay set via defaults of the model
    """
    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": resolution})

    # Set the delay via SetDefaults instead
    nest.SetDefaults("static_synapse", {"delay": 1.0})

    # Create neurons
    n1, n2 = nest.Create("iaf_psc_alpha", 2)
    n1.I_e = 1450.0

    # Create and connect voltmeter to n2
    vm = nest.Create("voltmeter", params={"interval": resolution})
    nest.Connect(vm, n2)

    # Connect neuron n1 to n2
    nest.Connect(n1, n2, syn_spec={"weight": 100.0})

    # Run the simulation
    nest.Simulate(100.0)

    # Extract voltmeter data
    events = nest.GetStatus(vm, "events")[0]
    times = np.array(events["times"]).reshape(-1, 1)
    V_m = np.array(events["V_m"]).reshape(-1, 1)
    results = np.hstack((times, V_m))

    actual, expected = testutil.get_comparable_timesamples(
        results,
        np.array(
            [
                [2.5, -70],
                [2.6, -70],
                [2.7, -70],
                [2.8, -70],
                [2.9, -70],
                [3.0, -70],
                [3.1, -70],
                [3.2, -70],
                [3.3, -70],
                [3.4, -70],
                [3.5, -70],
                [3.6, -70],
                [3.7, -70],
                [3.8, -70],
                [3.9, -70],
                [4.0, -70],
                [4.1, -69.9974],
                [4.2, -69.9899],
                [4.3, -69.9781],
                [4.4, -69.9624],
                [4.5, -69.9434],
                [4.6, -69.9213],
                [4.7, -69.8967],
                [4.8, -69.8699],
                [4.9, -69.8411],
                [5.0, -69.8108],
                [5.1, -69.779],
                [5.2, -69.7463],
                [5.3, -69.7126],
                [5.4, -69.6783],
                [5.5, -69.6435],
                [5.6, -69.6084],
                [5.7, -69.5732],
            ]
        ),
    )
    assert actual == expected


@pytest.mark.parametrize("delay,resolution", [(2.0, 0.1)])
@pytest.mark.parametrize("min_delay", [0.1, 0.5, 2.0])
def test_1to2_mindelay_invariance(delay, resolution, min_delay):
    """
    Same test with different mindelays.
    """
    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": resolution})

    assert min_delay <= delay
    nest.set(min_delay=min_delay, max_delay=delay)

    # Create neurons
    n1, n2 = nest.Create("iaf_psc_alpha", 2)
    n1.I_e = 1450.0

    # Create and connect voltmeter to n2
    vm = nest.Create("voltmeter", params={"interval": resolution})
    nest.Connect(vm, n2, syn_spec={"delay": delay})

    # Connect neuron n1 to n2
    nest.Connect(n1, n2, syn_spec={"weight": 100.0, "delay": delay})

    # Run the simulation
    nest.Simulate(100.0)

    # Extract voltmeter data
    events = nest.GetStatus(vm, "events")[0]
    times = np.array(events["times"]).reshape(-1, 1)
    V_m = np.array(events["V_m"]).reshape(-1, 1)
    results = np.hstack((times, V_m))

    actual, expected = testutil.get_comparable_timesamples(
        results,
        np.array(
            [
                [0.1, -70],
                [0.2, -70],
                [0.3, -70],
                [0.4, -70],
                [0.5, -70],
                [2.8, -70],
                [2.9, -70],
                [3.0, -70],
                [3.1, -70],
                [3.2, -70],
                [3.3, -70],
                [3.4, -70],
                [3.5, -70],
                [4.8, -70],
                [4.9, -70],
                [5.0, -70],
                [5.1, -69.9974],
                [5.2, -69.9899],
                [5.3, -69.9781],
                [5.4, -69.9624],
                [5.5, -69.9434],
                [5.6, -69.9213],
                [5.7, -69.8967],
                [5.8, -69.8699],
                [5.9, -69.8411],
                [6.0, -69.8108],
            ]
        ),
    )
    assert actual == expected
