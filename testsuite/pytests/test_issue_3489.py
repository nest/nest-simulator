# -*- coding: utf-8 -*-
#
# test_issue_3489.py
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


@pytest.mark.parametrize("compressed", [True, False])
@pytest.mark.parametrize("simtime", [10.9, 11, 11.1])
def test_spike_delivered_before_connection_removed(compressed, simtime):
    """
    Test that spikes are properly transmitted before the connection infrastructure
    is updated.

    Simtime 10.9 means that Simulate() terminates before the test spike is delivered
    and as we then step in 0.1 ms steps, we ensure that connection updates happen only
    after spikes have been delivered at slice boundaries.
    """

    nest.ResetKernel()
    nest.use_compressed_spikes = compressed

    nodes_e = nest.Create("parrot_neuron", 3)
    sg = nest.Create("spike_generator", params={"spike_times": [9.7]})

    nest.Connect(sg, nodes_e[1])  # only neuron with GID 2 gets input
    nest.Connect(nodes_e[:2], nodes_e[2])  # neurons with GIDs 1 and 2 connect to 3

    # Neuron 2 spikes at 10.7, spike is stored, but not yet delivered
    # If we simulated here until 11.1, the spike would be delivered here and all is fine
    nest.Simulate(simtime)

    nest.Disconnect(nodes_e[0], nodes_e[2])

    while nest.biological_time < 11.1:
        print("TIME:", nest.biological_time)
        nest.Simulate(0.1)  # This call will trigger delivery of the spike, but now the connection is gone -> segfault
