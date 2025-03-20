# -*- coding: utf-8 -*-
#
# test_axonal_delay.py
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
Test setting axonal and dendritic delays for models supporting this and those that don't, which must throw an exception.

The test checks that weights and delays in a spatial network can be set to arbitrary values within the limits set by
resolution. The test creates a layer with a single row and uses linear functions for weights and delays. Expected
combined dendritic and axonal delays with resolution 0.04 are 1.0 0.96 0.96 0.92 0.92 0.88 0.84. One model supports
setting dendritic and axonal delays and one does not, which is expected to throw an exception.
"""

import nest
import numpy as np
import pytest


def build_network(synapse_model):
    """Build spatial network."""

    nest.ResetKernel()
    nest.resolution = 0.04
    nest.use_compressed_spikes = False

    p1 = nest.CreateParameter("constant", {"value": 0.5})
    p2 = nest.CreateParameter("constant", {"value": -0.01})
    p3 = nest.CreateParameter("distance", {})
    linear_parameter = p1 + p2 * p3

    syn_spec = {
        "synapse_model": synapse_model,
        "weight": linear_parameter,
        "dendritic_delay": linear_parameter,
        "axonal_delay": linear_parameter,
    }
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

    src_layer = nest.Create("iaf_psc_alpha", positions=pos)
    tgt_layer = nest.Create("iaf_psc_alpha", positions=pos)

    nest.Connect(src_layer, tgt_layer, conn_spec=conn_spec, syn_spec=syn_spec)

    return src_layer, tgt_layer


def test_layer_connections_dump_success(tmp_path, expected_conn_dump):
    """Test that layer connections dump matches expectation."""

    synapse_model = "stdp_pl_synapse_hom_ax_delay"
    src_layer, tgt_layer = build_network(synapse_model)

    fname = tmp_path / f"layer_conns.txt"
    nest.DumpLayerConnections(src_layer, tgt_layer, synapse_model, fname)

    # We need to sort results to be invariant against implementation-dependent output order
    actual_conn_dump = fname.read_text().splitlines()
    assert actual_conn_dump.sort() == expected_conn_dump.sort()


def test_layer_connections_dump_failure():
    """Test that this synapse model is not compatible with the layer parameters."""

    with pytest.raises(nest.kernel.NESTError):
        build_network("static_synapse")


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
