# -*- coding: utf-8 -*-
#
# test_multisynapse_models.py
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
Test properties of multisynapse models.
"""

import pytest

import nest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


multisyn_models = [model for model in nest.node_models if "_multisynapse" in model]


@pytest.mark.parametrize("multisyn_model", multisyn_models)
def test_multisynapse_model_rport_zero(multisyn_model):
    """
    Test that multisynapse neuron does not accept input to `rport` 0.
    """

    nrn = nest.Create(multisyn_model)
    with pytest.raises(nest.kernel.NESTError):
        nest.Connect(nrn, nrn)


@pytest.mark.parametrize("multisyn_model", multisyn_models)
def test_multisynapse_model_rport_one(multisyn_model):
    """
    Test that multisynapse neuron will accept input to `rport` 1 in default config.
    """

    nrn = nest.Create(multisyn_model)
    nest.Connect(nrn, nrn, syn_spec={"receptor_type": 1})

    expected_nconns = 1
    actual_nconns = nest.GetKernelStatus("num_connections")
    assert actual_nconns == expected_nconns


@pytest.mark.parametrize("multisyn_model", multisyn_models)
def test_multisynapse_model_empty_param_vector(multisyn_model):
    """
    Test setting multisynapse neuron parameters as empty vectors.

    Given being a valid model parameter, the test ensures that it is possible
    to set `E_rev`, `tau_syn`, `tau_rise` and `tau_decay` to empty vectors.
    """

    nrn = nest.Create(multisyn_model)
    default_params = nrn.get()

    empty_params = {}
    # Fill empty params with valid entries
    possible_param_names = ["E_rev", "tau_syn", "tau_rise", "tau_decay"]
    for possible_param_name in possible_param_names:
        if possible_param_name in default_params:
            empty_params[possible_param_name] = []

    # Try to set params as empty vectors
    nrn.set(empty_params)

    # Verify that the set params indeed are empty
    for set_param_name in empty_params:
        set_param_val = nrn.get(set_param_name)
        assert len(set_param_val) == 0
