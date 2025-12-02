# -*- coding: utf-8 -*-
#
# test_issue_3532.py
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


import random

import nest
import pytest

"""
Testing disconnection of connections with compressd spikes, toggled on/off, using a random edge order.
"""

cases = [(compressed_spikes, n_nodes) for compressed_spikes in [False, True] for n_nodes in [5, 10, 13, 42]]


def get_conn_pairs():
    conns = nest.GetConnections()
    return set(zip(conns.sources(), conns.targets()))


@pytest.fixture(params=cases, ids=[f"{compr}-{nn}" for compr, nn in cases])
def network(request):
    compressed_spikes, n_nodes = request.param

    random.seed(12345)
    nest.ResetKernel()
    nest.use_compressed_spikes = compressed_spikes
    nrns = nest.Create("parrot_neuron", n_nodes)
    nest.Connect(nrns, nrns)
    nest.GetConnections()  # force source-table creation
    yield nrns


@pytest.mark.parametrize("with_compressed_spikes", [False, True])
@pytest.mark.parametrize("n_nodes", [5, 10, 13, 42])
def test_disconnect_one_by_one(with_compressed_spikes, n_nodes):
    nest.ResetKernel()

    nest.use_compressed_spikes = with_compressed_spikes

    n = nest.Create("parrot_neuron", n_nodes)
    nest.Connect(n, n)

    nest.GetConnections()

    pairs = [(i, j) for i in range(1, n_nodes + 1) for j in range(1, n_nodes + 1)]
    random.shuffle(pairs)

    pre_conns = get_conn_pairs()
    for i, j in pairs:
        nest.Disconnect(n[i - 1], n[j - 1])

        post_conns = get_conn_pairs()

        # must pass tuple with single tuple-element to set() to get one-element set of tuples
        # instead of two-element set of node ids.
        assert pre_conns - post_conns == set(((i, j),))

        pre_conns = post_conns


def test_disconnect_one_to_one(network):
    n_nodes = len(network)
    nest.Disconnect(network, network)

    assert len(nest.GetConnections()) == n_nodes**2 - n_nodes


def test_disconnect_all_at_once(network):
    n_nodes = len(network)
    nest.Disconnect(network, network, conn_spec="all_to_all")

    assert nest.num_connections == 0


def test_disconnect_by_edge(network):
    n_nodes = len(network)

    c = nest.GetConnections()

    edges = [i for i in range(n_nodes**2)]
    random.shuffle(edges)

    for i in edges:
        c[i].disconnect()

    assert len(nest.GetConnections()) == 0


@pytest.mark.parametrize("use_compressed", [False, True])
@pytest.mark.parametrize("p_disconnect", [0.3, 0.9])
def test_disconnect_random_connections(use_compressed, p_disconnect):
    """
    Test if disconnection works when connections have been created in random order.
    """

    num_neurons = 17
    conn_reps = 3

    random.seed(12345)
    nest.ResetKernel()
    nest.use_compressed_spikes = use_compressed

    nrns = nest.Create("parrot_neuron", num_neurons)

    # Create all-to-all connectivity, where each s-t pair is connected conn_reps times
    # Ensure that connections are created in random order
    gids = nrns.global_id
    st_pairs = [(s, t) for s in gids for t in gids for _ in range(conn_reps)]
    random.shuffle(st_pairs)
    nest.Connect(*zip(*st_pairs), "one_to_one")

    # Delete random sample of connections. As SynapseCollections do not allow slicing etc,
    # we need to do this connection by connection. We also collect those that are not to
    # be disconnected: these form the expectation for the test.
    all_conns = nest.GetConnections()

    not_disconn = []
    for conn in all_conns:
        if random.random() < p_disconnect:
            nest.Disconnect(conn)
        else:
            not_disconn.append(tuple(conn.get(["source", "target"]).values()))

    post_conns = nest.GetConnections()
    assert sorted(not_disconn) == sorted(list(zip(*post_conns.get(["source", "target"]).values())))
