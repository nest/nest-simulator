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
import numpy as np

import nest

nest.set_verbosity('M_WARNING')

HAVE_OPENMP = nest.ll_api.sli_func("is_threaded")


class TestConnectArrays(unittest.TestCase):

    non_unique = np.array([1, 1, 3, 5, 4, 5, 9, 7, 2, 8], dtype=np.uint64)

    def setUp(self):
        nest.ResetKernel()

    def test_connect_arrays_unique(self):
        """Connecting NumPy arrays of unique node IDs"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = np.arange(1, n+1, dtype=np.uint64)
        weights = 1.5
        delays = 1.4

        nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays})

        conns = nest.GetConnections()

        self.assertEqual(len(conns), n*n)

        for c in conns:
            np.testing.assert_approx_equal(c.weight, weights)
            np.testing.assert_approx_equal(c.delay, delays)

    def test_connect_arrays_nonunique(self):
        """Connecting NumPy arrays with non-unique node IDs"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique
        weights = np.ones(n)
        delays = np.ones(n)

        nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays},
                     conn_spec='one_to_one')

        conns = nest.GetConnections()

        for s, t, w, d, c in zip(sources, targets, weights, delays, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.weight, w)
            self.assertEqual(c.delay, d)

    def test_connect_arrays_nonunique_dict_conn_spec(self):
        """Connecting NumPy arrays with non-unique node IDs and conn_spec as a dict"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique
        weights = 2 * np.ones(n)
        delays = 1.5 * np.ones(n)

        nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays},
                     conn_spec={'rule': 'one_to_one'})

        conns = nest.GetConnections()

        for s, t, w, d, c in zip(sources, targets, weights, delays, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.weight, w)
            self.assertEqual(c.delay, d)

    def test_connect_arrays_no_conn_spec(self):
        """Connecting NumPy arrays of node IDs without specifying conn_spec"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique

        with self.assertRaises(ValueError):
            nest.Connect(sources, targets)

    def test_connect_arrays_different_weights_delays(self):
        """Connecting NumPy arrays with different weights and delays"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique
        weights = np.linspace(0.6, 1.5, n)
        delays = np.linspace(0.4, 1.3, n)

        nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays},
                     conn_spec={'rule': 'one_to_one'})

        conns = nest.GetConnections()

        np.testing.assert_array_equal(conns.source, sources)
        np.testing.assert_array_equal(conns.target, targets)
        np.testing.assert_array_almost_equal(conns.weight, weights)
        np.testing.assert_array_almost_equal(conns.delay, delays)

    def test_connect_arrays_threaded(self):
        """Connecting NumPy arrays, threaded"""
        nest.SetKernelStatus({'local_num_threads': 2})
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'static_synapse'

        nest.Connect(sources, targets, conn_spec='one_to_one',
                     syn_spec={'weight': weights, 'delay': delays, 'synapse_model': syn_model})

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
        targets = self.non_unique
        weights = np.ones(n)

        nest.Connect(sources, targets, conn_spec='one_to_one', syn_spec={'weight': weights})

        conns = nest.GetConnections()

        for s, t, w, c in zip(sources, targets, weights, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.weight, w)

    def test_connect_array_list(self):
        """Connecting NumPy array and list"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = list(range(1, n + 1))
        targets = self.non_unique

        nest.Connect(sources, targets, conn_spec='one_to_one')

        conns = nest.GetConnections()

        for s, t, c in zip(sources, targets, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)

    def test_connect_arrays_no_weights(self):
        """Connecting NumPy arrays without specifying weights"""
        n = 10
        neurons = nest.Create('iaf_psc_alpha', n)
        targets = self.non_unique
        delays = np.ones(n)

        nest.Connect(neurons, targets, conn_spec='one_to_one', syn_spec={'delay': delays})

        conns = nest.GetConnections()

        for s, t, d, c in zip(neurons.tolist(), targets, delays, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.delay, d)

    def test_connect_arrays_rtype(self):
        """Connecting NumPy arrays with specified receptor_type"""
        n = 10
        nest.Create('iaf_psc_exp_multisynapse', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        receptor_type = np.ones(len(sources), dtype=np.uint64)
        syn_model = 'static_synapse'

        nest.Connect(sources, targets, conn_spec='one_to_one',
                     syn_spec={'weight': weights, 'delay': delays, 'receptor_type': receptor_type})

        conns = nest.GetConnections()

        for s, t, w, d, r, c in zip(sources, targets, weights, delays, receptor_type, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.weight, w)
            self.assertEqual(c.delay, d)
            self.assertEqual(c.receptor, r)

    def test_connect_arrays_additional_synspec_params(self):
        """Connecting NumPy arrays with additional syn_spec params"""
        n = 10
        nest.Create('iaf_psc_exp_multisynapse', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique
        weights = np.ones(len(sources))
        delays = np.ones(len(sources))
        syn_model = 'vogels_sprekeler_synapse'
        receptor_type = np.ones(len(sources), dtype=np.uint64)
        alpha = 0.1*np.ones(len(sources))
        tau = 20.*np.ones(len(sources))

        nest.Connect(sources, targets, conn_spec='one_to_one',
                     syn_spec={'weight': weights, 'delay': delays, 'synapse_model': syn_model,
                               'receptor_type': receptor_type, 'alpha': alpha, 'tau': tau})

        conns = nest.GetConnections()

        for s, t, w, d, r, a, tau, c in zip(sources, targets, weights, delays, receptor_type, alpha, tau, conns):
            self.assertEqual(c.source, s)
            self.assertEqual(c.target, t)
            self.assertEqual(c.weight, w)
            self.assertEqual(c.delay, d)
            self.assertEqual(c.receptor, r)
            self.assertEqual(c.alpha, a)
            self.assertEqual(c.tau, tau)

    def test_connect_arrays_float_rtype(self):
        """Raises exception when not using integer value for receptor_type"""
        n = 10
        nest.Create('iaf_psc_exp_multisynapse', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique
        weights = np.ones(n)
        delays = np.ones(n)
        syn_model = 'vogels_sprekeler_synapse'
        receptor_type = 1.5*np.ones(len(sources))

        with self.assertRaises(nest.kernel.NESTErrors.BadParameter):
            nest.Connect(sources, targets, conn_spec='one_to_one',
                         syn_spec={'weight': weights, 'delay': delays, 'synapse_model': syn_model,
                                   'receptor_type': receptor_type})

    def test_connect_arrays_wrong_dtype(self):
        """Raises exception when connecting NumPy arrays with wrong dtype"""
        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.double)
        targets = np.array(self.non_unique, dtype=np.double)
        weights = np.ones(n)
        delays = np.ones(n)
        syn_model = 'static_synapse'

        with self.assertRaises(nest.kernel.NESTErrors.ArgumentType):
            nest.Connect(sources, targets, syn_spec={'weight': weights, 'delay': delays},
                         conn_spec='one_to_one')

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

    @unittest.skipIf(not HAVE_OPENMP, 'NEST was compiled without multi-threading')
    def test_connect_arrays_receptor_type(self):
        """Connecting NumPy arrays with receptor type specified, threaded"""

        nest.SetKernelStatus({'local_num_threads': 2})

        n = 10
        nest.Create('iaf_psc_alpha', n)
        sources = np.arange(1, n+1, dtype=np.uint64)
        targets = self.non_unique

        weights = len(sources) * [2.]
        nest.Connect(sources, targets, conn_spec='one_to_one', syn_spec={'weight': weights, 'receptor_type': 0})

        self.assertEqual(len(sources) * [0], nest.GetConnections().receptor)

    @unittest.skipIf(not HAVE_OPENMP, 'NEST was compiled without multi-threading')
    def test_connect_arrays_differnt_alpha(self):
        """Connecting NumPy arrays with different alpha values in a threaded environment"""

        nest.SetKernelStatus({'local_num_threads': 4})

        neurons = nest.Create("iaf_psc_exp", 10)
        # syn_spec parameters are dependent on source, so we test with source id's not starting with 1
        source = np.array([2, 5, 3, 10, 1, 9, 4, 6, 8, 7])
        target = 1 + np.random.choice(10, 10, replace=True)

        weights = len(source) * [2.]
        alpha = np.array([0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.11])

        # Need to make sure the correct alpha value is used with the correct source
        src_alpha_ref = {key: val for key, val in zip(source, alpha)}

        nest.Connect(source, target, conn_spec='one_to_one',
                     syn_spec={'alpha': alpha, 'receptor_type': 0,
                               'weight': weights, 'synapse_model': "stdp_synapse"})

        conns = nest.GetConnections()
        src = conns.source
        alp = conns.alpha
        src_alpha = {key: val for key, val in zip(src, alp)}

        self.assertEqual(src_alpha_ref, src_alpha)


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(TestConnectArrays)
    return suite


if __name__ == '__main__':
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
