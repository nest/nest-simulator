# -*- coding: utf-8 -*-
#
# test_layerGIDCollection.py
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
General tests for layer GIDCollections.
"""

import unittest
import nest
import nest.topology as topo


class TestLayerGIDCollection(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_addLayerAndGIDCollection(self):
        """Test that concatenation of plain gc and layer is illegal."""
        nodes = nest.Create('iaf_psc_alpha', 10)
        layer = topo.CreateLayer({'rows': 5,
                                  'columns': 5,
                                  'elements': 'iaf_psc_alpha'})

        with self.assertRaises(nest.NESTError):
            c = nodes + layer
        with self.assertRaises(nest.NESTError):
            d = layer + nodes

    def test_addTwoLayers(self):
        "Test that concatenation of two layers is illegal"
        layer1 = topo.CreateLayer({'rows': 5,
                                  'columns': 5,
                                  'elements': 'iaf_psc_alpha'})
        layer2 = topo.CreateLayer({'rows': 5,
                                  'columns': 5,
                                  'elements': 'iaf_psc_alpha'})

        with self.assertRaises(nest.NESTError):
            c = layer1 + layer2


def suite():
    suite = unittest.makeSuite(TestLayerGIDCollection, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
