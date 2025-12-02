# -*- coding: utf-8 -*-
#
# test_iaf_psc_alpha_mindelay.py
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


import math

import nest
import numpy as np
import pytest
import testutil


@pytest.mark.parametrize("min_delay", [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 1.0, 2.0])
def test_iaf_psc_alpha_mindelay_create(min_delay):
    """
    Tests automatic adjustment of min_delay.

    The simulation is run with a range of different min_delays. All
    should give identical results. This is achieved by sampling the
    membrane potential at a fixed interval.
    """
    nest.ResetKernel()

    # Simulation variables
    delay = 2.0
    duration = 10.5

    neuron = nest.Create("iaf_psc_alpha")
    voltmeter = nest.Create("voltmeter", params={"interval": 0.1})
    spike_recorder = nest.Create("spike_recorder")
    dc_generator = nest.Create("dc_generator", params={"amplitude": 1000.0})

    nest.Connect(voltmeter, neuron, syn_spec={"weight": 1.0, "delay": delay})
    nest.Connect(neuron, spike_recorder, syn_spec={"weight": 1.0, "delay": delay})
    nest.Connect(dc_generator, neuron, syn_spec={"weight": 1.0, "delay": delay})

    # Force `min_delay` by connecting two throwaway neurons
    throwaway = nest.Create("iaf_psc_alpha", 2)
    nest.Connect(throwaway[0], throwaway[1], syn_spec={"weight": 1.0, "delay": min_delay})

    nest.Simulate(duration)

    results = np.column_stack((voltmeter.events["times"], voltmeter.events["V_m"]))

    actual, expected = testutil.get_comparable_timesamples(nest.resolution, results, expected_mindelay)
    np.testing.assert_allclose(actual, expected, rtol=1e-7)


@pytest.mark.parametrize("min_delay", [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 1.0, 2.0])
def test_iaf_psc_alpha_mindelay_set(min_delay):
    """
    Tests explicitly setting min_delay.

    The simulation is run with a range of different min_delays. All
    should give identical results. This is achieved by sampling the
    membrane potential at a fixed interval.
    """
    nest.ResetKernel()

    # Simulation variables
    delay = 2.0
    duration = 10.5

    # Set up test min_delay conditions
    nest.set(min_delay=min_delay, max_delay=delay)
    nest.SetDefaults("static_synapse", {"delay": delay})

    neuron = nest.Create("iaf_psc_alpha")
    voltmeter = nest.Create("voltmeter", params={"interval": 0.1})
    spike_recorder = nest.Create("spike_recorder")
    dc_generator = nest.Create("dc_generator", params={"amplitude": 1000.0})

    nest.Connect(voltmeter, neuron, syn_spec={"weight": 1.0, "delay": delay})
    nest.Connect(neuron, spike_recorder, syn_spec={"weight": 1.0, "delay": delay})
    nest.Connect(dc_generator, neuron, syn_spec={"weight": 1.0, "delay": delay})

    nest.Simulate(duration)

    results = np.column_stack((voltmeter.events["times"], voltmeter.events["V_m"]))

    actual, expected = testutil.get_comparable_timesamples(nest.resolution, results, expected_mindelay)
    np.testing.assert_allclose(actual, expected, rtol=1e-7)


@pytest.mark.parametrize("min_delay", [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 1.0, 2.0])
def test_iaf_psc_alpha_mindelay_simblocks(min_delay):
    """
    Tests explicitly setting min_delay across 21 simulation calls.

    The simulation is run with a range of different min_delays. All
    should give identical results. This is achieved by sampling the
    membrane potential at a fixed interval.
    """
    nest.ResetKernel()

    # Simulation variables
    delay = 2.0

    # Set up test min_delay conditions
    nest.set(min_delay=min_delay, max_delay=delay)
    nest.SetDefaults("static_synapse", {"delay": delay})

    neuron = nest.Create("iaf_psc_alpha")
    voltmeter = nest.Create("voltmeter", params={"interval": 0.1})
    spike_recorder = nest.Create("spike_recorder")
    dc_generator = nest.Create("dc_generator", params={"amplitude": 1000.0})

    nest.Connect(voltmeter, neuron, syn_spec={"weight": 1.0, "delay": delay})
    nest.Connect(neuron, spike_recorder, syn_spec={"weight": 1.0, "delay": delay})
    nest.Connect(dc_generator, neuron, syn_spec={"weight": 1.0, "delay": delay})

    for _ in range(22):
        nest.Simulate(0.5)

    results = np.column_stack((voltmeter.events["times"], voltmeter.events["V_m"]))

    actual, expected = testutil.get_comparable_timesamples(nest.resolution, results, expected_mindelay)

    assert len(actual) == len(expected_mindelay)
    np.testing.assert_allclose(actual, expected, rtol=1e-7)


def test_kernel_precision():
    nest.ResetKernel()
    nest.set(tics_per_ms=2**14, resolution=2**0)
    assert math.frexp(nest.ms_per_tic) == (0.5, -13)


expected_mindelay = np.array(
    [
        [1.000000e00, -7.000000e01],
        [2.000000e00, -7.000000e01],
        [3.000000e00, -6.655725e01],
        [4.000000e00, -6.307837e01],
        [5.000000e00, -5.993054e01],
        [6.000000e00, -5.708227e01],
        [7.000000e00, -7.000000e01],
        [8.000000e00, -7.000000e01],
        [9.000000e00, -6.960199e01],
        [1.000000e01, -6.583337e01],
    ]
)
