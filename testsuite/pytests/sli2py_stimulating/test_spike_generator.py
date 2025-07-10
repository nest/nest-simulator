# -*- coding: utf-8 -*-
#
# test_spike_generator.py
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
This set of tests checks the consistency of ``spike_generator``.
"""

import nest
import numpy.testing as nptest
import pytest


@pytest.fixture
def prepare_kernel():
    nest.ResetKernel()
    nest.local_num_threads = 1
    nest.resolution = 0.1


def test_check_spike_time_zero_error(prepare_kernel):
    """
    This test checks if setting spike time to 0 causes an exception.
    """

    sg = nest.Create("spike_generator")

    with pytest.raises(nest.NESTError, match="spike time cannot be set to 0"):
        sg.set({"spike_times": [0.0]})


def test_spike_generator_precise_time_false(prepare_kernel):
    """
    Check if truncating spike times to grid causes an assertion with ``precise_times`` set to ``False``.
    """

    sg = nest.Create("spike_generator")
    sg_params = {"precise_times": False, "spike_times": [4.33], "origin": 0.0, "start": 0.0, "stop": 0.0}

    with pytest.raises(nest.NESTError, match="Time point 4.33 is not representable in current resolution"):
        sg.set(sg_params)


@pytest.mark.parametrize(
    "spike_times, allow_offgrid_times, expected_spike_times",
    [
        ([2.9999, 4.3001], False, [30, 43]),
        ([1.0, 1.9999, 3.0001], False, [10, 20, 30]),
        ([1.0, 1.05, 3.0001], True, [10, 11, 30]),
    ],
)
def test_spike_generator(prepare_kernel, spike_times, allow_offgrid_times, expected_spike_times):
    """
    Check if the spikes are rounded up or down based on whether ``allow_offgrid_times`` is set to ``True`` or ``False``.

    If ``allow_offgrid_times=False``, spike times will be rounded to the nearest step if the spike time is less than
    ``tic/2`` from the step. If ``allow_offgrid_times=True``, spike times are rounded to the nearest step if
    within ``tic/2`` from the step and to the end of the time step otherwise.
    """

    sg_params = {
        "precise_times": False,
        "spike_times": spike_times,
        "allow_offgrid_times": allow_offgrid_times,
        "origin": 0.0,
        "start": 0.0,
        "stop": 6.0,
    }

    sg = nest.Create("spike_generator", params=sg_params)
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    nest.Connect(sg, sr, syn_spec={"delay": 1.0, "weight": 1.0})

    nest.Simulate(10.0)

    actual_spike_times = sr.events["times"]
    nptest.assert_array_equal(actual_spike_times, expected_spike_times)


def test_spike_generator_spike_not_res_multiple(prepare_kernel):
    """
    Check if the spike time is a multiple of the resolution with ``allow_offgrid_times=False`` (default).
    """

    sg = nest.Create("spike_generator")
    sg_params = {"spike_times": [1.0, 1.05, 3.0001], "origin": 0.0, "start": 0.0, "stop": 6.0}

    with pytest.raises(nest.NESTError, match="Time point 1.05 is not representable in current resolution"):
        sg.set(sg_params)


def test_spike_generator_precise_spikes(prepare_kernel):
    """
    Test spike times and offsets against expectations with ``precise_times`` set to ``True``.
    """

    sg_params = {
        "precise_times": True,
        "spike_times": [1.0, 1.05, 3.0001],
        "origin": 0.0,
        "start": 0.0,
        "stop": 6.0,
    }

    sg = nest.Create("spike_generator", params=sg_params)
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    nest.Connect(sg, sr, syn_spec={"delay": 1.0, "weight": 1.0})

    nest.Simulate(10.0)

    expected_spike_times = [10, 11, 31]
    actual_spike_times = sr.events["times"]
    nptest.assert_array_equal(actual_spike_times, expected_spike_times)

    expected_offsets = [0, 0.05, 0.0999]
    actual_offsets = sr.events["offsets"]
    nptest.assert_almost_equal(actual_offsets, expected_offsets)


def test_spike_generator_spike_time_at_simulation_end_time(prepare_kernel):
    """
    Ensure that spikes are not recorded after the simulation time.

    Here, the spike time is within ``tic/2`` of step 100, rounded down to 100 thus not in the future;
    spike will not be emitted.
    """

    sg = nest.Create("spike_generator")
    sr = nest.Create("spike_recorder")
    nest.Connect(sg, sr)

    nest.Simulate(10.0)

    sg_params = {"spike_times": [10.0001], "origin": 0.0, "start": 0.0, "stop": 16.0}
    sg.set(sg_params)

    nest.Simulate(10.0)

    assert sr.n_events == 0


def test_spike_generator_precise_time_future_spike(prepare_kernel):
    """
    Ensure correct behavior of future spike times with ``precise_times`` set to ``True``.

    In this test, the spike occurs at step 101, offset -0.0999 is in the future, and spike is shifted to the future.
    """

    sg = nest.Create("spike_generator", params={"precise_times": True, "origin": 0.0, "start": 0.0})
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    nest.Connect(sg, sr)

    nest.Simulate(10.0)

    sg.set({"spike_times": [10.0001]})

    nest.Simulate(10.0)

    expected_spike_times = [101]
    actual_spike_times = sr.events["times"]
    nptest.assert_array_equal(actual_spike_times, expected_spike_times)

    expected_offsets = [0.0999]
    actual_offsets = sr.events["offsets"]
    nptest.assert_almost_equal(actual_offsets, expected_offsets)


def test_spike_generator_with_shift_now_spikes(prepare_kernel):
    """
    Check spike times with ``shift_now_spikes`` set to ``True``.

    In this test, first the spike occurs at step 101 and shifted into the future.
    A second spike occurs at step 110 is not shifted, since it is in the future anyways.
    """

    sg = nest.Create("spike_generator", params={"shift_now_spikes": True, "origin": 0.0, "start": 0.0})
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    nest.Connect(sg, sr, syn_spec={"weight": 1.0, "delay": 1.0})

    nest.Simulate(10.0)

    sg.set({"spike_times": [10.0001, 11.0001]})

    nest.Simulate(10.0)

    expected_spike_times = [101, 110]
    actual_spike_times = sr.events["times"]
    nptest.assert_array_equal(actual_spike_times, expected_spike_times)

    expected_offsets = [0, 0]
    actual_offsets = sr.events["offsets"]
    nptest.assert_almost_equal(actual_offsets, expected_offsets)


def test_spike_generator_precise_times_and_allow_offgrid_times(prepare_kernel):
    """
    Ensure exclusivity between options ``precise_times`` and ``allow_offgrid_times``.
    """

    with pytest.raises(
        nest.NESTError,
        match="Option precise_times cannot be set to true when either allow_offgrid_times or "
        "shift_now_spikes is set to true.",
    ):
        sg = nest.Create("spike_generator", params={"precise_times": True, "allow_offgrid_times": True})


def test_spike_generator_precise_times_and_shift_now_spikes(prepare_kernel):
    """
    Ensure exclusivity between options ``precise_times`` and ``shift_now_spikes``.
    """

    with pytest.raises(
        nest.NESTError,
        match="Option precise_times cannot be set to true when either allow_offgrid_times or "
        "shift_now_spikes is set to true.",
    ):
        sg = nest.Create("spike_generator", params={"precise_times": True, "shift_now_spikes": True})


@pytest.mark.parametrize(
    "sg_params, expected_spike_times",
    [
        [{"spike_times": [0.1, 10.0, 10.5, 10.50001]}, [0.1, 10.0, 10.5, 10.5]],
        [
            {"spike_times": [0.1, 10.0, 10.5, 10.50001, 10.55], "allow_offgrid_times": True},
            [0.1, 10.0, 10.5, 10.5, 10.6],
        ],
        [{"spike_times": [0.1, 10.0, 10.5, 10.50001, 10.55], "precise_times": True}, [0.1, 10.0, 10.5, 10.5, 10.55]],
        [{"spike_times": [0.0, 10.0, 10.5, 10.50001], "shift_now_spikes": True}, [0.1, 10.0, 10.5, 10.5]],
    ],
)
def test_spike_generator_set_and_get(prepare_kernel, sg_params, expected_spike_times):
    """
    Test the ``set`` and ``get`` functions of ``spike_generator``.

    The test checks ``set`` and ``get`` with:
    - default values.
    - ``allow_offgrid_times`` set to ``True``.
    - ``precise_times`` set to ``True``.
    - ``shift_now_spikes`` set to ``True``.
    """

    sg = nest.Create("spike_generator")
    sg.set(sg_params)

    actual_spike_times = sg.get("spike_times")
    nptest.assert_almost_equal(actual_spike_times, expected_spike_times, decimal=5)


@pytest.mark.parametrize(
    "h",
    [
        0.1,
        0.2,
        0.5,
        1.0,
    ],
)
@pytest.mark.parametrize("expected_spike_times", [[5.3, 5.300001, 5.399999, 5.9, 6.0]])
def test_spike_generator_precise_times_different_resolution(h, expected_spike_times):
    """
    Test the precise times of spikes for different resolutions.
    """

    nest.ResetKernel()
    nest.resolution = h

    sg_params = {
        "precise_times": True,
        "spike_times": [0.1, 5.0, 5.3, 5.300001, 5.399999, 5.9, 6.0, 9.3],
        "origin": 0.0,
        "start": 5.0,
        "stop": 6.0,
    }
    sg = nest.Create("spike_generator", params=sg_params)
    sr = nest.Create("spike_recorder")
    nest.Connect(sg, sr, syn_spec={"delay": 1.0, "weight": 1.0})

    nest.Simulate(7.0)

    actual_spike_times = sr.events["times"]
    nptest.assert_almost_equal(actual_spike_times, expected_spike_times)
