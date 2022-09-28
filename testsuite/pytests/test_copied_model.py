# -*- coding: utf-8 -*-
#
# test_copied_model.py
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
Assert that all existing neuronal models are copied correctly
and new Ids are correctly assigned to the new copied models.
"""

import unittest
import nest


@nest.ll_api.check_stack
class CopyModelTestCase(unittest.TestCase):
    """nest.CopyModel Test"""

    def test_builtin_models(self):
        """Check the correctness of the nest.CopyModel on all built-in models"""

        for model in nest.node_models + nest.synapse_models:
            self.check_copied_model(model)

    def check_copied_model_ids(self, original_model):
        """Test if copied models get correct model IDs"""

        model_type = nest.GetDefaults(original_model, "element_type")

        model_name_key = "model"
        model_id_key = "model_id"
        if model_type == "synapse":
            model_name_key = "synapse_model"
            model_id_key = "synapse_modelid"

        original_model_id = nest.GetDefaults(original_model)[model_id_key]

        new_name = f"{original_model}_copy"
        nest.CopyModel(original_model, new_name)
        copied_model_id = nest.GetDefaults(new_name)[model_id_key]

        self.assertGreater(copied_model_id, original_model_id)

        if model_type == "neuron":
            original_model_instance = nest.Create(original_model)
            copied_model_instance = nest.Create(new_name)

            original_model_name = original_model_instance.get(model_name_key)
            copied_model_name = copied_model_instance.get(model_name_key)
            self.assertNotEqual(original_model_name, copied_model_name)
            self.assertNotEqual(copied_model_name, "UnknownNode")

            original_model_id = original_model_instance.get(model_id_key)
            copied_model_id = copied_model_instance.get(model_id_key)
            self.assertGreater(copied_model_id, original_model_id)

    def test_CopyModel(self):
        """Test CopyModel"""

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
