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
Tests for basic topology hl_api functions.
"""

import unittest
import nest

try:
    import numpy

    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False


class BasicsTestCase(unittest.TestCase):
    def test_create_layer(self):
        """Creating a single layer."""
        nr = 4
        nc = 5
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(nr, nc))
        self.assertEqual(len(l), nr * nc)

    def test_create_layer_with_param(self):
        """Creating a layer with parameters."""
        nr = 4
        nc = 5
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        params={'V_m': -55.0},
                        positions=nest.spatial.grid(nr, nc))
        layer_vm = l.get('V_m')
        for vm in layer_vm:
            self.assertEqual(vm, -55.0)

    def test_GetPosition(self):
        """Check if GetPosition returns proper positions."""
        pos = ((1.0, 0.0), (0.0, 1.0), (3.5, 1.5))
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(pos))

        # GetPosition of single node
        nodepos_exp = nest.GetPosition(l[:1])
        self.assertEqual(nodepos_exp, pos[0])

        nodepos_exp = nest.GetPosition(l[-1:])
        self.assertEqual(nodepos_exp, pos[-1])

        nodepos_exp = nest.GetPosition(l[1:2])
        self.assertEqual(nodepos_exp, pos[1])

        # GetPosition of all the nodes in the layer
        nodepos_exp = nest.GetPosition(l)

        for npe, npr in zip(nodepos_exp, pos):
            self.assertEqual(npe, npr)

        self.assertEqual(pos, nodepos_exp)

        # GetPosition on some of the GIDs
        nodepos_exp = nest.GetPosition(l[:2])
        self.assertEqual(nodepos_exp, (pos[0], pos[1]))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_Displacement(self):
        """Interface check on displacement calculations."""
        ldict = {'rows': 4, 'columns': 5}
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(
                            ldict['rows'], ldict['columns']))

        # gids -> gids, all displacements must be zero here
        d = nest.Displacement(l, l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all(dd == (0., 0.) for dd in d))

        # single gid -> gids
        d = nest.Displacement(l[:1], l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # gids -> single gid
        d = nest.Displacement(l, l[:1])
        self.assertEqual(len(d), len(l))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # Displacement between gid 1 and 6. They are on the same y-axis, and
        # directly next to each other on the x-axis, so the displacement on
        # x-axis should be approximately -dx, while the displacement on the
        # y-axis should be 0.
        d = nest.Displacement(l[:1], l[4:5])
        dx = 1. / ldict['columns']
        self.assertAlmostEqual(d[0][0], -dx, 3)
        self.assertEqual(d[0][1], 0.0)

        # Displacement between gid 1 and 2. They are on the same x-axis, and
        # directly next to each other on the y-axis, so the displacement on
        # x-axis should be 0, while the displacement on the y-axis should be
        # approximately dy.
        d = nest.Displacement(l[:1], l[1:2])
        dy = 1. / ldict['rows']
        self.assertEqual(d[0][0], 0.0)
        self.assertAlmostEqual(d[0][1], dy, 3)

        # Test that we get correct results if to_arg and from_arg are from two
        # different layers
        l2 = nest.Create('iaf_psc_alpha',
                         positions=nest.spatial.grid(
                             ldict['rows'], ldict['columns']))
        d = nest.Displacement(l[:1], l2[4:5])
        dx = 1. / ldict['columns']
        self.assertAlmostEqual(d[0][0], -dx, 3)
        self.assertEqual(d[0][1], 0.0)

        d = nest.Displacement(l[:1], l2[1:2])
        dy = 1. / ldict['rows']
        self.assertEqual(d[0][0], 0.0)
        self.assertAlmostEqual(d[0][1], dy, 3)

        # Test that an error is thrown if to_arg and from_arg have different
        # size.
        with self.assertRaises(nest.kernel.NESTError):
            d = nest.Displacement(l[1:3], l[2:7])

        # position -> gids
        d = nest.Displacement([(0.0, 0.0)], l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        from numpy import array

        # position -> gids
        d = nest.Displacement(array([0.0, 0.0]), l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # positions -> gids
        d = nest.Displacement([array([0.0, 0.0])] * len(l), l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all(len(dd) == 2 for dd in d))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_Distance(self):
        """Interface check on distance calculations."""
        ldict = {'rows': 4, 'columns': 5}
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(
                            ldict['rows'], ldict['columns']))

        # gids -> gids, all displacements must be zero here
        d = nest.Distance(l, l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all([dd == 0. for dd in d]))

        # single gid -> gids
        d = nest.Distance(l[:1], l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        # gids -> single gid
        d = nest.Distance(l, l[:1])
        self.assertEqual(len(d), len(l))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        # Distance between gid 1 and 6. They are on the same y-axis, and
        # directly next to each other on the x-axis, so the distance should be
        # approximately dx. The same is true for distance between gid 6 and 1.
        d = nest.Distance(l[:1], l[4:5])
        dx = 1. / ldict['columns']
        self.assertAlmostEqual(d[0], dx, 3)

        d = nest.Distance(l[4:5], l[:1])
        self.assertAlmostEqual(d[0], dx, 3)

        # Distance between gid 1 and 2. They are on the same x-axis, and
        # directly next to each other on the y-axis, so the distance should be
        # approximately dy. The same is true for distance between gid 2 and 1.
        d = nest.Distance(l[:1], l[1:2])
        dy = 1. / ldict['rows']
        self.assertAlmostEqual(d[0], dy, 3)

        d = nest.Distance(l[1:2], l[:1])
        self.assertAlmostEqual(d[0], dy, 3)

        # Test that we get correct results if to_arg and from_arg are from two
        # different layers
        l2 = nest.Create('iaf_psc_alpha',
                         positions=nest.spatial.grid(
                             ldict['rows'], ldict['columns']))
        d = nest.Distance(l[:1], l2[4:5])
        dx = 1. / ldict['columns']
        self.assertAlmostEqual(d[0], dx, 3)

        d = nest.Distance(l[4:5], l2[:1])
        self.assertAlmostEqual(d[0], dx, 3)

        d = nest.Distance(l[:1], l2[1:2])
        dy = 1. / ldict['rows']
        self.assertAlmostEqual(d[0], dy, 3)

        d = nest.Distance(l[1:2], l2[:1])
        self.assertAlmostEqual(d[0], dy, 3)

        # Test that an error is thrown if to_arg and from_arg have different
        # size.
        with self.assertRaises(nest.kernel.NESTError):
            d = nest.Distance(l[1:3], l[2:7])

        # position -> gids
        d = nest.Distance([[0.0, 0.0], ], l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        from numpy import array

        # position -> gids
        d = nest.Distance(array([0.0, 0.0]), l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

        # positions -> gids
        d = nest.Distance([array([0.0, 0.0])] * len(l), l)
        self.assertEqual(len(d), len(l))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))
        self.assertTrue(all([dd >= 0. for dd in d]))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_FindElements(self):
        """Interface and result check for finding nearest element.
           This function is Py only, so we also need to check results."""
        # nodes at [-1,0,1]x[-1,0,1], column-wise
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(3, 3, extent=(3., 3.)))

        # single location at center
        n = nest.FindNearestElement(l, (0., 0.))
        self.assertEqual(n, (5,))

        # two locations, one layer
        n = nest.FindNearestElement(l, ((0., 0.), (1., 1.)))
        self.assertEqual(n, (5, 7))

        # several closest locations, not all
        n = nest.FindNearestElement(l, (0.5, 0.5))
        self.assertEqual(len(n), 1)
        self.assertEqual(1, sum(n[0] == k for k in (4, 5, 7, 8)))

        # several closest locations, all
        n = nest.FindNearestElement(l, (0.5, 0.5), find_all=True)
        self.assertEqual(len(n), 1)
        self.assertEqual(n, ((4, 5, 7, 8),))

        # complex case
        n = nest.FindNearestElement(l, ((0., 0.), (0.5, 0.5)),
                                    find_all=True)
        self.assertEqual(n, ((5,), (4, 5, 7, 8)))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_GetCenterElement(self):
        """Interface and result check for finding center element.
           This function is Py only, so we also need to check results."""
        # nodes at [-1,0,1]x[-1,0,1], column-wise
        nest.ResetKernel()
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(3, 3, extent=(2., 2.)))

        # single layer
        n = nest.FindCenterElement(l)
        self.assertEqual(n, 5)

        # new layer
        l2 = nest.Create('iaf_psc_alpha',
                         positions=nest.spatial.grid(3, 3, extent=(2., 2.)))
        n = nest.FindCenterElement(l2)
        self.assertEqual(n, 14)

    def test_GetTargetNodes(self):
        """Interface check for finding targets."""

        cdict = {'connection_type': 'divergent',
                 'synapse_model': 'stdp_synapse',
                 'mask': {'grid': {'rows': 2, 'columns': 2}}}
        nest.ResetKernel()
        nest.SetKernelStatus({'sort_connections_by_source': False})

        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(3, 3,
                                                    extent=(2., 2.),
                                                    edge_wrap=True))

        # connect l -> l
        nest.ConnectLayers(l, l, cdict)

        t = nest.GetTargetNodes(l[:1], l)
        self.assertEqual(len(t), 1)

        t = nest.GetTargetNodes(l, l)
        self.assertEqual(len(t), len(l))
        # 2x2 mask -> four targets
        self.assertTrue(all([len(g) == 4 for g in t]))

        t = nest.GetTargetNodes(l, l, syn_model='static_synapse')
        self.assertEqual(len(t), len(l))
        self.assertTrue(all([len(g) == 0 for g in t]))  # no static syns

        t = nest.GetTargetNodes(l, l, syn_model='stdp_synapse')
        self.assertEqual(len(t), len(l))
        self.assertTrue(
            all([len(g) == 4 for g in t]))  # 2x2 mask  -> four targets

        t = nest.GetTargetNodes(l[:1], l)
        self.assertEqual(t, ([1, 2, 4, 5],))

        t = nest.GetTargetNodes(l[4:5], l)
        self.assertEqual(t, ([5, 6, 8, 9],))

        t = nest.GetTargetNodes(l[8:9], l)
        self.assertEqual(t, ([1, 3, 7, 9],))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_GetTargetPositions(self):
        """Test that GetTargetPosition works as expected"""

        cdict = {'connection_type': 'divergent',
                 'synapse_model': 'stdp_synapse'}

        nest.SetKernelStatus({'sort_connections_by_source': False})

        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(1, 1,
                                                    extent=(1., 1.),
                                                    edge_wrap=False))
        nest.ConnectLayers(l, l, cdict)

        # Simple test with one gid in the layer, should be placed in the origin
        p = nest.GetTargetPositions(l, l)
        self.assertTrue(p, [[(0.0, 0.0)]])

        # Test positions on a grid, we can calculate what they should be
        nest.ResetKernel()
        nest.SetKernelStatus({'sort_connections_by_source': False})

        x_extent = 1.
        y_extent = 1.
        no_rows = 3
        no_cols = 3

        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.grid(
                            no_rows, no_cols,
                            extent=[x_extent, y_extent],
                            edge_wrap=False))
        nest.ConnectLayers(l, l, cdict)

        p = nest.GetTargetPositions(l[:1], l)
        self.assertEqual(len(p), 1)
        self.assertTrue(all([len(pp) == 2 for pp in p[0]]))

        p = nest.GetTargetPositions(l, l)
        self.assertEqual(len(p), len(l))

        dx = x_extent / no_cols
        dy = y_extent / no_rows

        x = [-dx, -dx, -dx, 0.0, 0.0, 0.0, dx, dx, dx]
        y = [dy, 0.0, -dy, dy, 0.0, -dy, dy, 0.0, -dy]

        pos = [(x[i], y[i]) for i in range(len(x))]

        for indx in range(len(pos)):
            # 4 chosen randomly, they should all be the same, as all GIDs in
            # the layer are connected
            self.assertAlmostEqual(p[4][indx][0], pos[indx][0])
            self.assertAlmostEqual(p[4][indx][1], pos[indx][1])

        # Test that we get correct positions when we send in a positions array
        # when creating the layer
        nest.ResetKernel()
        nest.SetKernelStatus({'sort_connections_by_source': False})

        positions = [(numpy.random.uniform(-0.5, 0.5),
                      numpy.random.uniform(-0.5, 0.5)) for _ in range(50)]
        l = nest.Create('iaf_psc_alpha',
                        positions=nest.spatial.free(positions, edge_wrap=False))
        nest.ConnectLayers(l, l, cdict)

        p = nest.GetTargetPositions(l[:1], l)

        for indx in range(len(p[0])):
            self.assertAlmostEqual(positions[indx][0], p[0][indx][0])
            self.assertAlmostEqual(positions[indx][1], p[0][indx][1])

    def test_Parameter(self):

        x = nest.CreateTopologyParameter("constant", {"value": 2.0})
        y = nest.CreateTopologyParameter("constant", {"value": 3.0})

        z1 = x + y
        z2 = z1 * x
        z3 = z2 - x
        z4 = z3 / x
        z5 = z4 - y

        self.assertEqual(int(z1.GetValue((0.0, 0.0))), 5)
        self.assertEqual(int(z2.GetValue((1.0, 0.0))), 10)
        self.assertEqual(int(z3.GetValue((0.0, 1.0))), 8)
        self.assertEqual(int(z4.GetValue((1.0, 1.0))), 4)
        self.assertEqual(int(z5.GetValue((0.0, 0.0))), 1)


def suite():
    suite = unittest.makeSuite(BasicsTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
