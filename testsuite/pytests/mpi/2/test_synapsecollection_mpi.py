# -*- coding: utf-8 -*-
#
# test_synapsecollection_mpi.py
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


def testTooFewConnections():
    """Deadlock with empty SynapseCollection"""

    nest.ResetKernel()
    assert nest.GetKernelStatus("num_processes") == 2  # the test expects to be run with 2 MPI processes

    pre = nest.Create("iaf_psc_alpha", 5)
    post = nest.Create("iaf_psc_alpha", 1)

    nest.Connect(pre, post)  # with 2 processes only one process will have connections

    conns = nest.GetConnections()  # Checking that a deadlock does not occur here

    # Expect to get either an empty or a not empty SynapseCollection
    assert isinstance(conns, nest.SynapseCollection)
