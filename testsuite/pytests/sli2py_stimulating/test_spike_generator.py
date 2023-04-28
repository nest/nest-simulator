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
This test checks the spike_generator device and its consistency with the nest simulation kernel.
"""

import nest
import pytest
import numpy.testing as nptest


@pytest.fixture
def prepare_kernel():
    nest.ResetKernel()
    nest.local_num_threads = 1
    nest.resolution = 0.1


def test_check_spike_time_zero_error(prepare_kernel):
    sg = nest.Create("spike_generator")

    with pytest.raises(nest.kernel.NESTError, match="spike time cannot be set to 0"):
        sg.set({"spike_times": [0.]})


def test_spike_generator_precise_time_false(prepare_kernel):
    sg = nest.Create("spike_generator")
    sg_params = {"precise_times": False,
                 "spike_times": [4.33],
                 "origin": 0.,
                 "start": 0.,
                 "stop": 0.}
    with pytest.raises(nest.kernel.NESTError, match="Time point 4.33 is not representable in current resolution"):
        sg.set(sg_params)


@pytest.mark.parametrize("spike_times, allow_offgrid_times, expected_spike_times",
                         [([2.9999, 4.3001], False, [30, 43]),
                          ([1.0, 1.9999, 3.0001], False, [10, 20, 30]),
                          ([1.0, 1.05, 3.0001], True, [10, 11, 30])])
def test_spike_generator(prepare_kernel, spike_times, allow_offgrid_times, expected_spike_times):
    sg = nest.Create("spike_generator")
    sg_params = {"precise_times": False,
                 "spike_times": spike_times,
                 "allow_offgrid_times": allow_offgrid_times,
                 "origin": 0.,
                 "start": 0.,
                 "stop": 6.0}
    sg.set(sg_params)

    sr = nest.Create("spike_recorder")
    sr.set({"time_in_steps": True})
    nest.Connect(sg, sr, syn_spec={"delay": 1.0, "weight": 1.0})

    nest.Simulate(10.)

    spike_times = sr.get("events")["times"]
    nptest.assert_array_equal(spike_times, expected_spike_times)


def test_spike_generator_spike_not_res_multiple(prepare_kernel):
    sg = nest.Create("spike_generator")
    sg_params = {"spike_times": [1.0, 1.05, 3.0001],
                 "origin": 0.,
                 "start": 0.,
                 "stop": 6.0}
    with pytest.raises(nest.kernel.NESTError, match="Time point 1.05 is not representable in current resolution"):
        sg.set(sg_params)


def test_spike_generator_precise_spikes(prepare_kernel):
    sg = nest.Create("spike_generator")
    sg_params = {"precise_times": True,
                 "spike_times": [1.0, 1.05, 3.0001],
                 "origin": 0.,
                 "start": 0.,
                 "stop": 6.0}
    sg.set(sg_params)

    sr = nest.Create("spike_recorder")
    sr.set({"time_in_steps": True})
    nest.Connect(sg, sr, syn_spec={"delay": 1.0, "weight": 1.0})

    nest.Simulate(10.)

    spike_times = sr.get("events")["times"]
    expected_spike_times = [10, 11, 31]
    nptest.assert_array_equal(spike_times, expected_spike_times)

    offsets = sr.get("events")["offsets"]
    expected_offsets = [0, 0.05, 0.0999]
    assert offsets == pytest.approx(expected_offsets)


def test_spike_generator_spike_time_at_simulation_end_time(prepare_kernel):
    sg = nest.Create("spike_generator")
    sr = nest.Create("spike_recorder")
    nest.Connect(sg, sr)

    nest.Simulate(10.)
    sg_params = {"spike_times": [10.0001],
                 "origin": 0.,
                 "start": 0.,
                 "stop": 16.}
    sg.set(sg_params)

    nest.Simulate(10.)
    n_events = sr.get("n_events")
    assert n_events == 0


def test_spike_generator_precise_time_future_spike(prepare_kernel):
    sg = nest.Create("spike_generator")
    sg_params = {"precise_times": True,
                 "origin": 0.,
                 "start": 0.}
    sg.set(sg_params)
    sr = nest.Create("spike_recorder", {"time_in_steps": True})
    nest.Connect(sg, sr)

    nest.Simulate(10.)

    sg.set({"spike_times": [10.0001]})

    nest.Simulate(10.)

    spike_times = sr.get("events")["times"]
    offsets = sr.get("events")["offsets"]

    expected_spike_times = [101]
    nptest.assert_array_equal(spike_times, expected_spike_times)

    expected_offsets = [0.0999]
    assert offsets == pytest.approx(expected_offsets)


def test_spike_generator_with_shift_now_spikes(prepare_kernel):
    sg = nest.Create("spike_generator")
    sg_params = {"shift_now_spikes": True,
                 "origin": 0.,
                 "start": 0.}
    sg.set(sg_params)
    sr = nest.Create("spike_recorder", {"time_in_steps": True})
    nest.Connect(sg, sr, syn_spec={"weight": 1.0, "delay": 1.0})

    nest.Simulate(10.)

    sg.set({"spike_times": [10.0001, 11.0001]})

    nest.Simulate(10.)

    spike_times = sr.get("events")["times"]
    offsets = sr.get("events")["offsets"]

    expected_spike_times = [101, 110]
    nptest.assert_array_equal(spike_times, expected_spike_times)

    expected_offsets = [0, 0]
    assert offsets == pytest.approx(expected_offsets)


def test_spike_generator_precise_times_and_allow_offgrid_times(prepare_kernel):
    with pytest.raises(nest.kernel.NESTError,
                       match="Option precise_times cannot be set to true when either allow_offgrid_times or "
                             "shift_now_spikes is set to true."):
        sg = nest.Create("spike_generator", {"precise_times": True, "allow_offgrid_times": True})


def test_spike_generator_precise_times_and_shift_now_spikes(prepare_kernel):
    with pytest.raises(nest.kernel.NESTError,
                       match="Option precise_times cannot be set to true when either allow_offgrid_times or "
                             "shift_now_spikes is set to true."):
        sg = nest.Create("spike_generator", {"precise_times": True, "shift_now_spikes": True})


@pytest.mark.parametrize("sg_params, expected_spike_times",
                         [
                             [{"spike_times": [0.1, 10.0, 10.5, 10.50001]}, [0.1, 10.0, 10.5, 10.5]],
                             [{"spike_times": [0.1, 10.0, 10.5, 10.50001, 10.55], "allow_offgrid_times": True},
                              [0.1, 10.0, 10.5, 10.5, 10.6]],
                             [{"spike_times": [0.1, 10.0, 10.5, 10.50001, 10.55], "precise_times": True},
                              [0.1, 10.0, 10.5, 10.5, 10.55]],
                             [{"spike_times": [0.0, 10.0, 10.5, 10.50001], "shift_now_spikes": True},
                              [0.1, 10.0, 10.5, 10.5]]
                         ])
def test_spike_generator_set_and_get(prepare_kernel, sg_params, expected_spike_times):
    sg = nest.Create("spike_generator")
    sg.set(sg_params)

    # Get spike times
    spike_times = sg.get("spike_times")
    assert spike_times == pytest.approx(expected_spike_times)


@pytest.mark.parametrize("h", [0.1, 0.2, 0.5, 1.0,])
@pytest.mark.parametrize("expected_spike_times", [[5.3, 5.300001, 5.399999, 5.9, 6.0]])
def test_spike_generator_precise_times_different_resolution(h, expected_spike_times):
    nest.ResetKernel()
    nest.resolution = h
    sg = nest.Create("spike_generator")
    sg_params = {"precise_times": True,
                 "spike_times": [0.1, 5.0, 5.3, 5.300001, 5.399999, 5.9, 6.0, 9.3],
                 "origin": 0.,
                 "start": 5.,
                 "stop": 6.}
    sg.set(sg_params)

    sr = nest.Create("spike_recorder")
    nest.Connect(sg, sr, syn_spec={"delay": 1.0, "weight": 1.0})

    nest.Simulate(7.0)

    spike_times = sr.get("events")["times"]
    assert spike_times == pytest.approx(expected_spike_times)
