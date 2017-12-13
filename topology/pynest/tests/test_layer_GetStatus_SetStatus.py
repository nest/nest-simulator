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

try:
    import pandas
    import pandas.util.testing as pt
    HAVE_PANDAS = True
except ImportError:
    HAVE_PANDAS = False


class GetSetTestCase(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()
        ldict = {'elements': 'iaf_psc_alpha', 'rows': 3, 'columns': 3,
                 'extent': [2., 2.], 'edge_wrap': True}
        self.layer = topo.CreateLayer(ldict)

    def test_LayerGetStatus(self):
        """Test GetStatus on layer GIDCollection."""

        center = nest.GetStatus(self.layer)[0]['center']
        columns = nest.GetStatus(self.layer)[0]['columns']
        edge_wrap = nest.GetStatus(self.layer)[0]['edge_wrap']
        extent = nest.GetStatus(self.layer)[0]['extent']
        network_size = nest.GetStatus(self.layer)[0]['network_size']
        nodes = nest.GetStatus(self.layer)[0]['nodes']
        rows = nest.GetStatus(self.layer)[0]['rows']

        self.assertEqual(center, (0., 0.))
        self.assertEqual(columns, 3)
        self.assertTrue(edge_wrap)
        self.assertEqual(extent, (2., 2.))
        self.assertEqual(network_size, 9)
        self.assertEqual(rows, 3)
        self.assertTrue(isinstance(nodes, nest.GIDCollection))

        n = [gid for gid in nodes]
        self.assertEqual(n, [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(len(nest.GetStatus(nodes)), len(self.layer))
        self.assertEqual(nest.GetStatus(nodes, 'V_m'),
                         (-70., -70., -70., -70., -70.,
                          -70., -70., -70., -70.))

        self.assertEqual(len(nest.GetStatus(self.layer)), 1)

        self.assertTrue('V_m' not in nest.GetStatus(self.layer)[0])

    def test_LayerSetStatus(self):
        """Test SetStatus on layer GIDCollection."""

        with self.assertRaises(nest.NESTError):
            nest.SetStatus(self.layer, {'center': [1., 1.]})

        nodes = nest.GetStatus(self.layer)[0]['nodes']
        nest.SetStatus(nodes, 'V_m', -50.)

        nodes2 = nest.GetStatus(self.layer)[0]['nodes']
        self.assertEqual(nest.GetStatus(nodes2, 'V_m'),
                         (-50., -50., -50., -50., -50.,
                          -50., -50., -50., -50.))

    def test_LayerGet(self):
        """Test get function on layer GIDCollection."""

        center = self.layer.get('center')
        columns = self.layer.get('columns')
        edge_wrap = self.layer.get('edge_wrap')
        extent = self.layer.get('extent')
        network_size = self.layer.get('network_size')
        nodes = self.layer.get('nodes')
        rows = self.layer.get('rows')
        columns_rows = self.layer.get(['columns', 'rows'])

        self.assertEqual(center, (0.0, 0.0))
        self.assertEqual(columns, 3)
        self.assertTrue(edge_wrap)
        self.assertEqual(extent, (2., 2.))
        self.assertEqual(network_size, 9)
        self.assertEqual(rows, 3)
        self.assertTrue(isinstance(nodes, nest.GIDCollection))
        self.assertEqual(columns_rows, {'columns': 3, 'rows': 3})

        n = [gid for gid in nodes]
        self.assertEqual(n, [1, 2, 3, 4, 5, 6, 7, 8, 9])
        self.assertEqual(nodes.get('V_m'),
                         (-70., -70., -70., -70., -70.,
                          -70., -70., -70., -70.))

        with self.assertRaises(nest.NESTError):
            self.layer.get('V_m')

        # Test get all values
        all_values = self.layer.get()
        self.assertEqual(len(all_values.keys()), 7)
        self.assertEqual(all_values['center'], (0.0, 0.0))
        self.assertEqual(all_values['columns'], 3)
        self.assertTrue(all_values['edge_wrap'])
        self.assertEqual(all_values['extent'], (2., 2.))
        self.assertEqual(all_values['network_size'], 9)
        self.assertEqual(all_values['rows'], 3)

    @unittest.skipIf(not HAVE_PANDAS, 'Pandas package is not available')
    def test_LayerGet_pandas(self):
        """Test get function on layer GIDCollection with Pandas output."""
        # Literal argument
        value = self.layer.get('center', pandas_output=True)
        pt.assert_frame_equal(value, pandas.DataFrame({'center': [(0.0, 0.0)]},
                                                      columns=['layer']))

        # Array argument
        value = self.layer.get(['center', 'extent'], pandas_output=True)
        pt.assert_frame_equal(value, pandas.DataFrame({'center': [(0.0, 0.0)],
                                                       'extent': [(1.0, 1.0)]},
                                                      columns=['layer']))

        # Get all values
        all_values = self.layer.get(pandas_output=True)
        self.assertEqual(all_values.shape, (7, 1))
        self.assertEqual(all_values['layer']['center'], (0.0, 0.0))
        self.assertEqual(all_values['layer']['columns'], 3)
        self.assertTrue(all_values['layer']['edge_wrap'])
        self.assertEqual(all_values['layer']['extent'], (2., 2.))
        self.assertEqual(all_values['layer']['network_size'], 9)
        self.assertEqual(all_values['layer']['rows'], 3)

    def test_LayerSet(self):
        """Test set function on layer GIDCollection."""

        with self.assertRaises(nest.NESTError):
            self.layer.set({'center': [1., 1.]})

        nodes = self.layer.get('nodes')
        nodes.set('V_m', -50.)

        nodes2 = self.layer.get('nodes')
        self.assertEqual(nodes2.get('V_m'),
                         (-50., -50., -50., -50., -50.,
                          -50., -50., -50., -50.))


def suite():
    suite = unittest.makeSuite(GetSetTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
