# -*- coding: utf-8 -*-
#
# test_connectivity_with_compressed_spikes.py
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

    for i, j in pairs:
        nest.Disconnect(n[i - 1], n[j - 1])


@pytest.mark.parametrize("with_compressed_spikes", [False, True])
@pytest.mark.parametrize("n_nodes", [5, 10, 13, 42])
def test_disconnect_all_at_once(with_compressed_spikes, n_nodes):
    nest.ResetKernel()

    nest.use_compressed_spikes = with_compressed_spikes

    n = nest.Create("parrot_neuron", n_nodes)
    nest.Connect(n, n)

    nest.GetConnections()

    nest.Disconnect(n, n)


@pytest.mark.parametrize("with_compressed_spikes", [False, True])
@pytest.mark.parametrize("n_nodes", [5, 10, 13, 42])
def test_disconnect_by_edge(with_compressed_spikes, n_nodes):
    nest.ResetKernel()

    nest.use_compressed_spikes = with_compressed_spikes

    n = nest.Create("parrot_neuron", n_nodes)
    nest.Connect(n, n)

    c = nest.GetConnections()

    edges = [i for i in range(0, n_nodes * n_nodes)]
    random.shuffle(edges)

    for i in edges:
        c[i].disconnect()
