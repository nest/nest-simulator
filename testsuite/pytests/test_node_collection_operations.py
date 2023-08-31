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

import numpy as np
import pytest

import nest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_node_collection_equal():
    """Test equality of NodeCollections."""

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


def test_node_collection_indexing():
    """Test indexing of NodeCollections."""

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
