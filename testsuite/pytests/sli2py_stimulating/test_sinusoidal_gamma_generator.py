# -*- coding: utf-8 -*-
#
# test_sinusoidal_gamma_generator.py
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
This test asserts that
 - that /individual_spike_trains is true by default
 - the /individual_spike_trains property can be set on the model, but not on instances
 - that instances inherit the correct /individual_spike_trains value
 - that different targets (on same or different threads)
     * identical spike trains if /individual_spike_trains is false
     * different spike trains otherwise
 - that a multimeter can be connected to record the rate
     * independent of /individual_spike_trains ..., only a single trace is returned
 - the recorded rate profile is tested against expectation

This test DOES NOT test the statistical properties of the spike trains generated.
"""


import nest
import numpy as np
import pytest


pytestmark = pytest.mark.skipif_missing_gsl


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def prepare_network_with_spike_recorder(n):
    parrots = nest.Create("parrot_neuron", n)
    sr = nest.Create("spike_recorder", n)
    stim = nest.Create("sinusoidal_gamma_generator")

    nest.Connect(stim, parrots)
    nest.Connect(parrots, sr, "one_to_one")

    return sr


def prepare_network_with_multimeter(n):
    parrots = nest.Create("parrot_neuron", n)
    mm = nest.Create("multimeter", params={"record_from": ["rate"]})
    stim = nest.Create("sinusoidal_gamma_generator")

    nest.Connect(stim, parrots)
    nest.Connect(mm, stim, "one_to_one")

    return mm


def test_default_value_of_spike_trains():
    sgg = nest.Create("sinusoidal_gamma_generator")
    individual_spike_trains = sgg.get("individual_spike_trains")

    assert individual_spike_trains


def test_override_default_value():
    nest.SetDefaults("sinusoidal_gamma_generator", {"individual_spike_trains": False})
    sgg = nest.Create("sinusoidal_gamma_generator")
    individual_spike_trains = sgg.get("individual_spike_trains")

    assert not individual_spike_trains


def test_copied_model_with_new_default():
    new_default = False
    nest.CopyModel("sinusoidal_gamma_generator", "sinusoidal_gamma_generator_copy",
                   params={"individual_spike_trains": new_default})

    sg = nest.Create("sinusoidal_gamma_generator_copy")
    individual_spike_trains = sg.get("individual_spike_trains")

    assert individual_spike_trains == new_default


def test_setting_individual_spike_trains_on_instance():
    sgg = nest.Create("sinusoidal_gamma_generator")

    with pytest.raises(Exception):
        sgg.set(individual_spike_trains=False)


def check_arrays_equal(arrays):
    """Check if all arrays in the list are equal."""
    return all(np.array_equal(arrays[i], arrays[i + 1]) for i in range(len(arrays) - 1))


@pytest.mark.parametrize("individual_spike_trains", [False, True])
@pytest.mark.parametrize("num_threads", [1, 2])
@pytest.mark.parametrize("nrns_per_thread", [4])
def test_network_with_spike_recorder(nrns_per_thread, num_threads, individual_spike_trains):
    nest.local_num_threads = num_threads

    nest.SetDefaults("sinusoidal_gamma_generator", {"rate": 100, "amplitude": 50.,
                                                    "frequency": 10., "order": 3.,
                                                    "individual_spike_trains": individual_spike_trains})

    population_size = nrns_per_thread * num_threads

    srs = prepare_network_with_spike_recorder(population_size)

    nest.Simulate(1000)

    spike_times = list(map(lambda x: np.array(x["times"]), srs.get("events")))

    excepted = not individual_spike_trains

    assert check_arrays_equal(spike_times) == excepted


@pytest.mark.parametrize("individual_spike_trains", [False, True])
@pytest.mark.parametrize("num_threads", [1, 2])
@pytest.mark.parametrize("nrns_per_thread", [4])
def test_network_with_multimeter(nrns_per_thread, num_threads, individual_spike_trains):
    nest.local_num_threads = num_threads

    nest.SetDefaults("sinusoidal_gamma_generator", {"rate": 100, "amplitude": 50.,
                                                    "frequency": 10., "order": 3.,
                                                    "individual_spike_trains": individual_spike_trains})

    population_size = nrns_per_thread * num_threads

    mm = prepare_network_with_multimeter(population_size)

    tsim = 100.0
    nest.Simulate(tsim)

    expected = tsim - 1
    times = mm.get("events")["times"]
    rates = mm.get("events")["rate"]

    assert len(times) == expected
    assert len(rates) == expected


def test_recorded_rate():

    dc = 1.
    ac = 0.5
    freq = 10.0
    phi = 2.0
    order = 3.
    nest.SetDefaults("sinusoidal_gamma_generator", {"rate": dc, "amplitude": ac,
                                                    "frequency": freq, "order": order,
                                                    "phase": phi / np.pi * 180})

    mm = prepare_network_with_multimeter(1)
    mm.set(start=100)

    nest.Simulate(200.0)

    times = np.array(mm.get("events")["times"])
    rates = np.array(mm.get("events")["rate"])

    constant = (2 * np.pi) * (freq / 1000)

    scaled_times = times * constant
    shifted_times = scaled_times + phi
    res = np.sin(shifted_times) * ac + dc

    assert pytest.approx(rates) == res
