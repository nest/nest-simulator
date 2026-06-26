# -*- coding: utf-8 -*-
#
# test_issue_2629.py
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
Regression test for Issue #2629 (GitHub).

The issue was that ``DumpLayerConnections`` failed when a source layer was
connected to more than one target layer. The test ensures that this is no
longer the case.

For each connection between the specified source and target layer,
``DumpLayerConnections`` writes the following to file:

    source_node_id target_node_id weight delay dx dy [dz]

where (dx, dy [, dz]) is the displacement from source to target node.

This test uses the ``tmp_path`` Pytest fixture, which will provide a
temporary directory unique to the test invocation. ``tmp_path`` is a
``pathlib.Path`` object. Hence, the test also implicitly verifies that it
is possible to pass a ``pathlib.Path`` object as filename.
"""

import nest
import pytest


@pytest.fixture(scope="module")
def network():
    """Fixture for building network."""

    grid = nest.spatial.grid(shape=[2, 1])
    src_layer = nest.Create("iaf_psc_alpha", positions=grid)
    tgt_layer_1 = nest.Create("iaf_psc_alpha", positions=grid)
    tgt_layer_2 = nest.Create("iaf_psc_alpha", positions=grid)

    nest.Connect(src_layer, tgt_layer_1, "all_to_all")
    nest.Connect(src_layer, tgt_layer_2, "one_to_one")

    return src_layer, tgt_layer_1, tgt_layer_2


def test_dump_layer_connections_target_1(tmp_path, network):
    """Test that dumping connections with target layer 1 works."""

    src_layer, tgt_layer_1, _ = network

    fname_1 = tmp_path / "conns_1.txt"
    nest.DumpLayerConnections(src_layer, tgt_layer_1, "static_synapse", fname_1)
    expected_dump_1 = [
        "1 3 1 1 0 0",
        "1 4 1 1 0.5 0",
        "2 3 1 1 -0.5 0",
        "2 4 1 1 0 0",
    ]
    actual_dump_1 = fname_1.read_text().splitlines()
    assert actual_dump_1 == expected_dump_1


def test_dump_layer_connections_target_2(tmp_path, network):
    """Test that dumping connections with target layer 2 works."""

    src_layer, _, tgt_layer_2 = network

    fname_2 = tmp_path / "conns_2.txt"
    nest.DumpLayerConnections(src_layer, tgt_layer_2, "static_synapse", fname_2)
    expected_dump_2 = [
        "1 5 1 1 0 0",
        "2 6 1 1 0 0",
    ]
    actual_dump_2 = fname_2.read_text().splitlines()
    assert actual_dump_2 == expected_dump_2
