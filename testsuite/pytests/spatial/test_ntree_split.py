# -*- coding: utf-8 -*-
#
# test_ntree_split.py
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
Test cornercase when splitting the ``ntree``.

This test connects spatial populations where the positions of nodes are
defined such that they cause roundoff errors when the ``ntree`` is split.
"""

import nest
import numpy as np


def test_ntree_split():
    """
    Test that ``ntree`` split does not fail when node positions cause roundoff error.
    """

    # Generate positions. The values used here have caused rounding errors previously.
    # We need to have more than 100 position to force the ntree to split.
    positions = [[x, 0.0, 0.0] for x in np.linspace(-0.45, 0.45, 110)]

    # Create source layer based on the generated positions.
    pre = nest.Create("iaf_psc_alpha", positions=nest.spatial.free(positions, edge_wrap=True))

    # Create target layer with a single position.
    post = nest.Create(
        "iaf_psc_alpha", positions=nest.spatial.free([[1.0, 0.0, 0.0]], extent=[1.0, 1.0, 1.0], edge_wrap=True)
    )

    # We must specify a mask to generate a MaskedLayer, which splits the ntree.
    mask = {"box": {"lower_left": [-0.5, -0.5, -0.5], "upper_right": [0.5, 0.5, 0.5]}}

    # Probability intentionally set to zero because we don't have to actually create the connections in this test.
    nest.Connect(
        pre, post, conn_spec={"rule": "pairwise_bernoulli", "p": 0.0, "mask": mask, "allow_oversized_mask": True}
    )
