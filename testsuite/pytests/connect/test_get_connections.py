# -*- coding: utf-8 -*-
#
# test_get_connections.py
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
Tests for ``GetConnections``.

NOTE: This is supposed to be the pytest conversions of unittest tests in
test_getconnections.py and other GetConnections tests that may be placed
elsewhere.
"""

import nest
import pandas as pd
import pandas.testing as pdtest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_get_connections_all_with_node_collection():
    """
    Test that ``GetConnections`` works with ``NodeCollection``.
    """

    nodes = nest.Create("iaf_psc_alpha", 3)
    nest.Connect(nodes, nodes)

    conns_no_args = nest.GetConnections()
    conns_nodes_args = nest.GetConnections(nodes, nodes)

    assert conns_nodes_args == conns_no_args


def test_get_connections_with_node_collection_step():
    """
    Test that ``GetConnections`` works with ``NodeCollection`` sliced in step.
    """

    nodes = nest.Create("iaf_psc_alpha", 3)
    nest.Connect(nodes, nodes)

    conns = nest.GetConnections(nodes[::2])
    actual_sources = conns.get("source")
    actual_targets = conns.get("target")

    expected_sources = [1, 1, 1, 3, 3, 3]
    expected_targets = [1, 2, 3, 1, 2, 3]

    assert actual_sources == expected_sources
    assert actual_targets == expected_targets


def test_get_connections_with_sliced_node_collection():
    """Test that ``GetConnections`` works with sliced ``NodeCollection``."""

    nodes = nest.Create("iaf_psc_alpha", 11)
    nest.Connect(nodes, nodes)

    conns = nest.GetConnections(nodes[1:9:3])
    actual_sources = conns.get("source")

    expected_sources = [2] * 11 + [5] * 11 + [8] * 11
    assert actual_sources == expected_sources


def test_get_connections_with_sliced_node_collection_2():
    """Test that ``GetConnections`` works with sliced ``NodeCollection``."""

    nodes = nest.Create("iaf_psc_alpha", 11)
    nest.Connect(nodes, nodes)

    # ([ 2 3 4 ] + [ 8 9 10 11 ])[::3] -> [2 8 11]
    conns = nest.GetConnections((nodes[1:4] + nodes[7:])[::3])
    actual_sources = conns.get("source")

    expected_sources = [2] * 11 + [8] * 11 + [11] * 11
    assert actual_sources == expected_sources


def test_get_connections_bad_source_raises():
    """Test that ``GetConnections`` raises an error when called with 0."""

    nodes = nest.Create("iaf_psc_alpha", 3)
    nest.Connect(nodes, nodes)

    with pytest.raises(TypeError):
        nest.GetConnections([0, 1])


def test_get_connections_correct_table_with_node_collection_step():
    """
    Test that ``GetConnections`` table from ``NodeCollection`` sliced in step match expectations.
    """

    nodes = nest.Create("iaf_psc_alpha", 3)
    nest.Connect(nodes, nodes)

    conns = nest.GetConnections(nodes[::2])
    actual_row = [
        conns.get("source")[3],
        conns.get("target")[3],
        conns.get("target_thread")[3],
        conns.get("synapse_id")[3],
        conns.get("port")[3],
    ]

    expected_syn_id = nest.GetDefaults("static_synapse", "synapse_modelid")
    expected_row = [3, 1, 0, expected_syn_id, 6]

    assert actual_row == expected_row


def test_get_connections_correct_table_with_node_collection_index():
    """
    Test that ``GetConnections`` table from ``NodeCollection`` index match expectations.
    """

    nodes = nest.Create("iaf_psc_alpha", 3)
    nest.Connect(nodes, nodes)

    actual_conns = pd.DataFrame(
        nest.GetConnections(nodes[0]).get(["source", "target", "target_thread", "synapse_id", "port"])
    )

    expected_syn_id = nest.GetDefaults("static_synapse", "synapse_modelid")
    expected_conns = pd.DataFrame(
        [
            [1, 1, 0, expected_syn_id, 0],
            [1, 2, 0, expected_syn_id, 1],
            [1, 3, 0, expected_syn_id, 2],
        ],
        columns=["source", "target", "target_thread", "synapse_id", "port"],
    )

    pdtest.assert_frame_equal(actual_conns, expected_conns)
