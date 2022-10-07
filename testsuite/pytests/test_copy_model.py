# -*- coding: utf-8 -*-
#
# test_copy_model.py
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

import nest
import pytest


@nest.ll_api.check_stack
class TestCopyModel:
    """nest.CopyModel Test"""

    @pytest.fixture(autouse=True)
    def reset_kernel(self):
        """
        Reset kernel to clear copied models.
        """

        nest.ResetKernel()

    @pytest.mark.parametrize('org_model', nest.node_models)
    def test_copy_node_models(self, org_model):
        """
        Test that all built-in node models can be copied.

        Nodes created from copy must have higher model_id and correct name.
        """

        new_model = f"{org_model}_copy"
        nest.CopyModel(org_model, new_model)

        org_node = nest.Create(org_model)
        new_node = nest.Create(new_model)

        assert new_node.model_id > org_node.model_id
        assert new_node.model == new_model

    # Limit to first 66 models until #2492 is fixed
    @pytest.mark.parametrize('org_model', nest.synapse_models[:66])
    def test_copy_synapse_models(self, org_model):
        """
        Test that all built-in synapse models can be copied.

        Name and id only checked on model and not on actual synapse
        because some synapse models only work for some neuron models.
        """

        new_model = f"{org_model}_copy"
        nest.CopyModel(org_model, new_model)

        assert (nest.GetDefaults(new_model)['synapse_modelid']
                > nest.GetDefaults(org_model)['synapse_modelid'])
        assert nest.GetDefaults(new_model)['synapse_model'] == new_model

    def test_set_param_on_copy_neuron(self):
        """
        Test that parameter is correctly set when neuron model is copied.
        """

        test_params = {'V_m': 10.0, 'tau_m': 100.0}
        nest.CopyModel('iaf_psc_alpha', 'new_neuron', test_params)
        n = nest.Create('new_neuron')
        for k, v in test_params.items():
            assert n.get(k) == pytest.approx(v)

    def test_set_param_on_copy_synapse(self):
        """
        Test that parameter is correctly set when neuron model is copied.
        """

        test_params = {'weight': 10.0, 'delay': 2.0, 'alpha': 99.0}
        nest.CopyModel('stdp_synapse', 'new_synapse', test_params)
        n = nest.Create('iaf_psc_alpha')
        nest.Connect(n, n, syn_spec='new_synapse')
        conn = nest.GetConnections()
        for k, v in test_params.items():
            assert conn.get(k) == pytest.approx(v)

    @pytest.mark.parametrize('org_model', [nest.node_models[0],
                                           nest.synapse_models[0]])
    def test_cannot_copy_to_existing_model(self, org_model):
        """
        Test that we cannot copy to an existing model.
        """

        try:
            org_name = nest.GetDefaults(org_model)['model']
        except KeyError:
            org_name = nest.GetDefaults(org_model)['synapse_model']

        with pytest.raises(nest.kernel.NESTError, match='NewModelNameExists'):
            nest.CopyModel(org_model, org_model)

    @pytest.mark.parametrize('org_model', [nest.node_models[0],
                                           nest.synapse_models[0]])
    def test_cannot_copy_twice(self, org_model):
        """
        Test that we cannot copy twice to the same name.
        """

        new_model = f"{org_model}_copy"
        nest.CopyModel(org_model, new_model)
        with pytest.raises(nest.kernel.NESTError, match='NewModelNameExists'):
            nest.CopyModel(org_model, new_model)
