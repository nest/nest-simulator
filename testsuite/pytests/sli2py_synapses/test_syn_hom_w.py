# -*- coding: utf-8 -*-
#
# test_syn_hom_w.py
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
Name: test_syn_hom_w - sli script for test synapse with homogeneous weight and delay.

Description:
Test of the overall function of static_synapse_hom_w.
A simple integrate-and-fire neuron is created. It is connected to a spike
generator and a voltmeter by static synapses with homogeneous weight.

Test ported from SLI regression test.

FirstVersion: April 2008

Author: Moritz Helias, Susanne Kunkel
"""

import nest
import numpy as np
import pytest
import testutil

# Reference data: numpy array with [time_step, voltage] pairs
# Times are in steps and will be converted to milliseconds by multiplying by resolution
REFERENCE_DATA = np.array(
    [
        [1, -70],
        [2, -70],
        [3, -70],
        [4, -70],
        [27, -70],
        [28, -70],
        [29, -70],
        [30, -70],
        [31, -6.999740e01],
        [32, -6.998990e01],
        [33, -6.997810e01],
        [34, -6.996240e01],
        [35, -6.994340e01],
        [45, -6.964350e01],
        [46, -6.960840e01],
        [47, -6.957320e01],
        [48, -6.953800e01],
        [49, -6.950290e01],
        [50, -6.946810e01],
        [51, -6.943360e01],
        [52, -6.939950e01],
        [53, -6.936600e01],
        [54, -6.933300e01],
        [55, -6.930080e01],
        [60, -6.915080e01],
    ]
)


def test_syn_hom_w():
    """
    Test static_synapse_hom_w with homogeneous weight and delay.
    """
    delay = 1.0  # in ms
    dt = 0.1  # resolution in ms

    nest.ResetKernel()
    nest.set(resolution=dt, local_num_threads=1)

    sg = nest.Create("spike_generator")
    sg.set(
        {
            "precise_times": False,
            "origin": 0.0,  # in ms
            "spike_times": [2.0],  # in ms
            "start": 1.0,  # in ms
            "stop": 3.0,  # in ms
        }
    )

    neuron = nest.Create("iaf_psc_alpha")

    vm = nest.Create("voltmeter", params={"time_in_steps": True, "interval": dt})

    sr = nest.Create("spike_recorder", params={"time_in_steps": True})

    nest.SetDefaults("static_synapse_hom_w", {"weight": 100.0, "delay": delay})

    nest.Connect(sg, neuron, syn_spec={"synapse_model": "static_synapse_hom_w"})
    nest.Connect(vm, neuron, syn_spec={"synapse_model": "static_synapse_hom_w"})
    nest.Connect(neuron, sr, syn_spec={"synapse_model": "static_synapse_hom_w"})

    simulation_time = 7.0  # in ms
    nest.Simulate(simulation_time)

    events = vm.get("events")
    times = events["times"]
    voltages = events["V_m"]

    # Filter reference data to only include time steps within the simulation time
    # Maximum time step = simulation_time / resolution
    max_time_step = int(simulation_time / dt)
    ref_data_filtered = REFERENCE_DATA[REFERENCE_DATA[:, 0] <= max_time_step]

    if len(ref_data_filtered) == 0:
        pytest.skip(f"No reference data points within simulation time {simulation_time} ms " f"for resolution dt={dt}")

    ref_time_steps = set(ref_data_filtered[:, 0].astype(int))

    # Convert times from steps to milliseconds for get_comparable_timesamples()
    # Since time_in_steps=True, times are in steps, so multiply by resolution
    times_ms = times * dt

    # Filter actual data to only include time steps that are in the filtered reference data
    # This ensures we only compare at the exact time points specified in the reference
    mask = np.isin(times.astype(int), list(ref_time_steps))
    filtered_times_ms = times_ms[mask]
    filtered_voltages = voltages[mask]

    # Prepare actual data array: [time_ms, voltage]
    actual_data = np.column_stack((filtered_times_ms, filtered_voltages))

    # Convert reference data from steps to milliseconds
    # ref_data_filtered has [time_step, voltage] pairs, convert to (time_ms, voltage) array
    # Times are in steps, so multiply by resolution to get milliseconds
    expected_data = np.column_stack((ref_data_filtered[:, 0] * dt, ref_data_filtered[:, 1]))

    # Compare actual and expected using get_comparable_timesamples()
    # This function matches on tics (computed from milliseconds), so both arrays must have times in ms
    actual, expected = testutil.get_comparable_timesamples(dt, actual_data, expected_data)

    # Check that the function did not return empty arrays
    assert len(actual) > 0, (
        f"get_comparable_timesamples() returned empty arrays - " f"no matching time points found for resolution dt={dt}"
    )
    assert len(expected) > 0, (
        f"get_comparable_timesamples() returned empty arrays - " f"no matching time points found for resolution dt={dt}"
    )

    # Verify we matched the expected number of reference points
    assert len(actual) == len(expected), (
        f"Mismatch in number of matched points: got {len(actual)} actual vs "
        f"{len(expected)} expected for resolution dt={dt}"
    )
    assert len(actual) == len(ref_data_filtered), (
        f"Expected {len(ref_data_filtered)} reference points but got " f"{len(actual)} matches for resolution dt={dt}"
    )

    # Assert approximate equality
    # Reference data has 6 decimal places (scientific notation), so use appropriate tolerance
    np.testing.assert_allclose(actual, expected, rtol=1e-6)
