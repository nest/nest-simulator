# -*- coding: utf-8 -*-
#
# test_corr_det.py
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
Feeds correlation detector with two hand-crafted spike trains with
known correlation. Correlation detector parameters are set in model.

Remarks:
  The test does not test weighted correlations.

"""
import nest
import numpy as np
import pytest


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def test_changing_params():
    new_params = {"delta_tau": 2.0, "tau_max": 20.0}

    original_model_instance = nest.Create("correlation_detector")
    original_model_instance.set(new_params)

    nest.SetDefaults("correlation_detector", new_params)
    modified_model_instance = nest.Create("correlation_detector")

    original = original_model_instance.get(new_params.keys())
    modified = modified_model_instance.get(new_params.keys())

    assert modified == original


def test_setting_invalid_delta_tau():
    nest.resolution = 0.1
    with pytest.raises(Exception):
        nest.SetDefaults("correlation_detector", {"delta_tau": 0.25})


def test_setting_invalid_tau_max():
    nest.resolution = 0.1
    with pytest.raises(Exception):
        nest.SetDefaults("correlation_detector", {"delta_tau": 1.0, "tau_max": 2.5})


def test_setting_invalid_resolution():
    nest.resolution = 0.1
    nest.SetDefaults("correlation_detector", {"delta_tau": 0.1})
    with pytest.raises(Exception):
        nest.resolution = 1.0
        detector = nest.Create("correlation_detector")


def test_setting_num_of_histogram_bins():
    nest.resolution = 0.2
    nest.SetDefaults("correlation_detector", {"delta_tau": 1.0, "tau_max": 5.0})

    detector = nest.Create("correlation_detector")

    nest.Simulate(1)

    histogram_size = len(detector.get("histogram"))

    assert histogram_size == 11


def prepare_correlation_detector(spike_times_array):
    detector = nest.Create("correlation_detector")

    sg1 = nest.Create("spike_generator")
    sg1.set(precise_times=False, spike_times=spike_times_array[0])

    nest.SetDefaults("static_synapse", {"receptor_type": 0})

    nest.Connect(sg1, detector)

    sg2 = nest.Create("spike_generator")
    sg2.set(precise_times=False, spike_times=spike_times_array[1])

    nest.SetDefaults("static_synapse", {"receptor_type": 1})

    nest.Connect(sg2, detector)

    all_spike_times = []
    all_spike_times.extend(spike_times_array[0])
    all_spike_times.extend(spike_times_array[1])
    max_value = np.max(all_spike_times)

    min_delay = nest.GetKernelStatus()["min_delay"]
    t_sim = min_delay * 2 + max_value

    nest.Simulate(t_sim)

    return detector


def diff_at_center():
    spikes_times = [[1.0, 2.0, 6.0], [2.0, 4.0]]
    histogram = [0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0]
    return (spikes_times, histogram)


def diff_at_edge():
    spikes_times = [[6.0], [0.5, 5.4, 5.5, 5.6, 6.4, 6.5, 6.6, 11.5]]
    histogram = [1, 0, 0, 0, 1, 3, 2, 0, 0, 0, 0]
    return (spikes_times, histogram)


@pytest.mark.parametrize("spikes_times, histogram", [diff_at_center(), diff_at_edge()])
def test_histogram_correlation(spikes_times, histogram):
    nest.resolution = 0.1
    nest.SetDefaults("correlation_detector", {"delta_tau": 1.0, "tau_max": 5.0})

    detector = prepare_correlation_detector(spikes_times)

    n_events = detector.get("n_events")
    spikes_times_size = list(map(lambda x: len(x), spikes_times))
    assert (n_events == spikes_times_size).all()

    detector_histogram = detector.get("histogram")
    assert (detector_histogram == histogram).all()


def test_setting_invalid_n_events():
    """
    test to ensure [1 1] not allowed for /n_events
    """
    detector = nest.Create("correlation_detector")
    with pytest.raises(Exception):
        detector.set(n_events=[1, 1])


def test_reset():
    nest.resolution = 0.1
    nest.SetDefaults("correlation_detector", {"delta_tau": 1.0, "tau_max": 5.0})

    spikes_times = [[1.0, 2.0, 6.0], [2.0, 4.0]]
    detector = prepare_correlation_detector(spikes_times)

    n_events = detector.get("n_events")

    has_zero_entries = np.any(n_events == 0)

    if not has_zero_entries:
        detector.set(n_events=[0, 0])
        assert np.all(detector.get("n_events") == 0)
        assert np.all(detector.get("histogram") == 0)
