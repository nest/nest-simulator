# -*- coding: utf-8 -*-
#
# test_create.py
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
Creation tests
"""

import unittest
import warnings
import nest


@nest.ll_api.check_stack
class CreateTestCase(unittest.TestCase):
    """Creation tests"""

    def setUp(self):
        nest.ResetKernel()

    def test_ModelCreate(self):
        """Model Creation"""

        for model in nest.Models(mtype='nodes'):
            node = nest.Create(model)
            self.assertGreater(node.get('global_id'), 0)

    def test_ModelCreateN(self):
        """Model Creation with N"""

        num_nodes = 10
        for model in nest.Models(mtype='nodes'):
            nodes = nest.Create(model, num_nodes)
            self.assertEqual(len(nodes), num_nodes)

    def test_ModelCreateNdict(self):
        """Model Creation with N and dict"""

        num_nodes = 10
        voltage = 12.0
        n = nest.Create('iaf_psc_alpha', num_nodes, {'V_m': voltage})

        self.assertEqual(nest.GetStatus(n, 'V_m'), (voltage, ) * num_nodes)

    def test_erroneous_param_to_create(self):
        """Erroneous param to Create raises exception"""
        num_nodes = 3
        nest_errors = nest.kernel.NESTErrors
        params = [(tuple(), TypeError),
                  ({'V_m': [-50]}, IndexError),
                  ({'V_mm': num_nodes*[-50.]}, nest_errors.DictError),
                  ({'V_m': num_nodes*[-50]}, nest_errors.TypeMismatch),
                  ({'V_m': -50, 'C_m': num_nodes*[20.]}, nest_errors.TypeMismatch),
                  ]

        for p, err in params:
            with warnings.catch_warnings(record=True) as w:
                warnings.simplefilter('always')
                self.assertRaises(err, nest.Create, 'iaf_psc_alpha', num_nodes, p)
                self.assertEqual(len(w), 1, 'warning was not issued')
                self.assertTrue(issubclass(w[0].category, UserWarning))

    def test_ModelDicts(self):
        """IAF Creation with N and dicts"""

        num_nodes = 10
        V_m = (0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)
        n = nest.Create('iaf_psc_alpha', num_nodes, [{'V_m': v} for v in V_m])

        self.assertEqual(nest.GetStatus(n, 'V_m'), V_m)

    def test_CopyModel(self):
        """CopyModel"""

        nest.CopyModel('iaf_psc_alpha', 'new_neuron', {'V_m': 10.0})
        vm = nest.GetDefaults('new_neuron')['V_m']
        self.assertEqual(vm, 10.0)

        n = nest.Create('new_neuron', 10)
        vm = nest.GetStatus(n[0])[0]['V_m']
        self.assertEqual(vm, 10.0)

        nest.CopyModel('static_synapse', 'new_synapse', {'weight': 10.})
        nest.Connect(n[0], n[1], syn_spec='new_synapse')
        w = nest.GetDefaults('new_synapse')['weight']
        self.assertEqual(w, 10.0)

        self.assertRaisesRegex(
            nest.kernel.NESTError, "NewModelNameExists",
            nest.CopyModel, 'iaf_psc_alpha', 'new_neuron')


def suite():
    suite = unittest.makeSuite(CreateTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
