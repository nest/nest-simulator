# -*- coding: utf-8 -*-
#
# test_common_properties_setting.py
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
Test that common properties can be Set/Read via Defaults, but not individual connections.

These tests only check the setting/getting mechanics for one parameter per synapse to confirm
that the overall mechanics "work". Testing the setting of specific parameters must be done in
model-specific tests.
"""

import nest
import pytest


def set_volume_transmitter():
    vt = nest.Create("volume_transmitter")
    nest.SetDefaults("stdp_dopamine_synapse", {"volume_transmitter": vt})


def set_default_delay_resolution():
    nest.resolution = nest.GetDefaults("eprop_synapse")["delay"]


# This list shall contain all synapse models extending the CommonSynapseProperties class.
# For each model, specify which parameter to test with and which test value to use. A
# setup function can be provided if preparations are required. Provide also supported neuron model.
common_prop_models = {
    "eprop_synapse": {
        "parameter": "batch_size",
        "value": 10,
        "setup": set_default_delay_resolution,
        "neuron": "eprop_iaf_psc_delta",
    },
    "jonke_synapse": {"parameter": "tau_plus", "value": 10, "setup": None, "neuron": "iaf_psc_alpha"},
    "stdp_dopamine_synapse": {
        "parameter": "tau_plus",
        "value": 10,
        "setup": set_volume_transmitter,
        "neuron": "iaf_psc_alpha",
    },
    "stdp_facetshw_synapse_hom": {"parameter": "tau_plus", "value": 10, "setup": None, "neuron": "iaf_psc_alpha"},
    "stdp_pl_synapse_hom": {"parameter": "tau_plus", "value": 10, "setup": None, "neuron": "iaf_psc_alpha"},
    "stdp_synapse_hom": {"parameter": "tau_plus", "value": 10, "setup": None, "neuron": "iaf_psc_alpha"},
    "static_synapse_hom_w": {"parameter": "weight", "value": 10, "setup": None, "neuron": "iaf_psc_alpha"},
    "tsodyks_synapse_hom": {"parameter": "tau_psc", "value": 10, "setup": None, "neuron": "iaf_psc_alpha"},
}

# Filter models that may not be built in
available_cp_models = {k: v for k, v in common_prop_models.items() if k in nest.synapse_models}


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


@pytest.mark.parametrize("syn_model, specs", available_cp_models.items())
def test_set_common_properties(syn_model, specs):
    """Test that setting a parameter works"""

    if specs["setup"]:
        specs["setup"]()

    old_val = nest.GetDefaults(syn_model)[specs["parameter"]]
    assert old_val != specs["value"]  # test would be meaningless otherwise

    nest.SetDefaults(syn_model, {specs["parameter"]: specs["value"]})
    new_val = nest.GetDefaults(syn_model)[specs["parameter"]]
    assert new_val == specs["value"]


@pytest.mark.parametrize("syn_model, specs", available_cp_models.items())
def test_copy_common_properties(syn_model, specs):
    """Test that parameters set on a model are copied and that setting on the copy does not touch the original"""

    if specs["setup"]:
        specs["setup"]()

    old_val = nest.GetDefaults(syn_model)[specs["parameter"]]
    assert old_val != specs["value"]  # test would be meaningless otherwise

    nest.SetDefaults(syn_model, {specs["parameter"]: specs["value"]})
    new_model = syn_model + "_copy"
    nest.CopyModel(syn_model, new_model)
    new_val = nest.GetDefaults(new_model)[specs["parameter"]]
    assert new_val == specs["value"]

    # Set parameter back on copied model, original must not be changed
    nest.SetDefaults(new_model, {specs["parameter"]: old_val})
    assert nest.GetDefaults(syn_model)[specs["parameter"]] == specs["value"]


@pytest.mark.parametrize("syn_model, specs", available_cp_models.items())
def test_no_setting_on_connection(syn_model, specs):
    """Test that common property cannot be set on individual connection"""

    if specs["setup"]:
        specs["setup"]()

    n = nest.Create(specs["neuron"])
    nest.Connect(n, n, syn_spec={"synapse_model": syn_model})
    conn = nest.GetConnections()
    with pytest.raises(nest.kernel.NESTErrors.DictError):
        conn.set({specs["parameter"]: specs["value"]})


@pytest.mark.parametrize("syn_model, specs", available_cp_models.items())
def test_no_setting_on_connect(syn_model, specs):
    """Test that common property cannot be set in a Connect call"""

    if specs["setup"]:
        specs["setup"]()

    n = nest.Create(specs["neuron"])
    with pytest.raises(nest.kernel.NESTErrors.NotImplemented):
        nest.Connect(n, n, syn_spec={"synapse_model": syn_model, specs["parameter"]: specs["value"]})
