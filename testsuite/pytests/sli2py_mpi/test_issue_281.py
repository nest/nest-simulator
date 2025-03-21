# -*- coding: utf-8 -*-
#
# test_issue_2119.py
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


import pandas as pd
import pytest
from mpi_test_wrapper import MPITestAssertCompletes


@MPITestAssertCompletes([1, 2, 4])
def test_issue_281():
    """
    Confirm that ConnectLayers works MPI-parallel for fixed fan-out.
    """

    import nest
    import pandas as pd

    l = nest.Create("parrot_neuron", positions=nest.spatial.grid(shape=[3, 3]))
    nest.Connect(
        l,
        l,
        {"rule": "fixed_indegree", "indegree": 8, "allow_multapses": False, "allow_autapses": False},
        # weights are randomized to check that global RNGs stay in sync
        {"weight": nest.random.uniform(min=1, max=2)},
    )

    # Ensure by simulation that global RNGs are still in sync
    nest.Simulate(10)
