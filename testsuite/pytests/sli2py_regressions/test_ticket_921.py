# -*- coding: utf-8 -*-
#
# test_ticket_921.py
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


"""This test ensures that synapse defaults are unaffected if Connect is called with
a syn_spec containing parameter values, especially that receptor_type is unchanged."""

import nest
import pytest


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def test_synapse_defaults_unchanged_after_usage_in_connection():
    synapse_defaults = nest.GetDefaults("static_synapse")

    population = nest.Create("iaf_psc_exp_multisynapse", params={"tau_syn": [0.1, 0.2]})
    nest.Connect(
        population,
        population,
        syn_spec={"synapse_model": "static_synapse", "delay": 2.3, "weight": 1234.0, "receptor_type": 2},
        conn_spec={"rule": "one_to_one"},
    )

    updated_synapse_defaults = nest.GetDefaults("static_synapse")

    params = ["delay", "weight", "receptor_type"]
    for param in params:
        assert synapse_defaults[param] == updated_synapse_defaults[param]


def test_synapse_defaults_unchanged_after_usage_in_connection_with_error():
    synapse_defaults = nest.GetDefaults("static_synapse")

    population = nest.Create("iaf_psc_exp_multisynapse", params={"tau_syn": [0.1, 0.2]})
    try:
        nest.Connect(
            population,
            population,
            syn_spec={"synapse_model": "static_synapse", "delay": 2.3, "weight": 1234.0, "receptor_type": 5},
            conn_spec={"rule": "one_to_one"},
        )
    except nest.NESTErrors.IncompatibleReceptorType:
        pass

    updated_synapse_defaults = nest.GetDefaults("static_synapse")
    params = ["delay", "weight", "receptor_type"]
    for param in params:
        assert synapse_defaults[param] == updated_synapse_defaults[param]


def test_synapse_defaults_unchanged_after_usage_in_connection_with_additional_parameters():
    synapse_defaults = nest.GetDefaults("stdp_synapse")

    population = nest.Create("iaf_psc_exp_multisynapse", params={"tau_syn": [0.1, 0.2]})
    nest.Connect(
        population,
        population,
        syn_spec={
            "synapse_model": "stdp_synapse",
            "delay": 2.3,
            "weight": 1234.0,
            "receptor_type": 2,
            "alpha": 7.0,
            "lambda": 0.05,
            "mu_minus": 2.0,
            "mu_plus": 3.0,
            "tau_plus": 50.0,
            "Wmax": 99.0,
        },
        conn_spec={"rule": "one_to_one"},
    )

    updated_synapse_defaults = nest.GetDefaults("stdp_synapse")
    params = ["delay", "weight", "receptor_type", "alpha", "lambda", "mu_minus", "mu_plus", "tau_plus", "Wmax"]
    for param in params:
        assert synapse_defaults[param] == updated_synapse_defaults[param]
