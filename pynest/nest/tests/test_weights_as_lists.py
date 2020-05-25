# -*- coding: utf-8 -*-
#
# test_weights_as_lists.py
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
Weights given as lists with the different connection rules
"""

import unittest
import nest


@nest.ll_api.check_stack
class WeightsAsListTestCase(unittest.TestCase):
    """Test weights given as lists"""

    def setUp(self):
        nest.ResetKernel()

    def test_OneToOneWeight(self):
        """Weight given as list, when connection rule is one_to_one"""

        src = nest.Create('iaf_psc_alpha', 3)
        tgt = nest.Create('iaf_psc_delta', 3)

        # weight has to be a list with dimension (n_sources x 1) when one_to_one is used
        ref_weights = [1.2, -3.5, 0.4]

        conn_dict = {'rule': 'one_to_one'}
        syn_dict = {'weight': ref_weights}
        nest.Connect(src, tgt, conn_dict, syn_dict)

        conns = nest.GetConnections()
        weights = conns.weight

        self.assertEqual(weights, ref_weights)

    def test_AllToAllWeight(self):
        """Weight given as list of lists, when connection rule is all_to_all"""

        src = nest.Create('iaf_psc_alpha', 3)
        tgt = nest.Create('iaf_psc_delta', 2)

        # weight has to be a list of lists with dimension (n_target x n_sources) when all_to_all is used
        ref_weights = [[1.2, -3.5, 2.5], [0.4, -0.2, 0.7]]

        conn_dict = {'rule': 'all_to_all'}
        syn_dict = {'weight': ref_weights}
        nest.Connect(src, tgt, conn_dict, syn_dict)

        conns = nest.GetConnections()
        weights = conns.weight

        # Need to flatten ref_weights in order to compare with the weights given by the SynapseCollection.
        ref_weights = [w for sub_weights in ref_weights for w in sub_weights]

        self.assertEqual(weights.sort(), ref_weights.sort())

    def test_FixedIndegreeWeight(self):
        """Weight given as list of list, when connection rule is fixed_indegree"""

        src = nest.Create('iaf_psc_alpha', 5)
        tgt = nest.Create('iaf_psc_delta', 3)

        # weight has to be a list of lists with dimension (n_target x indegree) when fixed_indegree is used
        ref_weights = [[1.2, -3.5], [0.4, -0.2], [0.6, 2.2]]

        conn_dict = {'rule': 'fixed_indegree', 'indegree': 2}
        syn_dict = {'weight': ref_weights}
        nest.Connect(src, tgt, conn_dict, syn_dict)

        conns = nest.GetConnections()
        weights = conns.weight

        # Need to flatten ref_weights in order to compare with the weights given by the SynapseCollection.
        ref_weights = [w for sub_weights in ref_weights for w in sub_weights]

        self.assertEqual(weights.sort(), ref_weights.sort())

    def test_FixedOutdegreeWeight(self):
        """Weight given as list of lists, when connection rule is fixed_outdegree"""

        src = nest.Create('iaf_psc_alpha', 2)
        tgt = nest.Create('iaf_psc_delta', 5)

        # weight has to be a list of lists with dimension (n_source x outegree) when fixed_outdegree is used
        ref_weights = [[1.2, -3.5, 0.4], [-0.2, 0.6, 2.2]]

        conn_dict = {'rule': 'fixed_outdegree', 'outdegree': 3}
        syn_dict = {'weight': ref_weights}
        nest.Connect(src, tgt, conn_dict, syn_dict)

        conns = nest.GetConnections()
        weights = conns.weight

        # Need to flatten ref_weights in order to compare with the weights given by the SynapseCollection.
        ref_weights = [w for sub_weights in ref_weights for w in sub_weights]

        self.assertEqual(weights.sort(), ref_weights.sort())

    def test_FixedTotalNumberWeight(self):
        """Weight given as list, when connection rule is fixed_total_number"""

        src = nest.Create('iaf_psc_alpha', 3)
        tgt = nest.Create('iaf_psc_delta', 4)
        conn_dict = {'rule': 'fixed_total_number', 'N': 4}

        # weight has to be a list with dimension (n_conns x 1) when fixed_total_number is used
        ref_weights = [1.2, -3.5, 0.4, -0.2]

        syn_dict = {'weight': ref_weights}
        nest.Connect(src, tgt, conn_dict, syn_dict)

        conns = nest.GetConnections()
        weights = conns.weight

        self.assertEqual(weights, ref_weights)


def suite():
    suite = unittest.makeSuite(WeightsAsListTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
