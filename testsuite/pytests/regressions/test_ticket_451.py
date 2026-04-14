# -*- coding: utf-8 -*-
#
# test_ticket_451.py
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
Regression test for Ticket #451.

Test ported from SLI regression test.
Guard against infinite loops in random connection routines when multapse/autapse constraints make the request
infeasible.
"""


def _create_populations():
    nodes = nest.Create("iaf_psc_alpha", 3)
    first = nodes[:1]
    return nodes, first


IMPOSSIBLE_SCENARIOS = (
    (
        {"rule": "fixed_indegree", "indegree": 4, "allow_multapses": False, "allow_autapses": True},
        False,
        "Indegree",
    ),
    (
        {"rule": "fixed_indegree", "indegree": 4, "allow_multapses": False, "allow_autapses": False},
        False,
        "Indegree",
    ),
    (
        {"rule": "fixed_outdegree", "outdegree": 4, "allow_multapses": False, "allow_autapses": True},
        True,
        "Outdegree",
    ),
    (
        {"rule": "fixed_outdegree", "outdegree": 4, "allow_multapses": False, "allow_autapses": False},
        True,
        "Outdegree",
    ),
)


@pytest.mark.parametrize("conn_spec,use_first_as_source,error_pattern", IMPOSSIBLE_SCENARIOS)
def test_ticket_451_rejects_impossible_random_connect_requests(conn_spec, use_first_as_source, error_pattern):
    """
    Ensure random connection rules raise for infeasible multapse/autapse constraints.
    """

    nest.ResetKernel()
    nodes, first = _create_populations()

    sources, targets = (first, nodes) if use_first_as_source else (nodes, first)

    with pytest.raises(nest.NESTErrors.BadProperty, match=error_pattern):
        nest.Connect(sources, targets, conn_spec)


VALID_SCENARIOS = (
    ({"rule": "fixed_indegree", "indegree": 3, "allow_multapses": False, "allow_autapses": True}, False, 3),
    ({"rule": "fixed_indegree", "indegree": 2, "allow_multapses": False, "allow_autapses": True}, False, 2),
    ({"rule": "fixed_indegree", "indegree": 2, "allow_multapses": False, "allow_autapses": False}, False, 2),
    ({"rule": "fixed_indegree", "indegree": 1, "allow_multapses": True, "allow_autapses": True}, False, 1),
    ({"rule": "fixed_outdegree", "outdegree": 3, "allow_multapses": False, "allow_autapses": True}, True, 3),
    ({"rule": "fixed_outdegree", "outdegree": 2, "allow_multapses": False, "allow_autapses": True}, True, 2),
    ({"rule": "fixed_outdegree", "outdegree": 2, "allow_multapses": False, "allow_autapses": False}, True, 2),
    ({"rule": "fixed_outdegree", "outdegree": 1, "allow_multapses": True, "allow_autapses": False}, True, 1),
)


@pytest.mark.parametrize("conn_spec,use_first_as_source,expected_connections", VALID_SCENARIOS)
def test_ticket_451_accepts_feasible_random_connect_requests(conn_spec, use_first_as_source, expected_connections):
    """
    Ensure random connection rules succeed when the requested indegree/outdegree is feasible.
    """

    nest.ResetKernel()
    nodes, first = _create_populations()

    sources, targets = (first, nodes) if use_first_as_source else (nodes, first)

    nest.Connect(sources, targets, conn_spec)

    connections = nest.GetConnections(source=sources, target=targets)

    assert len(connections) == expected_connections
