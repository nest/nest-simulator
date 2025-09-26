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


@pytest.fixture(params=cases, ids=[f"{compr}-{nn}" for compr, nn in cases])
def network(request):
    compressed_spikes, n_nodes = request.param
    nest.ResetKernel()
    nest.use_compressed_spikes = compressed_spikes
    nrns = nest.Create("parrot_neuron", n_nodes)
    nest.Connect(nrns, nrns)
    nest.GetConnections()  # force source-table creation
    yield nrns


def test_disconnect_one_by_one(network):
    n_nodes = len(network)
    pairs = [(i, j) for i in range(n_nodes) for j in range(n_nodes)]
    random.shuffle(pairs)

    for i, j in pairs:
        nest.Disconnect(network[i], network[j])

    assert len(nest.GetConnections()) == 0


def test_disconnect_one_to_one(network):
    n_nodes = len(network)
    nest.Disconnect(network, network)

    assert len(nest.GetConnections()) == n_nodes**2 - n_nodes


def test_disconnect_by_edge(network):
    n_nodes = len(network)

    c = nest.GetConnections()

    edges = [i for i in range(n_nodes**2)]
    random.shuffle(edges)

    for i in edges:
        c[i].disconnect()

    assert len(nest.GetConnections()) == 0
