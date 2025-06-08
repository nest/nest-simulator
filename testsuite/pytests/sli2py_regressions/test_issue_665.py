# -*- coding: utf-8 -*-
#
# test_issue_665.py
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
Regression test for Issue #665 (GitHub).

This test ensures that ConnectLayers correctly handles devices with thread
siblings as sources and targets.
"""

import nest
import pytest


@pytest.mark.skipif_not_threaded
def test_connect_layers_with_devices():
    """
    Test that ConnectLayers correctly handles devices with thread siblings.
    """
    # Define connection specifications
    connspec = {"connection_type": "pairwise_bernoulli_on_source"}

    # Reset the kernel and set the number of threads
    nest.ResetKernel()
    nest.SetKernelStatus({"local_num_threads": 4})

    # Create layers
    pgl = nest.CreateLayer({"elements": "poisson_generator", "shape": [1, 1]})
    nnl = nest.CreateLayer({"elements": "iaf_psc_alpha", "shape": [2, 2]})

    # Connect layers
    nest.ConnectLayers(pgl, nnl, connspec)

    # Get connections and targets
    src = nest.GetNodes(pgl)[0]
    tgts = nest.GetNodes(nnl)
    conns = nest.GetConnections(source=src)
    ctgts = sorted(conn[1] for conn in conns)

    # Check if targets match expected
    assert tgts == ctgts

    # Test different connection types
    for conn_type in ["pairwise_bernoulli_on_source", "pairwise_bernoulli_on_target"]:
        connspec["connection_type"] = conn_type
        nest.ConnectLayers(pgl, nnl, connspec)
        conns = nest.GetConnections(source=src)
        ctgts = sorted(conn[1] for conn in conns)
        assert tgts == ctgts

    # Test with specific number of connections
    connspec["connection_type"] = "pairwise_bernoulli_on_source"
    connspec["number_of_connections"] = 1
    nest.ConnectLayers(pgl, nnl, connspec)
    conns = nest.GetConnections(source=src)
    ctgts = sorted(conn[1] for conn in conns)
    assert tgts == ctgts

    connspec["connection_type"] = "pairwise_bernoulli_on_target"
    connspec["number_of_connections"] = 4
    connspec["allow_multapses"] = False
    nest.ConnectLayers(pgl, nnl, connspec)
    conns = nest.GetConnections(source=src)
    ctgts = sorted(conn[1] for conn in conns)
    assert tgts == ctgts

    # Second set of tests: Neuron layer to single recorder
    srl = nest.CreateLayer({"elements": "spike_recorder", "shape": [1, 1]})
    nest.ConnectLayers(nnl, srl, connspec)

    tgt = nest.GetNodes(srl)[0]
    srcs = nest.GetNodes(nnl)
    conns = nest.GetConnections(target=tgt)
    csrcs = sorted(conn[0] for conn in conns)

    assert srcs == csrcs

    # Test different connection types for layer to recorder
    for conn_type in ["pairwise_bernoulli_on_source", "pairwise_bernoulli_on_target"]:
        connspec["connection_type"] = conn_type
        nest.ConnectLayers(nnl, srl, connspec)
        conns = nest.GetConnections(target=tgt)
        csrcs = sorted(conn[0] for conn in conns)
        assert srcs == csrcs

    # Test with specific number of connections for layer to recorder
    connspec["connection_type"] = "pairwise_bernoulli_on_source"
    connspec["number_of_connections"] = 4
    connspec["allow_multapses"] = False
    nest.ConnectLayers(nnl, srl, connspec)
    conns = nest.GetConnections(target=tgt)
    csrcs = sorted(conn[0] for conn in conns)
    assert srcs == csrcs

    connspec["connection_type"] = "pairwise_bernoulli_on_target"
    connspec["number_of_connections"] = 1
    nest.ConnectLayers(nnl, srl, connspec)
    conns = nest.GetConnections(target=tgt)
    csrcs = sorted(conn[0] for conn in conns)
    assert srcs == csrcs
