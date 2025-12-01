# -*- coding: utf-8 -*-
#
# test_ticket_564.py
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

"""
Regression test for Ticket #564.

Test ported from SLI regression test.
Ensure that building many connections on multi-threaded kernels yields the expected number.

Author: Hans Ekkehard Plesser, 2012-05-27
"""


@pytest.mark.skipif_missing_threads
def test_ticket_564_fixed_indegree_multiple_trials():
    """
    Ensure that repeated fixed_indegree Connect calls under multithreading yield the expected number of connections.
    """

    num_neurons = 100
    indegree = 100
    num_trials = 10
    expected_connections = num_neurons * indegree

    connection_counts = []

    for _ in range(num_trials):
        nest.ResetKernel()
        nest.total_num_virtual_procs = 4

        nodes = nest.Create("iaf_psc_alpha", n=num_neurons)
        nest.Connect(nodes, nodes, conn_spec={"rule": "fixed_indegree", "indegree": indegree})
        connection_counts.append(nest.num_connections)

    assert connection_counts == [expected_connections] * num_trials
