# -*- coding: utf-8 -*-
#
# test_getconnections_multiple_syn_models.py
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
Test ``GetConnections`` in the context of more than one synapse model.

The test constructs a network where the connections are modeled by two
synapse models. A subset of the neurons have one of the synapse models,
and some also have both.
"""

import nest
import numpy.testing as nptest
import pandas as pd
import pytest


def build_net(num_threads=1):
    """
    Build network with specified number of threads.

    The following network is built:
        - Create 100 neurons
            - 1, 3, ..., 99 are sources
            - 2, 4, ..., 100 are targets
        - Connect 1, 3, ..., 69 -> 2, 4, ..., 70 with `static_synapse`
        - Connect 31, 33, ..., 99   -> 32, 34, ..., 100 with `stdp_synapse`
    """

    nest.ResetKernel()

    nest.local_num_threads = num_threads

    nest.Create("iaf_psc_alpha", 100)
    static_sources = nest.NodeCollection(list(range(1, 70, 2)))
    static_targets = nest.NodeCollection(list(range(2, 71, 2)))
    stdp_sources = nest.NodeCollection(list(range(31, 100, 2)))
    stdp_targets = nest.NodeCollection(list(range(32, 101, 2)))

    nest.Connect(static_sources, static_targets, "one_to_one", "static_synapse")
    nest.Connect(stdp_sources, stdp_targets, "one_to_one", "stdp_synapse")

    return {
        "static_synapse": {"sources": static_sources, "targets": static_targets},
        "stdp_synapse": {"sources": stdp_sources, "targets": stdp_targets},
    }


@pytest.fixture(scope="module")
def nodes():
    """
    Fixture that returns the nodes of the network built on a single thread.
    """

    nodes = build_net(num_threads=1)
    return nodes


@pytest.mark.parametrize("syn_model", ["static_synapse", "stdp_synapse"])
def test_retrieve_correct_sources_and_targets(nodes, syn_model):
    """
    Verify that the expected sources and targets are retrieved.
    """

    conns = nest.GetConnections(synapse_model=syn_model)
    actual_sources = conns.get("source")
    actual_targets = conns.get("target")

    expected_sources = nodes[f"{syn_model}"]["sources"].tolist()
    expected_targets = nodes[f"{syn_model}"]["targets"].tolist()

    nptest.assert_array_equal(actual_sources, expected_sources)
    nptest.assert_array_equal(actual_targets, expected_targets)


def test_retrieve_all_connections(nodes):
    """
    Test retrieval of all connections.

    Note that ``static_synapse`` connections will come first.
    """

    conns = nest.GetConnections()

    actual_sources = conns.get("source")
    actual_targets = conns.get("target")

    expected_static_sources = nodes["static_synapse"]["sources"].tolist()
    expected_stdp_sources = nodes["stdp_synapse"]["sources"].tolist()
    expected_all_sources = expected_static_sources + expected_stdp_sources

    expected_static_targets = nodes["static_synapse"]["targets"].tolist()
    expected_stdp_targets = nodes["stdp_synapse"]["targets"].tolist()
    expected_all_targets = expected_static_targets + expected_stdp_targets

    nptest.assert_array_equal(actual_sources, expected_all_sources)
    nptest.assert_array_equal(actual_targets, expected_all_targets)


@pytest.mark.parametrize(
    ("source_filter", "target_filter"),
    [(True, False), (False, True), (True, True)],
)
def test_retrieve_connections_with_sliced_node_collections(nodes, source_filter, target_filter):
    """
    Test retrieval of connections for a subset of sources and targets.

    The test ensures that retrieval of a subset of connections works when
    filtering by source and/or target nodes.

    .. note::

        The source and target ``NodeCollection``s returned by the fixture are
        first converted to lists of node ids. The source and target lists are
        then concatenated and a new ``NodeCollection`` with the sliced node ids
        is created if the nodes will be used as filter. We have to convert to
        lists first because it is not possible to add a ``NodeCollection`` to
        a sliced composite.
    """

    # Take first 3 static sources and targets
    src_static = nodes["static_synapse"]["sources"][:3].tolist()
    tgt_static = nodes["static_synapse"]["targets"][:3].tolist()

    # Take final 3 stpd sources and targes to avoid those with static+stdp
    src_stdp = nodes["stdp_synapse"]["sources"][-3:].tolist()
    tgt_stpd = nodes["stdp_synapse"]["targets"][-3:].tolist()

    src_all = src_static + src_stdp
    tgt_all = tgt_static + tgt_stpd

    sources = nest.NodeCollection(src_all) if source_filter else None
    targets = nest.NodeCollection(tgt_all) if target_filter else None

    conns = nest.GetConnections(source=sources, target=targets)

    actual_sources = conns.get("source")
    actual_targets = conns.get("target")

    nptest.assert_array_equal(actual_sources, src_all)
    nptest.assert_array_equal(actual_targets, tgt_all)


def test_retrieve_connections_with_nodes_and_synapse(nodes):
    """
    Test retrieval of connections with node subset and synapse model.

    The test ensures that retrieval of a subset of connections works when
    filtering by nodes and synapse model.
    """

    # Take first 3 static sources and targets
    src_static = nodes["static_synapse"]["sources"][:3].tolist()
    tgt_static = nodes["static_synapse"]["targets"][:3].tolist()

    # Take final 3 stpd sources and targes to avoid those with static+stdp
    src_stdp = nodes["stdp_synapse"]["sources"][-3:].tolist()
    tgt_stpd = nodes["stdp_synapse"]["targets"][-3:].tolist()

    src_all = src_static + src_stdp
    tgt_all = tgt_static + tgt_stpd

    conns = nest.GetConnections(
        source=nest.NodeCollection(src_all), target=nest.NodeCollection(tgt_all), synapse_model="static_synapse"
    )

    actual_sources = conns.get("source")
    actual_targets = conns.get("target")

    nptest.assert_array_equal(actual_sources, src_static)
    nptest.assert_array_equal(actual_targets, tgt_static)


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("syn_model", ["static_synapse", "stdp_synapse"])
def test_retrieve_connections_multithreaded(syn_model):
    """
    Test multithreaded retrieval of connections filtered by synapse model.

    The relative ordering of connection data from threads is random under
    OpenMP. Hence, the retrieved connections must be sorted before comparing
    with expectation.
    """

    nodes = build_net(num_threads=4)

    syn_collection = nest.GetConnections(synapse_model=syn_model)
    actual_connections = pd.DataFrame.from_dict(syn_collection.get(["source", "target"]))
    actual_connections.sort_values(by=["source", "target"], ignore_index=True, inplace=True)
    actual_sources = actual_connections["source"]
    actual_targets = actual_connections["target"]

    expected_sources = nodes[f"{syn_model}"]["sources"].tolist()
    expected_targets = nodes[f"{syn_model}"]["targets"].tolist()

    nptest.assert_array_equal(actual_sources, expected_sources)
    nptest.assert_array_equal(actual_targets, expected_targets)
