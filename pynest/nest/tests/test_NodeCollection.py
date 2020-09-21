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

    def test_NodeCollection_to_list(self):
        """Conversion from NodeCollection to list"""

        n_neurons = 10
        n = nest.Create('iaf_psc_alpha', n_neurons)
        n_list = n.tolist()
        self.assertEqual(n_list, list(range(1, n_neurons + 1)))

    def test_list_to_NodeCollection(self):
        """Conversion from list to NodeCollection"""

        # Creating NodeCollection from list without creating the nodes first
        node_ids_in = [5, 10, 15, 20]
        with self.assertRaises(nest.kernel.NESTError):
            nc = nest.NodeCollection(node_ids_in)

        # Creating composite NodeCollection from list
        nest.Create('iaf_psc_alpha', 20)
        node_ids_in = [5, 10, 15, 20]
        nc = nest.NodeCollection(node_ids_in)
        for node_id, compare in zip(nc, node_ids_in):
            self.assertEqual(node_id.global_id, compare)

        nest.ResetKernel()

        # Creating primitive NodeCollection from list
        nest.Create('iaf_psc_alpha', 10)
        node_ids_in = list(range(2, 8))
        nc = nest.NodeCollection(node_ids_in)
        self.assertEqual(nc.tolist(), node_ids_in)

    def test_NodeCollection_to_numpy(self):
        """Conversion from NodeCollection to NumPy array"""
        if HAVE_NUMPY:
            n_neurons = 10
            nc = nest.Create('iaf_psc_alpha', n_neurons)

            # direct array conversion
            n_arr = np.array(nc)

            self.assertEqual(n_arr.tolist(), nc.tolist())

            # incorporation to bigger array
            arr = np.zeros(2*n_neurons, dtype=int)

            start = 2

            arr[start:start + n_neurons] = nc

            self.assertEqual(arr[start:start + n_neurons].tolist(), nc.tolist())

    def test_equal(self):
        """Equality of NodeCollections"""

        n = nest.Create('iaf_psc_exp', 10)
        n_list = n.tolist()

        nest.ResetKernel()

        n_new = nest.Create('iaf_psc_exp', 10)
        new_list = n_new.tolist()
        self.assertEqual(n_list, new_list)
        self.assertEqual(n, n_new)

        nest.ResetKernel()

        nc = nest.Create("iaf_psc_alpha", 10)
        ngc = nest.NodeCollection(nc.tolist())
        self.assertEqual(nc, ngc)

        self.assertNotEqual(nc, n)

    def test_indexing(self):
        """Index of NodeCollections"""

        n = nest.Create('iaf_psc_alpha', 5)
        nc_0 = nest.NodeCollection([1])
        nc_2 = nest.NodeCollection([3])
        nc_4 = nest.NodeCollection([5])

        self.assertEqual(n[0], nc_0)
        self.assertEqual(n[2], nc_2)
        self.assertEqual(n[4], nc_4)
        self.assertEqual(n[-1], nc_4)
        self.assertEqual(n[-3], nc_2)
        self.assertEqual(n[-5], nc_0)
        with self.assertRaises(IndexError):
            n[5]
        with self.assertRaises(IndexError):
            n[-6]

        nest.ResetKernel()

        nodes = nest.Create("iaf_psc_alpha", 10)
        counter = 1
        for nc in nodes:
            self.assertEqual(nc.get('global_id'), counter)
            counter += 1
        for i in range(10):
            nc = nest.NodeCollection([i+1])
            self.assertEqual(nc, nodes[i])

    def test_slicing(self):
        """Slices of NodeCollections"""

        n = nest.Create('iaf_psc_alpha', 10)
        n_slice = n[:5]
        n_list = n_slice.tolist()
        self.assertEqual(n_list, [1, 2, 3, 4, 5])

        n_slice_middle = n[2:7]
        n_list_middle = n_slice_middle.tolist()
        self.assertEqual(n_list_middle, [3, 4, 5, 6, 7])

        n_slice_skip = n[::2]
        n_list_skip = n_slice_skip.tolist()
        self.assertEqual(n_list_skip, [1, 3, 5, 7, 9])

        n_slice_skip_part = n[1:6:3]
        n_list_skip_part = n_slice_skip_part.tolist()
        self.assertEqual(n_list_skip_part, [2, 5])

        n_slice_end = n[5:]
        n_list_end = n_slice_end.tolist()
        self.assertEqual(n_list_end, [6, 7, 8, 9, 10])

        n_slice_negative = n[-4:]
        n_list_negative = n_slice_negative.tolist()
        self.assertEqual(n_list_negative, [7, 8, 9, 10])

        n_slice_negative_end = n[:-3:]
        n_list_negative_end = n_slice_negative_end.tolist()
        self.assertEqual(n_list_negative_end, [1, 2, 3, 4, 5, 6, 7])

        n_slice_negative_start_end = n[-7:-4]
        n_list_negative_start_end = n_slice_negative_start_end.tolist()
        self.assertEqual(n_list_negative_start_end, [4, 5, 6])

        n_slice_start_outside = n[-15:]
        n_list_start_outside = n_slice_start_outside.tolist()
        self.assertEqual(n_list_start_outside, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

        n_slice_stop_outside = n[:15]
        n_list_stop_outside = n_slice_stop_outside.tolist()
        self.assertEqual(n_list_stop_outside, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

        n_slice_start_stop_outside = n[-13:17]
        n_list_start_stop_outside = n_slice_start_stop_outside.tolist()
        self.assertEqual(n_list_start_stop_outside, [1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

        with self.assertRaises(IndexError):
            n[::-3]

    def test_correct_index(self):
        """Multiple NodeCollection calls give right indexing"""
        compare_begin = 1
        compare_end = 11
        for model in nest.Models(mtype='nodes'):
            n = nest.Create(model, 10)
            n_list = n.tolist()
            compare = list(range(compare_begin, compare_end))
            compare_begin += 10
            compare_end += 10
            self.assertEqual(n_list, compare)

    def test_iterating(self):
        """Iteration of NodeCollections"""

        n = nest.Create('iaf_psc_alpha', 15)
        compare = 0
        for nc in n:
            self.assertEqual(nc, n[compare])
            compare += 1

    def test_NodeCollection_addition(self):
        """Addition of NodeCollections"""

        nodes_a = nest.Create("iaf_psc_alpha", 2)
        nodes_b = nest.Create("iaf_psc_alpha", 2)
        all_nodes = nodes_a + nodes_b
        all_nodes_list = all_nodes.tolist()
        test_list = [1, 2, 3, 4]
        self.assertEqual(all_nodes_list, test_list)

        nest.ResetKernel()

        n_neurons_a = 10
        n_neurons_b = 15
        n_neurons_c = 7
        n_a_b = n_neurons_a + n_neurons_b
        n_a_b_c = n_a_b + n_neurons_c
        nodes_a = nest.Create('iaf_psc_alpha', n_neurons_a)
        nodes_b = nest.Create('iaf_psc_alpha', n_neurons_b)
        nodes_c = nest.Create('iaf_psc_exp', n_neurons_c)

        node_b_a = nodes_b + nodes_a
        node_b_a_list = node_b_a.tolist()
        test_b_a_list = (list(range(1, n_neurons_a + 1)) +
                         list(range(n_neurons_a + 1, n_a_b + 1)))
        self.assertEqual(node_b_a_list, test_b_a_list)

        node_a_c = nodes_a + nodes_c
        node_a_c_list = node_a_c.tolist()
        test_a_c_list = (list(range(1, n_neurons_a + 1)) +
                         list(range(n_a_b + 1, n_a_b_c + 1)))
        self.assertEqual(node_a_c_list, test_a_c_list)

        nest.ResetKernel()

        n_list = []
        n_models = 0
        for model in nest.Models(mtype='nodes'):
            n = nest.Create(model, 10)
            n_list += n.tolist()
            n_models += 1

        compare_list = list(range(1, n_models * 10 + 1))
        self.assertEqual(n_list, compare_list)

        nest.ResetKernel()

        nc_a = nest.Create('iaf_psc_alpha', 10)
        nc_b = nest.Create('iaf_psc_exp', 7)
        nc_c = nest.NodeCollection([6, 8, 10, 12, 14])

        with self.assertRaises(nest.kernel.NESTError):
            nc_sum = nc_a + nc_b + nc_c

    def test_NodeCollection_membership(self):
        """Membership in NodeCollections"""
        def check_membership(nc, reference, inverse_ref):
            """Checks that all node IDs in reference are in nc, and that elements in inverse_ref are not in the nc."""
            for i in reference:
                self.assertTrue(i in nc, 'i={}'.format(i))
            for j in inverse_ref:
                self.assertFalse(j in nc)

            self.assertFalse(reference[-1] + 1 in nc)
            self.assertFalse(0 in nc)
            self.assertFalse(-1 in nc)

        # Primitive NodeCollection
        N = 10
        primitive = nest.Create('iaf_psc_alpha', N)
        check_membership(primitive, range(1, N+1), [])

        # Composite NodeCollection
        exp_N = 5
        N += exp_N
        composite = primitive + nest.Create('iaf_psc_exp', exp_N)
        check_membership(composite, range(1, N+1), [])

        # Sliced NodeCollection
        low = 3
        high = 12
        sliced = composite[low:high]
        inverse_reference = list(range(1, N))
        del inverse_reference[low:high]
        check_membership(sliced, range(low+1, high+1), inverse_reference)

        # NodeCollection with step
        step = 3
        stepped = composite[::step]
        inverse_reference = list(range(1, N))
        del inverse_reference[::step]
        check_membership(stepped, range(1, N+1, step), inverse_reference)

        # Sliced NodeCollection with step
        sliced_stepped = composite[low:high:step]
        inverse_reference = list(range(1, N))
        del inverse_reference[low:high:step]
        check_membership(sliced_stepped, range(low+1, high+1, step), inverse_reference)

    def test_NodeCollection_index(self):
        """NodeCollections index function"""
        def check_index_against_list(nc, inverse_ref):
            """Checks NC index against list index, and that elements specified in inverse_ref are not found."""
            for i in nc.tolist():
                self.assertEqual(nc.index(i), nc.tolist().index(i), 'i={}'.format(i))
            for j in inverse_ref:
                with self.assertRaises(ValueError):
                    nc.index(j)
            with self.assertRaises(ValueError):
                nc.index(nc.tolist()[-1] + 1)
            with self.assertRaises(ValueError):
                nc.index(0)
            with self.assertRaises(ValueError):
                nc.index(-1)

        # Primitive NodeCollection
        N = 10
        primitive = nest.Create('iaf_psc_alpha', N)
        check_index_against_list(primitive, [])

        # Composite NodeCollection
        exp_N = 5
        composite = primitive + nest.Create('iaf_psc_exp', exp_N)
        check_index_against_list(composite, [])

        # Sliced NodeCollection
        low = 3
        high = 12
        sliced = composite[low:high]
        inverse_reference = list(range(1, N))
        del inverse_reference[low:high]
        check_index_against_list(sliced, inverse_reference)

        # NodeCollection with step
        step = 3
        stepped = composite[::step]
        inverse_reference = list(range(1, N))
        del inverse_reference[::step]
        check_index_against_list(stepped, inverse_reference)

        # Sliced NodeCollection with step
        sliced_stepped = composite[low:high:step]
        inverse_reference = list(range(1, N))
        del inverse_reference[low:high:step]
        check_index_against_list(sliced_stepped, inverse_reference)

    def test_correct_len_on_NodeCollection(self):
        """len function on NodeCollection"""

        a = nest.Create('iaf_psc_exp', 10)
        self.assertEqual(len(a), 10)

        b = nest.Create('iaf_psc_alpha', 7)
        nodes = a + b
        self.assertEqual(len(nodes), 17)

        c = nest.Create('iaf_psc_delta', 20)
        c = c[3:17:4]
        self.assertEqual(len(c), 4)

    def test_raises_with_nonunique_nodes(self):
        """Non-unique nodes in NodeCollection raises error"""
        n = nest.Create('iaf_psc_alpha', 10)

        with self.assertRaises(nest.kernel.NESTError):
            n[1:3] + n[2:5]
        with self.assertRaises(nest.kernel.NESTError):
            nest.NodeCollection([2, 2])
        with self.assertRaises(nest.kernel.NESTError):
            nest.NodeCollection([2]) + nest.NodeCollection([1, 2])

    def test_from_list_unsorted_raises(self):
        """Creating NodeCollection from unsorted list raises error"""
        nest.Create('iaf_psc_alpha', 10)

        with self.assertRaises(nest.kernel.NESTError):
            nest.NodeCollection([5, 4, 6])
        with self.assertRaises(nest.kernel.NESTError):
            nest.NodeCollection([5, 6, 4])

    def test_slice_with_unsorted_raises(self):
        """Slicing NodeCollection with unsorted list raises error"""
        n = nest.Create('iaf_psc_alpha', 10)

        with self.assertRaises(nest.kernel.NESTError):
            n[[6, 5, 4]]
        with self.assertRaises(nest.kernel.NESTError):
            n[[5, 4, 6]]
        with self.assertRaises(nest.kernel.NESTError):
            n[[5, 6, 4]]

    def test_composite_NodeCollection(self):
        """Tests composite NodeCollection with patched node IDs"""

        num_a = 10
        num_b = 15
        num_c = 30

        n_a = nest.Create('iaf_psc_exp', num_a)
        n_b = nest.Create('iaf_psc_alpha', num_b)
        n_c = nest.Create('iaf_psc_delta', num_c)

        nodes = n_a + n_c
        nodes_step = nodes[::2]
        nodes_list = nodes_step.tolist()
        compare_list = (list(range(1, 11))[::2] + list(range(26, 55))[::2])
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

    def test_composite_wrong_slice(self):
        """
        A NESTError is raised when trying to add a sliced composite and
        NodeCollection
        """

        a = nest.Create('iaf_psc_alpha', 10)
        b = nest.Create('iaf_psc_exp', 7)
        c = a + b
        d = c[::2]
        e = nest.Create('iaf_psc_delta', 13)

        with self.assertRaises(nest.kernel.NESTError):
            f = d + e

    def test_model(self):
        """Correct NodeCollection model"""

        n = nest.Create('iaf_psc_alpha')

        nest.ll_api.sli_run("modeldict")
        model_dict = nest.ll_api.sli_pop()

        models = model_dict.keys()

        for model in models:
            n += nest.Create(model)

        self.assertTrue(len(n) > 0)

        models = ['iaf_psc_alpha'] + list(models)
        for count, nc in enumerate(n):
            self.assertEqual(nc.get('model'), models[count])

    def test_connect(self):
        """Connect works with NodeCollections"""

        n = nest.Create('iaf_psc_exp', 10)
        nest.Connect(n, n, {'rule': 'one_to_one'})
        connections = nest.GetKernelStatus('num_connections')
        self.assertEqual(connections, 10)

        for nc in n:
            nest.Connect(nc, nc)
        self.assertEqual(nest.GetKernelStatus('num_connections'), 20)

        nest.ResetKernel()

        n = nest.Create('iaf_psc_alpha', 2)
        nest.Connect(n[0], n[1])
        self.assertEqual(nest.GetKernelStatus('num_connections'), 1)

    def test_SetStatus_and_GetStatus(self):
        """
        Test that SetStatus and GetStatus works as expected with
        NodeCollection
        """

        num_nodes = 10
        n = nest.Create('iaf_psc_alpha', num_nodes)
        nest.SetStatus(n, {'V_m': 3.5})
        self.assertEqual(nest.GetStatus(n, 'V_m')[0], 3.5)

        V_m = [1., 2., 3., 4., 5., 6., 7., 8., 9., 10.]
        nest.SetStatus(n, 'V_m', V_m)
        for i in range(num_nodes):
            self.assertEqual(nest.GetStatus(n, 'V_m')[i], V_m[i])

        with self.assertRaises(TypeError):
            nest.SetStatus(n, [{'V_m': 34.}, {'V_m': -5.}])

        nest.ResetKernel()

        nc = nest.Create('iaf_psc_exp', 5)

        with self.assertRaises(nest.kernel.NESTError):
            nest.SetStatus(n, {'V_m': -40.})
        with self.assertRaises(nest.kernel.NESTError):
            nest.GetStatus(n)

        nest.ResetKernel()
        n = nest.Create('iaf_psc_alpha', 3)
        nest.SetStatus(n, [{'V_m': 10.}, {'V_m': -10.}, {'V_m': -20.}])
        self.assertEqual(nest.GetStatus(n, 'V_m'), (10., -10., -20.))

    def test_GetConnections(self):
        """
        GetConnection works as expected
        """

        n = nest.Create('iaf_psc_alpha', 3)
        nest.Connect(n, n)

        get_conn = nest.GetConnections()
        get_conn_all = nest.GetConnections(n, n)
        get_conn_some = nest.GetConnections(n[::2])

        self.assertEqual(get_conn_all, get_conn)

        compare_source = [1, 1, 1, 3, 3, 3]
        compare_target = [1, 2, 3, 1, 2, 3]
        self.assertEqual(get_conn_some.get('source'), compare_source)
        self.assertEqual(get_conn_some.get('target'), compare_target)

        compare_list = [3, 1, 0, 15, 6]
        conn = [get_conn_some.get('source')[3],
                get_conn_some.get('target')[3],
                get_conn_some.get('target_thread')[3],
                get_conn_some.get('synapse_id')[3],
                get_conn_some.get('port')[3]]
        self.assertEqual(conn, compare_list)

        conns = nest.GetConnections(n[0]).get()

        connections = [[conns['source'][i],
                        conns['target'][i],
                        conns['target_thread'][i],
                        conns['synapse_id'][i],
                        conns['port'][i]]
                       for i in range(len(nest.GetConnections(n[0])))]

        ref = [[1, 1, 0, 15, 0], [1, 2, 0, 15, 1], [1, 3, 0, 15, 2]]
        for conn, conn_ref in zip(connections, ref):
            self.assertEqual(conn, conn_ref)

    def test_GetConnections_with_slice(self):
        """
        GetConnection with sliced works NodeCollections
        """

        nodes = nest.Create('iaf_psc_alpha', 11)
        nest.Connect(nodes, nodes)

        conns = nest.GetConnections(nodes[1:9:3])
        source = conns.get('source')
        source_ref = [2] * 11 + [5] * 11 + [8] * 11

        self.assertEqual(source_ref, source)

    def test_GetConnections_bad_source(self):
        """
        GetConnection raises a TypeError when called with 0
        """

        n = nest.Create('iaf_psc_alpha', 3)
        nest.Connect(n, n)

        with self.assertRaises(TypeError):
            nest.GetConnections([0, 1])

    def test_senders_and_targets(self):
        """
        Senders and targets for weight recorder works as NodeCollection and list
        """

        wr = nest.Create('weight_recorder')
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

    def test_apply(self):
        """
        NodeCollection apply
        """
        n = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid([2, 2]))
        param = nest.spatial.pos.x
        ref_positions = np.array(nest.GetPosition(n))
        self.assertEqual(param.apply(n), tuple(ref_positions[:, 0]))
        self.assertEqual(param.apply(n[0]), (ref_positions[0, 0],))
        self.assertEqual(param.apply(n[::2]), tuple(ref_positions[::2, 0]))

        with self.assertRaises(nest.kernel.NESTError):
            nest.spatial.pos.z.apply(n)

    def test_apply_positions(self):
        """
        NodeCollection apply with positions
        """
        n = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid([2, 2]))
        param = nest.spatial.distance
        # Single target position
        target = [[1., 2.], ]
        for source in n:
            source_x, source_y = nest.GetPosition(source)
            target_x, target_y = (target[0][0], target[0][1])
            ref_distance = np.sqrt((target_x - source_x)**2 + (target_y - source_y)**2)
            self.assertEqual(param.apply(source, target), ref_distance)

        # Multiple target positions
        targets = np.array(nest.GetPosition(n))
        for source in n:
            source_x, source_y = nest.GetPosition(source)
            ref_distances = np.sqrt((targets[:, 0] - source_x)**2 + (targets[:, 1] - source_y)**2)
            self.assertEqual(param.apply(source, list(targets)), tuple(ref_distances))

        # Raises when passing source with multiple node IDs
        with self.assertRaises(ValueError):
            param.apply(n, target)

        # Erroneous position specification
        source = n[0]
        with self.assertRaises(nest.kernel.NESTError):
            param.apply(source, [[1., 2., 3.], ])  # Too many dimensions
        with self.assertRaises(TypeError):
            param.apply(source, [1., 2.])  # Not a list of lists
        with self.assertRaises(ValueError):
            param.apply(source, [[1., 2.], [1., 2., 3.]])  # Not consistent dimensions

    def test_Create_accepts_empty_params_dict(self):
        """
        Create with empty parameter dictionary
        """
        nest.Create('iaf_psc_delta', params={})

    def test_array_indexing(self):
        """NodeCollection array indexing"""
        n = nest.Create('iaf_psc_alpha', 10)
        cases = [[1, 2],
                 [2, 5],
                 [0, 2, 5, 7, 9],
                 (2, 5),
                 []
                 ]
        fail_cases = [([5, 10, 15], IndexError),  # Index not in NodeCollection
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
        n = nest.Create('iaf_psc_alpha', 5)
        cases = [[True for _ in range(len(n))],
                 [False for _ in range(len(n))],
                 [True, False, True, False, True],
                 ]
        fail_cases = [([True for _ in range(len(n)-1)], IndexError),  # Too few bools
                      ([True for _ in range(len(n)+1)], IndexError),  # Too many bools
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

    def test_empty_nc(self):
        """Connection with empty NodeCollection raises error"""
        nodes = nest.Create('iaf_psc_alpha', 5)

        for empty_nc in [nest.NodeCollection(), nest.NodeCollection([])]:

            with self.assertRaises(nest.kernel.NESTErrors.IllegalConnection):
                nest.Connect(nodes, empty_nc)

            with self.assertRaises(nest.kernel.NESTErrors.IllegalConnection):
                nest.Connect(empty_nc, nodes)

            with self.assertRaises(nest.kernel.NESTErrors.IllegalConnection):
                nest.Connect(empty_nc, empty_nc)

            with self.assertRaises(ValueError):
                empty_nc.get()

            with self.assertRaises(AttributeError):
                empty_nc.V_m

            self.assertFalse(empty_nc)
            self.assertTrue(nodes)
            self.assertIsNone(empty_nc.set())  # Also checking that it does not raise an error

    def test_empty_nc_addition(self):
        """Combine NodeCollection with empty NodeCollection and connect"""
        n = 5
        vm = -50.

        nodes_a = nest.NodeCollection()
        nodes_a += nest.Create('iaf_psc_alpha', n)
        nest.Connect(nodes_a, nodes_a)
        self.assertEqual(nest.GetKernelStatus('num_connections'), n*n)
        self.assertTrue(nodes_a)
        self.assertIsNotNone(nodes_a.get())
        nodes_a.V_m = vm
        self.assertEqual(nodes_a.V_m, n * (vm,))

        nest.ResetKernel()

        nodes_b = nest.Create('iaf_psc_alpha', n)
        nodes_b += nest.NodeCollection([])
        nest.Connect(nodes_b, nodes_b)
        self.assertEqual(nest.GetKernelStatus('num_connections'), n*n)
        self.assertTrue(nodes_b)
        self.assertIsNotNone(nodes_b.get())
        nodes_b.V_m = vm
        self.assertEqual(nodes_b.V_m, n * (vm,))


def suite():
    suite = unittest.makeSuite(TestNodeCollection, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
