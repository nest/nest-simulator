# -*- coding: utf-8 -*-
#
# test_default_delays_weights.py
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
Test that correct delays and weights are set if synaptic defaults
are given or overridden. This in particular also tests that NaN-detection works
on an architecture
"""
import nest
import pytest

d = 15.5
w = 23.4


@pytest.fixture
def prepare_test():
    nest.ResetKernel()
    nest.SetDefaults("static_synapse", {"delay": 1.5, "weight": 2.3})
    defaults = nest.GetDefaults("static_synapse")
    neurons = nest.Create("iaf_psc_alpha", 2)
    return defaults, neurons


def test_default_delay_and_weight(prepare_test):
    defaults, neurons = prepare_test
    nest.Connect(neurons[0], neurons[1], "one_to_one", syn_spec={"synapse_model": "static_synapse"})
    conn = nest.GetConnections(source=neurons[0])[0].get()

    assert defaults["weight"] == conn["weight"]
    assert defaults["delay"] == conn["delay"]


def test_default_weight_non_default_delay(prepare_test):
    defaults, neurons = prepare_test
    nest.Connect(neurons[0], neurons[1], "one_to_one", syn_spec={"synapse_model": "static_synapse", "delay": d})
    conn = nest.GetConnections(source=neurons[0])[0].get()

    assert defaults["weight"] == conn["weight"]
    assert d == conn["delay"]


def test_non_default_weight_default_delay(prepare_test):
    defaults, neurons = prepare_test
    nest.Connect(neurons[0], neurons[1], "one_to_one", syn_spec={"synapse_model": "static_synapse", "weight": w})
    conn = nest.GetConnections(source=neurons[0])[0].get()

    assert w == conn["weight"]
    assert defaults["delay"] == conn["delay"]


def test_non_default_weight_and_delay(prepare_test):
    defaults, neurons = prepare_test
    nest.Connect(neurons[0], neurons[1], "one_to_one", syn_spec={"synapse_model": "static_synapse", "weight": w, "delay": d})
    conn = nest.GetConnections(source=neurons[0])[0].get()

    assert w == conn["weight"]
    assert d == conn["delay"]
