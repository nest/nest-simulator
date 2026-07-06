# -*- coding: utf-8 -*-
#
# test_issue_3106.py
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


@pytest.mark.skipif_missing_threads
def test_connect_with_threads_slice_and_mpi():
    """
    Test that connection with sliced layer is possible on multiple threads.
    """

    num_neurons = 10
    nest.local_num_threads = 4

    layer = nest.Create(
        model="parrot_neuron",
        n=num_neurons,
        positions=nest.spatial.free(pos=nest.random.uniform(min=-1, max=1), num_dimensions=2, edge_wrap=False),
    )

    tgts = layer[::3]

    # Distance-dependent weight forces use of layer-connect
    nest.Connect(layer, tgts, {"rule": "pairwise_bernoulli", "p": 1}, {"weight": nest.spatial.distance})
    # nest.Connect(layer, tgts, {"rule": "fixed_indegree", "indegree": 5})

    assert nest.num_connections == len(layer) * len(tgts)
