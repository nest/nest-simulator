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
import nest
import numpy as np

nest.set_verbosity('M_WARNING')


class TestConnectArrays(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_connect_arrays(self):
        """Connecting Numpy arrays of node IDs"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = np.arange(1, n+1, dtype=np.uint64)
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        nest.Connect(sources, targets, 'one_to_one', {'weight': weights, 'delay': delays,
                                                      'synapse_model': syn_model})

        conns = nest.GetConnections()
        for s, t, w, d, c in zip(sources, targets, weights, delays, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.weight, w)
            self.assertEqual(c.delay, d)

    def test_connect_arrays_wrong_dtype(self):
        """Raises exception when connecting Numpy arrays with wrong dtype"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.double)
        targets = np.arange(1, n+1, dtype=np.double)
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        with self.assertRaises(TypeError):
            nest.Connect(sources, targets, 'one_to_one', {'weight': weights, 'delay': delays,
                                                          'synapse_model': syn_model})

    def test_connect_arrays_wrong_arraytype(self):
        """Raises exception when connecting arrays with wrong array type"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = list(range(1, n+1))
        targets = np.arange(1, n+1, dtype=np.double)
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        with self.assertRaises(TypeError):
            nest.Connect(sources, targets, 'one_to_one', {'weight': weights, 'delay': delays,
                                                          'synapse_model': syn_model})

    def test_connect_arrays_unknown_nodes(self):
        """Raises exception when connecting Numpy arrays with unknown nodes"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+2, dtype=np.uint64)
        targets = np.arange(1, n+2, dtype=np.uint64)
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(sources, targets, 'one_to_one', {'weight': weights, 'delay': delays,
                                                          'synapse_model': syn_model})


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestConnectArrays)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
