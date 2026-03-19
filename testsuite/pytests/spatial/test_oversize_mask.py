# -*- coding: utf-8 -*-
#
# test_oversize_mask.py
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
This set of tests ensures that oversized masks are only accepted when explicitly allowed.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


@pytest.fixture
def grid_layer():
    """Fixture for creating a grid layer."""

    grid_layer = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid(shape=[1, 1], edge_wrap=True))
    return grid_layer


@pytest.fixture
def free_layer():
    """Fixture for creating a free layer."""

    free_layer = nest.Create(
        "iaf_psc_alpha", positions=nest.spatial.free([[0.0, 0.0]], extent=[1.0, 1.0], edge_wrap=True)
    )
    return free_layer


@pytest.mark.parametrize(
    "mask, allow_oversized_mask", [({"grid": {"shape": [1, 1]}}, False), ({"grid": {"shape": [2, 2]}}, True)]
)
def test_grid_layer_grid_mask_correct(grid_layer, mask, allow_oversized_mask):
    """
    Test connection of grid layers with correct grid mask.

    The first grid mask in the parametrization is correct and the second is too
    wide and tall but oversized mask is allowed.
    """

    nest.Connect(
        grid_layer,
        grid_layer,
        conn_spec={"rule": "pairwise_bernoulli", "mask": mask, "allow_oversized_mask": allow_oversized_mask},
    )


@pytest.mark.parametrize(
    "mask", [{"grid": {"shape": [2, 1]}}, {"grid": {"shape": [1, 2]}}, {"grid": {"shape": [2, 2]}}]
)
def test_grid_layer_grid_mask_incorrect(grid_layer, mask):
    """
    Test connection of grid layers with incorrect grid mask.

    The first grid mask in the parametrization is too wide, the second too tall
    and the third too wide and tall.
    """

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.Connect(
            grid_layer,
            grid_layer,
            conn_spec={"rule": "pairwise_bernoulli", "mask": mask},
        )


@pytest.mark.parametrize(
    "mask, allow_oversized_mask", [({"circular": {"radius": 0.5}}, False), ({"circular": {"radius": 0.51}}, True)]
)
def test_free_layer_circular_mask_correct(free_layer, mask, allow_oversized_mask):
    """
    Test connection of free layers with correct circular mask.

    The first circular mask in the parametrization is correct and the second is too
    wide but oversized mask is allowed.
    """

    nest.Connect(
        free_layer,
        free_layer,
        conn_spec={"rule": "pairwise_bernoulli", "mask": mask, "allow_oversized_mask": allow_oversized_mask},
    )


@pytest.mark.parametrize("mask", [{"circular": {"radius": 0.51}}])
def test_free_layer_circular_mask_incorrect(free_layer, mask):
    """
    Test connection of free layers with incorrect circular mask.

    The circular mask in the parametrization is too wide.
    """

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.Connect(
            free_layer,
            free_layer,
            conn_spec={"rule": "pairwise_bernoulli", "mask": mask},
        )


@pytest.mark.parametrize(
    "mask, allow_oversized_mask",
    [
        ({"doughnut": {"inner_radius": 0.2, "outer_radius": 0.5}}, False),
        ({"doughnut": {"inner_radius": 0.2, "outer_radius": 0.51}}, True),
    ],
)
def test_free_layer_doughnut_mask_correct(free_layer, mask, allow_oversized_mask):
    """
    Test connection of free layers with correct doughnut mask.

    The first doughnut mask in the parametrization is correct and the second is too
    wide but oversized mask is allowed.
    """

    nest.Connect(
        free_layer,
        free_layer,
        conn_spec={"rule": "pairwise_bernoulli", "mask": mask, "allow_oversized_mask": allow_oversized_mask},
    )


@pytest.mark.parametrize("mask", [{"doughnut": {"inner_radius": 0.2, "outer_radius": 0.51}}])
def test_free_layer_doughnut_mask_incorrect(free_layer, mask):
    """
    Test connection of free layers with incorrect doughnut mask.

    The doughnut mask in the parametrization is too wide.
    """

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.Connect(
            free_layer,
            free_layer,
            conn_spec={"rule": "pairwise_bernoulli", "mask": mask},
        )


@pytest.mark.parametrize(
    "mask, allow_oversized_mask",
    [
        ({"rectangular": {"lower_left": [-0.5, -0.5], "upper_right": [0.5, 0.5]}}, False),
        ({"rectangular": {"lower_left": [-0.5, -0.5], "upper_right": [0.51, 0.5]}}, True),
    ],
)
def test_free_layer_rectangular_mask_correct(free_layer, mask, allow_oversized_mask):
    """
    Test connection of free layers with correct rectangular mask.

    The first rectangular mask in the parametrization is correct and the second is too
    wide but oversized mask is allowed.
    """

    nest.Connect(
        free_layer,
        free_layer,
        conn_spec={"rule": "pairwise_bernoulli", "mask": mask, "allow_oversized_mask": allow_oversized_mask},
    )


@pytest.mark.parametrize("mask", [{"rectangular": {"lower_left": [-0.5, -0.5], "upper_right": [0.51, 0.5]}}])
def test_free_layer_rectangular_mask_incorrect(free_layer, mask):
    """
    Test connection of free layers with incorrect rectangular mask.

    The rectangular mask in the parametrization is too wide.
    """

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.Connect(
            free_layer,
            free_layer,
            conn_spec={"rule": "pairwise_bernoulli", "mask": mask},
        )
