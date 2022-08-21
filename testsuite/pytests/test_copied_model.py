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

    def check_copied_model(self, original_model, is_synapse=False):
        """Test if the new copied neuron or synapse model has been assigned a correct ID
        and if the new name is correctly stored in the new instances."""

        keys_to_query = ["model", "model_id"]
        if is_synapse:
            keys_to_query = ["synapse_model", "synapse_modelid"]

        new_name = f"{original_model}_copy"
        current_model_id = nest.GetDefaults(original_model)[keys_to_query[1]]

        nest.CopyModel(original_model, new_name)
        copied_model_new_id = nest.GetDefaults(new_name)[keys_to_query[1]]

        self.assertGreater(copied_model_new_id, current_model_id)

        if not is_synapse:
            original_model_instance = nest.Create(original_model)
            copied_model_instance = nest.Create(new_name)

            original_model_instance_info = original_model_instance.get(keys_to_query)

            copied_model_instance_info = copied_model_instance.get(keys_to_query)

            self.assertNotEqual(copied_model_instance_info[keys_to_query[0]],
                                original_model_instance_info[keys_to_query[0]])

            self.assertNotEqual(original_model_instance_info[keys_to_query[0]], "UnknownNode")

            self.assertGreater(copied_model_instance_info[keys_to_query[1]],  original_model_instance_info[keys_to_query[1]])

    def check_copy_model_correctness(self, models, synapse=False):
        """Iterates over all registered and available model
        and checks the correctness of the nest.CopyModel on each model."""

        for model in models:
            self.check_copied_model(model, is_synapse=synapse)

    def test_builtin_neuron_models(self):
        """Checks the correctness of the nest.CopyModel on neurons only"""

        neurons = nest.node_models
        self.check_copy_model_correctness(neurons, synapse=False)

    def test_builtin_synapse_models(self):
        """Checks the correctness of the nest.CopyModel on synapses only"""
        synapses = nest.synapse_models
        self.check_copy_model_correctness(synapses, synapse=True)
