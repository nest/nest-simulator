# -*- coding: utf-8 -*-
#
# test_model_node_init.py
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
Makeshift test to see if setting model params and then creating a neuron
and creating a neuron and then setting node params lead to the same
results.

Works by connecting device to iaf_psc_alpha, measuring voltage trace over 1s
and comparing traces.
"""

import nest
import pytest


def _get_network_state(nc):
    neuron = nest.Create("iaf_psc_alpha")
    voltmeter = nest.Create("voltmeter")

    nest.Connect(nc, neuron)
    nest.Connect(voltmeter, neuron)
    nest.Simulate(1000)
    volts = voltmeter.get("events")["V_m"]

    return (volts, nc.get())


@pytest.fixture()
def model_data():
    return {"model": "mip_generator", "params": {"rate": 100.0, "p_copy": 0.5}}


@pytest.fixture()
def use_set_defaults(model_data):
    nest.ResetKernel()
    nest.set(overwrite_files=True)
    nest.SetDefaults(model_data["model"], model_data["params"])
    model_instance = nest.Create(model_data["model"])
    return _get_network_state(model_instance)


@pytest.fixture()
def use_set_status(model_data):
    nest.ResetKernel()
    nest.set(overwrite_files=True)
    model_instance = nest.Create(model_data["model"])
    model_instance.set(**model_data["params"])
    return _get_network_state(model_instance)


def test_set_status_vs_set_defaults(use_set_defaults, use_set_status):
    assert (use_set_defaults[0] == use_set_status[0]).all()
    assert use_set_defaults[1] == use_set_status[1]
