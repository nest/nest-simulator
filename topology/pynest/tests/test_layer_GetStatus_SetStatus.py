# -*- coding: utf-8 -*-
#
# test_layer_GetStatus_SetStatus.py
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
Tests for GetStatus, SetStatus, get and set calls for layer GIDCollections.
"""

import unittest
import nest
import nest.topology as topo


class GetSetTestCase(unittest.TestCase):
    def test_LayerGetStatus(self):
        """Test GetStatus on layer GIDCollection."""
        nest.ResetKernel()

        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        l = topo.CreateLayer(ldict)

        center = nest.GetStatus(l)[0]['center']
        columns = nest.GetStatus(l)[0]['columns']
        edge_wrap = nest.GetStatus(l)[0]['edge_wrap']
        extent = nest.GetStatus(l)[0]['extent']
        network_size = nest.GetStatus(l)[0]['network_size']
        nodes = nest.GetStatus(l)[0]['nodes']
        rows = nest.GetStatus(l)[0]['rows']

        self.assertEqual(center, (0., 0.))
        self.assertEqual(columns, 3)
        self.assertTrue(edge_wrap)
        self.assertEqual(extent, (2., 2.))
        self.assertEqual(network_size, 9)
        self.assertEqual(rows, 3)
        self.assertTrue(isinstance(nodes, nest.GIDCollection))

        n = [gid for gid in nodes]
        self.assertEqual(n, [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(len(nest.GetStatus(nodes)), len(l))
        self.assertEqual(nest.GetStatus(nodes, 'V_m'),
                         (-70., -70., -70., -70., -70.,
                          -70., -70., -70., -70.))

        self.assertEqual(len(nest.GetStatus(l)), 1)

        self.assertTrue('V_m' not in nest.GetStatus(l)[0])

    def test_LayerSetStatus(self):
        """Test SetStatus on layer GIDCollection."""
        nest.ResetKernel()

        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        l = topo.CreateLayer(ldict)

        with self.assertRaises(nest.NESTError):
            nest.SetStatus(l, {'center': [1., 1.]})

        nodes = nest.GetStatus(l)[0]['nodes']
        nest.SetStatus(nodes, 'V_m', -50.)

        nodes2 = nest.GetStatus(l)[0]['nodes']
        self.assertEqual(nest.GetStatus(nodes2, 'V_m'),
                         (-50., -50., -50., -50., -50.,
                          -50., -50., -50., -50.))

    def test_LayerGet(self):
        """Test get function on layer GIDCollection."""
        nest.ResetKernel()

        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        l = topo.CreateLayer(ldict)

        center = l.get('center')
        columns = l.get('columns')
        edge_wrap = l.get('edge_wrap')
        extent = l.get('extent')
        network_size = l.get('network_size')
        nodes = l.get('nodes')
        rows = l.get('rows')

        self.assertEqual(center, (0.0, 0.0))
        self.assertEqual(columns, 3)
        self.assertTrue(edge_wrap)
        self.assertEqual(extent, (2., 2.))
        self.assertEqual(network_size, 9)
        self.assertEqual(rows, 3)
        self.assertTrue(isinstance(nodes, nest.GIDCollection))

        nodes = nodes

        n = [gid for gid in nodes]
        self.assertEqual(n, [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(nodes.get('V_m'),
                         (-70., -70., -70., -70., -70.,
                          -70., -70., -70., -70.))

        with self.assertRaises(nest.NESTError):
            l.get('V_m')

    def test_LayerSet(self):
        """Test set function on layer GIDCollection."""
        nest.ResetKernel()

        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        l = topo.CreateLayer(ldict)

        with self.assertRaises(nest.NESTError):
            l.set({'center': [1., 1.]})

        nodes = l.get('nodes')
        nodes.set('V_m', -50.)

        nodes2 = l.get('nodes')
        self.assertEqual(nodes2.get('V_m'),
                         (-50., -50., -50., -50., -50.,
                          -50., -50., -50., -50.))


def suite():
    suite = unittest.makeSuite(GetSetTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
