# -*- coding: utf-8 -*-
#
# test_parameter_apply.py
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
Test ``Parameter`` ``apply`` method for spatial ``NodeCollection``.
"""

import nest
import numpy as np
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_parameter_apply_given_node_collection():
    """Test parameter apply with just passing the ``NodeCollection``."""

    nc = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid([2, 2]))
    param = nest.spatial.pos.x
    ref_positions = np.array(nest.GetPosition(nc))

    assert param.apply(nc) == tuple(ref_positions[:, 0])
    assert param.apply(nc[0]) == (ref_positions[0, 0],)
    assert param.apply(nc[::2]) == tuple(ref_positions[::2, 0])

    # raises for non-existing dimension
    with pytest.raises(nest.NESTError):
        nest.spatial.pos.z.apply(nc)


def test_parameter_apply_given_node_collection_and_single_target_position():
    """Test parameter apply with passing the ``NodeCollection`` and single target position."""

    nc = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid([2, 2]))
    param = nest.spatial.distance

    # Single target position
    target = [
        [1.0, 2.0],
    ]

    for source in nc:
        source_x, source_y = nest.GetPosition(source)
        target_x, target_y = (target[0][0], target[0][1])
        ref_distance = np.sqrt((target_x - source_x) ** 2 + (target_y - source_y) ** 2)

        assert param.apply(source, target) == ref_distance


def test_parameter_apply_given_node_collection_and_multiple_target_positions():
    """Test parameter apply with passing the ``NodeCollection`` and multiple target positions."""

    nc = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid([2, 2]))
    param = nest.spatial.distance

    # Multiple target positions
    targets = np.array(nest.GetPosition(nc))

    for source in nc:
        source_x, source_y = nest.GetPosition(source)
        ref_distances = np.sqrt((targets[:, 0] - source_x) ** 2 + (targets[:, 1] - source_y) ** 2)

        assert param.apply(source, list(targets)) == tuple(ref_distances)


def test_parameter_apply_source_multiple_node_ids_raises():
    """Test parameter apply with passing source ``NodeCollection`` with multiple node IDs raises."""

    source = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid([2, 2]))
    target = [[1.0, 2.0]]
    param = nest.spatial.distance

    with pytest.raises(ValueError):
        param.apply(source, target)


def test_parameter_apply_erroneous_position_specification():
    """Test parameter apply with erroneous position specification raises an error."""

    nc = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid([2, 2]))
    source = nc[0]
    param = nest.spatial.distance

    # Too many dimensions
    with pytest.raises(nest.NESTError):
        param.apply(source, [[1.0, 2.0, 3.0]])

    # Not a list of lists
    with pytest.raises(TypeError):
        param.apply(source, [1.0, 2.0])

    # Not consistent dimensions
    with pytest.raises(ValueError):
        param.apply(source, [[1.0, 2.0], [1.0, 2.0, 3.0]])
