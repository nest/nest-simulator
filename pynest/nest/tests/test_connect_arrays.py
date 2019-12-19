# -*- coding: utf-8 -*-
#
# test_connect_arrays.py
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

import unittest
import warnings
import nest
import numpy as np

nest.set_verbosity('M_WARNING')


class TestConnectArrays(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_connect_arrays_nonunique_node_ids(self):
        """Connecting arrays with nonunique node IDs"""
        nest.Create('iaf_psc_alpha', 4)
        source = [1, 1, 2, 2]
        target = [3, 3, 4, 4]
        nest.Connect(source, target)
        conns = nest.GetConnections()
        st_pairs = np.array([(s, t) for s in source for t in target])
        self.assertTrue(np.array_equal(st_pairs[:, 0], list(conns.sources())))
        self.assertTrue(np.array_equal(st_pairs[:, 1], list(conns.targets())))

    def test_connect_numpy_arrays_node_ids(self):
        """Connecting numpy arrays with nonunique node IDs"""
        nest.Create('iaf_psc_alpha', 4)
        source = np.array([1, 1, 2, 2])
        target = np.array([3, 3, 4, 4])
        nest.Connect(source, target)
        conns = nest.GetConnections()
        st_pairs = np.array([(s, t) for s in source for t in target])
        self.assertTrue(np.array_equal(st_pairs[:, 0], list(conns.sources())))
        self.assertTrue(np.array_equal(st_pairs[:, 1], list(conns.targets())))

    def test_connect_arrays_unique_node_ids(self):
        """Connecting arrays with unique node IDs"""
        n = nest.Create('iaf_psc_alpha', 4)
        node_ids = n.tolist()
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")
            nest.Connect(node_ids, node_ids)
            self.assertEqual(len(w), 1)
            self.assertTrue(issubclass(w[-1].category, UserWarning))
            self.assertTrue('unique' in str(w[-1].message))
        conns = nest.GetConnections()
        st_pairs = np.array([(s, t) for s in node_ids for t in node_ids])
        self.assertTrue(np.array_equal(st_pairs[:, 0], list(conns.sources())))
        self.assertTrue(np.array_equal(st_pairs[:, 1], list(conns.targets())))

    def test_connect_array_with_nc(self):
        """Connecting one array with a NodeCollection"""
        nc = nest.Create('iaf_psc_alpha', 4)
        node_ids = [1, 1, 2, 2]
        with self.assertRaises(TypeError):
            nest.Connect(node_ids, nc)
        with self.assertRaises(TypeError):
            nest.Connect(nc, node_ids)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestConnectArrays)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
