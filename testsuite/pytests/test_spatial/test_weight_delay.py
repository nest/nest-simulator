# -*- coding: utf-8 -*-
#
# test_weight_delay.py
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
Test setting weights and delays to arbitrary values limited by resolution.

The test checks that weights and delays in a spatial network can be set to
arbitrary values within the limits set by resolution. The test creates a layer
with a single row and uses linear functions for weights and delays. Expected
delays with resolution 0.04 are 1.0 0.96 0.96 0.92 0.92 0.88 0.84 and expected
weights are 1.0, 0.98, 0.96, 0.94, 0.92, 0.90, 0.88, 0.86, 0.84, 0.82.
"""

import nest
import numpy as np
import pytest


def build_network(layer_type):
    """Build spatial network with specified layer type."""

    nest.ResetKernel()
    nest.resolution = 0.04
    nest.use_compressed_spikes = False

    p1 = nest.CreateParameter("constant", {"value": 1.0})
    p2 = nest.CreateParameter("constant", {"value": -0.02})
    p3 = nest.CreateParameter("distance", {})
    linear_parameter = p1 + p2 * p3

    syn_spec = {
        "weight": linear_parameter,
        "delay": linear_parameter,
    }

    if layer_type == "grid":
        pos = nest.spatial.grid(
            shape=[10, 1],
            extent=[10.0, 1.0],
            center=[0.0, 0.0],
            edge_wrap=True,
        )
        conn_spec = {
            "rule": "pairwise_bernoulli",
            "use_on_source": False,
            "mask": {"grid": {"shape": [10, 1]}, "anchor": [0, 0]},
        }
    elif layer_type == "free":
        pos = nest.spatial.free(
            [[x, 0.0] for x in np.linspace(-4.5, 4.5, 10)],
            extent=[10.0, 1.0],
            edge_wrap=True,
        )
        conn_spec = {
            "rule": "pairwise_bernoulli",
            "use_on_source": False,
            "mask": {"rectangular": {"lower_left": [-0.5, -0.5], "upper_right": [9.5, 0.5]}},
        }

    src_layer = nest.Create("iaf_psc_alpha", positions=pos)
    tgt_layer = nest.Create("iaf_psc_alpha", positions=pos)

    nest.Connect(src_layer, tgt_layer, conn_spec=conn_spec, syn_spec=syn_spec)

    return src_layer, tgt_layer


@pytest.mark.parametrize("layer_type", ["grid", "free"])
def test_source_layer_nodes_dump(tmp_path, expected_source_nodes_dump, layer_type):
    """Test that source layer nodes dump mathces expectation."""

    src_layer, _ = build_network(layer_type)

    fname = tmp_path / f"{layer_type}_source_nodes.txt"
    nest.DumpLayerNodes(src_layer, fname)

    actual_source_nodes_dump = fname.read_text().splitlines()
    assert actual_source_nodes_dump == expected_source_nodes_dump


@pytest.mark.parametrize("layer_type", ["grid", "free"])
def test_target_layer_nodes_dump(tmp_path, expected_target_nodes_dump, layer_type):
    """Test that target layer nodes dump mathces expectation."""

    _, tgt_layer = build_network(layer_type)

    fname = tmp_path / f"{layer_type}_target_nodes.txt"
    nest.DumpLayerNodes(tgt_layer, fname)

    actual_target_nodes_dump = fname.read_text().splitlines()
    assert actual_target_nodes_dump == expected_target_nodes_dump


@pytest.mark.parametrize("layer_type", ["grid", "free"])
def test_layer_connections_dump(tmp_path, expected_conn_dump, layer_type):
    """Test that layer connections dump mathces expectation."""

    src_layer, tgt_layer = build_network(layer_type)

    fname = tmp_path / f"{layer_type}_layer_conns.txt"
    nest.DumpLayerConnections(src_layer, tgt_layer, "static_synapse", fname)

    # We need to sort results to be invariant against implementation-dependent output order
    actual_conn_dump = fname.read_text().splitlines()
    assert actual_conn_dump.sort() == expected_conn_dump.sort()


@pytest.fixture(scope="module")
def expected_source_nodes_dump():
    expected_source_nodes_dump = [
        "1 -4.5 0",
        "2 -3.5 0",
        "3 -2.5 0",
        "4 -1.5 0",
        "5 -0.5 0",
        "6 0.5 0",
        "7 1.5 0",
        "8 2.5 0",
        "9 3.5 0",
        "10 4.5 0",
    ]
    return expected_source_nodes_dump


@pytest.fixture(scope="module")
def expected_target_nodes_dump():
    expected_target_nodes_dump = [
        "11 -4.5 0",
        "12 -3.5 0",
        "13 -2.5 0",
        "14 -1.5 0",
        "15 -0.5 0",
        "16 0.5 0",
        "17 1.5 0",
        "18 2.5 0",
        "19 3.5 0",
        "20 4.5 0",
    ]
    return expected_target_nodes_dump


@pytest.fixture(scope="module")
def expected_conn_dump():
    expected_conn_dump = [
        "1 11 1 1 0 0",
        "1 12 0.98 1 1 0",
        "1 13 0.96 0.96 2 0",
        "1 14 0.94 0.96 3 0",
        "1 15 0.92 0.92 4 0",
        "1 16 0.9 0.92 -5 0",
        "1 17 0.92 0.92 -4 0",
        "1 18 0.94 0.96 -3 0",
        "1 19 0.96 0.96 -2 0",
        "1 20 0.98 1 -1 0",
        "2 11 0.98 1 -1 0",
        "2 12 1 1 0 0",
        "2 13 0.98 1 1 0",
        "2 14 0.96 0.96 2 0",
        "2 15 0.94 0.96 3 0",
        "2 16 0.92 0.92 4 0",
        "2 17 0.9 0.92 -5 0",
        "2 18 0.92 0.92 -4 0",
        "2 19 0.94 0.96 -3 0",
        "2 20 0.96 0.96 -2 0",
        "3 11 0.96 0.96 -2 0",
        "3 12 0.98 1 -1 0",
        "3 13 1 1 0 0",
        "3 14 0.98 1 1 0",
        "3 15 0.96 0.96 2 0",
        "3 16 0.94 0.96 3 0",
        "3 17 0.92 0.92 4 0",
        "3 18 0.9 0.92 -5 0",
        "3 19 0.92 0.92 -4 0",
        "3 20 0.94 0.96 -3 0",
        "4 11 0.94 0.96 -3 0",
        "4 12 0.96 0.96 -2 0",
        "4 13 0.98 1 -1 0",
        "4 14 1 1 0 0",
        "4 15 0.98 1 1 0",
        "4 16 0.96 0.96 2 0",
        "4 17 0.94 0.96 3 0",
        "4 18 0.92 0.92 4 0",
        "4 19 0.9 0.92 -5 0",
        "4 20 0.92 0.92 -4 0",
        "5 11 0.92 0.92 -4 0",
        "5 12 0.94 0.96 -3 0",
        "5 13 0.96 0.96 -2 0",
        "5 14 0.98 1 -1 0",
        "5 15 1 1 0 0",
        "5 16 0.98 1 1 0",
        "5 17 0.96 0.96 2 0",
        "5 18 0.94 0.96 3 0",
        "5 19 0.92 0.92 4 0",
        "5 20 0.9 0.92 -5 0",
        "6 11 0.9 0.92 -5 0",
        "6 12 0.92 0.92 -4 0",
        "6 13 0.94 0.96 -3 0",
        "6 14 0.96 0.96 -2 0",
        "6 15 0.98 1 -1 0",
        "6 16 1 1 0 0",
        "6 17 0.98 1 1 0",
        "6 18 0.96 0.96 2 0",
        "6 19 0.94 0.96 3 0",
        "6 20 0.92 0.92 4 0",
        "7 11 0.92 0.92 4 0",
        "7 12 0.9 0.92 -5 0",
        "7 13 0.92 0.92 -4 0",
        "7 14 0.94 0.96 -3 0",
        "7 15 0.96 0.96 -2 0",
        "7 16 0.98 1 -1 0",
        "7 17 1 1 0 0",
        "7 18 0.98 1 1 0",
        "7 19 0.96 0.96 2 0",
        "7 20 0.94 0.96 3 0",
        "8 11 0.94 0.96 3 0",
        "8 12 0.92 0.92 4 0",
        "8 13 0.9 0.92 -5 0",
        "8 14 0.92 0.92 -4 0",
        "8 15 0.94 0.96 -3 0",
        "8 16 0.96 0.96 -2 0",
        "8 17 0.98 1 -1 0",
        "8 18 1 1 0 0",
        "8 19 0.98 1 1 0",
        "8 20 0.96 0.96 2 0",
        "9 11 0.96 0.96 2 0",
        "9 12 0.94 0.96 3 0",
        "9 13 0.92 0.92 4 0",
        "9 14 0.9 0.92 -5 0",
        "9 15 0.92 0.92 -4 0",
        "9 16 0.94 0.96 -3 0",
        "9 17 0.96 0.96 -2 0",
        "9 18 0.98 1 -1 0",
        "9 19 1 1 0 0",
        "9 20 0.98 1 1 0",
        "10 11 0.98 1 1 0",
        "10 12 0.96 0.96 2 0",
        "10 13 0.94 0.96 3 0",
        "10 14 0.92 0.92 4 0",
        "10 15 0.9 0.92 -5 0",
        "10 16 0.92 0.92 -4 0",
        "10 17 0.94 0.96 -3 0",
        "10 18 0.96 0.96 -2 0",
        "10 19 0.98 1 -1 0",
        "10 20 1 1 0 0",
    ]
    return expected_conn_dump
