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


class GetSetTestCase(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_LayerSetStatus(self):
        """Test SetStatus on layer GIDCollection."""

        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.grid(
                                shape=[3, 3],
                                extent=[2., 2.],
                                edge_wrap=True))

        with self.assertRaises(KeyError):
            nest.SetStatus(layer, {'center': [1., 1.]})

        nest.SetStatus(layer, 'V_m', -50.)

        self.assertEqual(nest.GetStatus(layer, 'V_m'),
                         (-50., -50., -50., -50., -50.,
                          -50., -50., -50., -50.))

    def test_LayerSpatial(self):
        """Test spatial parameter on layer GIDCollection."""

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            shape=[3, 3], extent=[2., 2.], edge_wrap=True))

        center = layer.spatial['center']
        shape_x = layer.spatial['shape'][0]
        edge_wrap = layer.spatial['edge_wrap']
        extent = layer.spatial['extent']
        network_size = layer.spatial['network_size']
        shape_y = layer.spatial['shape'][1]

        self.assertEqual(center, (0.0, 0.0))
        self.assertEqual(shape_x, 3)
        self.assertTrue(edge_wrap)
        self.assertEqual(extent, (2., 2.))
        self.assertEqual(network_size, 9)
        self.assertEqual(shape_y, 3)

        self.assertEqual(layer.get('V_m'),
                         (-70., -70., -70., -70., -70.,
                          -70., -70., -70., -70.))

        # Test get all values
        all_values = layer.spatial
        self.assertEqual(len(all_values.keys()), 5)
        self.assertEqual(all_values['center'], (0.0, 0.0))
        self.assertEqual(all_values['shape'][0], 3)
        self.assertTrue(all_values['edge_wrap'])
        self.assertEqual(all_values['extent'], (2., 2.))
        self.assertEqual(all_values['network_size'], 9)
        self.assertEqual(all_values['shape'][1], 3)

    def test_SingleElementLayerSpatial(self):
        """Test spatial parameter on single element layer."""

        layer = nest.Create(
            'iaf_psc_alpha', positions=nest.spatial.grid(shape=[1, 1]))

        self.assertEqual(len(layer), 1)
        center = layer.spatial['center']
        columns = layer.spatial['shape'][0]
        all_values = layer.spatial

        self.assertEqual(center, (0., 0.))
        self.assertEqual(columns, 1)
        self.assertEqual(all_values['center'], (0.0, 0.0))

    def test_LayerGet(self):
        """Test get function on layer GIDCollection"""

        layer = nest.Create(
            'iaf_psc_alpha', positions=nest.spatial.grid(shape=[2, 2]))

        self.assertEqual(layer.get('V_m'), (-70., -70., -70., -70.))

    def test_LayerSet(self):
        """Test set function on layer GIDCollection."""

        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid(
            shape=[3, 3], extent=[2., 2.], edge_wrap=True))

        with self.assertRaises(KeyError):
            layer.set({'center': [1., 1.]})

        layer.set(V_m=-50.)

        self.assertEqual(layer.get('V_m'),
                         (-50., -50., -50., -50., -50.,
                          -50., -50., -50., -50.))


def suite():
    suite = unittest.makeSuite(GetSetTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
