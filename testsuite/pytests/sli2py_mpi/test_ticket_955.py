# -*- coding: utf-8 -*-
#
# test_ticket_955.py
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
from mpi_test_wrapper import MPITestAssertCompletes


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.parametrize(
    "spec",
    [
        {"pos": [[0, 0]], "extent": [1, 1]},
        {"pos": [[0, 0, 0]], "extent": [1, 1, 1]},
        {"shape": [1, 1]},
        {"shape": [1, 1, 1]},
    ],
)
@MPITestAssertCompletes([1, 2, 4])
def test_ticket_955(spec):
    """
    Confirm one can create a layer with just a single element also when running in parallel.

    This test only confirms completion, no data is tested.
    """

    import nest

    geom = nest.spatial.free(**spec) if "pos" in spec else nest.spatial.grid(**spec)
    nest.Create("parrot_neuron", positions=geom)
