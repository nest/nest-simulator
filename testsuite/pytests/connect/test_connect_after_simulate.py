# -*- coding: utf-8 -*-
#
# test_connect_after_simulate.py
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
Tests that it is possible to create connections between two
Simulate calls.
Checks that the number of connections that GetConnection
returns and the number of detected spikes increases accordingly
when another connection is created.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


@pytest.mark.parametrize("use_compressed_spikes", [True, False])
def test_connections_before_after_simulate(use_compressed_spikes):
    """Pre-condition:
    Test that GetConnection returns the same connection handles before
    and after calling Simulate for the case that connections are sorted
    by source and for the case that they are not sorted"""

    nest.SetKernelStatus({"use_compressed_spikes": use_compressed_spikes})

    neurons = nest.Create("iaf_psc_alpha", 10)
    nest.Connect(neurons, neurons, conn_spec={"rule": "fixed_indegree", "indegree": 2})

    connections_before = nest.GetConnections()

    nest.Simulate(1.0)

    connections_after = nest.GetConnections()

    assert connections_before == connections_after


@pytest.mark.parametrize("use_compressed_spikes", [True, False])
def test_connect_after_simulate(use_compressed_spikes):
    """
    The actual test for sorted and unsorted connections
    """
    nest.SetKernelStatus({"use_compressed_spikes": use_compressed_spikes})

    neuron = nest.Create("iaf_psc_delta", params={"I_e": 500.0})
    parrot = nest.Create("parrot_neuron")
    dummies = nest.Create("iaf_psc_delta", 10)
    recorder = nest.Create("spike_recorder")

    nest.Connect(neuron, parrot)
    nest.Connect(dummies, dummies, conn_spec={"rule": "all_to_all"})
    nest.Connect(parrot, recorder)

    # Neuron will spike once
    nest.Simulate(20.0)

    connections = nest.GetConnections(target=parrot)
    assert len(connections) == 1
    assert connections[0].get("port") == 0
    # One spike, one connection to parrot -> 1 event
    assert recorder.n_events == 1

    nest.Connect(neuron, parrot)

    connections = nest.GetConnections(target=parrot)
    assert len(connections) == 2
    assert connections[0].get("port") == 0

    if use_compressed_spikes:
        nest.GetConnections(target=parrot)[1].get("port") == 1
    else:
        nest.GetConnections(target=parrot)[1].get("port") == 101

    # Neuron will spike once more
    nest.Simulate(20.0)

    connections = nest.GetConnections(target=parrot)
    assert len(connections) == 2
    assert connections[0].get("port") == 0

    if use_compressed_spikes:
        nest.GetConnections(target=parrot)[0].get("port") == 1
    else:
        nest.GetConnections(target=parrot)[0].get("port") == 101
    # One spike from before, additionally 1 more spike,
    # now 2 connections to parrot -> 3 events in total
    assert recorder.n_events == 3
