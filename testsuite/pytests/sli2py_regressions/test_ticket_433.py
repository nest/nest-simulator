# -*- coding: utf-8 -*-
#
# test_ticket_433.py
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
Regression test for Ticket #433.

Test against receptor_type mishandling in ht_neuron.

Test ported from SLI regression test
This test ensures that:
- ht_neuron rejects connections with invalid or missing receptor_type.
- receptor_type is properly passed in spatial connections.
- All models with receptor_types reject connections with plain static_synapse.

Author: Hans Ekkehard Plesser, 2010-06-28
"""

import nest
import pytest

pytestmark = pytest.mark.skipif("ht_neuron" not in nest.node_models, reason="ht_neuron not available")


@pytest.fixture
def nodes():
    ht_layer = nest.Create("ht_neuron", positions=nest.spatial.grid(shape=[2, 3]))
    ampa_id = nest.GetDefaults("ht_neuron")["receptor_types"]["AMPA"]
    yield ht_layer, ampa_id


def test_explicit_receptor_type(nodes):
    """
    Test that explicit receptor_type works for ht_neuron.
    """

    ht_layer, ampa_id = nodes
    nest.Connect(ht_layer, ht_layer, "one_to_one", {"receptor_type": ampa_id})


def test_missing_receptor_type(nodes):
    """
    Test that explicit receptor_type works for ht_neuron.
    """

    ht_layer, ampa_id = nodes
    with pytest.raises(nest.kernel.NESTError, match="UnknownReceptorType"):
        nest.Connect(ht_layer, ht_layer, "one_to_one")


def test_connectlayers_receptor_type(nodes):
    """
    Test that receptor_type is properly passed in spatial Connect.
    """

    ht_layer, ampa_id = nodes
    nest.Connect(
        ht_layer,
        ht_layer,
        {
            "rule": "pairwise_bernoulli",
            "p": 1.0,
            "mask": {"circular": {"radius": 1.0}},
        },
        syn_spec={"receptor_type": ampa_id, "synapse_model": "ht_synapse"},
    )


def test_connectlayers_receptor_type_in_synapse(nodes):
    """
    Test that receptor_type in custom synapse model works in spatial Connect.
    """

    ht_layer, ampa_id = nodes
    nest.CopyModel("ht_synapse", "ht_syn_ampa", {"receptor_type": ampa_id})
    nest.Connect(
        ht_layer,
        ht_layer,
        {
            "rule": "pairwise_bernoulli",
            "p": 1.0,
            "mask": {"circular": {"radius": 1.0}},
        },
        syn_spec={"receptor_type": ampa_id, "synapse_model": "ht_syn_ampa"},
    )


def test_all_models_with_receptor_types_reject_static_synapse():
    """
    Assert that all models with receptor_types reject connections with plain static_synapse.
    """

    src = nest.Create("parrot_neuron")

    for model in nest.node_models:
        # Need to avoid eprop_readout neurons because they handle spike input via static_synapse
        # and use receptor types only to distiguish readout and target signal for recording.
        if not model.startswith("eprop_readout") and "receptor_types" in (dflts := nest.GetDefaults(model)):
            # Avoid models with auto-generated ports (length 0 or less)
            if len(dflts["receptor_types"]) > 0:
                tgt = nest.Create(model)
                with pytest.raises(nest.kernel.NESTError, match="UnknownReceptorType|IncompatibleReceptorType"):
                    nest.Connect(src, tgt, syn_spec="static_synapse")
