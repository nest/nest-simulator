# -*- coding: utf-8 -*-
#
# test_GIDCollection.py
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
GIDCollection tests
"""

import unittest
import nest


@nest.check_stack
class TestGIDCollection(unittest.TestCase):
    """GIDCollection tests"""

    def test_GIDCollection_to_list(self):

        nest.ResetKernel()

        n_neurons = 10
        n = nest.Create('iaf_neuron', n_neurons)
        n_list = [x for x in n]
        self.assertEqual(n_list, list(range(1, n_neurons+1)))

    def test_list_to_GIDCollection(self):

        nest.ResetKernel()

        gids = nest.GIDCollection([5, 10, 15, 20])
        compare = 5
        for gid in gids:
            self.assertEqual(gid, compare)
            compare += compare + 10

    def test_equal(self):

        nest.ResetKernel()

        n = nest.Create('iaf_neuron', 10)
        n_list = [x for x in n]

        nest.ResetKernel()

        n = nest.Create('iaf_neuron', 10)
        new_list = [x for x in n]
        self.assertEqual(n_list, new_list)

    def test_indexing(self):

        nest.ResetKernel()

        n = nest.Create('iaf_neuron', 5)
        self.assertEqual(n[2], 3)

    def test_slicing(self):

        nest.ResetKernel()

        n = nest.Create('iaf_neuron', 10)
        n = n[:5]
        n_list = [x for x in n]
        self.assertEqual(n_list, [1, 2, 3, 4, 5])

    def test_correct_index(self):

        nest.ResetKernel()

        compare_begin = 1
        compare_end = 11
        for model in nest.Models(mtype='nodes'):
            n = nest.Create(model, 10)
            n_list = [x for x in n]
            compare = [i for i in range(compare_begin, compare_end)]
            compare_begin += 10
            compare_end += 10
            self.assertEqual(n_list, compare)

    def test_connect(self):

        nest.ResetKernel()

        n = nest.Create('aeif_cond_alpha', 10)
        nest.Connect(n, n, {'rule': 'one_to_one'})
        connections = nest.GetKernelStatus('num_connections')
        self.assertEqual(connections, 10)

    def test_iterating(self):

        nest.ResetKernel()

        Enrns = nest.Create('iaf_psc_alpha', 15)
        compare = 1
        for gid in Enrns:
            self.assertEqual(gid, compare)
            compare += 1


def suite():
    suite = unittest.makeSuite(TestGIDCollection, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

if __name__ == "__main__":
    run()
