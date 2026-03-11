# -*- coding: utf-8 -*-
#
# test_issue_1085.py
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

"""
Regression test for Issue #1085 (GitHub).

This set of tests check that `GetConnections` filter by synapse label.
"""

import nest
import pytest


@pytest.fixture(scope="module")
def network():
    """
    Fixture for building network.
    """

    nodes = nest.Create("iaf_psc_alpha", 5)
    nest.Connect(
        nodes,
        nodes,
        conn_spec={"rule": "all_to_all"},
        syn_spec={"synapse_model": "static_synapse_lbl", "synapse_label": 123},
    )
    nest.Connect(
        nodes,
        nodes,
        conn_spec={"rule": "one_to_one"},
        syn_spec={"synapse_model": "static_synapse_lbl", "synapse_label": 456},
    )

    return nodes


@pytest.mark.parametrize(
    ("source_filter", "target_filter"),
    [(True, False), (False, True), (True, True)],
)
def test_getconnections_filter_by_synapse_label(request, source_filter, target_filter):
    """
    Test that `GetConnections` filter correctly by synapse label.

    The test is parametrized such that the following arguments will be passed
    to `GetConnections`:

        * `GetConnections(source=nodes, target=None, synapse_label=123)`
        * `GetConnections(source=None, target=nodes, synapse_label=123)`
        * `GetConnections(source=nodes, target=nodes, synapse_label=123)`

    The `nodes` are retrieved from the `network` fixture.
    """

    sources = request.getfixturevalue("network") if source_filter else None
    targets = request.getfixturevalue("network") if target_filter else None

    conns = nest.GetConnections(source=sources, target=targets, synapse_label=123)
    assert len(conns) == 25
