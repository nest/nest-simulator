# -*- coding: utf-8 -*-
#
# test_connect_node_collection.py
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
Test basic connection with ``NodeCollection``.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_connect_node_collection():
    """Test that ``Connect`` works with ``NodeCollection``."""

    nc = nest.Create("iaf_psc_exp", 10)
    nest.Connect(nc, nc, {"rule": "one_to_one"})

    assert nest.num_connections == 10

    for node in nc:
        nest.Connect(node, node)

    assert nest.num_connections == 20


def test_connect_node_collection_index():
    """Test that ``Connect`` works with indexed ``NodeCollection``."""

    nc = nest.Create("iaf_psc_alpha", 2)
    nest.Connect(nc[0], nc[1])
    assert nest.num_connections == 1


@pytest.mark.parametrize("empty_nc", [nest.NodeCollection(), nest.NodeCollection([])])
def test_connect_empty_node_collection_raises(empty_nc):
    """Test that ``Connect`` with empty ``NodeCollection`` raises an error."""

    nc = nest.Create("iaf_psc_alpha", 5)

    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(nc, empty_nc)

    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(empty_nc, nc)

    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(empty_nc, empty_nc)
