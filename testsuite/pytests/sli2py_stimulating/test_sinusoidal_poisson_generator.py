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

import pytest

import nest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def build_network_with_spike_recorders(n):
    """
    Function for building `n` neuron network with `n` spike recorders.
    """

    parrots = nest.Create("parrot_neuron", n)
    srecs = nest.Create("spike_recorder", n)
    sspg = nest.Create("sinusoidal_poisson_generator")

    nest.Connect(sspg, parrots)
    nest.Connect(parrots, srecs, "one_to_one")

    return srecs


def test_individual_spike_trains_true_by_default():
    """
    Test that `individual_spike_trains` is true by default.
    """

    sspg = nest.Create("sinusoidal_poisson_generator")
    individual_spike_trains_bool = sspg.get("individual_spike_trains")
    assert individual_spike_trains_bool


def test_set_individual_spike_trains_on_set_defaults():
    """
    Test whether `individual_spike_trains` can be set on `SetDefaults`.
    """

    nest.SetDefaults("sinusoidal_poisson_generator", {"individual_spike_trains": False})
    sspg = nest.Create("sinusoidal_poisson_generator")
    individual_spike_trains_bool = sspg.get("individual_spike_trains")
    assert not individual_spike_trains_bool


def test_set_individual_spike_trains_on_creation():
    """
    Test whether `individual_spike_trains` can be set on model creation.
    """

    sspg = nest.Create(
        "sinusoidal_poisson_generator", params={"individual_spike_trains": False}
    )
    individual_spike_trains_bool = sspg.get("individual_spike_trains")
    assert not individual_spike_trains_bool


def test_set_individual_spike_trains_on_copy_model():
    """
    Test whether the set `individual_spike_trains` is passed on `CopyModel`.
    """

    nest.CopyModel(
        "sinusoidal_poisson_generator",
        "sinusoidal_poisson_generator_copy",
        params={"individual_spike_trains": False},
    )
    sspg = nest.Create("sinusoidal_poisson_generator_copy")
    individual_spike_trains_bool = sspg.get("individual_spike_trains")
    assert not individual_spike_trains_bool


def test_set_individual_spike_trains_on_instance():
    """
    Test that `individual_spike_trains` cannot be set on an instance.
    """

    sspg = nest.Create("sinusoidal_poisson_generator")

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        sspg.individual_spike_trains = False
