# -*- coding: utf-8 -*-
#
# test_nodeParametrization.py
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
Node Parametrization tests
"""

import nest
import numpy as np
import unittest


class TestNodeParametrization(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_create_with_list(self):
        """Test Create with list as parameter"""
        Vm_ref = [-11., -12., -13.]
        nodes = nest.Create('iaf_psc_alpha', 3, {'V_m': Vm_ref})

        self.assertEqual(list(nest.GetStatus(nodes, 'V_m')), Vm_ref)

    def test_create_with_several_lists(self):
        """Test Create with several lists as parameters"""
        Vm_ref = [-22., -33., -44.]
        Cm_ref = 124.
        Vmin_ref = [-1., -2., -3.]

        nodes = nest.Create('iaf_psc_alpha', 3, {'V_m': Vm_ref,
                                                 'C_m': Cm_ref,
                                                 'V_min': Vmin_ref})

        self.assertEqual(list(nest.GetStatus(nodes, 'V_m')), Vm_ref)
        self.assertEqual(nest.GetStatus(nodes, 'C_m'),
                         (Cm_ref, Cm_ref, Cm_ref))
        self.assertEqual(list(nest.GetStatus(nodes, 'V_min')), Vmin_ref)

    def test_create_with_spike_generator(self):
        """Test Create with list that should not be split"""
        spike_times = [10., 20., 30.]
        sg = nest.Create('spike_generator', 2, {'spike_times': spike_times})

        st = nest.GetStatus(sg, 'spike_times')

        self.assertEqual(list(st[0]), spike_times)
        self.assertEqual(list(st[1]), spike_times)

    def test_create_with_numpy(self):
        """Test Create with numpy array as parameter"""
        Vm_ref = [-80., -90., -100.]
        nodes = nest.Create('iaf_psc_alpha', 3, {'V_m': np.array(Vm_ref)})

        self.assertEqual(list(nest.GetStatus(nodes, 'V_m')), Vm_ref)

    def test_create_uniform(self):
        """Test Create with random.uniform as parameter"""
        min_val = -75.
        max_val = -55.
        N = 3
        nodes = nest.Create('iaf_psc_alpha', N,
                            {'V_m': nest.random.uniform(
                                min=min_val, max=max_val)})
        self.assertEqual(len(nodes.get('V_m')), N)
        self.assertEqual(len(nodes.get('V_m')), len(np.unique(nodes.get('V_m'))),
                         'Values from random distribution are not unique')
        for vm in nodes.get('V_m'):
            self.assertGreaterEqual(vm, min_val)
            self.assertLessEqual(vm, max_val)

    def test_create_normal(self):
        """Test Create with random.normal as parameter"""
        N = 3
        nodes = nest.Create('iaf_psc_alpha', N,
                            {'V_m': nest.random.normal(
                                mean=10.0, std=5.0)})
        self.assertEqual(len(nodes.get('V_m')), N)
        self.assertEqual(len(nodes.get('V_m')), len(np.unique(nodes.get('V_m'))),
                         'Values from random distribution are not unique')

    def test_create_exponential(self):
        """Test Create with random.exonential as parameter"""
        N = 3
        nodes = nest.Create('iaf_psc_alpha', N,
                            {'V_m': nest.random.exponential(beta=1.0)})
        self.assertEqual(len(nodes.get('V_m')), N)
        self.assertEqual(len(nodes.get('V_m')), len(np.unique(nodes.get('V_m'))),
                         'Values from random distribution are not unique')
        for vm in nodes.get('V_m'):
            self.assertGreaterEqual(vm, 0.)

    def test_create_lognormal(self):
        """Test Create with random.lognormal as parameter"""
        N = 3
        nodes = nest.Create('iaf_psc_alpha', N,
                            {'V_m': nest.random.lognormal(
                                mean=10., std=20.)})
        self.assertEqual(len(nodes.get('V_m')), N)
        self.assertEqual(len(nodes.get('V_m')), len(np.unique(nodes.get('V_m'))),
                         'Values from random distribution are not unique')

    def test_create_adding(self):
        """Test Create with different parameters added"""
        nodes = nest.Create('iaf_psc_alpha', 3,
                            {'V_m': -80.0 +
                             nest.random.exponential(beta=0.1)})

        for vm in nodes.get('V_m'):
            self.assertGreaterEqual(vm, -80.0)

        nodes = nest.Create('iaf_psc_alpha', 3,
                            {'V_m': 30.0 + nest.random.uniform(-75., -55.)})

        for vm in nodes.get('V_m'):
            self.assertGreaterEqual(vm, -45.)
            self.assertLessEqual(vm, -25.)

    def test_SetStatus_with_dict(self):
        """Test SetStatus with dict"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        Vm_ref = (-60., -60., -60.)
        nest.SetStatus(nodes, {'V_m': -60.})

        self.assertEqual(nest.GetStatus(nodes, 'V_m'), Vm_ref)

    def test_SetStatus_with_dict_several(self):
        """Test SetStatus with multivalue dict"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        Vm_ref = (-27., -27., -27.)
        Cm_ref = (111., 111., 111.)
        nest.SetStatus(nodes, {'V_m': -27., 'C_m': 111.})

        self.assertEqual(nest.GetStatus(nodes, 'V_m'), Vm_ref)
        self.assertEqual(nest.GetStatus(nodes, 'C_m'), Cm_ref)

    def test_SetStatus_with_list_with_dicts(self):
        """Test SetStatus with list of dicts"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        Vm_ref = (-70., -20., -88.)
        nest.SetStatus(nodes, [{'V_m': -70.}, {'V_m': -20.}, {'V_m': -88.}])

        self.assertEqual(nest.GetStatus(nodes, 'V_m'), Vm_ref)

    def test_SetStatus_with_dict_with_single_list(self):
        """Test SetStatus with dict with list"""

        nodes = nest.Create('iaf_psc_alpha', 3)
        Vm_ref = [-30., -40., -50.]
        nest.SetStatus(nodes, {'V_m': Vm_ref})

        self.assertEqual(list(nest.GetStatus(nodes, 'V_m')), Vm_ref)

    def test_SetStatus_with_dict_with_lists(self):
        """Test SetStatus with dict with lists"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        Vm_ref = [-11., -12., -13.]
        Cm_ref = 177.
        tau_minus_ref = [22., 24., 26.]
        nest.SetStatus(nodes, {'V_m': Vm_ref,
                               'C_m': Cm_ref,
                               'tau_minus': tau_minus_ref})

        self.assertEqual(list(nest.GetStatus(nodes, 'V_m')), Vm_ref)
        self.assertEqual(nest.GetStatus(nodes, 'C_m'),
                         (Cm_ref, Cm_ref, Cm_ref))
        self.assertEqual(list(nest.GetStatus(nodes, 'tau_minus')),
                         tau_minus_ref)

    def test_SetStatus_with_dict_with_single_element_lists(self):
        """Test SetStatus with dict with single element lists"""
        node = nest.Create('iaf_psc_alpha')
        Vm_ref = (-13.,)
        Cm_ref = (222.,)
        nest.SetStatus(node, {'V_m': [-13.], 'C_m': [222.]})

        self.assertEqual(nest.GetStatus(node, 'V_m'), Vm_ref)
        self.assertEqual(nest.GetStatus(node, 'C_m'), Cm_ref)

    def test_SetStatus_with_dict_with_bool(self):
        """Test SetStatus with dict with bool"""
        nodes = nest.Create('spike_detector', 3)
        withport_ref = (True, True, True)
        nest.SetStatus(nodes, {'time_in_steps': True})

        self.assertEqual(nest.GetStatus(nodes, 'time_in_steps'), withport_ref)

    def test_SetStatus_with_dict_with_list_with_bools(self):
        """Test SetStatus with dict with list of bools"""
        nodes = nest.Create('spike_detector', 3)
        withport_ref = (True, False, True)
        nest.SetStatus(nodes, {'time_in_steps': [True, False, True]})

        self.assertEqual(nest.GetStatus(nodes, 'time_in_steps'), withport_ref)

    def test_SetStatus_on_spike_generator(self):
        """Test SetStatus with dict with list that is not to be split"""
        sg = nest.Create('spike_generator')
        nest.SetStatus(sg, {'spike_times': [1., 2., 3.]})

        self.assertEqual(list(nest.GetStatus(sg, 'spike_times')[0]),
                         [1., 2., 3.])

    def test_SetStatus_with_dict_with_numpy(self):
        """Test SetStatus with dict with numpy"""
        nodes = nest.Create('iaf_psc_alpha', 3)

        Vm_ref = np.array([-22., -33., -44.])
        nest.SetStatus(nodes, {'V_m': Vm_ref})

        self.assertEqual(list(nest.GetStatus(nodes, 'V_m')), list(Vm_ref))

    def test_SetStatus_with_random(self):
        """Test SetStatus with dict with random.uniform"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        nest.SetStatus(nodes, {'V_m': nest.random.uniform(-75., -55.)})

        for vm in nodes.get('V_m'):
            self.assertGreater(vm, -75.)
            self.assertLess(vm, -55.)

    def test_SetStatus_with_random_as_val(self):
        """Test SetStatus with val as random.uniform"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        nest.SetStatus(nodes, 'V_m', nest.random.uniform(-75., -55.))

        for vm in nodes.get('V_m'):
            self.assertGreater(vm, -75.)
            self.assertLess(vm, -55.)

    def test_set_with_dict_with_single_list(self):
        """Test set with dict with list"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        Vm_ref = [-30., -40., -50.]
        nodes.set({'V_m': Vm_ref})

        self.assertEqual(list(nodes.get('V_m')), Vm_ref)

    def test_set_with_dict_with_lists(self):
        """Test set with dict with lists"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        Vm_ref = [-11., -12., -13.]
        Cm_ref = 177.
        tau_minus_ref = [22., 24., 26.]
        nodes.set({'V_m': Vm_ref,
                   'C_m': Cm_ref,
                   'tau_minus': tau_minus_ref})

        self.assertEqual(list(nodes.get('V_m')), Vm_ref)
        self.assertEqual(nodes.get('C_m'), (Cm_ref, Cm_ref, Cm_ref))
        self.assertEqual(list(nodes.get('tau_minus')), tau_minus_ref)

    def test_set_with_dict_with_single_element_lists(self):
        """Test set with dict with single element lists"""
        node = nest.Create('iaf_psc_alpha')
        Vm_ref = -13.
        Cm_ref = 222.
        node.set({'V_m': [Vm_ref], 'C_m': [Cm_ref]})

        self.assertEqual(node.get('V_m'), Vm_ref)
        self.assertEqual(node.get('C_m'), Cm_ref)

    def test_set_with_dict_with_list_with_bools(self):
        """Test set with dict with list with bool"""
        nodes = nest.Create('spike_detector', 3)
        withport_ref = (True, False, True)
        nodes.set({'time_in_steps': [True, False, True]})

        self.assertEqual(nodes.get('time_in_steps'), withport_ref)

    def test_set_on_spike_generator(self):
        """Test set with dict with list that is not to be split"""
        sg = nest.Create('spike_generator')
        sg.set({'spike_times': [1., 2., 3.]})

        self.assertEqual(list(sg.get('spike_times')), [1., 2., 3.])

    def test_set_with_random(self):
        """Test set with dict with random parameter"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        nodes.set({'V_m': nest.random.uniform(-75., -55.)})

        for vm in nodes.get('V_m'):
            self.assertGreater(vm, -75.)
            self.assertLess(vm, -55.)

    def test_set_with_random_as_val(self):
        """Test set with random parameter as val"""
        nodes = nest.Create('iaf_psc_alpha', 3)
        nodes.set(V_m=nest.random.uniform(-75., -55.))

        for vm in nodes.get('V_m'):
            self.assertGreater(vm, -75.)
            self.assertLess(vm, -55.)

    def test_parameter_arithmetic(self):
        """Test parameter arithmetic"""
        p1 = nest.hl_api.CreateParameter('constant', {'value': 3.0})
        p2 = nest.hl_api.CreateParameter('constant', {'value': 2.0})
        self.assertEqual((p1 + p2).GetValue(), 5.0)
        self.assertEqual((p1 - p2).GetValue(), 1.0)
        self.assertEqual((p1 / p2).GetValue(), 1.5)
        self.assertEqual((p1 * p2).GetValue(), 6.0)

    def test_syn_spec_parameter(self):
        """Test parameter in syn_spec"""
        n = nest.Create('iaf_psc_alpha', 2)
        p = nest.hl_api.CreateParameter('constant', {'value': 2.0})
        nest.Connect(n, n, syn_spec={'weight': p})
        conns = nest.GetConnections()
        weights = conns.get('weight')
        for w in weights:
            self.assertEqual(w, 2.0)

    def test_conn_spec_parameter(self):
        """Test parameter in conn_spec"""
        p = nest.hl_api.CreateParameter('constant', {'value': 1.0})
        p2 = nest.hl_api.CreateParameter('constant', {'value': 2.0})
        rule_specs = {
            'pairwise_bernoulli': [['p', p], ],
            'fixed_outdegree': [['outdegree', p2], ],
            'fixed_indegree': [['indegree', p2], ]
        }
        for rule, specs_list in rule_specs.items():
            for specs in specs_list:
                nest.ResetKernel()
                n = nest.Create('iaf_psc_alpha', 2)
                param, p = specs
                nest.Connect(n, n, conn_spec={'rule': rule,
                                              param: p})
                self.assertEqual(nest.GetKernelStatus()['num_connections'], 4,
                                 'Error with {}'.format(rule))

    def test_node_pos_parameter(self):
        """Test node-position parameter"""
        positions = [[x, 0.5*x, 0.1+0.2*x] for x in np.linspace(0, 0.5, 5)]
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.free(positions))

        layer.set({'V_m': nest.spatial.pos.x})
        layer.set({'E_L': nest.spatial.pos.y})
        layer.set({'C_m': nest.spatial.pos.z})

        status = layer.get()
        self.assertEqual(status['V_m'], tuple(np.linspace(0, 0.5, 5)))
        self.assertEqual(status['E_L'], tuple(np.linspace(0, 0.5*0.5, 5)))
        self.assertEqual(status['C_m'],
                         tuple([0.1 + 0.2*x for x in np.linspace(0, 0.5, 5)]))

    def test_node_pos_parameter_wrong_dimension(self):
        """Test node-position parameter with wrong dimension"""
        positions = [[x, 0.5*x] for x in np.linspace(0, 0.5, 5)]
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.free(positions))

        with self.assertRaises(nest.kernel.NESTError):
            layer.set({'V_m': nest.spatial.pos.n(-1)})

        with self.assertRaises(nest.kernel.NESTError):
            layer.set({'V_m': nest.spatial.pos.z})

    def test_conn_distance_parameter(self):
        """Test connection distance parameter"""
        positions = [[x, x, x] for x in np.linspace(0, 0.5, 5)]
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.free(positions))

        conn_spec = {
            'rule': 'fixed_outdegree',
            'outdegree': 5,
            'p': 1.0
        }
        nest.Connect(layer, layer, conn_spec=conn_spec, syn_spec={'weight': nest.spatial.distance})
        conns = nest.GetConnections()
        conn_status = conns.get()

        for s, t, w in zip(conn_status['source'],
                           conn_status['target'],
                           conn_status['weight']):
            s_pos = positions[s-1]
            sx = s_pos[0]
            sy = s_pos[1]
            sz = s_pos[2]
            t_pos = positions[t-1]
            tx = t_pos[0]
            ty = t_pos[1]
            tz = t_pos[2]

            dist = np.sqrt((tx-sx)**2 + (ty-sy)**2 + (tz-sz)**2)
            self.assertEqual(w, dist)

    def test_conn_dimension_distance_parameter(self):
        """Test dimension specific connection distance parameter"""
        positions = [[x, x, x] for x in np.linspace(0, 0.5, 5)]

        for i, parameter in enumerate([nest.spatial.distance.x,
                                       nest.spatial.distance.y,
                                       nest.spatial.distance.z]):
            nest.ResetKernel()
            layer = nest.Create('iaf_psc_alpha',
                                positions=nest.spatial.free(positions))
            nest.Connect(layer, layer,
                         conn_spec={'rule': 'pairwise_bernoulli', 'p': 1.},
                         syn_spec={'weight': parameter})
            conns = nest.GetConnections()
            conn_status = conns.get()

            for s, t, w in zip(conn_status['source'],
                               conn_status['target'],
                               conn_status['weight']):
                s_pos = positions[s-1]
                sn = s_pos[0]
                t_pos = positions[t-1]
                tn = t_pos[0]

                n_dist = np.abs(tn-sn)
                self.assertEqual(w, n_dist)

    def test_conn_distance_parameter_wrong_dimension(self):
        """Test connection distance parameter with wrong dimension"""
        positions = [[x, x] for x in np.linspace(0, 0.5, 5)]
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.free(positions))

        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(layer, layer,
                         syn_spec={'weight':
                                   nest.spatial.distance.z})

        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(layer, layer,
                         syn_spec={'weight':
                                   nest.spatial.distance.n(-1)})

    def test_src_tgt_position_parameter(self):
        """Test source and target position parameter"""
        positions = [[x, x, x] for x in np.linspace(0.1, 1.0, 5)]
        source_positions = (nest.spatial.source_pos.x,
                            nest.spatial.source_pos.y,
                            nest.spatial.source_pos.z)
        target_positions = (nest.spatial.target_pos.x,
                            nest.spatial.target_pos.y,
                            nest.spatial.target_pos.z)
        for i in range(3):
            nest.ResetKernel()
            layer = nest.Create('iaf_psc_alpha',
                                positions=nest.spatial.free(positions))
            # Scale up delay because of limited number of digits.
            nest.Connect(layer, layer,
                         conn_spec={'rule': 'pairwise_bernoulli', 'p': 1.},
                         syn_spec={'weight': source_positions[i],
                                   'delay': 100*target_positions[i]})
            conns = nest.GetConnections()
            conn_status = conns.get()

            for s, t, w, d in zip(conn_status['source'],
                                  conn_status['target'],
                                  conn_status['weight'],
                                  conn_status['delay']):
                s_pos = positions[s-1]
                t_pos = positions[t-1]

                # Almost equal because of roundoff errors.
                self.assertAlmostEqual(w, s_pos[i], places=12)
                self.assertAlmostEqual(d, 100*t_pos[i], places=12)

    def test_src_tgt_position_parameter_wrong_args(self):
        """Test source and target position parameter with wrong arguments"""
        positions = [[x, x] for x in np.linspace(0.1, 1.0, 5)]
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.free(positions))

        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(layer, layer, syn_spec={
                'weight': nest.spatial.source_pos.z})

        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(layer, layer, syn_spec={
                'weight': nest.spatial.source_pos.n(-1)})

        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(layer, layer, syn_spec={
                'weight': nest.spatial.target_pos.z})

        with self.assertRaises(nest.kernel.NESTError):
            nest.Connect(layer, layer, syn_spec={
                'weight': nest.spatial.target_pos.n(-1)})

    def test_exp_parameter(self):
        """Test exponential of a parameter"""
        for value in np.linspace(-5.0, 5.0, 15):
            p = nest.hl_api.CreateParameter('constant', {'value': value})
            self.assertEqual(nest.math.exp(p).GetValue(), np.exp(value))

    def test_cos_parameter(self):
        """Test cosine of a parameter"""
        for value in np.linspace(-5.0, 5.0, 15):
            p = nest.hl_api.CreateParameter('constant', {'value': value})
            self.assertEqual(nest.math.cos(p).GetValue(), np.cos(value))

    def test_sin_parameter(self):
        """Test sine of a parameter"""
        for value in np.linspace(-5.0, 5.0, 15):
            p = nest.hl_api.CreateParameter('constant', {'value': value})
            self.assertEqual(nest.math.sin(p).GetValue(), np.sin(value))

    def test_power_parameter(self):
        """Test parameter raised to the power of an exponent"""
        # Negative values
        for value in np.linspace(-5.0, 0.0, 15):
            p = nest.hl_api.CreateParameter('constant', {'value': value})
            # Exponents must be integers
            for exponent in range(-15, 15):
                self.assertEqual((p**exponent).GetValue(),
                                 value**exponent)

        # Positive values
        for value in np.linspace(0.0, 5.0, 15):
            p = nest.hl_api.CreateParameter('constant', {'value': value})
            for exponent in np.linspace(-5.0, 5.0, 15):
                self.assertEqual((p**exponent).GetValue(),
                                 value**exponent)

    def test_min_parameter(self):
        """Test min of parameter"""
        reference_value = 2.0
        for value in np.linspace(-5.0, 5.0, 21):
            p = nest.hl_api.CreateParameter('constant', {'value': value})
            self.assertEqual(nest.math.min(p, reference_value).GetValue(),
                             np.minimum(value, reference_value))

    def test_max_parameter(self):
        """Test max of parameter"""
        reference_value = 2.0
        for value in np.linspace(-5.0, 5.0, 21):
            p = nest.hl_api.CreateParameter('constant', {'value': value})
            self.assertEqual(nest.math.max(p, reference_value).GetValue(),
                             np.maximum(value, reference_value))

    def test_redraw_parameter(self):
        """Test redraw of parameter"""
        min_value = 1.0
        max_value = 1.5
        p = nest.random.normal()
        for _ in range(100):
            value = nest.math.redraw(p, min_value, max_value).GetValue()
            self.assertGreaterEqual(value, min_value)
            self.assertLessEqual(value, max_value)

    def test_redraw_wrong_limits(self):
        """Test redraw of parameter with wrong limits"""
        min_value = 1.5
        max_value = 1.0
        p = nest.random.normal()
        with self.assertRaises(nest.kernel.NESTError):
            nest.math.redraw(p, min_value, max_value)

    def test_redraw_value_impossible(self):
        """Test redraw of parameter with impossible to satisfy limits"""

        min_value = 1.5
        max_value = 2.0
        p = nest.random.uniform(min=0.0, max=1.0)
        with self.assertRaises(nest.kernel.NESTError):
            nest.math.redraw(p, min_value, max_value).GetValue()

    def test_parameter_comparison(self):
        """Test comparison of parameters"""
        p1 = nest.hl_api.CreateParameter('constant', {'value': 1.0})
        p2 = nest.hl_api.CreateParameter('constant', {'value': 2.0})

        self.assertTrue((p1 < p2).GetValue())
        self.assertTrue((p1 <= p2).GetValue())
        self.assertTrue((p1 == p2-p1).GetValue())
        self.assertTrue((p1 != p2).GetValue())
        self.assertTrue((p2 >= p1).GetValue())
        self.assertTrue((p2 > p1).GetValue())

        self.assertFalse((p2 < p1).GetValue())
        self.assertFalse((p2 <= p1).GetValue())
        self.assertFalse((p1 == p2).GetValue())
        self.assertFalse((p1 != p2-p1).GetValue())
        self.assertFalse((p1 >= p2).GetValue())
        self.assertFalse((p1 > p2).GetValue())

    def test_parameter_conditional(self):
        """Test conditional parameter"""
        positions = [[x, x, x] for x in np.linspace(0, 0.5, 20)]
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.free(positions))

        layer.set({'V_m': nest.logic.conditional(nest.spatial.pos.x > 0.3,
                                                 nest.spatial.pos.x,
                                                 -nest.spatial.pos.x)})
        status = layer.get()

        for pos, vm in zip(positions, status['V_m']):
            x_pos = pos[0]
            # Almost equal because of roundoff errors.
            self.assertAlmostEqual(vm,
                                   x_pos if x_pos > 0.3 else -x_pos,
                                   places=12)

    def test_parameter_conditional_scalars(self):
        """Test conditional parameter with scalars"""
        positions = [[x, x, x] for x in np.linspace(0, 0.5, 20)]
        layer = nest.Create('iaf_psc_alpha',
                            positions=nest.spatial.free(positions))

        vm_low_x = -50
        vm_high_x = -42
        layer.set({'V_m': nest.logic.conditional(nest.spatial.pos.x > 0.3,
                                                 vm_high_x,
                                                 vm_low_x)})
        status = layer.get()

        for pos, vm in zip(positions, status['V_m']):
            x_pos = pos[0]
            # Almost equal because of roundoff errors.
            self.assertAlmostEqual(vm,
                                   vm_high_x if x_pos > 0.3 else vm_low_x,
                                   places=12)

    def test_parameter_pos(self):
        """Test positions specified by parameter"""
        positions = nest.random.normal()

        for n_dim in [2, 3]:
            nest.ResetKernel()
            layer = nest.Create('iaf_psc_alpha',
                                10,
                                positions=nest.spatial.free(
                                    positions, num_dimensions=n_dim))
            self.assertEqual(len(layer.spatial['center']), n_dim)
            self.assertEqual(len(layer.spatial['extent']), n_dim)
            self.assertEqual(len(layer.spatial['positions']), 10)
            self.assertEqual(len(layer.spatial['positions'][0]), n_dim)
            self.assertEqual(
                len(np.unique(layer.spatial['positions'][0])), n_dim)

    def test_parameter_is_spatial(self):
        """Test parameter is_spatial function"""
        constant_parameter = nest.hl_api.CreateParameter('constant', {'value': 1.0})
        spatial_parameters = [nest.spatial.pos.x,
                              nest.spatial.distance,
                              nest.spatial.distance.x]
        non_spatial_parameters = [constant_parameter,
                                  nest.random.uniform(),
                                  nest.random.normal(),
                                  nest.random.lognormal(),
                                  nest.random.exponential()]

        def apply_assert(assert_func, p):
            assert_func(p.is_spatial())
            assert_func(nest.math.cos(p).is_spatial())
            assert_func(nest.math.sin(p).is_spatial())
            assert_func(nest.math.exp(p).is_spatial())
            assert_func(nest.math.max(p, 1.0).is_spatial())
            assert_func(nest.math.min(p, 1.0).is_spatial())
            assert_func(nest.math.redraw(p, 0.0, 1.0).is_spatial())
            assert_func((p + constant_parameter).is_spatial())
            assert_func((constant_parameter + p).is_spatial())
            assert_func(nest.logic.conditional(p, constant_parameter, constant_parameter).is_spatial())
            assert_func(nest.logic.conditional(constant_parameter, p, constant_parameter).is_spatial())
            assert_func(nest.logic.conditional(constant_parameter, constant_parameter, p).is_spatial())
        for p in spatial_parameters:
            apply_assert(self.assertTrue, p)
        for p in non_spatial_parameters:
            apply_assert(self.assertFalse, p)

    def test_parameter_apply(self):
        """Parameter apply function"""
        positions = [[x, 0.5*x] for x in np.linspace(0, 0.5, 5)]
        position_array = np.array(positions)
        layer = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(positions))

        # Spatial nc only
        self.assertEqual(tuple(position_array[:, 0]), nest.spatial.pos.x.apply(layer))
        self.assertEqual(tuple(position_array[:, 1]), nest.spatial.pos.y.apply(layer))

        # Spatial nc with a single node_id, and list of positions
        distance_reference = tuple(np.sqrt(position_array[:, 0]**2 + position_array[:, 1]**2))
        self.assertEqual(distance_reference, nest.spatial.distance.apply(layer[0], positions))


def suite():
    suite = unittest.makeSuite(TestNodeParametrization, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
