# -*- coding: utf-8 -*-
#
# test_multimeter_offset.py
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
This set of tests verify the behavior of the offset attribute of multimeter.
"""

import nest
import numpy as np
import numpy.testing as nptest
import pytest


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


def test_recorded_times_relative_to_offset():
    """
    Test whether recorded times are indeed relative to an offset.
    """

    nest.resolution = 2**-3  # Set to power of two to avoid rounding issues.
    nrn = nest.Create("iaf_psc_alpha")
    mm_params = {
        "interval": 3.0,  # different from default
        "offset": 5.0,  # different from default
        "record_from": ["V_m"],
    }
    mm = nest.Create("multimeter", params=mm_params)

    nest.Connect(mm, nrn)
    nest.Simulate(15.0)

    actual_times = mm.events["times"]
    expected_times = [5.0, 8.0, 11.0, 14.0]

    nptest.assert_array_equal(actual_times, expected_times)


def test_correct_data_logger_initialization():
    """
    Test whether data logger is initialized correctly.
    """

    nest.resolution = 2**-3  # Set to power of two to avoid rounding issues.

    # Create and connect one multimeter and simulate
    nrn = nest.Create("iaf_psc_alpha")
    mm1_params = {"start": 20.0, "stop": 30.0, "interval": 3.0, "offset": 5.0, "record_from": ["V_m"]}
    mm1 = nest.Create("multimeter", params=mm1_params)

    nest.Connect(mm1, nrn)
    nest.Simulate(10.0)

    # Create and connect a second multimeter then simulate further
    mm2_params = {"start": 20.0, "stop": 30.0, "interval": 3.0, "offset": 5.0, "record_from": ["V_m"]}
    mm2 = nest.Create("multimeter", params=mm2_params)

    nest.Connect(mm2, nrn)
    nest.Simulate(20.0)

    # Compare recordings with reference data
    expected_times = [23.0, 26.0, 29.0]
    mm1_actual_times = mm1.events["times"]
    mm2_actual_times = mm2.events["times"]

    nptest.assert_array_equal(mm1_actual_times, expected_times)
    nptest.assert_array_equal(mm2_actual_times, expected_times)


def test_offset_cannot_be_changed_after_connect():
    """
    Ensure offset cannot be changed after connecting to a node.
    """

    nrn = nest.Create("iaf_psc_exp")
    mm = nest.Create("multimeter")
    nest.Connect(mm, nrn)

    with pytest.raises(nest.NESTErrors.BadProperty):
        mm.offset = 5.0


def test_offset_wrt_origin_start_stop():
    """
    Test correct offset behavior w.r.t. origin / start / stop.

    The test set start and stop different from their initial values,
    simulates and check the recordings. Then, we modify the origin,
    simulate again and check for consistency.
    """

    expected_times_1 = [5.0, 8.0, 11.0, 14.0]
    expected_times_2 = [5.0, 8.0, 11.0, 14.0, 26.0, 29.0, 32.0, 35.0]

    nest.resolution = 2**-3  # Set to power of two to avoid rounding issues.

    # Create and connect one multimeter and simulate
    nrn = nest.Create("iaf_psc_exp")
    mm_params = {"start": 3.0, "stop": 15.0, "interval": 3.0, "offset": 5.0, "record_from": ["V_m"]}
    mm = nest.Create("multimeter", params=mm_params)

    nest.Connect(mm, nrn)
    nest.Simulate(20.0)

    # Compare to first reference data
    actual_times_1 = mm.events["times"]
    nptest.assert_array_equal(actual_times_1, expected_times_1)

    # Change origin; this shouldn't affect the offset but only shift start-stop window
    mm.origin = 20.0
    nest.Simulate(20.0)

    # Compare to extended reference
    actual_times_2 = mm.events["times"]
    nptest.assert_array_equal(actual_times_2, expected_times_2)


def test_creation_after_initial_simulation():
    """
    Test correct behavior of multimeter creation after initial simulation.

    Ensure offsets behave correctly when a second multimeter is created
    after an initial simulation. The offset and start time indicates that
    all samples should be taken from the second simulation, and hence the
    recordings from both multimeters should be equal.
    """

    nest.resolution = 0.1

    mm_params = {"start": 20.0, "stop": 30.0, "interval": 3.0, "offset": 5.0, "record_from": ["V_m"]}
    conn_spec = {"rule": "all_to_all"}
    syn_spec = {"delay": 0.1}

    # Create and connect one multimeter and simulate
    nrn = nest.Create("iaf_psc_exp")
    mm1 = nest.Create("multimeter", params=mm_params)

    nest.Connect(mm1, nrn, conn_spec, syn_spec)

    nest.Simulate(10.0)

    # Create and connect a second multimeter then simulate further
    mm2 = nest.Create("multimeter", params=mm_params)

    nest.Connect(mm2, nrn, conn_spec, syn_spec)
    nest.Simulate(20.0)

    # Compare multimeter recordings
    mm1_times = mm1.events["times"]
    mm2_times = mm2.events["times"]

    nptest.assert_array_equal(mm1_times, mm2_times)


def test_offset_after_initial_simulation():
    """
    Test correct behavior of offset after initial simulation time.

    The sample should occur at 'offset' when initial simulation time is
    less than offset and we have not recorded any parameters yet.
    """

    nest.resolution = 0.1

    init_sim_time = 110.0
    sim_time = 250.0
    interval = 60.0
    offset = 170.3

    expected_times = np.arange(offset, init_sim_time + sim_time, interval)

    # Create neuron and perform initial simulation
    nrn = nest.Create("iaf_psc_exp")
    nest.Simulate(init_sim_time)

    # Create multimeter and continue simulation
    mm_params = {"interval": interval, "offset": offset, "record_from": ["V_m"]}
    conn_spec = {"rule": "all_to_all"}
    syn_spec = {"delay": 0.1}

    mm = nest.Create("multimeter", params=mm_params)

    nest.Connect(mm, nrn, conn_spec, syn_spec)
    nest.Simulate(sim_time)

    # Compare to reference data
    actual_times = mm.events["times"]
    nptest.assert_array_equal(actual_times, expected_times)


def test_initial_simulation_longer_than_offset():
    """
    Test correct behavior when initial simulation is longer than offset.

    First recorded event should be between init simtime and interval.
    """

    nest.resolution = 0.1

    init_sim_time = 250.0
    sim_time = 250.0
    interval = 60.0
    offset = 170.3

    expected_times = np.arange(offset + 2 * interval, init_sim_time + sim_time, interval)

    # Create neuron and perform initial simulation
    nrn = nest.Create("iaf_psc_exp")
    nest.Simulate(init_sim_time)

    # Create multimeter and continue simulation
    mm_params = {"interval": interval, "offset": offset, "record_from": ["V_m"]}
    conn_spec = {"rule": "all_to_all"}
    syn_spec = {"delay": 0.1}

    mm = nest.Create("multimeter", params=mm_params)

    nest.Connect(mm, nrn, conn_spec, syn_spec)
    nest.Simulate(sim_time)

    # Compare to reference data
    actual_times = mm.events["times"]
    nptest.assert_array_equal(actual_times, expected_times)
