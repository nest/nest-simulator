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
        # Test 1: pairwise_bernoulli_on_source (driver = source layer)
        {"rule": "pairwise_bernoulli", "p": 1.0, "allow_autapses": True},
        # Test 2: pairwise_bernoulli_on_target (driver = target layer)
        {"rule": "pairwise_bernoulli", "p": 1.0, "use_on_source": True, "allow_autapses": True},
        # Test 3: pairwise_bernoulli_on_source with number_of_connections (fixed fan-out)
        {"rule": "fixed_outdegree", "outdegree": NUM_NODES, "allow_autapses": True},
        # Test 4: pairwise_bernoulli_on_target with number_of_connections (fixed fan-in)
        {"rule": "fixed_indegree", "indegree": NUM_NODES, "allow_autapses": True},
    ),
)
def test_ticket_881_multithreaded_spatial_connectivity(conn_spec):
    """
    Ensure ConnectLayers establishes the full all-to-all connectivity for different configuration modes.

    This test corresponds to the SLI regression test ticket-881.sli which tests:
    1. pairwise_bernoulli_on_source (default pairwise_bernoulli, driver=source)
    2. pairwise_bernoulli_on_target (pairwise_bernoulli with use_on_source=True, driver=target)
    3. fixed_outdegree (prescribed fan-out, equivalent to pairwise_bernoulli_on_source with number_of_connections)
    4. fixed_indegree (prescribed fan-in, equivalent to pairwise_bernoulli_on_target with number_of_connections)
    """

    nest.ResetKernel()
    nest.local_num_threads = NUM_THREADS

    layer = _create_layer()

    nest.Connect(layer, layer, conn_spec=conn_spec)

    connections = nest.GetConnections()

    assert len(connections) == NUM_EXPECTED
