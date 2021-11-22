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

import unittest
import math
import nest


class SynapseCollectionDistance(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def calculate_distance(self, conns, s_nodes, t_nodes):
        """Calculate a reference distance between source and target nodes"""

        s_pos = nest.GetPosition(s_nodes)
        t_pos = nest.GetPosition(t_nodes)

        src = conns.source
        trgt = conns.target

        in_3d = len(s_pos[0]) == 3

        ref_distance = []
        for s, t in zip(src, trgt):
            x_ref = t_pos[t_nodes.index(t)][0] - s_pos[s_nodes.index(s)][0]
            y_ref = t_pos[t_nodes.index(t)][1] - s_pos[s_nodes.index(s)][1]
            z_ref = 0.0
            if in_3d:
                z_ref = t_pos[t_nodes.index(t)][2] - s_pos[s_nodes.index(s)][2]

            ref_dist = math.sqrt(x_ref * x_ref + y_ref * y_ref + z_ref * z_ref)
            ref_distance.append(ref_dist)

        return tuple(ref_distance)

    def test_SynapseCollection_distance_simple(self):
        """Test distance on SynapseCollection where source and target are equal"""

        s_nodes = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(shape=[3, 3]))

        nest.Connect(s_nodes, s_nodes, {'rule': 'one_to_one'})
        conns = nest.GetConnections()
        dist = conns.distance

        self.assertTrue(all([dd == 0. for dd in dist]))

    def test_SynapseCollection_distance(self):
        """Test SynapseCollection distance function for grid positions"""

        s_nodes = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(shape=[3, 1]))
        t_nodes = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(shape=[1, 3]))

        nest.Connect(s_nodes, t_nodes)
        conns = nest.GetConnections()
        dist = conns.distance

        ref_distance = self.calculate_distance(conns, s_nodes, t_nodes)

        self.assertEqual(ref_distance, dist)

    def test_SynapseCollection_distance_free(self):
        """Test SynapseCollection distance function for positions placed freely in space"""

        positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
        s_nodes = nest.Create('iaf_psc_alpha', n=5, positions=positions)
        t_nodes = nest.Create('iaf_psc_alpha', n=7, positions=positions)

        nest.Connect(s_nodes, t_nodes, {'rule': 'pairwise_bernoulli', 'p': 0.7})
        conns = nest.GetConnections()
        dist = conns.distance

        ref_distance = self.calculate_distance(conns, s_nodes, t_nodes)

        self.assertEqual(ref_distance, dist)

    def test_SynapseCollection_distance_3D(self):
        """Test SynapseCollection distance function for spatial nodes in 3D"""

        positions = nest.spatial.free(nest.random.uniform(), num_dimensions=3)
        s_nodes = nest.Create('iaf_psc_alpha', n=8, positions=positions)
        t_nodes = nest.Create('iaf_psc_alpha', n=11, positions=positions)

        nest.Connect(s_nodes, t_nodes)
        conns = nest.GetConnections()
        dist = conns.distance

        ref_distance = self.calculate_distance(conns, s_nodes, t_nodes)

        self.assertEqual(ref_distance, dist)

    def test_SynapseCollection_distance_non_spatial(self):
        """Test SynapseCollection distance function on non-spatial nodes"""

        s_nodes = nest.Create('iaf_psc_alpha', 3)
        t_nodes = nest.Create('iaf_psc_alpha', 2)

        nest.Connect(s_nodes, t_nodes)
        conns = nest.GetConnections()
        dist = conns.distance

        dist_is_nan = [math.isnan(d) for d in dist]

        self.assertTrue(dist_is_nan)

    def test_SynapseCollection_distance_mixed(self):
        """Test SynapseCollection distance function on non-spatial and spatial nodes"""

        num_snodes_nonspatial = 3
        num_tnodes_nonspatial = 2
        num_conns_nonsparial = num_snodes_nonspatial * num_tnodes_nonspatial
        s_nodes_nonspatial = nest.Create('iaf_psc_alpha', num_snodes_nonspatial)
        t_nodes_nonspatial = nest.Create('iaf_psc_alpha', num_tnodes_nonspatial)

        positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
        s_nodes_spatial = nest.Create('iaf_psc_alpha', n=6, positions=positions)
        t_nodes_spatial = nest.Create('iaf_psc_alpha', n=7, positions=positions)

        nest.Connect(s_nodes_nonspatial, t_nodes_nonspatial)
        nest.Connect(s_nodes_spatial, t_nodes_spatial)
        conns = nest.GetConnections()
        dist = conns.distance

        # Check part that is spatial
        ref_distance = self.calculate_distance(conns[num_conns_nonsparial:], s_nodes_spatial, t_nodes_spatial)
        self.assertEqual(ref_distance, dist[num_conns_nonsparial:])

        # Check part that is non-spatial
        dist_is_nan = [math.isnan(d) for d in dist[:num_conns_nonsparial]]
        self.assertTrue(dist_is_nan)

    def test_SynapseCollection_distance_spatial_nonspatial_connected(self):
        """Test SynapseCollection distance function on non-spatial and spatial nodes that are connected"""

        num_snodes = 5
        num_tnodes = 11
        s_nodes_nonspatial = nest.Create('iaf_psc_alpha', num_snodes)

        positions = nest.spatial.free(nest.random.uniform(), num_dimensions=2)
        t_nodes_spatial = nest.Create('iaf_psc_alpha', n=num_tnodes, positions=positions)

        nest.Connect(s_nodes_nonspatial, t_nodes_spatial)
        conns = nest.GetConnections()
        dist = conns.distance

        # All should be nan
        dist_is_nan = [math.isnan(d) for d in dist]
        self.assertTrue(dist_is_nan)


def suite():
    suite = unittest.makeSuite(SynapseCollectionDistance, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
