# -*- coding: utf-8 -*-
#
# test_mask_operators.py
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

"""Test that mask operators work as expected."""

import nest
import pytest


@pytest.fixture
def two_masks():
    m1 = nest.CreateMask("rectangular", {"lower_left": [0, 0], "upper_right": [2, 2]})
    m2 = nest.CreateMask("rectangular", {"lower_left": [1, 1], "upper_right": [3, 3]})
    yield m1, m2


test_points = [(0.0, 0.0), (0.5, 0.5), (2.0, 2.0), (2.5, 2.5), (0.5, 3.5)]  # m1  # m1  # m1, m2  # m2  # none mask


@pytest.mark.parametrize(
    "op,inside_expected",
    [
        [lambda l, r: l | r, [True, True, True, True, False]],
        [lambda l, r: l & r, [False, False, True, False, False]],
        [lambda l, r: l - r, [True, True, False, False, False]],
        [lambda l, r: r - l, [False, False, False, True, False]],
    ],
    ids=["union", "intersection", "l - r", "r - l"],
)
def test_mask_operators(op, inside_expected, two_masks):
    """
    Confirm that correct inside results are obtained for combined masks.
    """

    assert len(test_points) == len(inside_expected)

    m1, m2 = two_masks
    for point, expected in zip(test_points, inside_expected):
        assert op(m1, m2).Inside(point) == expected


@pytest.mark.parametrize(
    "op,inside_expected",
    [
        [lambda l, r: l | r, [True, True, True, True, False]],
        [lambda l, r: l & r, [False, False, True, False, False]],
        [lambda l, r: l - r, [True, True, False, False, False]],
        [lambda l, r: r - l, [False, False, False, True, False]],
    ],
    ids=["union", "intersection", "l - r", "r - l"],
)
def test_mask_operator_node_selection(op, inside_expected, two_masks):
    """
    Confirm that correct neurons are selected for combined mask.
    """

    assert len(test_points) == len(inside_expected)

    nrns = nest.Create("parrot_neuron", positions=nest.spatial.free(pos=test_points))
    m1, m2 = two_masks
    selected = nest.SelectNodesByMask(nrns, anchor=(0, 0), mask_obj=op(m1, m2))
    assert selected == nrns[inside_expected]
