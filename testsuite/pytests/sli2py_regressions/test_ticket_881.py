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

node_pos = [[0.0, 0.0]] * 4
num_nodes = len(node_pos)
num_expected = num_nodes * num_nodes

# Map SLI connection_type to NEST 3 rule/use_on_source
connection_specs = [
    # pairwise_bernoulli_on_source: use_on_source=True
    {"rule": "pairwise_bernoulli", "use_on_source": True},
    # pairwise_bernoulli_on_target: use_on_source=False
    {"rule": "pairwise_bernoulli", "use_on_source": False},
    # pairwise_bernoulli_on_source with number_of_connections
    #    {"rule": "pairwise_bernoulli", "use_on_source": True, "number_of_connections": num_nodes},
    # pairwise_bernoulli_on_target with number_of_connections
    #   {"rule": "pairwise_bernoulli", "use_on_source": False, "number_of_connections": num_nodes},
]


@pytest.mark.parametrize("conn_spec", connection_specs)
def test_connectlayers_all_to_all(conn_spec):
    """
    Regression test for Ticket #881.

    Ensure ConnectLayers creates the correct number of connections in all-to-all scenarios with multiple threads.
    Uses NEST 3 idioms for pairwise_bernoulli connection rules.
    """
    nest.ResetKernel()
    nest.local_num_threads = 4
    layer = nest.Create("iaf_psc_alpha", positions=node_pos)
    nest.Connect(layer, layer, conn_spec=conn_spec)
    conns = nest.GetConnections()
    assert len(conns) == num_expected, f"Expected {num_expected} connections, got {len(conns)}"
