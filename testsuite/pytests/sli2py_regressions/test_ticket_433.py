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


@pytest.fixture(autouse=True)
def prepare_kernel():
    nest.ResetKernel()
    nest.set_verbosity("M_INFO")


@pytest.mark.skipif("ht_neuron" not in nest.node_models, reason="ht_neuron not available")
def test_explicit_receptor_type():
    """
    Test that explicit receptor_type works for ht_neuron.
    """
    nest.ResetKernel()
    nest.CopyModel("ht_neuron", "MyHT")
    nest.CopyModel("poisson_generator", "RetinaGen", {"rate": 10.0})
    nest.CopyModel("parrot_neuron", "RetinaNode")
    ampa = nest.GetDefaults("MyHT")["receptor_types"]["AMPA"]
    shape = [10, 10]
    extent = [8.0, 8.0]
    retina_gen = nest.Create("RetinaGen", positions=nest.spatial.grid(shape=shape, extent=extent))
    retina = nest.Create("RetinaNode", positions=nest.spatial.grid(shape=shape, extent=extent))
    Tp = nest.Create("MyHT", positions=nest.spatial.grid(shape=shape, extent=extent))
    nest.Connect(retina_gen, retina, conn_spec="one_to_one")
    # Explicit receptor_type
    nest.Connect(retina_gen, Tp, conn_spec={"rule": "one_to_one"}, syn_spec={"receptor_type": ampa})
    nest.Simulate(10.0)


@pytest.mark.skipif("ht_neuron" not in nest.node_models, reason="ht_neuron not available")
def test_implicit_receptor_type_fails():
    """
    Test that missing receptor_type raises an error for ht_neuron.
    """
    nest.ResetKernel()
    nest.CopyModel("ht_neuron", "MyHT")
    nest.CopyModel("poisson_generator", "RetinaGen", {"rate": 10.0})
    nest.CopyModel("parrot_neuron", "RetinaNode")
    shape = [10, 10]
    extent = [8.0, 8.0]
    retina_gen = nest.Create("RetinaGen", positions=nest.spatial.grid(shape=shape, extent=extent))
    retina = nest.Create("RetinaNode", positions=nest.spatial.grid(shape=shape, extent=extent))
    Tp = nest.Create("MyHT", positions=nest.spatial.grid(shape=shape, extent=extent))
    nest.Connect(retina_gen, retina, conn_spec="one_to_one")
    # Missing receptor_type should fail
    with pytest.raises(nest.NESTError):
        nest.Connect(retina_gen, Tp, conn_spec={"rule": "one_to_one"})


@pytest.mark.skipif("ht_neuron" not in nest.node_models, reason="ht_neuron not available")
def test_connectlayers_receptor_type():
    """
    Test that receptor_type is properly passed in spatial Connect.
    """
    nest.ResetKernel()
    nest.CopyModel("ht_neuron", "MyHT")
    nest.CopyModel("poisson_generator", "RetinaGen", {"rate": 10.0})
    nest.CopyModel("parrot_neuron", "RetinaNode")
    ampa = nest.GetDefaults("MyHT")["receptor_types"]["AMPA"]
    shape = [10, 10]
    extent = [8.0, 8.0]
    retina = nest.Create("RetinaNode", positions=nest.spatial.grid(shape=shape, extent=extent))
    Tp = nest.Create("MyHT", positions=nest.spatial.grid(shape=shape, extent=extent))
    # receptor_type passed in syn_spec
    nest.Connect(
        retina,
        Tp,
        conn_spec={
            "rule": "pairwise_bernoulli",
            "p": 1.0,
            "mask": {"circular": {"radius": 2.0}},
        },
        syn_spec={"receptor_type": ampa, "synapse_model": "ht_synapse"},
    )
    nest.Simulate(10.0)


@pytest.mark.skipif("ht_neuron" not in nest.node_models, reason="ht_neuron not available")
def test_connectlayers_receptor_type_in_synapse():
    """
    Test that receptor_type in custom synapse model works in spatial Connect.
    """
    nest.ResetKernel()
    nest.CopyModel("ht_neuron", "MyHT")
    nest.CopyModel("poisson_generator", "RetinaGen", {"rate": 10.0})
    nest.CopyModel("parrot_neuron", "RetinaNode")
    ampa = nest.GetDefaults("MyHT")["receptor_types"]["AMPA"]
    # Create a custom synapse model with receptor_type set
    nest.CopyModel("ht_synapse", "ht_syn_ampa", {"receptor_type": ampa})
    shape = [10, 10]
    extent = [8.0, 8.0]
    retina = nest.Create("RetinaNode", positions=nest.spatial.grid(shape=shape, extent=extent))
    Tp = nest.Create("MyHT", positions=nest.spatial.grid(shape=shape, extent=extent))
    nest.Connect(
        retina,
        Tp,
        conn_spec={
            "rule": "pairwise_bernoulli",
            "p": 1.0,
            "mask": {"circular": {"radius": 2.0}},
        },
        syn_spec={"synapse_model": "ht_syn_ampa"},
    )
    nest.Simulate(10.0)


def test_all_models_with_receptor_types_reject_static_synapse():
    """
    Assert that all models with receptor_types reject connections with plain static_synapse.
    """
    for mod in nest.node_models:
        dflts = nest.GetDefaults(mod)
        if "receptor_types" in dflts:
            # Avoid models with auto-generated ports (length 0 or less)
            if len(dflts["receptor_types"]) > 0:
                nest.ResetKernel()
                src = nest.Create(mod)
                tgt = nest.Create(mod)
                with pytest.raises(nest.NESTError):
                    nest.Connect(src, tgt, syn_spec="static_synapse")
