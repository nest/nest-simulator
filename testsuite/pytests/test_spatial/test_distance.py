# -*- coding: utf-8 -*-
#
# test_distance.py
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
Simple tests for distance calculations.
"""

import math

import pytest

import nest


def test_fixed_grid_distance():
    """
    Test distance between nodes on fixed grid.

    Nodes are at [-1, 0, 1] x [-1, 0, 1].
    """
    positions = nest.spatial.grid(
        shape=[3, 3],  # the number of rows and column in this grid ...
        extent=[3.0, 3.0],  # the size of the grid in mm
    )
    layer = nest.Create("iaf_psc_alpha", positions=positions)

    expected_dists = [
        # first column
        0.0,
        1.0,
        2.0,
        # second column
        1.0,
        math.sqrt(2.0),
        math.sqrt(5.0),
        # third column
        2.0,
        math.sqrt(5.0),
        math.sqrt(8.0),
    ]

    actual_dists = nest.Distance(layer, layer[:1])

    assert actual_dists == pytest.approx(expected_dists)
