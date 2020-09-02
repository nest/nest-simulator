# -*- coding: utf-8 -*-
#
# test_multiple_synapses.py
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
Multiple synapses tests
"""

import unittest
import warnings
import nest


@nest.ll_api.check_stack
class MultipleSynapsesTestCase(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def sort_connections(self, conns):
        """
        Sort connection dictionary based on source, target, weight and synapse model.

        Sorting order is source, then target, then weight and lastly synapse_model.
        Returns a list with tuples of source, target, weight, synapse_model.
        """
        src = conns.source
        trgt = conns.target
        weight = conns.weight
        syn_model = conns.synapse_model

        sorted_conn_list = [(s, t, w, sm) for s, t, w, sm in sorted(zip(src, trgt, weight, syn_model))]

        return sorted_conn_list

    def test_MultipleSynapses(self):
        """Test list of synapses for very simple connection"""
        node = nest.Create('iaf_psc_alpha')

        nest.Connect(node, node, syn_spec=nest.Colocate({'weight': -2.}, {'weight': 3.}))

        self.assertEqual(2, nest.GetKernelStatus('num_connections'))

        conns = nest.GetConnections()
        self.assertEqual([-2, 3], conns.weight)

    def test_MultipleSynapses_one_to_one(self):
        """Test list of synapses when we use one_to_one as connection rule"""
        num_src = 7
        num_trg = 7
        syn_spec = nest.Colocate({'synapse_model': 'stdp_synapse', 'weight': -5.},
                                 {'weight': -1.5},
                                 {'synapse_model': 'stdp_synapse', 'weight': 3})

        src = nest.Create('iaf_psc_alpha', num_src)
        trgt = nest.Create('iaf_psc_alpha', num_trg)

        nest.Connect(src, trgt, 'one_to_one', syn_spec=syn_spec)

        conns = nest.GetConnections()

        self.assertEqual(num_src * len(syn_spec), len(conns))

        # source id's are 1, 2, 3, 4, 5, 6, 7
        ref_src = [1] * len(syn_spec) + [2] * len(syn_spec) + [3] * len(syn_spec) + [4] * len(syn_spec) + \
                  [5] * len(syn_spec) + [6] * len(syn_spec) + [7] * len(syn_spec)
        # target id's are 8, 9, 10, 11, 12, 13, 14
        ref_trgt = [8] * len(syn_spec) + [9] * len(syn_spec) + [10] * len(syn_spec) + [11] * len(syn_spec) + \
                   [12] * len(syn_spec) + [13] * len(syn_spec) + [14] * len(syn_spec)

        ref_weight = [-5., -1.5, 3.] * num_src
        ref_synapse_modules = ['stdp_synapse', 'static_synapse', 'stdp_synapse'] * num_src

        ref_conn_list = [(s, t, w, sm) for s, t, w, sm in zip(ref_src, ref_trgt, ref_weight, ref_synapse_modules)]
        sorted_conn_list = self.sort_connections(conns)

        self.assertEqual(ref_conn_list, sorted_conn_list)

    def test_MultipleSynapses_all_to_all(self):
        """Test list of synapses when we use all_to_all as connection rule"""
        num_src = 3
        num_trg = 5
        syn_spec = nest.Colocate({'weight': -2.},
                                 {'synapse_model': 'stdp_synapse', 'weight': -1.5},
                                 {'weight': 3})

        src = nest.Create('iaf_psc_alpha', num_src)
        trgt = nest.Create('iaf_psc_alpha', num_trg)

        nest.Connect(src, trgt, 'all_to_all', syn_spec=syn_spec)

        conns = nest.GetConnections()

        self.assertEqual(num_src * num_trg * len(syn_spec), len(conns))

        # source id's are 1, 2, 3
        ref_src = [1] * num_trg * len(syn_spec) + [2] * num_trg * len(syn_spec) + [3] * num_trg * len(syn_spec)
        # target id's are 4, 5, 6, 7, 8
        ref_trgt = []
        for t in [4, 5, 6, 7, 8]*num_src:
            # there are 3 elements in the syn_spec list
            ref_trgt.extend([t, t, t])

        ref_weight = [-2., -1.5, 3.] * num_src * num_trg
        ref_synapse_modules = ['static_synapse', 'stdp_synapse', 'static_synapse'] * num_src * num_trg

        ref_conn_list = [(s, t, w, sm) for s, t, w, sm in zip(ref_src, ref_trgt, ref_weight, ref_synapse_modules)]
        sorted_conn_list = self.sort_connections(conns)

        self.assertEqual(ref_conn_list, sorted_conn_list)

    def test_MultipleSynapses_fixed_indegree(self):
        """Test list of synapses when we use fixed_indegree as connection rule"""
        num_src = 7
        num_trg = 3
        indegree = 2
        syn_spec = nest.Colocate({'weight': -2.},
                                 {'synapse_model': 'stdp_synapse', 'weight': -1.5},
                                 {'synapse_model': 'stdp_synapse', 'weight': 3})

        src = nest.Create('iaf_psc_alpha', num_src)
        trgt = nest.Create('iaf_psc_alpha', num_trg)

        nest.Connect(src, trgt, {'rule': 'fixed_indegree', 'indegree': indegree}, syn_spec=syn_spec)

        conns = nest.GetConnections()

        self.assertEqual(num_trg * indegree * len(syn_spec), len(conns))

        ref_trgt = [t for t in trgt.tolist() for _ in range(indegree * len(syn_spec))]
        ref_sm = (['static_synapse'] * num_trg * indegree +
                  ['stdp_synapse'] * num_trg * indegree +
                  ['stdp_synapse'] * num_trg * indegree)

        self.assertEqual(sorted(ref_trgt), sorted(conns.target))
        self.assertEqual(sorted(ref_sm), sorted(conns.synapse_model))

    def test_MultipleSynapses_spatial_network(self):
        """test list of synapses for spatial networks with fixed indegree"""
        num_src = 11
        num_trgt = 37
        indegree = 3

        spatial_nodes_src = nest.Create('iaf_psc_alpha', n=num_src,
                                        positions=nest.spatial.free(nest.random.uniform(), num_dimensions=2))
        spatial_nodes_trgt = nest.Create('iaf_psc_alpha', n=num_trgt,
                                         positions=nest.spatial.free(nest.random.uniform(), num_dimensions=2))

        nest.Connect(spatial_nodes_src, spatial_nodes_trgt, {'rule': 'fixed_indegree', 'indegree': indegree},
                     nest.Colocate({'weight': -3.},
                                   {'weight': nest.spatial_distributions.exponential(nest.spatial.distance),
                                    'delay': 1.4}))

        conns = nest.GetConnections()

        self.assertEqual(num_trgt * indegree * 2, len(conns))

        weights = conns.weight
        self.assertEqual(sorted(weights)[:num_trgt * indegree], [-3] * num_trgt * indegree)

    def test_MultipleSynapses_spatial_network_fixedOutdegree(self):
        """test list of synapses for spatial networks with fixed outdegree"""
        num_src = 17
        num_trgt = 23
        outdegree = 4

        spatial_nodes_src = nest.Create('iaf_psc_alpha', n=num_src,
                                        positions=nest.spatial.free(nest.random.uniform(), num_dimensions=2))
        spatial_nodes_trgt = nest.Create('iaf_psc_alpha', n=num_trgt,
                                         positions=nest.spatial.free(nest.random.uniform(), num_dimensions=2))

        nest.Connect(spatial_nodes_src, spatial_nodes_trgt, {'rule': 'fixed_outdegree', 'outdegree': outdegree},
                     nest.Colocate({'weight': -3., 'synapse_model': 'stdp_synapse'},
                                   {'synapse_model': 'tsodyks_synapse'},
                                   {'weight': nest.spatial_distributions.exponential(nest.spatial.distance),
                                    'delay': 1.4}))

        conns = nest.GetConnections()

        self.assertEqual(num_src * outdegree * 3, len(conns))

        weights = conns.weight
        self.assertEqual(sorted(weights)[:num_src * outdegree], [-3] * num_src * outdegree)

        ref_synapse_model = (['stdp_synapse'] * num_src * outdegree +
                             ['tsodyks_synapse'] * num_src * outdegree +
                             ['static_synapse'] * num_src * outdegree)
        self.assertEqual(sorted(conns.synapse_model), sorted(ref_synapse_model))

    def test_MultipleSynapses_spatial_network_bernoulliSource(self):
        """test list of synapses for 3D spatial networks with pairwice bernoulli on source"""
        num_src = 7
        num_trgt = 19
        p = 0.6

        spatial_nodes_src = nest.Create('iaf_psc_alpha', n=num_src,
                                        positions=nest.spatial.free(nest.random.uniform(), num_dimensions=3))
        spatial_nodes_trgt = nest.Create('iaf_psc_alpha', n=num_trgt,
                                         positions=nest.spatial.free(nest.random.uniform(), num_dimensions=3))

        nest.Connect(spatial_nodes_src, spatial_nodes_trgt, {'rule': 'pairwise_bernoulli', 'p': p},
                     nest.Colocate({'delay': 1.7, 'weight': nest.spatial_distributions.gaussian(nest.spatial.distance)},
                                   {'synapse_model': 'tsodyks_synapse'},
                                   {'weight': -nest.spatial_distributions.gaussian(nest.spatial.distance),
                                    'delay': 1.4}))

        conns = nest.GetConnections()
        num_conns = len(conns)

        self.assertLess(num_src * num_trgt * p * 2, len(conns))

        delays = [round(d, 1) for d in conns.delay]
        ref_delays = [1.] * (num_conns // 3) + [1.4] * (num_conns // 3) + [1.7] * (num_conns // 3)

        self.assertEqual(sorted(delays), ref_delays)

        ref_synapse_model = (['tsodyks_synapse'] * (num_conns // 3) +
                             ['static_synapse'] * (2 * num_conns // 3))
        self.assertEqual(sorted(conns.synapse_model), sorted(ref_synapse_model))

        for w in sorted(conns.weight)[:(num_conns // 3)]:
            self.assertLess(w, 0)

    def test_MultipleSynapses_spatial_network_bernoulliTarget(self):
        """test list of synapses for 3D spatial networks with pairwice bernoulli on target"""
        num_src = 57
        num_trgt = 21
        p = 0.3

        spatial_nodes_src = nest.Create('iaf_psc_alpha', n=num_src,
                                        positions=nest.spatial.free(nest.random.uniform(), num_dimensions=3))
        spatial_nodes_trgt = nest.Create('iaf_psc_alpha', n=num_trgt,
                                         positions=nest.spatial.free(nest.random.uniform(), num_dimensions=3))

        nest.Connect(spatial_nodes_src, spatial_nodes_trgt,
                     {'rule': 'pairwise_bernoulli', 'p': p, 'use_on_source': False},
                     nest.Colocate({'delay': 1.7, 'weight': -1.4},
                                   {'delay': nest.spatial_distributions.gaussian(nest.spatial.distance),
                                    'weight': 1.4}))

        conns = nest.GetConnections()
        num_conns = len(conns)

        self.assertGreater(num_src * num_trgt * p * 3, num_conns)

        delays = [round(d, 1) for d in sorted(conns.delay)[(num_conns // 2):]]
        ref_delays = [1.7] * (num_conns // 2)

        self.assertEqual(sorted(delays), ref_delays)

        ref_weights = [-1.4] * (num_conns // 2) + [1.4] * (num_conns // 2)
        self.assertEqual(sorted(conns.weight), ref_weights)


def suite():
    suite = unittest.makeSuite(MultipleSynapsesTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
