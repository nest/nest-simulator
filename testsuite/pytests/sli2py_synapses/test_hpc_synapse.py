# -*- coding: utf-8 -*-
#
# test_hpc_synapse.py
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
Test basic properties of HPC synapses as follows:

1. For all known synapses with _hpc ending and counterparts without _hpc
   connect spike generator to one neuron with normal, one with _hpc synapse,
   ensure that simulation yields identical membrane potentials.

2. Check that adding and freezing/thawing of nodes either is blocked or
   does not affect results, i.e., that the TargetIdentifierIndicies are ok.
   These tests proceed as follows:
   1. Connect spike_generator to N neurons with different weights,
      disable spiking in receptors. We use membrane potential after simulation
      as indicator for through which synpse a neuron received input.
   2. Build network once with plain static_synapse to get reference data.
   3. Then build with respective test cases and compare.
"""

import nest
import numpy.testing as nptest
import pytest

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()
    nest.verbosity = nest.VerbosityLevel.ERROR


def has_hpc_suffix(syn_model):
    return syn_model.endswith("_hpc")


def get_hpc_models():
    ignore_list = [
        "clopath_synapse_hpc",
        "eprop_synapse_hpc",
        "eprop_synapse_bsshslm_2020_hpc",
        "stdp_dopamine_synapse_hpc",
        "urbanczik_synapse_hpc",
    ]

    hpc_models = [model for model in nest.synapse_models if has_hpc_suffix(model) and model not in ignore_list]

    return hpc_models


def prepare_neuron(syn):
    nest.ResetKernel()
    nest.local_num_threads = 4

    sg = nest.Create("spike_generator", params={"spike_times": [5.0]})
    pn = nest.Create("parrot_neuron")
    neuron = nest.Create("iaf_psc_alpha", params={"V_th": 100000.0})

    nest.Connect(sg, pn)
    nest.Connect(pn, neuron, syn_spec={"synapse_model": syn})

    nest.Simulate(10)
    return neuron.get("V_m")


def prepare_neurons(syn):
    nest.ResetKernel()
    nest.local_num_threads = 4

    sg = nest.Create("spike_generator", params={"spike_times": [1.0]})
    pn = nest.Create("parrot_neuron")
    nest.Connect(sg, pn)

    nodes = nest.Create("iaf_psc_alpha", 4, {"V_th": 100000.0})
    for node in nodes:
        nest.Connect(pn, node, syn_spec={"synapse_model": syn, "weight": node.tolist()[0] * 100})

    nest.Simulate(10)

    return nodes.get("V_m")


def prepare_frozen_neuron(syn, first_node_to_connect=1, freeze_before_connect=True):
    nest.ResetKernel()
    nest.local_num_threads = 4

    sg = nest.Create("spike_generator", params={"spike_times": [1.0]})
    pn = nest.Create("parrot_neuron")

    nodes = nest.Create("iaf_psc_alpha", 4, {"V_th": 100000.0})

    if freeze_before_connect:
        nodes[0].set(frozen=True)

    nest.Connect(sg, pn)

    for node in nodes[first_node_to_connect:]:
        nest.Connect(pn, node, syn_spec={"synapse_model": syn, "weight": node.tolist()[0] * 100})

    if not freeze_before_connect:
        nodes[0].set(frozen=True)

    nest.Simulate(10)

    return nodes[first_node_to_connect:].get("V_m")


@pytest.mark.parametrize("syn", get_hpc_models())
def test_hpc_synapse(syn):
    synapse_without_hpc_suffix = syn[: len(syn) - 4]

    net_without_hpc_suffix = prepare_neuron(synapse_without_hpc_suffix)
    net_with_hpc_suffix = prepare_neuron(syn)

    nptest.assert_equal(net_with_hpc_suffix, net_without_hpc_suffix)


def test_static_synapse():
    using_static_synapse = prepare_neurons("static_synapse")
    using_static_synapse_hpc = prepare_neurons("static_synapse_hpc")

    nptest.assert_equal(using_static_synapse_hpc, using_static_synapse)


def test_frozen_disconnected_neuron_before_connect():
    using_static_synapse = prepare_frozen_neuron("static_synapse")
    using_static_synapse_hpc = prepare_frozen_neuron("static_synapse_hpc")

    nptest.assert_equal(using_static_synapse_hpc, using_static_synapse)


def test_frozen_connected_neuron_before_connect():
    using_static_synapse = prepare_frozen_neuron("static_synapse", 0)
    using_static_synapse_hpc = prepare_frozen_neuron("static_synapse_hpc", 0)

    nptest.assert_equal(using_static_synapse_hpc, using_static_synapse)


def test_frozen_connected_neuron_after_connect():
    using_static_synapse = prepare_frozen_neuron("static_synapse", 0, True)
    using_static_synapse_hpc = prepare_frozen_neuron("static_synapse_hpc", 0, True)

    nptest.assert_equal(using_static_synapse_hpc, using_static_synapse)
