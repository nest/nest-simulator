# -*- coding: utf-8 -*-
#
# test_corr_matrix_det.py
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
Minimal test of ``correlomatrix_detector``.

In this test, we feed ``correlomatrix_detector`` with hand-crafted spike
trains with known correlation. The ``correlomatrix_detector`` parameters are
set on the model instance.

.. note::

    The test does not test weighted correlations.
"""

import nest
import numpy as np
import numpy.testing as nptest
import pytest


@pytest.fixture()
def model_params():
    return {"delta_tau": 0.5, "tau_max": 10.0, "N_channels": 8}


def create_set_param(model_params):
    cd = nest.Create("correlomatrix_detector")
    cd.set(**model_params)
    return cd


def set_default_create(model_params):
    nest.SetDefaults("correlomatrix_detector", model_params)
    cd = nest.Create("correlomatrix_detector")
    return cd


def test_setting_params_on_model_vs_instance(model_params):
    nest.ResetKernel()

    create_set_param_instance = create_set_param(model_params).get(model_params.keys())
    set_defaults_param_create_instance = set_default_create(model_params).get(model_params.keys())

    assert create_set_param_instance == set_defaults_param_create_instance


@pytest.mark.parametrize("value", [0, -1])
def test_setting_invalid_channel(value):
    nest.ResetKernel()
    with pytest.raises(Exception):
        nest.SetDefaults("correlomatrix_detector", {"N_channels": value})


@pytest.mark.parametrize("value", [0.25, 1.0])
def test_setting_invalid_delta_tau(value):
    nest.ResetKernel()
    nest.resolution = 0.1
    with pytest.raises(Exception):
        nest.SetDefaults("correlomatrix_detector", {"delta_tau": value})


def test_setting_invalid_tau_max():
    nest.ResetKernel()
    nest.resolution = 0.1
    with pytest.raises(Exception):
        nest.SetDefaults("correlomatrix_detector", {"delta_tau": 1.1, "tau_max": 2.5})


def test_setting_invalid_resolution():
    nest.ResetKernel()
    nest.resolution = 0.1
    nest.SetDefaults("correlomatrix_detector", {"delta_tau": 0.1})

    with pytest.raises(Exception):
        nest.resolution = 1.0
        nest.Create("correlomatrix_detector")


def test_number_of_histogram_bins():
    nest.ResetKernel()
    nest.resolution = 0.5

    nest.SetDefaults("correlomatrix_detector", {"delta_tau": 0.5, "tau_max": 2.5})
    cd = nest.Create("correlomatrix_detector")

    nest.Simulate(1)

    covariance_size = len(cd.covariance[0][0])
    assert covariance_size == 6


def prepare_correlomatrix_detector(spike_times_array):
    detector = nest.Create("correlomatrix_detector")

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
    spikes_times = [[1.5, 2.5, 4.5], [0.5, 2.5]]
    covariance = [1.0, 0.0, 1.0, 0.0, 2.0]
    return (spikes_times, covariance)


@pytest.mark.parametrize("spikes_times, expected_covariance", [diff_at_center()])
def test_histogram_correlation(spikes_times, expected_covariance):
    nest.ResetKernel()

    nest.resolution = 0.1
    nest.SetDefaults("correlomatrix_detector", {"delta_tau": 0.5, "tau_max": 2.0, "N_channels": 2})

    detector = prepare_correlomatrix_detector(spikes_times)
    spikes_times_size = list(map(lambda x: len(x), spikes_times))

    nptest.assert_array_equal(detector.n_events, spikes_times_size)

    nptest.assert_array_equal(detector.covariance[0][1], expected_covariance)


def test_reset():
    nest.ResetKernel()

    nest.resolution = 0.1
    nest.SetDefaults("correlomatrix_detector", {"delta_tau": 0.5, "tau_max": 5.0, "N_channels": 8})

    spikes_times = [[1.0, 2.0, 6.0], [2.0, 4.0]]
    detector = prepare_correlomatrix_detector(spikes_times)

    has_zero_entries = np.any(detector.covariance == 0)

    if not has_zero_entries:
        detector.set(N_channels=8)
        nptest.assert_equal(detector.covariance[0][0], 0.0)
