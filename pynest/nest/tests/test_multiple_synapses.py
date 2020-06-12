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

        nest.Connect(node, node, syn_spec=[{'weight': -2.}, {'weight': 3.}])

        self.assertEqual(2, nest.GetKernelStatus('num_connections'))

        conns = nest.GetConnections()
        self.assertEqual([-2, 3], conns.weight)

    def test_MultipleSynapses_one_to_one(self):
        """Test list of synapses when we use one_to_one as connection rule"""
        num_src = 7
        num_trg = 7
        syn_spec = [{'synapse_model': 'stdp_synapse', 'weight': -5.},
                    {'weight': -1.5},
                    {'synapse_model': 'stdp_synapse', 'weight': 3}]

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
        syn_spec = [{'weight': -2.},
                    {'synapse_model': 'stdp_synapse', 'weight': -1.5},
                    {'weight': 3}]

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
            ref_trgt.append(t)
            ref_trgt.append(t)
            ref_trgt.append(t)

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
        syn_spec = [{'weight': -2.},
                    {'synapse_model': 'stdp_synapse', 'weight': -1.5},
                    {'synapse_model': 'stdp_synapse', 'weight': 3}]

        src = nest.Create('iaf_psc_alpha', num_src)
        trgt = nest.Create('iaf_psc_alpha', num_trg)

        nest.Connect(src, trgt, {'rule': 'fixed_indegree', 'indegree': indegree}, syn_spec=syn_spec)

        conns = nest.GetConnections()

        self.assertEqual(num_trg * indegree * len(syn_spec), len(conns))

        ref_trgt = trgt.tolist() + trgt.tolist()
        ref_sm = (['static_synapse'] * num_trg * indegree +
                  ['stdp_synapse'] * num_trg * indegree +
                  ['stdp_synapse'] * num_trg * indegree)

        self.assertEqual(ref_trgt.sort(), conns.target.sort())
        self.assertEqual(ref_sm.sort(), conns.synapse_model.sort())


def suite():
    suite = unittest.makeSuite(MultipleSynapsesTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
