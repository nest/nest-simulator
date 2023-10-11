# -*- coding: utf-8 -*-
#
# test_sinusoidal_poisson_generator.py
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
Test basic properties of `sinusoidal_poisson_generator`.
"""

import nest
import numpy as np
import numpy.testing as nptest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_individual_spike_trains_true_by_default():
    """
    Test that `individual_spike_trains` is true by default.
    """

    sspg = nest.Create("sinusoidal_poisson_generator")
    individual_spike_trains = sspg.get("individual_spike_trains")
    assert individual_spike_trains


def test_set_individual_spike_trains_on_set_defaults():
    """
    Test whether `individual_spike_trains` can be set on `SetDefaults`.
    """

    nest.SetDefaults("sinusoidal_poisson_generator", {"individual_spike_trains": False})
    sspg = nest.Create("sinusoidal_poisson_generator")
    individual_spike_trains = sspg.get("individual_spike_trains")
    assert not individual_spike_trains


def test_set_individual_spike_trains_on_creation():
    """
    Test whether `individual_spike_trains` can be set on model creation.
    """

    sspg = nest.Create("sinusoidal_poisson_generator", params={"individual_spike_trains": False})
    individual_spike_trains = sspg.get("individual_spike_trains")
    assert not individual_spike_trains


def test_set_individual_spike_trains_on_copy_model():
    """
    Test whether the set `individual_spike_trains` is inherited on `CopyModel`.
    """

    nest.CopyModel(
        "sinusoidal_poisson_generator",
        "sinusoidal_poisson_generator_copy",
        params={"individual_spike_trains": False},
    )
    sspg = nest.Create("sinusoidal_poisson_generator_copy")
    individual_spike_trains = sspg.get("individual_spike_trains")
    assert not individual_spike_trains


def test_set_individual_spike_trains_on_instance():
    """
    Test that `individual_spike_trains` cannot be set on an instance.
    """

    sspg = nest.Create("sinusoidal_poisson_generator")

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        sspg.individual_spike_trains = False


@pytest.mark.skipif_missing_threads()
@pytest.mark.parametrize("individual_spike_trains", [False, True])
@pytest.mark.parametrize("num_threads", [1, 2])
def test_sinusoidal_poisson_generator_with_spike_recorder(num_threads, individual_spike_trains):
    """
    Test spike recording with both `False` and `True` `individual_spike_trains`.

    The test builds a network with `num_threads x 4` parrot neurons that
    receives spikes from a `sinusoidal_poisson_generator`. A spike recorder is
    connected to each parrot neuron. The test ensures that different targets
    (on the same or different threads) receives identical spike trains if
    `individual_spike_trains` is false and different spike trains otherwise.
    """

    nest.local_num_threads = num_threads

    nest.SetDefaults(
        "sinusoidal_poisson_generator",
        {
            "rate": 100,
            "amplitude": 50.0,
            "frequency": 10.0,
            "individual_spike_trains": individual_spike_trains,
        },
    )

    nrns_per_thread = 4
    total_num_nrns = num_threads * nrns_per_thread

    parrots = nest.Create("parrot_neuron", total_num_nrns)
    sspg = nest.Create("sinusoidal_poisson_generator")
    srecs = nest.Create("spike_recorder", total_num_nrns)

    nest.Connect(sspg, parrots)
    nest.Connect(parrots, srecs, "one_to_one")

    nest.Simulate(500.0)

    # Nested list of recorded spike times from each sender
    spikes_all_nrns = srecs.get("events", "times")

    # Check that we actually obtained a spike times array for each neuron
    assert len(spikes_all_nrns) == total_num_nrns

    if individual_spike_trains:
        # all trains must be pairwise different
        assert all(
            not np.array_equal(left, right)
            for idx, left in enumerate(spikes_all_nrns[:-1])
            for right in spikes_all_nrns[(idx + 1) :]
        )
    else:
        # all trains should be equal
        assert all(np.array_equal(spikes_all_nrns[0], right) for right in spikes_all_nrns[1:])


@pytest.mark.skipif_missing_threads()
@pytest.mark.parametrize("individual_spike_trains", [False, True])
@pytest.mark.parametrize("num_threads", [1, 2])
def test_sinusoidal_poisson_generator_with_multimeter(num_threads, individual_spike_trains):
    """
    Test multimeter recording with both `False` and `True` `individual_spike_trains`.

    The test builds a network with `num_threads x 4` parrot neurons that
    receives spikes from a `sinusoidal_poisson_generator`. A `multimeter` is
    then connected to the generator and the system simulated. The test ensures
    that a `multimeter` can be connected to record the rate, independent of
    `individual_spike_tains`. Since only a single trace should be returned,
    the test checks that the multimeter recording contains the expected number
    of data points.
    """

    nest.local_num_threads = num_threads
    T_sim = 100.0

    nest.SetDefaults(
        "sinusoidal_poisson_generator",
        {
            "rate": 100,
            "amplitude": 50.0,
            "frequency": 10.0,
            "individual_spike_trains": individual_spike_trains,
        },
    )

    nrns_per_thread = 4
    total_num_nrns = num_threads * nrns_per_thread

    parrots = nest.Create("parrot_neuron", total_num_nrns)
    sspg = nest.Create("sinusoidal_poisson_generator")
    mm = nest.Create("multimeter", {"record_from": ["rate"]})

    nest.Connect(sspg, parrots)
    nest.Connect(mm, sspg)

    nest.Simulate(T_sim)

    times = mm.get("events")["times"]
    rates = mm.get("events")["rate"]

    # Check that times and rates contain ndata points
    expected_ndata = T_sim - 1
    assert len(times) == expected_ndata
    assert len(rates) == expected_ndata


def test_sinusoidal_poisson_generator_rate_profile():
    """
    Test recorded rate of `sinusoidal_poisson_generator` against expectation.

    The test checks that the recorded rate profile with `multimeter` is the
    same as the analytical expectation.
    """

    dc = 1.0
    ac = 0.5
    freq = 10.0
    phi = 2.0

    nest.SetDefaults(
        "sinusoidal_poisson_generator",
        {"rate": dc, "amplitude": ac, "frequency": freq, "phase": phi / np.pi * 180},
    )

    parrots = nest.Create("parrot_neuron")
    sspg = nest.Create("sinusoidal_poisson_generator")
    mm = nest.Create("multimeter", {"record_from": ["rate"]})

    nest.Connect(sspg, parrots)
    nest.Connect(mm, sspg)

    nest.Simulate(100.0)

    times = mm.get("events")["times"]
    actual_rates = mm.get("events")["rate"]

    scaled_times = times * 2 * np.pi * freq / 1000
    shifted_times = scaled_times + phi
    expected_rates = np.sin(shifted_times) * ac + dc

    nptest.assert_allclose(actual_rates, expected_rates)
