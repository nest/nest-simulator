# -*- coding: utf-8 -*-
#
# test_node_collection_operations.py
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
Test basic operations with ``NodeCollection``.
"""

import nest
import numpy as np
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_node_collection_equal():
    """Test ``NodeCollection`` equality."""

    nc_exp_1 = nest.Create("iaf_psc_exp", 10)
    nc_exp_list_1 = nc_exp_1.tolist()

    nest.ResetKernel()

    nc_exp_2 = nest.Create("iaf_psc_exp", 10)
    nc_exp_list_2 = nc_exp_2.tolist()

    assert nc_exp_1 == nc_exp_2
    assert nc_exp_list_1 == nc_exp_list_2

    nest.ResetKernel()

    nc_alpha_1 = nest.Create("iaf_psc_alpha", 10)
    nc_alpha_2 = nest.NodeCollection(nc_alpha_1.tolist())

    assert nc_alpha_1 == nc_alpha_2
    assert nc_alpha_1 != nc_exp_1


def test_primitive_node_collection_add():
    """Test primitive ``NodeCollection`` addition."""

    nodes_a = nest.Create("iaf_psc_alpha", 2)
    nodes_b = nest.Create("iaf_psc_alpha", 2)
    prim_nc = nodes_a + nodes_b
    assert prim_nc.tolist() == [1, 2, 3, 4]


def test_primitive_node_collection_reverse_add_sorted():
    """Test primitive ``NodeCollection`` reverse addition sorted correctly."""

    nodes_a = nest.Create("iaf_psc_alpha", 10)
    nodes_b = nest.Create("iaf_psc_alpha", 15)
    prim_nc = nodes_b + nodes_a

    expected = nodes_a.tolist() + nodes_b.tolist()
    assert prim_nc.tolist() == expected


def test_composite_node_collection_addition():
    """Test composite ``NodeCollection`` addition."""

    nodes_a = nest.Create("iaf_psc_alpha", 10)
    nodes_b = nest.Create("iaf_psc_alpha", 15)
    nodes_c = nest.Create("iaf_psc_exp", 7)
    nodes_d = nest.Create("iaf_psc_delta", 5)

    comp_nc_ac = nodes_a + nodes_c
    comp_nc_bd = nodes_b + nodes_d

    comp_nc_abcd = comp_nc_bd + comp_nc_ac

    expected = nodes_a.tolist() + nodes_b.tolist() + nodes_c.tolist() + nodes_d.tolist()

    assert comp_nc_abcd.tolist() == expected


def test_node_collection_add_all_node_models():
    """Test against expectation when adding all possible ``NodeCollection`` neuron models."""

    models = nest.node_models
    nc_list = []

    for model in models:
        nc = nest.Create(model, 10)
        nc_list += nc.tolist()

    expected = list(range(1, len(models) * 10 + 1))

    assert nc_list == expected


def test_node_collection_add_overlapping_raises():
    """Test that joining overlapping ``NodeCollection``s raises an error."""

    nc_a = nest.Create("iaf_psc_alpha", 10)
    nc_b = nest.Create("iaf_psc_exp", 7)
    nc_c = nest.NodeCollection([6, 8, 10, 12, 14])

    with pytest.raises(nest.NESTError):
        nc_a + nc_b + nc_c


def test_empty_node_collection_add():
    """Test left add of empty ``NodeCollection``."""

    n_nrns = 5

    nc = nest.NodeCollection()
    nc += nest.Create("iaf_psc_alpha", n_nrns)

    nest.Connect(nc, nc)

    assert nest.num_connections == n_nrns * n_nrns


def test_node_collection_add_zero():
    """Test that adding zero and ``NodeCollection`` results in same ``NodeCollection``."""

    nc = nest.Create("iaf_psc_alpha")

    assert nc + 0 == nc
    assert 0 + nc == nc


def test_single_node_collection_summation():
    """Test that sum over single ``NodeCollection`` results in the same ``NodeCollection``."""

    nc = nest.Create("iaf_psc_alpha")
    assert sum(nc) == nc


def test_primitive_node_collection_summation():
    """Test primitive ``NodeCollection`` summation."""

    nc_a = nest.Create("iaf_psc_alpha", 2)
    nc_b = nest.Create("iaf_psc_alpha", 4)

    expected_prim_nc = nc_a + nc_b
    assert sum([nc_a, nc_b]) == expected_prim_nc


def test_composite_node_collection_summation():
    """Test composite ``NodeCollection`` summation."""

    nc_a = nest.Create("iaf_psc_alpha", 2)
    nc_b = nest.Create("iaf_psc_exp", 4)

    expected_comp_nc = nc_a + nc_b
    assert sum([nc_a, nc_b]) == expected_comp_nc


@pytest.mark.parametrize("invalid_type", [1, [], [0], (0,)])
def test_node_collection_add_invalid_type_raises(invalid_type):
    """Test that adding invalid type to ``NodeCollection`` raises error."""

    nc = nest.Create("iaf_psc_alpha")

    # test right add
    with pytest.raises(TypeError):
        nc + invalid_type

    # test left add
    with pytest.raises(TypeError):
        invalid_type + nc


def test_node_collection_iteration():
    """Test ``NodeCollection`` iteration."""

    nc = nest.Create("iaf_psc_alpha", 15)
    i = 0
    for node in nc:
        assert node == nc[i]
        i += 1


def test_node_collection_membership():
    """
    Test ``NodeCollection`` membership.

    Test that all node IDs in reference are in NodeCollection, and that
    elements in inverse reference are not.
    """

    N_alpha = 10
    N_exp = 5
    N_tot = N_alpha + N_exp
    start = 3
    stop = 12
    step = 3

    # case 1: primitive NodeCollection
    prim_nc = nest.Create("iaf_psc_alpha", N_alpha)
    prim_ref = range(1, N_alpha + 1)
    prim_inv_ref = []

    # case 2: composite NodeCollection
    comp_nc = prim_nc + nest.Create("iaf_psc_exp", N_exp)
    comp_ref = range(1, N_tot + 1)
    comp_inv_ref = []

    # case 3: sliced NodeCollection
    sliced_nc = comp_nc[start:stop]
    sliced_ref = range(start + 1, stop + 1)
    sliced_inv_ref = list(range(1, N_tot))
    del sliced_inv_ref[start:stop]

    # case 4: NodeCollection with step
    step_nc = comp_nc[::step]
    step_ref = range(1, N_tot + 1, step)
    step_inv_ref = list(range(1, N_tot))
    del step_inv_ref[::step]

    # case 5: sliced NodeCollection with step
    sliced_step_nc = comp_nc[start:stop:step]
    sliced_step_ref = range(start + 1, stop + 1, step)
    sliced_step_inv_ref = list(range(1, N_tot))
    del sliced_step_inv_ref[start:stop:step]

    # concatenate cases
    ncs = [prim_nc, comp_nc, sliced_nc, step_nc, sliced_step_nc]
    refs = [prim_ref, comp_ref, sliced_ref, step_ref, sliced_step_ref]
    inv_refs = [prim_inv_ref, comp_inv_ref, sliced_inv_ref, step_inv_ref, sliced_step_inv_ref]

    # iterate and test the different cases
    for nc, ref, inv_ref in zip(ncs, refs, inv_refs):
        for node_id in ref:
            assert node_id in nc
        for node_id in inv_ref:
            assert node_id not in nc
        assert ref[-1] + 1 not in nc
        assert 0 not in nc
        assert -1 not in nc


def test_composite_node_collection_add_sliced_raises():
    """
    Test that an error is raised when trying to add a sliced composite and ``NodeCollection``.
    """

    # composite NodeCollection
    comp_nc = nest.Create("iaf_psc_alpha", 10) + nest.Create("iaf_psc_exp", 7)

    # sliced composite
    sliced_comp_nc = comp_nc[::2]

    nc = nest.Create("iaf_psc_delta")

    with pytest.raises(nest.NESTErrors.BadProperty):
        sliced_comp_nc + nc
