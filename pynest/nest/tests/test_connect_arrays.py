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
        """Connecting NumPy arrays of node IDs"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = np.arange(1, n+1, dtype=np.uint64)
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays,
                                                 'synapse_model': syn_model})

        conns = nest.GetConnections()
        for s, t, w, d, c in zip(sources, targets, weights, delays, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.weight, w)
            self.assertEqual(c.delay, d)

    def test_connect_arrays_threaded(self):
        """Connecting NumPy arrays, threaded"""
        nest.SetKernelStatus({'local_num_threads': 2})
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = np.arange(1, n+1, dtype=np.uint64)
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays,
                                                 'synapse_model': syn_model})

        conns = nest.GetConnections()
        # Sorting connection information by source to make it equivalent to the reference.
        conn_info = [(c.source, c.target, c.weight, c.delay) for c in conns]
        conn_info.sort(key=lambda conn: conn[0])
        for s, t, w, d, c in zip(sources, targets, weights, delays, conn_info):
            conn_s, conn_t, conn_w, conn_d = c
            self.assertEqual(conn_s, s)
            self.assertEqual(conn_t, t)
            self.assertEqual(conn_w, w)
            self.assertEqual(conn_d, d)

    def test_connect_arrays_no_delays(self):
        """Connecting NumPy arrays without specifying delays"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = np.arange(1, n+1, dtype=np.uint64)
        weights = np.ones(len(sources))
        # delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        nest.Connect(sources, targets, syn_spec={'weight': weights, 'synapse_model': syn_model})

        conns = nest.GetConnections()
        for s, t, w, c in zip(sources, targets, weights, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.weight, w)

    def test_connect_arrays_no_weights(self):
        """Connecting NumPy arrays without specifying weights"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = np.arange(1, n+1, dtype=np.uint64)
        # weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        nest.Connect(sources, targets, syn_spec={'delay': delays, 'synapse_model': syn_model})

        conns = nest.GetConnections()
        for s, t, d, c in zip(sources, targets, delays, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.delay, d)

    def test_connect_arrays_rtype(self):
        """Connecting NumPy arrays with specified receptor_type"""
        n = 10
        nest.Create('iaf_psc_exp_multisynapse', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = np.arange(1, n+1, dtype=np.uint64)
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        receptor_type = np.ones(len(sources), dtype=np.uint64)
        syn_model = 'static_synapse'

        nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays, 'receptor_type': receptor_type,
                                                 'synapse_model': syn_model})

        conns = nest.GetConnections()
        for s, t, w, d, r, c in zip(sources, targets, weights, delays, receptor_type, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.weight, w)
            self.assertEqual(c.delay, d)
            self.assertEqual(c.receptor, r)

    def test_connect_arrays_wrong_dtype(self):
        """Raises exception when connecting NumPy arrays with wrong dtype"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.double)
        targets = np.arange(1, n+1, dtype=np.double)
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        with self.assertRaises(TypeError):
            nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays,
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
            nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays,
                                                     'synapse_model': syn_model})

    def test_connect_arrays_unknown_nodes(self):
        """Raises exception when connecting NumPy arrays with unknown nodes"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+2, dtype=np.uint64)
        targets = np.arange(1, n+2, dtype=np.uint64)
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        with self.assertRaises(nest.kernel.NESTErrors.UnknownNode):
            nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays,
                                                     'synapse_model': syn_model})


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestConnectArrays)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
