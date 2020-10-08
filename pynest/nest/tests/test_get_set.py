# -*- coding: utf-8 -*-
#
# test_get_set.py
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
NodeCollection get/set tests
"""

import unittest
import nest
import json

try:
    import numpy as np
    HAVE_NUMPY = True
except ImportError:
    HAVE_NUMPY = False

try:
    import pandas
    import pandas.testing as pt
    HAVE_PANDAS = True
except ImportError:
    HAVE_PANDAS = False


@nest.ll_api.check_stack
class TestNodeCollectionGetSet(unittest.TestCase):
    """NodeCollection get/set tests"""

    def setUp(self):
        nest.ResetKernel()

    def test_get(self):
        """
        Test that get function works as expected.
        """

        nodes = nest.Create('iaf_psc_alpha', 10)

        C_m = nodes.get('C_m')
        node_ids = nodes.get('global_id')
        E_L = nodes.get('E_L')
        V_m = nodes.get('V_m')
        t_ref = nodes.get('t_ref')
        g = nodes.get(['local', 'thread', 'vp'])
        local = g['local']
        thread = g['thread']
        vp = g['vp']

        self.assertEqual(C_m, (250.0, 250.0, 250.0, 250.0, 250.0,
                               250.0, 250.0, 250.0, 250.0, 250.0))
        self.assertEqual(node_ids, tuple(range(1, 11)))
        self.assertEqual(E_L, (-70.0, -70.0, -70.0, -70.0, -70.0,
                               -70.0, -70.0, -70.0, -70.0, -70.0))
        self.assertEqual(V_m, (-70.0, -70.0, -70.0, -70.0, -70.0,
                               -70.0, -70.0, -70.0, -70.0, -70.0))
        self.assertEqual(t_ref, (2.0, 2.0, 2.0, 2.0, 2.0,
                                 2.0, 2.0, 2.0, 2.0, 2.0))
        self.assertTrue(local)
        self.assertEqual(thread, (0, 0, 0, 0, 0, 0, 0, 0, 0, 0))
        self.assertEqual(vp, (0, 0, 0, 0, 0, 0, 0, 0, 0, 0))

        g_reference = {'local': (True, True, True, True, True,
                                 True, True, True, True, True),
                       'thread': (0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
                       'vp': (0, 0, 0, 0, 0, 0, 0, 0, 0, 0)}
        self.assertEqual(g, g_reference)

    def test_get_sliced(self):
        """
        Test that get works on sliced NodeCollections
        """
        nodes = nest.Create('iaf_psc_alpha', 10)

        V_m = nodes[2:5].get('V_m')
        g = nodes[5:7].get(['t_ref', 'tau_m'])
        C_m = nodes[2:9:2].get('C_m')

        self.assertEqual(V_m, (-70.0, -70.0, -70.0))
        self.assertEqual(g['t_ref'], (2.0, 2.0))
        self.assertEqual(C_m, (250.0, 250.0, 250.0, 250.0))

    def test_get_composite(self):
        """
        Test that get function works on composite NodeCollections
        """
        n1 = nest.Create('iaf_psc_alpha', 2)
        n2 = nest.Create('iaf_psc_delta', 2)
        n3 = nest.Create('iaf_psc_exp')
        n4 = nest.Create('iaf_psc_alpha', 3)

        n1.set(V_m=[-77., -88.])
        n3.set({'V_m': -55.})

        n1.set(C_m=[251., 252.])
        n2.set(C_m=[253., 254.])
        n3.set({'C_m': 255.})
        n4.set(C_m=[256., 257., 258.])

        n5 = n1 + n2 + n3 + n4

        status_dict = n5.get()

        # Check that we get values in correct order
        vm_ref = (-77., -88., -70., -70., -55, -70., -70., -70.)
        self.assertEqual(status_dict['V_m'], vm_ref)

        # Check that we get None where not applicable
        # tau_syn_ex is part of iaf_psc_alpha
        tau_ref = (2., 2., None, None, 2., 2., 2., 2.)
        self.assertEqual(status_dict['tau_syn_ex'], tau_ref)

        # refractory_input is part of iaf_psc_delta
        refrac_ref = (None, None,
                      False, False,
                      None, None,
                      None, None)

        self.assertEqual(status_dict['refractory_input'], refrac_ref)

        # Check that calling get with string works on composite NCs, both on
        # parameters all the models have, and on individual parameters.
        Cm_ref = [x * 1. for x in range(251, 259)]
        Cm = n5.get('C_m')
        self.assertEqual(list(Cm), Cm_ref)

        refrac = n5.get('refractory_input')
        self.assertEqual(refrac, refrac_ref)

    @unittest.skipIf(not HAVE_NUMPY, 'NumPy package is not available')
    def test_get_different_size(self):
        """
        Test get with different input for different sizes of NodeCollections
        """
        single_sr = nest.Create('spike_recorder', 1)
        multi_sr = nest.Create('spike_recorder', 10)
        empty_array_float = np.array([], dtype=np.float64)
        empty_array_int = np.array([], dtype=np.int64)

        # Single node, literal parameter
        self.assertEqual(single_sr.get('start'), 0.0)

        # Single node, array parameter
        self.assertEqual(single_sr.get(['start', 'time_in_steps']),
                         {'start': 0.0, 'time_in_steps': False})

        # Single node, hierarchical with literal parameter
        np.testing.assert_array_equal(single_sr.get('events', 'times'),
                                      empty_array_float)

        # Multiple nodes, hierarchical with literal parameter
        values = multi_sr.get('events', 'times')
        for v in values:
            np.testing.assert_array_equal(v, empty_array_float)

        # Single node, hierarchical with array parameter
        values = single_sr.get('events', ['senders', 'times'])
        self.assertEqual(len(values), 2)
        self.assertTrue('senders' in values)
        self.assertTrue('times' in values)
        np.testing.assert_array_equal(values['senders'], empty_array_int)
        np.testing.assert_array_equal(values['times'], empty_array_float)

        # Multiple nodes, hierarchical with array parameter
        values = multi_sr.get('events', ['senders', 'times'])
        self.assertEqual(len(values), 2)
        self.assertTrue('senders' in values)
        self.assertTrue('times' in values)
        self.assertEqual(len(values['senders']), len(multi_sr))
        for v in values['senders']:
            np.testing.assert_array_equal(v, empty_array_int)
        for v in values['times']:
            np.testing.assert_array_equal(v, empty_array_float)

        # Single node, no parameter (gets all values)
        values = single_sr.get()
        num_values_single_sr = len(values.keys())
        self.assertEqual(values['start'], 0.0)

        # Multiple nodes, no parameter (gets all values)
        values = multi_sr.get()
        self.assertEqual(len(values.keys()), num_values_single_sr)
        self.assertEqual(values['start'],
                         tuple(0.0 for i in range(len(multi_sr))))

    @unittest.skipIf(not HAVE_PANDAS, 'Pandas package is not available')
    def test_get_pandas(self):
        """
        Test that get function with Pandas output works as expected.
        """
        single_sr = nest.Create('spike_recorder', 1)
        multi_sr = nest.Create('spike_recorder', 10)
        empty_array_float = np.array([], dtype=np.float64)

        # Single node, literal parameter
        pt.assert_frame_equal(single_sr.get('start', output='pandas'),
                              pandas.DataFrame({'start': [0.0]},
                                               index=tuple(single_sr.tolist())))

        # Multiple nodes, literal parameter
        pt.assert_frame_equal(multi_sr.get('start', output='pandas'),
                              pandas.DataFrame(
                                  {'start': [0.0 for i in range(
                                      len(multi_sr))]},
                                  index=tuple(multi_sr.tolist())))

        # Single node, array parameter
        pt.assert_frame_equal(single_sr.get(['start', 'n_events'],
                                            output='pandas'),
                              pandas.DataFrame({'start': [0.0],
                                                'n_events': [0]},
                                               index=tuple(single_sr.tolist())))

        # Multiple nodes, array parameter
        ref_dict = {'start': [0.0 for i in range(len(multi_sr))],
                    'n_events': [0]}
        pt.assert_frame_equal(multi_sr.get(['start', 'n_events'],
                                           output='pandas'),
                              pandas.DataFrame(ref_dict,
                                               index=tuple(multi_sr.tolist())))

        # Single node, hierarchical with literal parameter
        pt.assert_frame_equal(single_sr.get('events', 'times',
                                            output='pandas'),
                              pandas.DataFrame({'times': [[]]},
                                               index=tuple(single_sr.tolist())))

        # Multiple nodes, hierarchical with literal parameter
        ref_dict = {'times': [empty_array_float
                              for i in range(len(multi_sr))]}
        pt.assert_frame_equal(multi_sr.get('events', 'times',
                                           output='pandas'),
                              pandas.DataFrame(ref_dict,
                                               index=tuple(multi_sr.tolist())))

        # Single node, hierarchical with array parameter
        ref_df = pandas.DataFrame(
            {'times': [[]], 'senders': [[]]}, index=tuple(single_sr.tolist()))
        ref_df = ref_df.reindex(sorted(ref_df.columns), axis=1)
        pt.assert_frame_equal(single_sr.get(
            'events', ['senders', 'times'], output='pandas'),
            ref_df)

        # Multiple nodes, hierarchical with array parameter
        ref_dict = {'times': [[] for i in range(len(multi_sr))],
                    'senders': [[] for i in range(len(multi_sr))]}
        ref_df = pandas.DataFrame(
            ref_dict,
            index=tuple(multi_sr.tolist()))
        ref_df = ref_df.reindex(sorted(ref_df.columns), axis=1)
        sr_df = multi_sr.get('events', ['senders', 'times'], output='pandas')
        sr_df = sr_df.reindex(sorted(sr_df.columns), axis=1)
        pt.assert_frame_equal(sr_df,
                              ref_df)

        # Single node, no parameter (gets all values)
        values = single_sr.get(output='pandas')
        num_values_single_sr = values.shape[1]
        self.assertEqual(values['start'][tuple(single_sr.tolist())[0]], 0.0)

        # Multiple nodes, no parameter (gets all values)
        values = multi_sr.get(output='pandas')
        self.assertEqual(values.shape, (len(multi_sr), num_values_single_sr))
        pt.assert_series_equal(values['start'],
                               pandas.Series({key: 0.0
                                              for key in tuple(multi_sr.tolist())},
                                             dtype=np.float64,
                                             name='start'))

        # With data in events
        nodes = nest.Create('iaf_psc_alpha', 10)
        pg = nest.Create('poisson_generator', {'rate': 70000.0})
        nest.Connect(pg, nodes)
        nest.Connect(nodes, single_sr)
        nest.Connect(nodes, multi_sr, 'one_to_one')
        nest.Simulate(50)

        ref_values = single_sr.get('events', ['senders', 'times'])
        ref_df = pandas.DataFrame({key: [ref_values[key]] for key in ['senders', 'times']},
                                  index=tuple(single_sr.tolist()))
        sd_df = single_sr.get('events', ['senders', 'times'], output='pandas')
        pt.assert_frame_equal(sd_df, ref_df)

        ref_values = multi_sr.get('events', ['senders', 'times'])
        ref_df = pandas.DataFrame(ref_values, index=tuple(multi_sr.tolist()))
        sd_df = multi_sr.get('events', ['senders', 'times'], output='pandas')
        pt.assert_frame_equal(sd_df, ref_df)

    def test_get_JSON(self):
        """
        Test that get function with json output works as expected.
        """
        single_sr = nest.Create('spike_recorder', 1)
        multi_sr = nest.Create('spike_recorder', 10)

        # Single node, literal parameter
        self.assertEqual(json.loads(
            single_sr.get('start', output='json')), 0.0)

        # Multiple nodes, literal parameter
        self.assertEqual(
            json.loads(multi_sr.get('start', output='json')),
            len(multi_sr) * [0.0])

        # Single node, array parameter
        ref_dict = {'start': 0.0, 'n_events': 0}
        self.assertEqual(
            json.loads(single_sr.get(['start', 'n_events'], output='json')),
            ref_dict)

        # Multiple nodes, array parameter
        ref_dict = {'start': len(multi_sr) * [0.0],
                    'n_events': len(multi_sr) * [0]}
        self.assertEqual(
            json.loads(multi_sr.get(['start', 'n_events'], output='json')),
            ref_dict)

        # Single node, hierarchical with literal parameter
        self.assertEqual(json.loads(single_sr.get(
            'events', 'times', output='json')), [])

        # Multiple nodes, hierarchical with literal parameter
        ref_list = len(multi_sr) * [[]]
        self.assertEqual(
            json.loads(multi_sr.get('events', 'times', output='json')),
            ref_list)

        # Single node, hierarchical with array parameter
        ref_dict = {'senders': [], 'times': []}
        self.assertEqual(
            json.loads(single_sr.get(
                'events', ['senders', 'times'], output='json')),
            ref_dict)

        # Multiple nodes, hierarchical with array parameter
        ref_dict = {'times': len(multi_sr) * [[]],
                    'senders': len(multi_sr) * [[]]}
        self.assertEqual(
            json.loads(multi_sr.get(
                'events', ['senders', 'times'], output='json')),
            ref_dict)

        # Single node, no parameter (gets all values)
        values = json.loads(single_sr.get(output='json'))
        num_values_single_sr = len(values)
        self.assertEqual(values['start'], 0.0)

        # Multiple nodes, no parameter (gets all values)
        values = json.loads(multi_sr.get(output='json'))
        self.assertEqual(len(values), num_values_single_sr)
        self.assertEqual(values['start'], len(multi_sr) * [0.0])

        # With data in events
        nodes = nest.Create('iaf_psc_alpha', 10)
        pg = nest.Create('poisson_generator', {'rate': 70000.0})
        nest.Connect(pg, nodes)
        nest.Connect(nodes, single_sr)
        nest.Connect(nodes, multi_sr, 'one_to_one')
        nest.Simulate(50)

        sd_ref = single_sr.get('events', ['senders', 'times'])
        sd_json = single_sr.get('events', ['senders', 'times'], output='json')
        sd_dict = json.loads(sd_json)
        self.assertEqual(len(sd_dict.keys()), 2)
        self.assertEqual(sorted(sd_dict.keys()), sorted(sd_ref.keys()))
        for key in ['senders', 'times']:
            self.assertEqual(list(sd_ref[key]), list(sd_dict[key]))

        multi_sr_ref = multi_sr.get('events', ['senders', 'times'])
        multi_sr_json = multi_sr.get('events', ['senders', 'times'], output='json')
        multi_sr_dict = json.loads(multi_sr_json)
        self.assertEqual(len(multi_sr_dict.keys()), 2)
        self.assertEqual(sorted(multi_sr_dict.keys()), sorted(multi_sr_ref.keys()))
        for key in ['senders', 'times']:
            multi_sr_ref_element = [list(element) for element in multi_sr_ref[key]]
            self.assertEqual(multi_sr_ref_element, multi_sr_dict[key])

    def test_set(self):
        """
        Test that set function works as expected.
        """

        nodes = nest.Create('iaf_psc_alpha', 10)

        # Dict to set same value for all nodes.
        nodes.set({'C_m': 100.0})
        C_m = nodes.get('C_m')
        self.assertEqual(C_m, (100.0, 100.0, 100.0, 100.0, 100.0,
                               100.0, 100.0, 100.0, 100.0, 100.0))

        # Set same value for all nodes.
        nodes.set(tau_Ca=500.0)
        tau_Ca = nodes.get('tau_Ca')
        self.assertEqual(tau_Ca, (500.0, 500.0, 500.0, 500.0, 500.0,
                                  500.0, 500.0, 500.0, 500.0, 500.0))

        # List of dicts, where each dict corresponds to a single node.
        nodes.set(({'V_m': 10.0}, {'V_m': 20.0}, {'V_m': 30.0}, {'V_m': 40.0},
                   {'V_m': 50.0}, {'V_m': 60.0}, {'V_m': 70.0}, {'V_m': 80.0},
                   {'V_m': 90.0}, {'V_m': -100.0}))
        V_m = nodes.get('V_m')
        self.assertEqual(V_m, (10.0, 20.0, 30.0, 40.0, 50.0,
                               60.0, 70.0, 80.0, 90.0, -100.0))

        # Set value of a parameter based on list. List must be length of nodes.
        nodes.set(V_reset=[-85., -82., -80., -77., -75.,
                           -72., -70., -67., -65., -62.])
        V_reset = nodes.get('V_reset')
        self.assertEqual(V_reset, (-85., -82., -80., -77., -75.,
                                   -72., -70., -67., -65., -62.))

        with self.assertRaises(IndexError):
            nodes.set(V_reset=[-85., -82., -80., -77., -75.])

        # Set different parameters with a dictionary.
        nodes.set({'t_ref': 44.0, 'tau_m': 2.0, 'tau_minus': 42.0})
        g = nodes.get(['t_ref', 'tau_m', 'tau_minus'])
        self.assertEqual(g['t_ref'], (44.0, 44.0, 44.0, 44.0, 44.0,
                                      44.0, 44.0, 44.0, 44.0, 44.0))
        self.assertEqual(g['tau_m'], (2.0, 2.0, 2.0, 2.0, 2.0,
                                      2.0, 2.0, 2.0, 2.0, 2.0))
        self.assertEqual(g['tau_minus'], (42.0, 42.0, 42.0, 42.0, 42.0,
                                          42.0, 42.0, 42.0, 42.0, 42.0))

        with self.assertRaises(nest.kernel.NESTError):
            nodes.set({'vp': 2})

    def test_set_composite(self):
        """
        Test that set works on composite NodeCollections
        """
        nodes = nest.Create('iaf_psc_alpha', 10)

        nodes[2:5].set(({'V_m': -50.0}, {'V_m': -40.0}, {'V_m': -30.0}))
        nodes[5:7].set({'t_ref': 4.4, 'tau_m': 3.0})
        nodes[2:9:2].set(C_m=111.0)
        V_m = nodes.get('V_m')
        g = nodes.get(['t_ref', 'tau_m'])
        C_m = nodes.get('C_m')

        self.assertEqual(V_m, (-70.0, -70.0, -50.0, -40.0, -30.0,
                               -70.0, -70.0, -70.0, -70.0, -70.0,))
        self.assertEqual(g, {'t_ref': (2.0, 2.0, 2.0, 2.0, 2.0,
                                       4.4, 4.4, 2.0, 2.0, 2.0),
                             'tau_m': (10.0, 10.0, 10.0, 10.0, 10.0,
                                       3.00, 3.00, 10.0, 10.0, 10.0)})
        self.assertEqual(C_m, (250.0, 250.0, 111.0, 250.0, 111.0,
                               250.0, 111.0, 250.0, 111.0, 250.0))

    def test_get_attribute(self):
        """Test get using getattr"""
        nodes = nest.Create('iaf_psc_alpha', 10)
        self.assertEqual(nodes.C_m, (250.0, 250.0, 250.0, 250.0, 250.0,
                                     250.0, 250.0, 250.0, 250.0, 250.0))
        self.assertEqual(nodes.global_id, tuple(range(1, 11)))
        self.assertEqual(nodes.E_L, (-70.0, -70.0, -70.0, -70.0, -70.0,
                                     -70.0, -70.0, -70.0, -70.0, -70.0))
        self.assertEqual(nodes.V_m, (-70.0, -70.0, -70.0, -70.0, -70.0,
                                     -70.0, -70.0, -70.0, -70.0, -70.0))
        self.assertEqual(nodes.t_ref, (2.0, 2.0, 2.0, 2.0, 2.0,
                                       2.0, 2.0, 2.0, 2.0, 2.0))
        with self.assertRaises(KeyError):
            print(nodes.nonexistent_attribute)

        self.assertIsNone(nodes.spatial)
        spatial_nodes = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid([2, 2]))
        self.assertIsNotNone(spatial_nodes.spatial)
        spatial_reference = {'network_size': 4,
                             'center': (0.0, 0.0),
                             'edge_wrap': False,
                             'extent': (1.0, 1.0),
                             'shape': (2, 2)}
        self.assertEqual(spatial_nodes.spatial, spatial_reference)

    def test_set_attribute(self):
        """Test set using setattr"""
        nodes = nest.Create('iaf_psc_alpha', 10)
        nodes.C_m = 100.0
        self.assertEqual(nodes.get('C_m'), (100.0, 100.0, 100.0, 100.0, 100.0,
                                            100.0, 100.0, 100.0, 100.0, 100.0))
        v_reset_reference = (-85., -82., -80., -77., -75., -72., -70., -67., -65., -62.)
        nodes.V_reset = v_reset_reference
        self.assertEqual(nodes.get('V_reset'), v_reset_reference)

        with self.assertRaises(IndexError):
            nodes.V_reset = [-85., -82., -80., -77., -75.]

        with self.assertRaises(nest.kernel.NESTError):
            nodes.nonexistent_attribute = 1.


def suite():
    suite = unittest.makeSuite(TestNodeCollectionGetSet, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
