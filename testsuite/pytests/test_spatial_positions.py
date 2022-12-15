# -*- coding: utf-8 -*-
#
# test_spatial_positions.py
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

import nest
import pytest


@pytest.fixture
def reset():
    nest.ResetKernel()


def connect_and_get_connections(pre, post):
    conn_spec = {'rule': 'pairwise_bernoulli',
                 'p': 1.0,
                 'mask': {'circular': {'radius': 1.0}}}
    nest.Connect(pre, post, conn_spec, {'weight': nest.spatial.distance})
    return nest.GetConnections()


def testFreePositions(reset):
    """Correct positions used in Connect with free positions"""

    positions = [[0.1 * x, 0.0] for x in range(1, 10)]
    nodes = nest.Create('iaf_psc_alpha', positions=nest.spatial.free(positions))
    pre = nest.Create('iaf_psc_alpha', positions=nest.spatial.free([[0.0, 0.0]], extent=[1.0, 1.0]))

    conns = connect_and_get_connections(pre, nodes)

    # The expected weight is the distance, which corresponds to the position on the x-axis,
    # which corresponds to the node ID.
    expected_weight = [0.1 * t for t in conns.target]
    assert conns.weight == expected_weight


def testGridPositions(reset):
    """Correct positions used in Connect with grid positions"""

    nodes = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid([1, 9], center=[0, -5], extent=[1.0, 9.0]))
    pre = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid([1, 1]))

    conns = connect_and_get_connections(pre, nodes)

    # The expected weight is the distance, which corresponds to the position on the x-axis,
    # which corresponds to the node ID.
    expected_weight = conns.target
    assert conns.weight == expected_weight
