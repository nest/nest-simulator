# -*- coding: utf-8 -*-
#
# test_issue_3108.py
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


def test_get_spatial_with_slice_and_mpi():
    """
    Test that spatial information can be collect from slice of layer.
    """

    num_neurons = 7
    pick = 6

    nest.ResetKernel()
    layer = nest.Create(
        model="parrot_neuron",
        n=num_neurons,
        positions=nest.spatial.free(pos=nest.random.uniform(min=-1, max=1), num_dimensions=2, edge_wrap=False),
    )
    layer[pick].spatial


def test_connect_with_slice_and_mpi():
    """
    Test that connection with sliced layer is possible on multiple mpi processes.
    """

    num_neurons = 5
    pick = 3

    nest.ResetKernel()
    layer = nest.Create(
        model="parrot_neuron",
        n=num_neurons,
        positions=nest.spatial.free(pos=nest.random.uniform(min=-1, max=1), num_dimensions=2, edge_wrap=False),
    )

    # space-dependent syn_spec passed only to force use of ConnectLayers
    nest.Connect(layer[pick], layer, {"rule": "pairwise_bernoulli", "p": 1.0}, {"weight": nest.spatial.distance})
