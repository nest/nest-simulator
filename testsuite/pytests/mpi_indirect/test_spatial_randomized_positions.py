# -*- coding: utf-8 -*-
#
# test_spatial_randomized_positions.py
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

import pytest
from mpi_test_wrapper import MPITestAssertEqual

"""
Confirm that spatial positions are consistently generated from random parameters.
"""


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.skipif_missing_threads
@MPITestAssertEqual([1, 2, 4], debug=False)
def test_spatial_connections():
    """
    Confirm that spatial positions are created consistently.

    The test is performed on connection data written to OTHER_LABEL.
    """

    import nest
    import numpy as np
    import pandas as pd

    nest.total_num_virtual_procs = 4

    layer = nest.Create(
        "parrot_neuron",
        n=10,
        positions=nest.spatial.free([nest.random.uniform(0, 1), nest.random.uniform(2, 3), nest.random.uniform(4, 5)]),
    )
    pos = pd.DataFrame(layer.spatial["positions"], columns=["x", "y", "z"])

    assert all((0 <= pos.x) & (pos.x <= 1))
    assert all((2 <= pos.y) & (pos.y <= 3))
    assert all((4 <= pos.z) & (pos.z <= 5))

    pos.to_csv(OTHER_LABEL.format(nest.num_processes, nest.Rank()), index=False, sep="\t")  # noqa: F821
