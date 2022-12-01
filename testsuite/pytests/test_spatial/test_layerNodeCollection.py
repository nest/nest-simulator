# -*- coding: utf-8 -*-
#
# test_layerNodeCollection.py
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
General tests for layer NodeCollections.
"""

import unittest
import nest
import numpy as np


class TestLayerNodeCollection(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_addLayerAndNodeCollection(self):
        """Test that concatenation of plain nc and layer is illegal."""
        nodes = nest.Create('iaf_psc_alpha', 10)
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[5, 5]))

        with self.assertRaises(nest.kernel.NESTError):
            c = nodes + layer
        with self.assertRaises(nest.kernel.NESTError):
            d = layer + nodes

    def test_addTwoLayers(self):
        "Test that concatenation of two layers is illegal"
        layer1 = nest.Create('iaf_psc_alpha',
                             positions=nest.spatial.grid(shape=[5, 5]))
        layer2 = nest.Create('iaf_psc_alpha',
                             positions=nest.spatial.grid(shape=[5, 5]))

        with self.assertRaises(nest.kernel.NESTError):
            c = layer1 + layer2

    def test_extent_center_mask(self):
        """Correct extent and center when connecting with mask"""
        nest.SetKernelStatus({'rng_seed': 1234})
        # Allowing up to 5 percent relative difference from expected number of connections,
        # because of randomness. With 100 different seeds the standard deviation is 0.007,
        # with a maximum relative difference of 1.8 percent. Without the fix from #2459 the
        # relative difference would be 20-30 percent.
        rel_limit = 0.05
        num_nodes = 100
        r = 0.5

        for num_dimensions in (2, 3):
            # Iterate over values of lower left and upper right.
            for ll, ur in ((-3, -1),
                           (-2, 0),
                           (-1, 1),
                           (0, 2),
                           (1, 3)):
                nest.ResetKernel()
                param = nest.random.uniform(ll * r, ur * r)
                free_positions = nest.spatial.free(param, num_dimensions=num_dimensions, edge_wrap=True)

                nodes = nest.Create('iaf_psc_alpha', num_nodes, positions=free_positions)
                spatial = nodes.spatial  # Extract spatial information
                for value in spatial["extent"]:
                    self.assertAlmostEqual(value, 2 * r)

                mask_radius = r
                if num_dimensions == 2:
                    mask = {'circular': {'radius': mask_radius}}
                    n_dim_volume = np.pi * mask_radius**2
                else:
                    mask = {'spherical': {'radius': mask_radius}}
                    n_dim_volume = 4/3 * np.pi * mask_radius**3
                density = num_nodes / (2 * r)**num_dimensions
                expected_conns_per_node = density * n_dim_volume
                expected_total_conns = expected_conns_per_node * num_nodes
                print(f'Expecting {expected_total_conns:.0f} connections')
                nest.Connect(nodes, nodes, {'rule': 'pairwise_bernoulli', 'p': 1.0, 'mask': mask})
                print(f'Num. connections: {nest.GetKernelStatus("num_connections")}')
                rel_diff = abs(nest.GetKernelStatus("num_connections") - expected_total_conns) / expected_total_conns
                self.assertLess(rel_diff, rel_limit)

    def test_extent_center_single(self):
        """Correct extent and center with single node"""
        r = 0.5
        param = nest.random.uniform(-r, r)
        free_positions = nest.spatial.free(param, num_dimensions=2, edge_wrap=True)

        with self.assertRaises(nest.kernel.NESTError):
            nest.Create('iaf_psc_alpha', positions=free_positions)

        extent = [2 * r, 2 * r]
        free_positions_extent = nest.spatial.free(param, edge_wrap=True, extent=extent)
        nodes = nest.Create('iaf_psc_alpha', positions=free_positions_extent)

        spatial = nodes.spatial  # Extract spatial information
        self.assertEqual(spatial["center"], spatial["positions"][0])  # Center will be at the position of the only node
        self.assertEqual(spatial["extent"], tuple(extent))


def suite():
    suite = unittest.makeSuite(TestLayerNodeCollection, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
