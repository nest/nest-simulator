# -*- coding: utf-8 -*-
#
# test_NodeCollection.py
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
NodeCollection tests
"""

import unittest

import nest

try:
    import numpy as np

    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False


@nest.ll_api.check_stack
class TestNodeCollection(unittest.TestCase):
    """NodeCollection tests"""

    def setUp(self):
        nest.ResetKernel()

    def test_NodeCollection_addition(self):
        """Addition of NodeCollections"""

        # DONE
        nodes_a = nest.Create("iaf_psc_alpha", 2)
        nodes_b = nest.Create("iaf_psc_alpha", 2)
        all_nodes = nodes_a + nodes_b
        all_nodes_list = all_nodes.tolist()
        test_list = [1, 2, 3, 4]
        self.assertEqual(all_nodes_list, test_list)

        nest.ResetKernel()

        # CURRENT TODO
        n_neurons_a = 10
        n_neurons_b = 15
        n_neurons_c = 7
        n_neurons_d = 5
        n_a_b = n_neurons_a + n_neurons_b
        n_a_b_c = n_a_b + n_neurons_c
        nodes_a = nest.Create("iaf_psc_alpha", n_neurons_a)
        nodes_b = nest.Create("iaf_psc_alpha", n_neurons_b)
        nodes_ba = nest.Create("iaf_psc_alpha", n_neurons_b)
        nodes_bb = nest.Create("iaf_psc_alpha", n_neurons_b)
        nodes_c = nest.Create("iaf_psc_exp", n_neurons_c)
        nodes_d = nest.Create("iaf_cond_alpha", n_neurons_d)

        node_b_a = nodes_b + nodes_a
        node_b_a_list = node_b_a.tolist()
        test_b_a_list = nodes_b.tolist() + nodes_a.tolist()
        test_b_a_list.sort()
        self.assertEqual(node_b_a_list, test_b_a_list)

        node_a_c = nodes_a + nodes_c
        node_a_c_list = node_a_c.tolist()
        test_a_c_list = nodes_a.tolist() + nodes_c.tolist()
        test_a_c_list.sort()
        self.assertEqual(node_a_c_list, test_a_c_list)

        # Add two composite NodeCollections
        node_a_c = nodes_a + nodes_c
        node_b_d = nodes_b + nodes_d
        node_abcd = node_b_d + node_a_c
        test_abcd_list = nodes_a.tolist() + nodes_b.tolist() + nodes_c.tolist() + nodes_d.tolist()
        test_abcd_list.sort()
        self.assertEqual(node_abcd.tolist(), test_abcd_list)

        node_a_ba = nodes_a + nodes_ba
        node_b_bb = nodes_b + nodes_bb
        node_aba_bbb = node_a_ba + node_b_bb
        test_aba_bbb_list = nodes_a.tolist() + nodes_ba.tolist() + nodes_b.tolist() + nodes_bb.tolist()
        test_aba_bbb_list.sort()
        self.assertEqual(node_aba_bbb.tolist(), test_aba_bbb_list)

        nest.ResetKernel()

        n_list = []
        n_models = 0
        for model in nest.node_models:
            n = nest.Create(model, 10)
            n_list += n.tolist()
            n_models += 1

        compare_list = list(range(1, n_models * 10 + 1))
        self.assertEqual(n_list, compare_list)

        nest.ResetKernel()

        nc_a = nest.Create("iaf_psc_alpha", 10)
        nc_b = nest.Create("iaf_psc_exp", 7)
        nc_c = nest.NodeCollection([6, 8, 10, 12, 14])

        with self.assertRaises(nest.kernel.NESTError):
            nc_sum = nc_a + nc_b + nc_c  # noqa: F841

    def test_composite_NodeCollection(self):
        """Tests composite NodeCollection with patched node IDs"""
        # Move to indexing and slicing
        # test_composite_node_collection_with_patched_node_ids

        num_a = 10
        num_b = 15
        num_c = 30

        n_a = nest.Create("iaf_psc_exp", num_a)
        n_b = nest.Create("iaf_psc_alpha", num_b)  # noqa: F841
        n_c = nest.Create("iaf_psc_delta", num_c)

        nodes = n_a + n_c
        nodes_step = nodes[::2]
        nodes_list = nodes_step.tolist()
        compare_list = list(range(1, 11))[::2] + list(range(26, 55))[::2]
        self.assertEqual(nodes_list, compare_list)

        self.assertEqual(nodes_list[2], 5)
        self.assertEqual(nodes_list[5], 26)
        self.assertEqual(nodes_list[19], 54)

        # Test iteration of sliced NodeCollection
        i = 0
        for n in nodes_step:
            self.assertEqual(n, nest.NodeCollection([compare_list[i]]))
            i += 1

        n_slice_first = nodes[:10]
        n_slice_middle = nodes[2:7]
        n_slice_middle_jump = nodes[2:12:2]

        n_list_first = n_slice_first.tolist()
        n_list_middle = n_slice_middle.tolist()
        n_list_middle_jump = n_slice_middle_jump.tolist()

        compare_list_first = list(range(1, 11))
        compare_list_middle = list(range(3, 8))
        compare_list_middle_jump = [3, 5, 7, 9, 26]
        self.assertEqual(n_list_first, compare_list_first)
        self.assertEqual(n_list_middle, compare_list_middle)
        self.assertEqual(n_list_middle_jump, compare_list_middle_jump)

    def test_senders_and_targets(self):
        """
        Senders and targets for weight recorder works as NodeCollection and list
        """

        # move to test_weight_recorder

        wr = nest.Create("weight_recorder")
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        # Senders and targets lists empty
        self.assertFalse(nest.GetStatus(wr, "senders")[0])
        self.assertFalse(nest.GetStatus(wr, "targets")[0])

        nest.SetStatus(wr, {"senders": pre[1:3], "targets": post[3:]})

        gss = nest.GetStatus(wr, "senders")[0]
        gst = nest.GetStatus(wr, "targets")[0]

        self.assertEqual(gss.tolist(), [3, 4])
        self.assertEqual(gst.tolist(), [10, 11])

        nest.SetStatus(wr, {"senders": [2, 6], "targets": [8, 9]})
        gss = nest.GetStatus(wr, "senders")[0]
        gst = nest.GetStatus(wr, "targets")[0]
        self.assertEqual(gss.tolist(), [2, 6])
        self.assertEqual(gst.tolist(), [8, 9])

    def test_array_indexing(self):
        """NodeCollection array indexing"""
        n = nest.Create("iaf_psc_alpha", 10)
        cases = [[1, 2], [2, 5], [0, 2, 5, 7, 9], (2, 5), []]
        fail_cases = [
            ([5, 10, 15], IndexError),  # Index not in NodeCollection
            ([2, 5.5], TypeError),  # Not all indices are ints
            ([[2, 4], [6, 8]], TypeError),  # Too many dimensions
            ([2, 2], ValueError),  # Non-unique elements
        ]
        if HAVE_NUMPY:
            cases += [np.array(c) for c in cases]
            fail_cases += [(np.array(c), e) for c, e in fail_cases]
        for case in cases:
            print(type(case), case)
            ref = [i + 1 for i in case]
            ref.sort()
            sliced = n[case]
            self.assertEqual(sliced.tolist(), ref)
        for case, err in fail_cases:
            print(type(case), case)
            with self.assertRaises(err):
                sliced = n[case]

    def test_array_indexing_bools(self):
        """NodeCollection array indexing with bools"""
        n = nest.Create("iaf_psc_alpha", 5)
        cases = [
            [True for _ in range(len(n))],
            [False for _ in range(len(n))],
            [True, False, True, False, True],
        ]
        fail_cases = [
            ([True for _ in range(len(n) - 1)], IndexError),  # Too few bools
            ([True for _ in range(len(n) + 1)], IndexError),  # Too many bools
            ([[True, False], [True, False]], TypeError),  # Too many dimensions
            ([True, False, 2.5, False, True], TypeError),  # Not all indices are bools
            ([1, False, 1, False, 1], TypeError),  # Mixing bools and ints
        ]
        if HAVE_NUMPY:
            cases += [np.array(c) for c in cases]
            # Cutting off fail_cases before cases that mix bools and ints,
            # because converting them to NumPy arrays converts bools to ints.
            fail_cases += [(np.array(c), e) for c, e in fail_cases[:-2]]
        for case in cases:
            print(type(case), case)
            ref = [i for i, b in zip(range(1, 11), case) if b]
            sliced = n[case]
            self.assertEqual(sliced.tolist(), ref)
        for case, err in fail_cases:
            print(type(case), case)
            with self.assertRaises(err):
                sliced = n[case]

    def test_empty_nc_addition(self):
        """Combine NodeCollection with empty NodeCollection and connect"""
        n = 5
        vm = -50.0

        nodes_a = nest.NodeCollection()
        nodes_a += nest.Create("iaf_psc_alpha", n)
        nest.Connect(nodes_a, nodes_a)
        self.assertEqual(nest.num_connections, n * n)
        self.assertTrue(nodes_a)
        self.assertIsNotNone(nodes_a.get())
        nodes_a.V_m = vm
        self.assertEqual(nodes_a.V_m, n * (vm,))

        nest.ResetKernel()

        nodes_b = nest.Create("iaf_psc_alpha", n)
        nodes_b += nest.NodeCollection([])
        nest.Connect(nodes_b, nodes_b)
        self.assertEqual(nest.num_connections, n * n)
        self.assertTrue(nodes_b)
        self.assertIsNotNone(nodes_b.get())
        nodes_b.V_m = vm
        self.assertEqual(nodes_b.V_m, n * (vm,))


def suite():
    suite = unittest.makeSuite(TestNodeCollection, "test")
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
