# -*- coding: utf-8 -*-
#
# test_issue_4004.py
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
Regression test for issue #4004.

With compressed spikes, a target-data exchange of several rounds added entries of an earlier round a
second time, so that some connections delivered a spike more than once. The test is relevant with
several MPI processes and is linked into ``mpi_direct``.
"""

import nest
import pytest


def test_each_connection_delivers_spike_once_with_several_exchange_rounds():
    """
    Every source spikes once, so the targets on each process receive one spike per local connection.

    The small target-data buffer forces several exchange rounds; the sources on one virtual process
    have more target data than the others, so that the chunks of a round fill unevenly.
    """

    nest.ResetKernel()
    nest.set(use_compressed_spikes=True)
    nest.max_buffer_size_target_data = 4 * nest.NumProcesses()

    n = 10 * nest.total_num_virtual_procs
    sources = nest.Create("parrot_neuron", n)
    targets = nest.Create("parrot_neuron", n)
    nest.Connect(nest.Create("spike_generator", params={"spike_times": [1.0]}), sources)

    nest.CopyModel("static_synapse", "second_static_synapse")
    one_vp = sources[:: nest.total_num_virtual_procs]
    for synapse_model in ("static_synapse", "second_static_synapse"):
        nest.Connect(one_vp, targets, syn_spec={"synapse_model": synapse_model})
    nest.Connect(sources, targets, {"rule": "fixed_indegree", "indegree": 3})

    recorder = nest.Create("spike_recorder")
    nest.Connect(targets, recorder)
    nest.Simulate(5.0)

    assert recorder.n_events == len(nest.GetConnections(source=sources, target=targets))
