# -*- coding: utf-8 -*-
#
# test_connect_layers.py
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
Tests of Connect with layers.
"""

import unittest
import nest
import numpy as np


class ConnectLayersTestCase(unittest.TestCase):
    def setUp(self):
        dim = [4, 5]
        extent = [10., 10.]
        nest.ResetKernel()
        nest.SetKernelStatus({'grng_seed': 123, 'rng_seeds': [456]})
        self.layer = nest.Create(
            'iaf_psc_alpha', positions=nest.spatial.grid(dim, extent=extent))

    def _check_connections(self, conn_spec, expected_num_connections):
        nest.Connect(self.layer, self.layer, conn_spec)
        conns = nest.GetConnections()
        self.assertEqual(len(conns), expected_num_connections)

    def _assert_connect_layers_autapses(self, autapses, expected_num_autapses):
        conn_spec = {
            'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'allow_autapses': autapses,
        }
        nest.Connect(self.layer, self.layer, conn_spec)
        conns = nest.GetConnections()
        n_autapses = 0
        for s, t in zip(conns.source(), conns.target()):
            if s == t:
                n_autapses += 1
        self.assertEqual(n_autapses, expected_num_autapses)

    def _assert_connect_layers_multapses(self, multapses):
        conn_spec = {
            'rule': 'fixed_indegree',
            'indegree': 10,
            'p': 1.0,
            'allow_autapses': False,
            'allow_multapses': multapses,
        }
        nest.Connect(self.layer, self.layer, conn_spec)
        conns = nest.GetConnections()
        conn_pairs = np.array([list(conns.source()), list(conns.target())]).T
        num_nonunique_conns = len(conn_pairs) - len(np.unique(conn_pairs, axis=0))
        if multapses:
            self.assertGreater(num_nonunique_conns, 0)
        else:
            self.assertEqual(num_nonunique_conns, 0)

    def test_connect_layers_indegree(self):
        """Connecting layers with fixed_indegree."""
        conn_spec = {'rule': 'fixed_indegree', 'indegree': 2, 'p': 1.}
        self._check_connections(conn_spec, 40)

    def test_connect_layers_outdegree(self):
        """Connecting layers with fixed_outdegree."""
        conn_spec = {'rule': 'fixed_outdegree', 'outdegree': 2, 'p': 1.}
        self._check_connections(conn_spec, 40)

    def test_connect_layers_bernoulli(self):
        """Connecting layers with pairwise_bernoulli."""
        conn_spec = {'rule': 'pairwise_bernoulli', 'p': 1.0, 'use_on_source': False}
        self._check_connections(conn_spec, 400)

    def test_connect_layers_bernoulli_source(self):
        """Connecting layers with pairwise_bernoulli."""
        conn_spec = {'rule': 'pairwise_bernoulli', 'p': 1.0, 'use_on_source': True}
        self._check_connections(conn_spec, 400)

    def test_connect_layers_indegree_mask(self):
        """Connecting layers with fixed_indegree and mask."""
        conn_spec = {
            'rule': 'fixed_indegree',
            'indegree': 1,
            'mask': {
                'rectangular': {
                    'lower_left': [-5., -5.],
                    'upper_right': [0., 0.]
                }
            },
        }
        self._check_connections(conn_spec, 20)

    def test_connect_layers_indegree_kernel(self):
        """Connecting layers with fixed_indegree and kernel."""
        conn_spec = {'rule': 'fixed_indegree', 'indegree': 1, 'p': 0.5}
        self._check_connections(conn_spec, 20)

    def test_connect_layers_indegree_kernel_mask(self):
        """Connecting layers with fixed_indegree, kernel and mask."""
        conn_spec = {
            'rule': 'fixed_indegree',
            'indegree': 1,
            'mask': {
                'rectangular': {
                    'lower_left': [-5., -5.],
                    'upper_right': [0., 0.]
                }
            },
            'p': 0.5
        }
        self._check_connections(conn_spec, 20)

    def test_connect_layers_outdegree_mask(self):
        """Connecting layers with fixed_outdegree and mask"""
        conn_spec = {
            'rule': 'fixed_outdegree',
            'outdegree': 1,
            'mask': {
                'rectangular': {
                    'lower_left': [-5., -5.],
                    'upper_right': [0., 0.]
                }
            }
        }
        self._check_connections(conn_spec, 20)

    def test_connect_layers_outdegree_kernel(self):
        """Connecting layers with fixed_outdegree and kernel"""
        conn_spec = {'rule': 'fixed_outdegree', 'outdegree': 1, 'p': 0.5}
        self._check_connections(conn_spec, 20)

    def test_connect_layers_outdegree_kernel_mask(self):
        """Connecting layers with fixed_outdegree, kernel and mask"""
        conn_spec = {
            'rule': 'fixed_outdegree',
            'outdegree': 1,
            'mask': {
                'rectangular': {
                    'lower_left': [-5., -5.],
                    'upper_right': [0., 0.]
                }
            },
            'p': 0.5
        }
        self._check_connections(conn_spec, 20)

    def test_connect_layers_bernoulli_mask(self):
        """Connecting layers with pairwise_bernoulli and mask"""
        conn_spec = {
            'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {
                'rectangular': {
                    'lower_left': [-5., -5.],
                    'upper_right': [0., 0.]
                }
            }
        }
        self._check_connections(conn_spec, 108)

    def test_connect_layers_bernoulli_kernel_mask(self):
        """Connecting layers with pairwise_bernoulli, kernel and mask"""
        conn_spec = {
            'rule': 'pairwise_bernoulli',
            'p': 0.5,
            'mask': {
                'rectangular': {
                    'lower_left': [-5., -5.],
                    'upper_right': [0., 0.]
                }
            }
        }
        self._check_connections(conn_spec, 52)

    def test_connect_layers_bernoulli_kernel_mask_source(self):
        """
        Connecting layers with pairwise_bernoulli, kernel and mask on source
        """
        conn_spec = {
            'rule': 'pairwise_bernoulli',
            'p': 0.5,
            'mask': {
                'rectangular': {
                    'lower_left': [-5., -5.],
                    'upper_right': [0., 0.]
                }
            },
            'use_on_source': True
        }
        self._check_connections(conn_spec, 52)

    def test_connect_nonlayers_mask(self):
        """Throw when connecting non-layer GIDCollections with mask."""
        neurons = nest.Create('iaf_psc_alpha', 20)
        conn_spec = {
            'rule': 'pairwise_bernoulli',
            'p': 1.0,
            'mask': {
                'rectangular': {
                    'lower_left': [-5., -5.],
                    'upper_right': [0., 0.]
                }
            }
        }
        with self.assertRaises(TypeError):
            nest.Connect(neurons, neurons, conn_spec)

    def test_connect_nonlayers_kernel(self):
        """Throw when connecting non-layer GIDCollections with kernel."""
        neurons = nest.Create('iaf_psc_alpha', 20)
        conn_spec = {
            'rule': 'fixed_outdegree',
            'outdegree': 1,
            'p': 1.0,
        }
        with self.assertRaises(TypeError):
            nest.Connect(neurons, neurons, conn_spec)

    def test_connect_kernel_mask_wrong_rule(self):
        """Throw when connecting with mask or kernel and wrong rule."""
        conn_spec_kernel = {'rule': 'all_to_all', 'p': 0.5}
        conn_spec_mask = {
            'rule': 'all_to_all',
            'mask': {
                'rectangular': {
                    'lower_left': [-5., -5.],
                    'upper_right': [0., 0.]
                }
            }
        }
        for conn_spec in [conn_spec_kernel, conn_spec_mask]:
            with self.assertRaises(nest.kernel.NESTError):
                nest.Connect(self.layer, self.layer, conn_spec)

    def test_connect_oversized_mask(self):
        """Connecting with specified oversized mask possible."""
        free_layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(
            [[0., 0.]], edge_wrap=True, extent=[1., 1.]))
        conn_spec = {'rule': 'pairwise_bernoulli', 'p': 1.0, 'mask': {'circular': {'radius': 2.}}}
        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(free_layer, free_layer, conn_spec)
        self.assertEqual(nest.GetKernelStatus('num_connections'), 0)
        conn_spec['allow_oversized_mask'] = True
        nest.Connect(free_layer, free_layer, conn_spec)
        self.assertEqual(nest.GetKernelStatus('num_connections'), 1)

    def test_connect_layers_weights(self):
        """Connecting layers with specified weights"""
        conn_spec = {
            'rule': 'pairwise_bernoulli',
            'p': 1.0,
        }
        syn_spec = {'weight': nest.random.uniform(min=0.5)}
        nest.Connect(self.layer, self.layer, conn_spec, syn_spec)
        conns = nest.GetConnections()
        conn_weights = np.array(conns.get('weight'))
        self.assertTrue(len(np.unique(conn_weights)) > 1)
        self.assertTrue((conn_weights >= 0.5).all())
        self.assertTrue((conn_weights <= 1.0).all())

    def test_connect_layers_delays(self):
        """Connecting layers with specified delays"""
        conn_spec = {
            'rule': 'pairwise_bernoulli',
            'p': 1.0,
        }
        syn_spec = {'delay': nest.random.uniform(min=0.5)}
        nest.Connect(self.layer, self.layer, conn_spec, syn_spec)
        conns = nest.GetConnections()
        conn_delays = np.array(conns.get('delay'))
        self.assertTrue(len(np.unique(conn_delays)) > 1)
        self.assertTrue((conn_delays >= 0.5).all())
        self.assertTrue((conn_delays <= 1.0).all())

    def test_connect_layers_autapses_possible(self):
        """Connecting layers with autapses possible"""
        self._assert_connect_layers_autapses(True, 20)

    def test_connect_layers_autapses_impossible(self):
        """Connecting layers with autapses impossible"""
        self._assert_connect_layers_autapses(False, 0)

    def test_connect_layers_multapses_possible(self):
        """Connecting layers with multapses possible"""
        self._assert_connect_layers_multapses(True)

    def test_connect_layers_multapses_impossible(self):
        """Connecting layers with multapses impossible"""
        self._assert_connect_layers_multapses(False)


def suite():
    suite = unittest.makeSuite(ConnectLayersTestCase, 'test')
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
