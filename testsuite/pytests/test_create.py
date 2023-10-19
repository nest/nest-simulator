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
Basic tests of the ``Create`` function.
"""

import warnings

import nest
import numpy as np
import numpy.testing as nptest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


@pytest.mark.parametrize("model", nest.node_models)
def test_create_model(model):
    """Test basic model creation."""

    node = nest.Create(model)
    assert node.global_id > 0


@pytest.mark.parametrize("model", nest.node_models)
def test_create_model_n_nodes(model):
    """Test creation of multiple nodes with the same model."""

    num_nodes = 10
    nodes = nest.Create(model, num_nodes)
    assert len(nodes) == num_nodes


def test_correct_node_collection_model_created():
    """
    Ensure that the correct model is created for node in ``NodeCollection``.
    """

    models = nest.node_models
    nc = nest.NodeCollection()

    for model in models:
        nc += nest.Create(model)

    assert len(nc) > 0

    for i, node in enumerate(nc):
        assert node.model == models[i]


def test_create_with_params_dict():
    """Test model creation with parameter dictionary."""

    num_nodes = 10
    voltage = 12.0
    nodes = nest.Create("iaf_psc_alpha", num_nodes, {"V_m": voltage})

    nptest.assert_equal(nodes.V_m, voltage)


def test_create_accepts_empty_params_dict():
    """
    Test creation with empty parameter dictionary.
    """

    nest.Create("iaf_psc_delta", params={})


def test_create_with_params_dicts():
    """Test model creation with multiple parameter dictionaries."""

    num_nodes = 10
    V_m = [0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0]
    nodes = nest.Create("iaf_psc_alpha", num_nodes, [{"V_m": v} for v in V_m])

    nptest.assert_array_equal(nodes.V_m, V_m)


def test_create_with_single_params_list():
    """Test ``Create`` with single parameter list."""

    Vm_ref = [-11.0, -12.0, -13.0]
    nodes = nest.Create("iaf_psc_alpha", 3, {"V_m": Vm_ref})

    nptest.assert_array_equal(nodes.V_m, Vm_ref)


def test_create_with_multiple_params_lists():
    """Test ``Create`` with multiple parameter lists."""

    Vm_ref = [-22.0, -33.0, -44.0]
    Cm_ref = 124.0
    Vmin_ref = [-1.0, -2.0, -3.0]
    params = {"V_m": Vm_ref, "C_m": Cm_ref, "V_min": Vmin_ref}
    nodes = nest.Create("iaf_psc_alpha", 3, params)

    nptest.assert_array_equal(nodes.V_m, Vm_ref)
    nptest.assert_array_equal(nodes.C_m, Cm_ref)
    nptest.assert_array_equal(nodes.V_min, Vmin_ref)


def test_create_with_params_numpy():
    """Test ``Create`` with NumPy array as parameter."""

    Vm_ref = np.array([-80.0, -90.0, -100.0])
    nodes = nest.Create("iaf_psc_alpha", 3, {"V_m": Vm_ref})

    nptest.assert_array_equal(nodes.V_m, Vm_ref)


def test_create_with_params_list_that_should_not_be_split():
    """Test ``Create`` with list that should not be split."""

    spikes_ref = [10.0, 20.0, 30.0]
    sgens = nest.Create("spike_generator", 2, {"spike_times": spikes_ref})

    spikes_sg1 = sgens[0].spike_times
    spikes_sg2 = sgens[1].spike_times

    nptest.assert_array_equal(spikes_sg1, spikes_ref)
    nptest.assert_array_equal(spikes_sg2, spikes_ref)


@pytest.mark.parametrize(
    "params, expected_error, expects_warning",
    [
        [tuple(), TypeError, False],
        [{"V_m": [-50]}, IndexError, True],
        [{"V_mm": 3 * [-50.0]}, nest.NESTErrors.UnaccessedDictionaryEntry, True],
    ],
)
def test_erroneous_param_to_create_raises(params, expected_error, expects_warning):
    """Ensure passing an erroneous parameter dictionary to ``Create`` raises exception."""

    with warnings.catch_warnings(record=True) as w:
        warnings.simplefilter("always")

        with pytest.raises(expected_error):
            nest.Create("iaf_psc_alpha", 3, params)

            if expects_warning:
                # verify that user warning was issued
                assert len(w) == 1
                assert issubclass(w[0].category, UserWarning)
