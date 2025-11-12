# -*- coding: utf-8 -*-
#
# test_ticket_881.py
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
from nest import spatial

"""
Regression test for Ticket #881.

Test ported from SLI regression test.
Ensure ConnectLayers creates the expected number of connections when running multithreaded.

Author: Hans Ekkehard Plesser
"""


NUM_THREADS = 4
NODE_POSITIONS = [[0.0, 0.0]] * 4
NUM_NODES = len(NODE_POSITIONS)
NUM_EXPECTED = NUM_NODES * NUM_NODES


def _create_layer():
    return nest.Create("iaf_psc_alpha", n=NUM_NODES, positions=spatial.free(NODE_POSITIONS))


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize(
    "conn_spec",
    (
        {"rule": "pairwise_bernoulli", "p": 1.0, "allow_autapses": True},
        {"rule": "pairwise_bernoulli", "p": 1.0, "allow_autapses": True},
        {"rule": "fixed_outdegree", "outdegree": NUM_NODES, "allow_autapses": True},
        {"rule": "fixed_indegree", "indegree": NUM_NODES, "allow_autapses": True},
    ),
)
def test_ticket_881_multithreaded_spatial_connectivity(conn_spec):
    """
    Ensure ConnectLayers establishes the full all-to-all connectivity for different configuration modes.
    """

    nest.ResetKernel()
    nest.local_num_threads = NUM_THREADS

    layer = _create_layer()

    nest.Connect(layer, layer, conn_spec=conn_spec)

    connections = nest.GetConnections()

    assert len(connections) == NUM_EXPECTED
