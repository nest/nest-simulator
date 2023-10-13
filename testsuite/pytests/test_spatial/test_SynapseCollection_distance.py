# -*- coding: utf-8 -*-
#
# test_SynapseCollection_distance.py
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
Tests distance between sources and targets of SynapseCollection
"""

import math

import nest
import numpy as np
import pytest


@pytest.fixture(autouse=True)
def _reset_kernel():
    nest.ResetKernel()


def _calculate_distance(conns, s_nodes, t_nodes):
    """Calculate a reference distance between source and target nodes"""

    s_pos = nest.GetPosition(s_nodes)
    t_pos = nest.GetPosition(t_nodes)

    src = conns.source
    trgt = conns.target

    dim = len(s_pos[0])

    ref_distance = [
        np.linalg.norm(np.array(t_pos[t_nodes.index(t)]) - np.array(s_pos[s_nodes.index(s)]), ord=2)
        for s, t in zip(conns.source, conns.target)
    ]

    return ref_distance


def test_SynapseCollection_distance_simple():
    """Test distance on SynapseCollection where source and target are equal"""

    s_nodes = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid(shape=[3, 3]))

    nest.Connect(s_nodes, s_nodes, {"rule": "one_to_one"})
    conns = nest.GetConnections()
    dist = conns.distance

    assert all(d == 0 for d in dist)


def test_SynapseCollection_distance():
    """Test SynapseCollection distance function for grid positions"""

    s_nodes = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid(shape=[3, 1]))
    t_nodes = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid(shape=[1, 3]))

    nest.Connect(s_nodes, t_nodes)
    conns = nest.GetConnections()
    dist = conns.distance

    ref_distance = _calculate_distance(conns, s_nodes, t_nodes)

    assert dist == pytest.approx(ref_distance)


def test_SynapseCollection_distance_free():
    """Test SynapseCollection distance function for positions placed freely in space"""

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
    s_nodes = nest.Create("iaf_psc_alpha", n=5, positions=positions)
    t_nodes = nest.Create("iaf_psc_alpha", n=7, positions=positions)

    nest.Connect(s_nodes, t_nodes, {"rule": "pairwise_bernoulli", "p": 0.7})
    conns = nest.GetConnections()
    dist = conns.distance

    ref_distance = _calculate_distance(conns, s_nodes, t_nodes)

    assert dist == pytest.approx(ref_distance)


def test_SynapseCollection_distance_3D():
    """Test SynapseCollection distance function for spatial nodes in 3D"""

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=3)
    s_nodes = nest.Create("iaf_psc_alpha", n=8, positions=positions)
    t_nodes = nest.Create("iaf_psc_alpha", n=11, positions=positions)

    nest.Connect(s_nodes, t_nodes)
    conns = nest.GetConnections()
    dist = conns.distance

    ref_distance = _calculate_distance(conns, s_nodes, t_nodes)

    assert dist == pytest.approx(ref_distance)


def test_SynapseCollection_distance_non_spatial():
    """Test SynapseCollection distance function on non-spatial nodes"""

    s_nodes = nest.Create("iaf_psc_alpha", 3)
    t_nodes = nest.Create("iaf_psc_alpha", 2)

    nest.Connect(s_nodes, t_nodes)
    conns = nest.GetConnections()
    dist = conns.distance

    assert all(math.isnan(d) for d in dist)


def test_SynapseCollection_distance_mixed():
    """Test SynapseCollection distance function on non-spatial and spatial nodes"""

    num_snodes_nonspatial = 3
    num_tnodes_nonspatial = 2
    num_conns_nonspatial = num_snodes_nonspatial * num_tnodes_nonspatial
    s_nodes_nonspatial = nest.Create("iaf_psc_alpha", num_snodes_nonspatial)
    t_nodes_nonspatial = nest.Create("iaf_psc_alpha", num_tnodes_nonspatial)

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
    s_nodes_spatial = nest.Create("iaf_psc_alpha", n=6, positions=positions)
    t_nodes_spatial = nest.Create("iaf_psc_alpha", n=7, positions=positions)

    nest.Connect(s_nodes_nonspatial, t_nodes_nonspatial)
    nest.Connect(s_nodes_spatial, t_nodes_spatial)
    conns = nest.GetConnections()
    dist = conns.distance

    # Check part that is spatial
    ref_distance = _calculate_distance(conns[num_conns_nonspatial:], s_nodes_spatial, t_nodes_spatial)
    assert dist[num_conns_nonspatial:] == pytest.approx(ref_distance)

    # Check part that is non-spatial
    assert all(math.isnan(d) for d in dist[:num_conns_nonspatial])


def test_SynapseCollection_distance_spatial_nonspatial_connected():
    """Test SynapseCollection distance function on non-spatial and spatial nodes that are connected"""

    num_snodes = 5
    num_tnodes = 11
    s_nodes_nonspatial = nest.Create("iaf_psc_alpha", num_snodes)

    positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
    t_nodes_spatial = nest.Create("iaf_psc_alpha", n=num_tnodes, positions=positions)

    nest.Connect(s_nodes_nonspatial, t_nodes_spatial)
    conns = nest.GetConnections()
    dist = conns.distance

    # All should be nan
    assert all(math.isnan(d) for d in dist)
