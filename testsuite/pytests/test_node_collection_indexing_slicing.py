# -*- coding: utf-8 -*-
#
# test_node_collection_indexing_slicing.py
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
Test ``NodeCollection`` indexing and slicing.
"""


import nest
import numpy as np
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_node_collection_indexing():
    """Test ``NodeCollection`` indexing."""

    nc = nest.Create("iaf_psc_alpha", 5)
    nc_0 = nest.NodeCollection([1])
    nc_2 = nest.NodeCollection([3])
    nc_4 = nest.NodeCollection([5])

    assert nc[0] == nc_0
    assert nc[2] == nc_2
    assert nc[4] == nc_4
    assert nc[-1] == nc_4
    assert nc[-3] == nc_2
    assert nc[-5] == nc_0

    with pytest.raises(IndexError):
        nc[5]

    with pytest.raises(IndexError):
        nc[-6]


def test_node_collection_index_method():
    """
    Test ``NodeCollection`` index method.

    Test NodeCollection index against list index, and that elements specified
    in inverse reference are not found.
    """

    N_alpha = 10
    N_exp = 5
    N_tot = N_alpha + N_exp
    start = 3
    stop = 12
    step = 3

    # case 1: primitive NodeCollection
    prim_nc = nest.Create("iaf_psc_alpha", N_alpha)
    prim_inv_ref = []

    # case 2: composite NodeCollection
    comp_nc = prim_nc + nest.Create("iaf_psc_exp", N_exp)
    comp_inv_ref = []

    # case 3: sliced NodeCollection
    sliced_nc = comp_nc[start:stop]
    sliced_inv_ref = list(range(1, N_tot))
    del sliced_inv_ref[start:stop]

    # case 4: NodeCollection with step
    step_nc = comp_nc[::step]
    step_inv_ref = list(range(1, N_tot))
    del step_inv_ref[::step]

    # case 5: sliced NodeCollection with step
    sliced_step_nc = comp_nc[start:stop:step]
    sliced_step_inv_ref = list(range(1, N_tot))
    del sliced_step_inv_ref[start:stop:step]

    # concatenate cases
    ncs = [prim_nc, comp_nc, sliced_nc, step_nc, sliced_step_nc]
    inv_refs = [prim_inv_ref, comp_inv_ref, sliced_inv_ref, step_inv_ref, sliced_step_inv_ref]

    # iterate and test the different cases
    for nc, inv_ref in zip(ncs, inv_refs):
        nc_list = nc.tolist()

        for i in nc_list:
            assert nc.index(i) == nc_list.index(i)

        for j in inv_ref:
            with pytest.raises(ValueError):
                nc.index(j)

        with pytest.raises(ValueError):
            nc.index(nc_list[-1] + 1)

        with pytest.raises(ValueError):
            nc.index(0)

        with pytest.raises(ValueError):
            nc.index(-1)


def test_node_collection_indexing_counter():
    """Test ``NodeCollection`` indexing with loop counter."""

    nodes = nest.Create("iaf_psc_alpha", 10)

    counter = 1
    for node in nodes:
        assert node.global_id == counter
        counter += 1

    for i in range(10):
        nc = nest.NodeCollection([i + 1])
        assert nc == nodes[i]


def test_multiple_node_collection_call_correct_index():
    """Test that multiple ``NodeCollection`` calls give correct indexing."""

    expected_begin = 1
    expected_end = 11
    for model in nest.node_models:
        nc = nest.Create(model, 10)
        expected_lst = list(range(expected_begin, expected_end))
        expected_begin += 10
        expected_end += 10
        assert nc.tolist() == expected_lst


def test_node_collection_slicing():
    """Test ``NodeCollection`` slicing."""

    nc = nest.Create("iaf_psc_alpha", 10)

    assert nc[:5].tolist() == [1, 2, 3, 4, 5]
    assert nc[2:7].tolist() == [3, 4, 5, 6, 7]
    assert nc[::2].tolist() == [1, 3, 5, 7, 9]
    assert nc[1:6:3].tolist() == [2, 5]
    assert nc[5:].tolist() == [6, 7, 8, 9, 10]
    assert nc[-4:].tolist() == [7, 8, 9, 10]
    assert nc[:-3:].tolist() == [1, 2, 3, 4, 5, 6, 7]
    assert nc[-7:-4].tolist() == [4, 5, 6]

    with pytest.raises(IndexError):
        nc[-15:]

    with pytest.raises(IndexError):
        nc[:15]

    with pytest.raises(IndexError):
        nc[-13:17]

    with pytest.raises(IndexError):
        nc[::-3]


def test_node_collection_primitive_composite_slicing():
    """Test primitive and composite ``NodeCollection`` slicing."""

    nc_prim = nest.Create("iaf_psc_alpha", 5)
    nc_comp = nc_prim + nest.Create("iaf_psc_exp")

    for nodes in [nc_prim, nc_comp]:
        nc_list = nodes.tolist()

        # slice without arguments
        assert nodes[:].tolist() == nc_list
        assert nodes[::].tolist() == nc_list

        # slice with start value
        for start in range(-len(nodes), len(nodes)):
            assert nodes[start:].tolist() == nc_list[start:]

        # slice with stop value
        for stop in range(-len(nodes) + 1, len(nodes) + 1):
            if stop == 0:
                continue  # slicing an empty NodeCollection is not allowed.
            assert nodes[:stop].tolist() == nc_list[:stop]

        # slice with step value
        for step in range(1, len(nodes)):
            assert nodes[::step].tolist() == nc_list[::step]

        # slice with start and step values
        for start in range(-len(nodes), len(nodes)):
            for step in range(1, len(nodes)):
                assert nodes[start::step].tolist() == nc_list[start::step]

        # slice with start and step values
        for stop in range(-len(nodes) + 1, len(nodes) + 1):
            if stop == 0:
                continue  # slicing an empty NodeCollection is not allowed.
            for step in range(1, len(nodes)):
                assert nodes[:stop:step].tolist() == nc_list[:stop:step]

        # slice with start, stop and step values
        for start in range(-len(nodes), len(nodes)):
            for stop in range(start + 1, len(nodes) + 1):
                if stop == 0 or (start < 0 and start + len(nodes) >= stop):
                    continue  # cannot slice an empty NodeCollection, or use stop <= start.
                for step in range(1, len(nodes)):
                    assert nodes[start:stop:step].tolist() == nc_list[start:stop:step]


def test_node_collection_slice_unsorted_raises():
    """Test that slicing ``NodeCollection`` with an unsorted list raises error."""

    nc = nest.Create("iaf_psc_alpha", 10)

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        nc[[6, 5, 4]]

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        nc[[5, 4, 6]]

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        nc[[5, 4, 6]]


def test_primitive_node_collection_correct_len():
    """Test that the ``len()`` function works as expected on primitive ``NodeCollection``."""

    prim_nc = nest.Create("iaf_psc_alpha", 10)
    assert len(prim_nc) == 10


def test_composite_node_collection_correct_len():
    """Test that the ``len()`` function works as expected on composite ``NodeCollection``."""

    comp_nc = nest.Create("iaf_psc_alpha", 10) + nest.Create("iaf_psc_exp", 7)
    assert len(comp_nc) == 17


def test_sliced_node_collection_correct_len():
    """Test that the ``len()`` function works as expected on sliced ``NodeCollection``."""

    nc = nest.Create("iaf_psc_delta", 20)
    assert len(nc[3:17:4]) == 4


def test_node_collection_with_nonunique_nodes_raises():
    """Test that non-unique nodes in ``NodeCollection`` raises an error."""

    nc = nest.Create("iaf_psc_alpha", 10)

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        nc[1:3] + nc[2:5]

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        nest.NodeCollection([2, 2])

    with pytest.raises(nest.kernel.NESTErrors.BadProperty):
        nest.NodeCollection([2]) + nest.NodeCollection([1, 2])


def test_composite_node_collection_with_patched_node_ids_correct_nodes():
    """Test correct node IDs for composite ``NodeCollection`` with patched node IDs."""

    nc_a = nest.Create("iaf_psc_exp", 10)
    nest.Create("iaf_psc_alpha", 15)  # will not be part of composite
    nc_c = nest.Create("iaf_psc_delta", 30)

    comp_nc = nc_a + nc_c
    step_comp_nc = comp_nc[::2]
    step_comp_nc_list = step_comp_nc.tolist()

    expected = list(range(1, 11))[::2] + list(range(26, 55))[::2]
    assert step_comp_nc_list == expected


def test_sliced_composite_node_collection_with_patched_node_ids_iteration():
    """Test iteration of sliced composite ``NodeCollection`` with patched node IDs."""

    nc_a = nest.Create("iaf_psc_exp", 10)
    nest.Create("iaf_psc_alpha", 15)  # will not be part of composite
    nc_c = nest.Create("iaf_psc_delta", 30)

    comp_nc = nc_a + nc_c
    step_comp_nc = comp_nc[::2]

    compare_list = list(range(1, 11))[::2] + list(range(26, 55))[::2]

    i = 0
    for node in step_comp_nc:
        assert node == nest.NodeCollection([compare_list[i]])
        i += 1


def test_composite_node_collection_with_patched_node_ids_slicing():
    """Test correct node IDs for sliced composite ``NodeCollection`` with patched node IDs."""

    nc_a = nest.Create("iaf_psc_exp", 10)
    nest.Create("iaf_psc_alpha", 15)  # will not be part of composite
    nc_c = nest.Create("iaf_psc_delta", 30)

    comp_nc = nc_a + nc_c

    nc_slice_first = comp_nc[:10]
    nc_slice_middle = comp_nc[2:7]
    nc_slice_middle_jump = comp_nc[2:12:2]

    expected_first = list(range(1, 11))
    expected_middle = list(range(3, 8))
    expected_middle_jump = [3, 5, 7, 9, 26]

    assert nc_slice_first.tolist() == expected_first
    assert nc_slice_middle.tolist() == expected_middle
    assert nc_slice_middle_jump.tolist() == expected_middle_jump


@pytest.mark.parametrize("indices", [[1, 2], [2, 5], [0, 2, 5, 7, 9], (2, 5), []])
def test_node_collection_array_indexing(indices):
    """Test ``NodeCollection`` array indexing."""

    nc = nest.Create("iaf_psc_alpha", 10)

    expected_node_ids = [i + 1 for i in indices]

    # indexing with standard Python data structures
    assert nc[indices].tolist() == expected_node_ids

    # indexing with NumPy array
    indices_numpy = np.array(indices)
    assert nc[indices_numpy].tolist() == expected_node_ids


@pytest.mark.parametrize(
    "indices, expected_error",
    [
        ([5, 10, 15], IndexError),  # Index not in NodeCollection
        ([2, 5.5], TypeError),  # Not all indices are ints
        ([[2, 4], [6, 8]], TypeError),  # Too many dimensions
        ([2, 2], ValueError),  # Non-unique elements
    ],
)
def test_node_collection_erroneous_array_indexing_raises(indices, expected_error):
    """Test that``NodeCollection`` erroneous array indexing raises error."""

    nc = nest.Create("iaf_psc_alpha", 10)

    # erroneous indexing with standard Python data structures
    with pytest.raises(expected_error):
        nc[indices]

    # erroneous indexing with Numpy array
    indices_numpy = np.array(indices)
    with pytest.raises(expected_error):
        nc[indices_numpy]


@pytest.mark.parametrize("indices", [[True] * 5, [False] * 5, [True, False, True, False, True]])
def test_node_collection_bool_array_indexing(indices):
    """Test ``NodeCollection`` bool array indexing."""

    nc = nest.Create("iaf_psc_alpha", 5)

    expected_node_ids = [i for i, b in zip(range(1, 6), indices) if b]

    # indexing with standard Python data structures
    assert nc[indices].tolist() == expected_node_ids

    # indexing with NumPy array
    indices_numpy = np.array(indices)
    assert nc[indices_numpy].tolist() == expected_node_ids


@pytest.mark.parametrize(
    "indices, expected_error",
    [
        ([True] * 4, IndexError),  # Too few bools
        ([True] * 6, IndexError),  # Too many bools
        ([[True, False], [True, False]], TypeError),  # Too many dimensions
        ([True, False, 2.5, False, True], TypeError),  # Not all indices are bools
        ([1, False, 1, False, 1], TypeError),  # Mixing bools and ints
    ],
)
def test_node_collection_erroneous_bool_array_indexing_raises(indices, expected_error):
    """Test that``NodeCollection`` erroneous array indexing raises error."""

    nc = nest.Create("iaf_psc_alpha", 5)

    # erroneous indexing with standard Python data structures
    with pytest.raises(expected_error):
        nc[indices]

    # erroneous indexing with NumPy array
    if all(isinstance(i, bool) for i in indices):
        # Omit cases that mix bools and ints, because converting them to
        # NumPy arrays converts bools to ints.
        indices_numpy = np.array(indices)
        with pytest.raises(expected_error):
            nc[indices_numpy]
