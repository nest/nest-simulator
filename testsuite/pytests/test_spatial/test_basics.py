# -*- coding: utf-8 -*-
#
# test_basics.py
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
Tests for basic hl_api_spatial functions.
"""

import unittest
import nest

try:
    import numpy as np

    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False


class BasicsTestCase(unittest.TestCase):
    def test_create_layer(self):
        """Creating a single layer."""
        shape = [5, 4]
        nest.ResetKernel()
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(shape=shape))
        self.assertEqual(len(layer), shape[0]*shape[1])

    def test_create_layer_with_param(self):
        """Creating a layer with parameters."""
        shape = [5, 4]
        nest.ResetKernel()
        layer = nest.Create('iaf_psc_alpha',
                            params={'V_m': -55.0},
                            positions=nest.spatial.grid(shape=shape))
        layer_vm = layer.get('V_m')
        for vm in layer_vm:
            self.assertEqual(vm, -55.0)

    def test_GetPosition(self):
        """Check if GetPosition returns proper positions."""
        pos = ((1.0, 0.0), (0.0, 1.0), (3.5, 1.5))
        nest.ResetKernel()
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        # GetPosition of single node
        nodepos_exp = nest.GetPosition(layer[:1])
        self.assertEqual(nodepos_exp, pos[0])

        nodepos_exp = nest.GetPosition(layer[-1:])
        self.assertEqual(nodepos_exp, pos[-1])

        nodepos_exp = nest.GetPosition(layer[1:2])
        self.assertEqual(nodepos_exp, pos[1])

        # GetPosition of all the nodes in the layer
        nodepos_exp = nest.GetPosition(layer)

        for npe, npr in zip(nodepos_exp, pos):
            self.assertEqual(npe, npr)

        self.assertEqual(pos, nodepos_exp)

        # GetPosition on some of the node IDs
        nodepos_exp = nest.GetPosition(layer[:2])
        self.assertEqual(nodepos_exp, (pos[0], pos[1]))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_Displacement(self):
        """Interface check on displacement calculations."""
        lshape = [5, 4]
        nest.ResetKernel()
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=lshape))

        # node IDs -> node IDs, all displacements must be zero here
        d = nest.Displacement(layer, layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all(dd == (0., 0.) for dd in d))

        # single node ID -> node IDs
        d = nest.Displacement(layer[:1], layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # node IDs -> single node ID
        d = nest.Displacement(layer, layer[:1])
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # Displacement between node ID 1 and 6. They are on the same y-axis, and
        # directly next to each other on the x-axis, so the displacement on
        # x-axis should be approximately -dx, while the displacement on the
        # y-axis should be 0.
        d = nest.Displacement(layer[:1], layer[4:5])
        dx = 1. / lshape[0]
        self.assertAlmostEqual(d[0][0], -dx, 3)
        self.assertEqual(d[0][1], 0.0)

        # Displacement between node ID 1 and 2. They are on the same x-axis, and
        # directly next to each other on the y-axis, so the displacement on
        # x-axis should be 0, while the displacement on the y-axis should be
        # approximately dy.
        d = nest.Displacement(layer[:1], layer[1:2])
        dy = 1. / lshape[1]
        self.assertEqual(d[0][0], 0.0)
        self.assertAlmostEqual(d[0][1], dy, 3)

        # Test that we get correct results if to_arg and from_arg are from two
        # different layers
        layer2 = nest.Create('iaf_psc_alpha',
                             positions=nest.spatial.grid(shape=lshape))
        d = nest.Displacement(layer[:1], layer2[4:5])
        dx = 1. / lshape[0]
        self.assertAlmostEqual(d[0][0], -dx, 3)
        self.assertEqual(d[0][1], 0.0)

        d = nest.Displacement(layer[:1], layer2[1:2])
        dy = 1. / lshape[1]
        self.assertEqual(d[0][0], 0.0)
        self.assertAlmostEqual(d[0][1], dy, 3)

        # Test that an error is thrown if to_arg and from_arg have different
        # size.
        with self.assertRaises(ValueError):
            d = nest.Displacement(layer[1:3], layer[2:7])

        # position -> node IDs
        d = nest.Displacement([(0.0, 0.0)], layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # position -> node IDs
        d = nest.Displacement(np.array([0.0, 0.0]), layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # positions -> node IDs
        d = nest.Displacement([np.array([0.0, 0.0])] * len(layer), layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all(len(dd) == 2 for dd in d))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_Distance(self):
        """Interface check on distance calculations."""
        lshape = [5, 4]
        nest.ResetKernel()
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=lshape))

        # node IDs -> node IDs, all displacements must be zero here
        d = nest.Distance(layer, layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all([dd == 0. for dd in d]))

        # single node ID -> node IDs
        d = nest.Distance(layer[:1], layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        # node IDs -> single node ID
        d = nest.Distance(layer, layer[:1])
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        # Distance between node ID 1 and 6. They are on the same y-axis, and
        # directly next to each other on the x-axis, so the distance should be
        # approximately dx. The same is true for distance between node ID 6 and 1.
        d = nest.Distance(layer[:1], layer[4:5])
        dx = 1. / lshape[0]
        self.assertAlmostEqual(d[0], dx, 3)

        d = nest.Distance(layer[4:5], layer[:1])
        self.assertAlmostEqual(d[0], dx, 3)

        # Distance between node ID 1 and 2. They are on the same x-axis, and
        # directly next to each other on the y-axis, so the distance should be
        # approximately dy. The same is true for distance between node ID 2 and 1.
        d = nest.Distance(layer[:1], layer[1:2])
        dy = 1. / lshape[1]
        self.assertAlmostEqual(d[0], dy, 3)

        d = nest.Distance(layer[1:2], layer[:1])
        self.assertAlmostEqual(d[0], dy, 3)

        # Test that we get correct results if to_arg and from_arg are from two
        # different layers
        layer2 = nest.Create('iaf_psc_alpha',
                             positions=nest.spatial.grid(shape=lshape))
        d = nest.Distance(layer[:1], layer2[4:5])
        dx = 1. / lshape[0]
        self.assertAlmostEqual(d[0], dx, 3)

        d = nest.Distance(layer[4:5], layer2[:1])
        self.assertAlmostEqual(d[0], dx, 3)

        d = nest.Distance(layer[:1], layer2[1:2])
        dy = 1. / lshape[1]
        self.assertAlmostEqual(d[0], dy, 3)

        d = nest.Distance(layer[1:2], layer2[:1])
        self.assertAlmostEqual(d[0], dy, 3)

        # Test that an error is thrown if to_arg and from_arg have different
        # size.
        with self.assertRaises(ValueError):
            d = nest.Distance(layer[1:3], layer[2:7])

        # position -> node IDs
        d = nest.Distance([[0.0, 0.0], ], layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        # position -> node IDs
        d = nest.Distance(np.array([0.0, 0.0]), layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        # positions -> node IDs
        d = nest.Distance([np.array([0.0, 0.0])] * len(layer), layer)
        self.assertEqual(len(d), len(layer))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_FindElements(self):
        """Interface and result check for finding nearest element.
           This function is Py only, so we also need to check results."""
        # nodes at [-1,0,1]x[-1,0,1], column-wise
        nest.ResetKernel()
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[3, 3], extent=(3., 3.)))

        # single location at center
        n = nest.FindNearestElement(layer, (0., 0.))
        self.assertEqual(n, layer[4])

        # two locations, one layer
        n = nest.FindNearestElement(layer, ((0., 0.), (1., 1.)))
        self.assertEqual(n[0], layer[4])
        self.assertEqual(n[1], layer[6])

        # several closest locations, not all
        n = nest.FindNearestElement(layer, (0.5, 0.5))
        self.assertEqual(len(n), 1)
        self.assertTrue(n.get('global_id') in nest.NodeCollection((4, 5, 7, 8)))

        # several closest locations, all
        n = nest.FindNearestElement(layer, (0.5, 0.5), find_all=True)
        self.assertEqual(len(n), 4)
        self.assertEqual(n[0], layer[3])
        self.assertEqual(n[3], layer[7])

        # complex case
        n = nest.FindNearestElement(layer, ((0., 0.), (0.5, 0.5)),
                                    find_all=True)
        self.assertEqual(len(n), 2)
        self.assertEqual(n[0], [layer[4]])
        self.assertEqual(n[1], [layer[3], layer[4], layer[6], layer[7]])

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_GetCenterElement(self):
        """Interface and result check for finding center element.
           This function is Py only, so we also need to check results."""
        # nodes at [-1,0,1]x[-1,0,1], column-wise
        nest.ResetKernel()
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[3, 3], extent=(2., 2.)))

        # single layer
        n = nest.FindCenterElement(layer)
        self.assertEqual(n, layer[4:5])

        # new layer
        layer2 = nest.Create('iaf_psc_alpha',
                             positions=nest.spatial.grid(shape=[3, 3], extent=(2., 2.)))
        n = nest.FindCenterElement(layer2)
        self.assertEqual(n, layer2[4:5])

    def test_GetTargetNodes(self):
        """Interface check for finding targets."""

        cdict = {'rule': 'pairwise_bernoulli',
                 'p': 1.,
                 'mask': {'grid': {'shape': [2, 2]}}}
        sdict = {'synapse_model': 'stdp_synapse'}
        nest.ResetKernel()
        # For co-dependent properties, we use `set()` instead of kernel attributes
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[3, 3],
                                                        extent=(2., 2.),
                                                        edge_wrap=True))

        # connect layer -> layer
        nest.Connect(layer, layer, cdict, sdict)

        t = nest.GetTargetNodes(layer[0], layer)
        self.assertEqual(len(t), 1)

        t = nest.GetTargetNodes(layer, layer)
        self.assertEqual(len(t), len(layer))
        # 2x2 mask -> four targets
        self.assertTrue(all([len(g) == 4 for g in t]))

        t = nest.GetTargetNodes(layer, layer, syn_model='static_synapse')
        self.assertEqual(len(t), len(layer))
        self.assertTrue(all([len(g) == 0 for g in t]))  # no static syns

        t = nest.GetTargetNodes(layer, layer, syn_model='stdp_synapse')
        self.assertEqual(len(t), len(layer))
        self.assertTrue(
            all([len(g) == 4 for g in t]))  # 2x2 mask  -> four targets

        t = nest.GetTargetNodes(layer[0], layer)
        self.assertEqual(len(t), 1)
        self.assertEqual(t[0], nest.NodeCollection([1, 2, 4, 5]))

        t = nest.GetTargetNodes(layer[4], layer)
        self.assertEqual(len(t), 1)
        self.assertEqual(t[0], nest.NodeCollection([5, 6, 8, 9]))

        t = nest.GetTargetNodes(layer[8], layer)
        self.assertEqual(len(t), 1)
        self.assertEqual(t[0], nest.NodeCollection([1, 3, 7, 9]))

    def test_GetSourceNodes(self):
        """Interface check for finding source"""

        cdict = {'rule': 'pairwise_bernoulli',
                 'p': 1.,
                 'mask': {'grid': {'shape': [2, 2]}}}
        sdict = {'synapse_model': 'stdp_synapse'}
        nest.ResetKernel()
        # For co-dependent properties, we use `set()` instead of kernel attributes
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[3, 3],
                                                        extent=(2., 2.),
                                                        edge_wrap=True))

        # connect layer -> layer
        nest.Connect(layer, layer, cdict, sdict)

        s = nest.GetSourceNodes(layer, layer[0])
        self.assertEqual(len(s), 1)

        s = nest.GetSourceNodes(layer, layer)
        self.assertEqual(len(s), len(layer))
        # 2x2 mask -> each nodes has four sources
        self.assertTrue(all([len(g) == 4 for g in s]))

        s = nest.GetSourceNodes(layer, layer, syn_model='static_synapse')
        self.assertEqual(len(s), len(layer))
        self.assertTrue(all([len(g) == 0 for g in s]))  # no static syns

        s = nest.GetSourceNodes(layer, layer, syn_model='stdp_synapse')
        self.assertEqual(len(s), len(layer))
        self.assertTrue(all([len(g) == 4 for g in s]))  # 2x2 mask  -> four sources

        s = nest.GetSourceNodes(layer, layer[0])
        self.assertEqual(len(s), 1)
        self.assertEqual(s[0], nest.NodeCollection([1, 3, 7, 9]))

        s = nest.GetSourceNodes(layer, layer[4])
        self.assertEqual(len(s), 1)
        self.assertEqual(s[0], nest.NodeCollection([1, 2, 4, 5]))

        s = nest.GetSourceNodes(layer, layer[8])
        self.assertEqual(len(s), 1)
        self.assertEqual(s[0], nest.NodeCollection([5, 6, 8, 9]))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_GetTargetPositions(self):
        """Test that GetTargetPosition works as expected"""

        cdict = {'rule': 'pairwise_bernoulli',
                 'p': 1.}
        sdict = {'synapse_model': 'stdp_synapse'}

        # For co-dependent properties, we use `set()` instead of kernel attributes
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[1, 1],
                                                        extent=(1., 1.),
                                                        edge_wrap=False))
        nest.Connect(layer, layer, cdict, sdict)

        # Simple test with one node ID in the layer, should be placed in the origin
        p = nest.GetTargetPositions(layer, layer)
        self.assertTrue(p, [[(0.0, 0.0)]])

        # Test positions on a grid, we can calculate what they should be
        nest.ResetKernel()
        # For co-dependent properties, we use `set()` instead of kernel attributes
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)

        x_extent = 1.
        y_extent = 1.
        shape = [3, 3]

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=shape,
                                                        extent=[x_extent, y_extent],
                                                        edge_wrap=False))
        nest.Connect(layer, layer, cdict, sdict)

        p = nest.GetTargetPositions(layer[:1], layer)
        self.assertEqual(len(p), 1)
        self.assertTrue(all([len(pp) == 2 for pp in p[0]]))

        p = nest.GetTargetPositions(layer, layer)
        self.assertEqual(len(p), len(layer))

        dx = x_extent / shape[0]
        dy = y_extent / shape[1]

        x = [-dx, -dx, -dx, 0.0, 0.0, 0.0, dx, dx, dx]
        y = [dy, 0.0, -dy, dy, 0.0, -dy, dy, 0.0, -dy]

        pos = [(x[i], y[i]) for i in range(len(x))]

        for indx in range(len(pos)):
            # 4 chosen randomly, they should all be the same, as all node IDs in
            # the layer are connected
            self.assertAlmostEqual(p[4][indx][0], pos[indx][0])
            self.assertAlmostEqual(p[4][indx][1], pos[indx][1])

        # Test that we get correct positions when we send in a positions array
        # when creating the layer
        nest.ResetKernel()
        # For co-dependent properties, we use `set()` instead of kernel attributes
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)

        positions = [(np.random.uniform(-0.5, 0.5),
                      np.random.uniform(-0.5, 0.5)) for _ in range(50)]
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.free(positions,
                                                        edge_wrap=False))
        nest.Connect(layer, layer, cdict, sdict)

        p = nest.GetTargetPositions(layer[:1], layer)

        for indx in range(len(p[0])):
            self.assertAlmostEqual(positions[indx][0], p[0][indx][0])
            self.assertAlmostEqual(positions[indx][1], p[0][indx][1])

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_GetSourcePositions(self):
        """Test that GetSourcePosition works as expected"""

        cdict = {'rule': 'pairwise_bernoulli',
                 'p': 1.}
        sdict = {'synapse_model': 'stdp_synapse'}

        # For co-dependent properties, we use `set()` instead of kernel attributes
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=[1, 1],
                                                        extent=(1., 1.),
                                                        edge_wrap=False))
        nest.Connect(layer, layer, cdict, sdict)

        # Simple test with one node ID in the layer, should be placed in the origin
        p = nest.GetSourcePositions(layer, layer)
        self.assertTrue(p, [[(0.0, 0.0)]])

        # Test positions on a grid, we can calculate what they should be
        nest.ResetKernel()
        # For co-dependent properties, we use `set()` instead of kernel attributes
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)

        x_extent = 1.
        y_extent = 1.
        shape = [3, 3]

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(shape=shape,
                                                        extent=[x_extent, y_extent],
                                                        edge_wrap=False))
        nest.Connect(layer, layer, cdict, sdict)

        p = nest.GetSourcePositions(layer, layer[:1])
        self.assertEqual(len(p), 1)
        self.assertTrue(all([len(pp) == 2 for pp in p[0]]))

        p = nest.GetSourcePositions(layer, layer)
        self.assertEqual(len(p), len(layer))

        dx = x_extent / shape[0]
        dy = y_extent / shape[1]

        x = [-dx, -dx, -dx, 0.0, 0.0, 0.0, dx, dx, dx]
        y = [dy, 0.0, -dy, dy, 0.0, -dy, dy, 0.0, -dy]

        pos = [(x[i], y[i]) for i in range(len(x))]

        for indx in range(len(pos)):
            # 4 chosen randomly, they should all be the same, as all node IDs in
            # the layer are connected
            self.assertAlmostEqual(p[4][indx][0], pos[indx][0])
            self.assertAlmostEqual(p[4][indx][1], pos[indx][1])

        # Test that we get correct positions when we send in a positions array
        # when creating the layer
        nest.ResetKernel()
        # For co-dependent properties, we use `set()` instead of kernel attributes
        nest.set(use_compressed_spikes=False, sort_connections_by_source=False)

        positions = [(np.random.uniform(-0.5, 0.5),
                      np.random.uniform(-0.5, 0.5)) for _ in range(50)]
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.free(positions,
                                                        edge_wrap=False))
        nest.Connect(layer, layer, cdict, sdict)

        p = nest.GetSourcePositions(layer, layer[:1])

        for indx in range(len(p[0])):
            self.assertAlmostEqual(positions[indx][0], p[0][indx][0])
            self.assertAlmostEqual(positions[indx][1], p[0][indx][1])

    def testSlicedPositions(self):
        """Correct positions from spatial attribute of sliced NodeCollection"""
        nest.ResetKernel()
        positions = nest.spatial.free(nest.random.uniform(min=-1, max=1), num_dimensions=2)
        nodes = nest.Create('iaf_psc_alpha', 10, positions=positions)
        all_positions = sum([list(nodes[i].spatial['positions']) for i in range(len(nodes))], start=[])
        self.assertEqual(tuple(all_positions), nodes.spatial['positions'])
        self.assertEqual(tuple(nodes[::2].spatial['positions']), nodes.spatial['positions'][::2])


def suite():
    suite = unittest.makeSuite(BasicsTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
