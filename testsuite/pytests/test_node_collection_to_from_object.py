# -*- coding: utf-8 -*-
#
# test_node_collection_to_from_object.py
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
Test ``NodeCollection`` to and from object functionality.
"""


import nest
import numpy as np
import numpy.testing as nptest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_node_collection_to_list():
    """Test conversion from ``NodeCollection`` to ``list``."""

    N = 10
    nc = nest.Create("iaf_psc_alpha", N)
    assert nc.tolist() == list(range(1, N + 1))


def test_node_collection_from_list_no_created_nodes_raises():
    """Ensure exception if creating ``NodeCollection`` from ``list`` without creating nodes first."""

    node_ids_in = [5, 10, 15, 20]
    with pytest.raises(nest.NESTErrors.UnknownNode):
        nc = nest.NodeCollection(node_ids_in)


def test_primitive_node_collection_from_list():
    """Test conversion from ``list`` to primitive ``NodeCollection``."""

    nest.Create("iaf_psc_alpha", 10)
    node_ids_in = list(range(2, 8))
    nc = nest.NodeCollection(node_ids_in)
    assert nc.tolist() == node_ids_in


def test_composite_node_collection_from_list():
    """Test conversion from ``list`` to composite ``NodeCollection``."""

    # Creating composite NodeCollection from list
    nest.Create("iaf_psc_alpha", 20)
    node_ids_in = [5, 10, 15, 20]
    nc = nest.NodeCollection(node_ids_in)
    for node, expected_node_id in zip(nc, node_ids_in):
        assert node.global_id == expected_node_id


def test_node_collection_to_numpy_direct():
    """Test conversion from ``NodeCollection`` to NumPy array."""

    N = 10
    nc = nest.Create("iaf_psc_alpha", N)

    # direct array conversion
    nc_arr = np.array(nc)

    expected_arr = np.linspace(1, N, N, dtype=int)
    nptest.assert_array_equal(nc_arr, expected_arr)


def test_node_collection_to_numpy_bigger_array():
    """Test conversion from ``NodeCollection`` to NumPy array twice the size."""

    N = 10
    nc = nest.Create("iaf_psc_alpha", N)

    # incorporation to bigger array
    arr = np.zeros(2 * N, dtype=int)
    start = 2
    arr[start : start + N] = nc

    expected_arr = np.array([0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0, 0, 0, 0, 0, 0, 0, 0])
    nptest.assert_array_equal(arr, expected_arr)


def test_node_collection_from_unsorted_list_raises():
    """Test that creating a ``NodeCollection`` from an unsorted ``list`` raises error."""

    nest.Create("iaf_psc_alpha", 10)

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.NodeCollection([5, 4, 6])

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.NodeCollection([5, 6, 4])
