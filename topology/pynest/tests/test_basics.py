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
import nest.topology as topo

try:
    import numpy

    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False


class BasicsTestCase(unittest.TestCase):
    def test_CreateLayer(self):
        """Creating a single layer from dict."""
        nr = 4
        nc = 5
        nest.ResetKernel()
        l = topo.CreateLayer({'elements': 'iaf_psc_alpha',
                              'rows': nr,
                              'columns': nc})
        self.assertEqual(len(l), 1)
        self.assertEqual(len(nest.hl_api.GetLeaves(l)[0]), nr * nc)

    def test_CreateLayerN(self):
        """Creating multiple layers from tuple of dicts."""
        nr = 4
        nc = 5
        ldict = {'elements': 'iaf_psc_alpha',
                 'rows': nr,
                 'columns': nc}
        nlayers = 3

        nest.ResetKernel()
        l = topo.CreateLayer((ldict,) * nlayers)
        self.assertEqual(len(l), nlayers)
        self.assertEqual([len(lvs) for lvs in nest.hl_api.GetLeaves(l)],
                         [nr * nc] * nlayers)

    def test_GetLayer(self):
        """Check if GetLayer returns correct information."""
        nr = 4
        nc = 5
        ldict = {'elements': 'iaf_psc_alpha',
                 'rows': nr,
                 'columns': nc}
        nlayers = 3
        nest.ResetKernel()
        l = topo.CreateLayer((ldict,) * nlayers)

        # obtain list containing list of results from GetLayer for all
        # nodes in layers
        layers_exp = (topo.GetLayer(node) for node in nest.hl_api.GetLeaves(l))

        # the list comprehension builds a list of lists of layer gids,
        # each list containing nr*nc copies of the layer gid
        layers_ref = zip(*((l,) * (nr * nc)))

        for le, lr in zip(layers_exp, layers_ref):
            self.assertEqual(le, lr)

    def test_GetPosition(self):
        """Check if GetPosition returns proper positions."""
        pos = ((1.0, 0.0), (0.0, 1.0), (3.5, 1.5))
        ldict = {'elements': 'iaf_psc_alpha',
                 'extent': (20., 20.),
                 'positions': pos}
        nlayers = 2
        nest.ResetKernel()
        l = topo.CreateLayer((ldict,) * nlayers)

        nodepos_ref = (pos,) * nlayers
        nodepos_exp = (topo.GetPosition(node) for node in nest.hl_api.GetLeaves(l))

        for npe, npr in zip(nodepos_exp, nodepos_ref):
            self.assertEqual(npe, npr)

    def test_GetElement(self):
        """Check if GetElement returns proper lists."""
        ldict = {'elements': 'iaf_psc_alpha',
                 'rows': 4, 'columns': 5}
        nest.ResetKernel()
        l = topo.CreateLayer((ldict, ldict))
        checkpos = [[0, 0], [1, 1], [4, 3]]

        # single gid, single coord gives 1-elem gid list
        n1 = topo.GetElement(l[:1], checkpos[0])
        self.assertEqual(len(n1), 1)
        self.assertIsInstance(n1[0], int)

        # multiple gid, single coord gives l-elem gid list
        n2 = topo.GetElement(l, checkpos[0])
        self.assertEqual(len(n2), len(l))
        self.assertTrue(all(nest.hl_api.is_sequence_of_gids(n) for n in n2))

        # single gid, multiple coord gives len(checkpos)-elem gid list
        n3 = topo.GetElement(l[:1], checkpos)
        self.assertEqual(len(n3), len(checkpos))
        self.assertTrue(all(nest.hl_api.is_sequence_of_gids(n) for n in n3))
        self.assertTrue(all(len(n) == 1 for n in n3))

        # multiple gid, multiple coord gives l*len(cp)-elem gid list
        n4 = topo.GetElement(l, checkpos)
        self.assertEqual(len(n4), len(l))

        self.assertTrue(all(nest.hl_api.is_iterable(n) for n in n4))
        self.assertTrue(all(len(n) == len(checkpos) for n in n4))
        self.assertTrue(all(nest.hl_api.is_sequence_of_gids(m)
                            for n in n4 for m in n))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_Displacement(self):
        """Interface check on displacement calculations."""
        ldict = {'elements': 'iaf_psc_alpha',
                 'rows': 4, 'columns': 5}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)
        n = nest.hl_api.GetLeaves(l)[0]

        # gids -> gids, all displacements must be zero here
        d = topo.Displacement(n, n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(dd == (0., 0.) for dd in d))

        # single gid -> gids
        d = topo.Displacement(n[:1], n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # gids -> single gid
        d = topo.Displacement(n, n[:1])
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        from numpy import array

        # position -> gids
        d = topo.Displacement(array([0.0, 0.0]), n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(len(dd) == 2 for dd in d))

        # positions -> gids
        d = topo.Displacement([array([0.0, 0.0])] * len(n), n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all(len(dd) == 2 for dd in d))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_Distance(self):
        """Interface check on distance calculations."""
        ldict = {'elements': 'iaf_psc_alpha',
                 'rows': 4, 'columns': 5}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)
        n = nest.hl_api.GetLeaves(l)[0]

        # gids -> gids, all displacements must be zero here
        d = topo.Distance(n, n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([dd == 0. for dd in d]))

        # single gid -> gids
        d = topo.Distance(n[:1], n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))

        # gids -> single gid
        d = topo.Distance(n, n[:1])
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))

        from numpy import array

        # position -> gids
        d = topo.Distance(array([0.0, 0.0]), n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))

        # positions -> gids
        d = topo.Distance([array([0.0, 0.0])] * len(n), n)
        self.assertEqual(len(d), len(n))
        self.assertTrue(all([isinstance(dd, float) for dd in d]))

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_FindElements(self):
        """Interface and result check for finding nearest element.
            This function is Py only, so we also need to check results."""
        # nodes at [-1,0,1]x[-1,0,1], column-wise
        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': (3., 3.)}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)

        # single location at center
        n = topo.FindNearestElement(l, (0., 0.))
        self.assertEqual(n, (6,))

        # single location, two layers
        n = topo.FindNearestElement(l * 2, (0., 0.))
        self.assertEqual(n, (6, 6))

        # two locations, one layer
        n = topo.FindNearestElement(l, ((0., 0.), (1., 1.)))
        self.assertEqual(n, (6, 8))

        # two locations, two layers
        n = topo.FindNearestElement(l * 2, ((0., 0.), (1., 1.)))
        self.assertEqual(n, ((6, 8),) * 2)

        # several closest locations, not all
        n = topo.FindNearestElement(l, (0.5, 0.5))
        self.assertEqual(len(n), 1)
        self.assertEqual(1, sum(n[0] == k for k in (5, 6, 8, 9)))

        # several closest locations, all
        n = topo.FindNearestElement(l, (0.5, 0.5), find_all=True)
        self.assertEqual(len(n), 1)
        self.assertEqual(n, ((5, 6, 8, 9),))

        # complex case
        n = topo.FindNearestElement(l * 2, ((0., 0.), (0.5, 0.5)),
                                    find_all=True)
        self.assertEqual(n, (((6,), (5, 6, 8, 9)),) * 2)

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_GetCenterElement(self):
        """Interface and result check for finding center element.
            This function is Py only, so we also need to check results."""
        # nodes at [-1,0,1]x[-1,0,1], column-wise
        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': (2., 2.)}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)

        # single layer
        n = topo.FindCenterElement(l)
        self.assertEqual(n, (6,))

        # two layers
        n = topo.FindCenterElement(l * 2)
        self.assertEqual(n, (6,) * 2)

    def test_GetTargetNodesPositions(self):
        """Interface check for finding targets."""
        ldict = {'elements': ['iaf_psc_alpha', 'iaf_psc_delta'], 'rows': 3,
                 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        cdict = {'connection_type': 'divergent',
                 'mask': {'grid': {'rows': 2, 'columns': 2}}}
        nest.ResetKernel()
        l = topo.CreateLayer(ldict)
        ian = [gid for gid in nest.hl_api.GetLeaves(l)[0]
               if nest.GetStatus([gid], 'model')[0] == 'iaf_psc_alpha']
        ipa = [gid for gid in nest.hl_api.GetLeaves(l)[0]
               if nest.GetStatus([gid], 'model')[0] == 'iaf_psc_delta']

        # connect ian -> all using static_synapse
        cdict.update({'sources': {'model': 'iaf_psc_alpha'},
                      'synapse_model': 'static_synapse'})
        topo.ConnectLayers(l, l, cdict)
        for k in ['sources', 'synapse_model']:
            cdict.pop(k)

        # connect ipa -> ipa using stdp_synapse
        cdict.update({'sources': {'model': 'iaf_psc_delta'},
                      'targets': {'model': 'iaf_psc_delta'},
                      'synapse_model': 'stdp_synapse'})
        topo.ConnectLayers(l, l, cdict)
        for k in ['sources', 'targets', 'synapse_model']:
            cdict.pop(k)

        t = topo.GetTargetNodes(ian[:1], l)
        self.assertEqual(len(t), 1)

        p = topo.GetTargetPositions(ian[:1], l)
        self.assertEqual(len(p), 1)
        self.assertTrue(all([len(pp) == 2 for pp in p[0]]))

        t = topo.GetTargetNodes(ian, l)
        self.assertEqual(len(t), len(ian))
        # 2x2 mask x 2 neurons / element -> eight targets
        self.assertTrue(all([len(g) == 8 for g in t]))

        p = topo.GetTargetPositions(ian, l)
        self.assertEqual(len(p), len(ian))

        t = topo.GetTargetNodes(ian, l, tgt_model='iaf_psc_alpha')
        self.assertEqual(len(t), len(ian))
        self.assertTrue(
            all([len(g) == 4 for g in t]))  # 2x2 mask  -> four targets

        t = topo.GetTargetNodes(ian, l, tgt_model='iaf_psc_delta')
        self.assertEqual(len(t), len(ian))
        self.assertTrue(
            all([len(g) == 4 for g in t]))  # 2x2 mask  -> four targets

        t = topo.GetTargetNodes(ipa, l)
        self.assertEqual(len(t), len(ipa))
        self.assertTrue(
            all([len(g) == 4 for g in t]))  # 2x2 mask  -> four targets

        t = topo.GetTargetNodes(ipa, l, syn_model='static_synapse')
        self.assertEqual(len(t), len(ipa))
        self.assertTrue(all([len(g) == 0 for g in t]))  # no static syns

        t = topo.GetTargetNodes(ipa, l, syn_model='stdp_synapse')
        self.assertEqual(len(t), len(ipa))
        self.assertTrue(
            all([len(g) == 4 for g in t]))  # 2x2 mask  -> four targets

    def test_Parameter(self):

        x = topo.CreateParameter("constant", {"value": 2.0})
        y = topo.CreateParameter("constant", {"value": 3.0})

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
