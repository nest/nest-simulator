# -*- coding: utf-8 -*-
#
# test_common_props_setting.py
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
Name: testsuite::test_common_props_setting - test that common properties can be set as defaults, not else

Synopsis: (test_common_props_setting) run -> compare response with reference data

Description:
For synapses with common properties, ensure that common and individual properties can be set by
SetDefaults and CopyModel, but that an exception is raised if they are set via an individual connection.

FirstVersion: November 2014

Author: Hans E Plesser
"""

import pytest

import nest


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()
    vt = nest.Create("volume_transmitter")
    nest.SetDefaults("stdp_dopamine_synapse", {"volume_transmitter": vt})


@pytest.mark.parametrize("synapse", ["stdp_synapse_hom", "stdp_facetshw_synapse_hom", "stdp_dopamine_synapse"])
class TestSettingCommonProps:
    def test_setting_common_props_on_original(self, synapse):
        expected_values = {"tau_plus": 5.0, "weight": 2.0}
        nest.SetDefaults(synapse, expected_values)

        actual_values = nest.GetDefaults(synapse, keys=expected_values.keys())
        assert actual_values == tuple(expected_values.values())

    def test_setting_common_props_on_copy(self, synapse):
        copied_syn = f"{synapse}_copy"
        expected_values = {"tau_plus": 15.0, "weight": 20.0}
        nest.CopyModel(synapse, copied_syn, expected_values)

        actual_values = nest.GetDefaults(copied_syn, keys=expected_values.keys())
        assert actual_values == tuple(expected_values.values())

    def test_setting_non_common_props_on_instance(self, synapse):
        neuron = nest.Create("iaf_psc_alpha")
        nest.Connect(neuron, neuron, syn_spec={"synapse_model": synapse})

        single_edge = nest.GetConnections(source=neuron)[0]
        single_edge.set(weight=3.0)

        assert single_edge.get("weight") == 3.0
        assert nest.GetDefaults(synapse)["weight"] != 3.0

    def test_setting_common_props_on_instance(self, synapse):
        tau_plus_ref = nest.GetDefaults(synapse)["tau_plus"]
        neuron = nest.Create("iaf_psc_alpha")
        nest.Connect(neuron, neuron, syn_spec={"synapse_model": synapse})

        single_edge = nest.GetConnections(source=neuron)[0]

        with pytest.raises(Exception):
            single_edge.set(tau_plus=(tau_plus_ref + 1) * 3)

        actual_tau_plus_value = nest.GetDefaults(synapse)["tau_plus"]
        assert actual_tau_plus_value == tau_plus_ref
