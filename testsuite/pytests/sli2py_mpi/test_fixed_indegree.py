# -*- coding: utf-8 -*-
#
# test_fixed_indegree.py
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
from mpi_test_wrapper import MPITestAssertEqualSums


@pytest.mark.skipif_incompatible_mpi
@MPITestAssertEqualSums([1, 2, 4])
def test_fixed_indegree():
    """
    Confirm that correct number of connections is created independent of number of local nodes.
    """

    import nest

    nrns = nest.Create("parrot_neuron", n=5)
    nest.Connect(nrns, nrns, {"rule": "fixed_indegree", "indegree": 2})

    # Write data to minimal CSV file
    with open(OTHER_LABEL.format(nest.num_processes, nest.Rank()), "w") as of:  # noqa: F821
        of.write("n_conn\n")
        of.write(f"{nest.num_connections}\n")
