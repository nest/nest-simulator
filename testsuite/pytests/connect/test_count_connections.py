# -*- coding: utf-8 -*-
#
# test_count_connections.py
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

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def test_count_connections():
    """
    Test that correct number of connections is created for fixed_indegree.

    Description:
    This test uses fixed-indegree Connect to connect a net of 100 neurons
    to itself, with 89 connections per neuron, a total of 8900 connections.
    It is checked whether the total number of connections and the number of
    static connections equals 8900. Then additional 8900 connections of type
    stdp_synapse are created. It is checked whether the total number of
    connections is 17800 and the number of stdp connections is 8900.

    Author: Susanne Kunkel, 2013-03-25
    """

    num_neurons = 100
    indegree = 89
    n_vp = 4
    expected_num_conns = num_neurons * indegree

    nest.total_num_virtual_procs = 4

    n = nest.Create("parrot_neuron", n=num_neurons)
    nest.Connect(n, n, conn_spec={"rule": "fixed_indegree", "indegree": indegree}, syn_spec="static_synapse")

    assert nest.num_connections == expected_num_conns
    assert nest.GetDefaults("static_synapse")["num_connections"] == expected_num_conns

    nest.Connect(n, n, conn_spec={"rule": "fixed_indegree", "indegree": indegree}, syn_spec="stdp_synapse")

    assert nest.num_connections == 2 * expected_num_conns
    assert nest.GetDefaults("static_synapse")["num_connections"] == expected_num_conns
    assert nest.GetDefaults("stdp_synapse")["num_connections"] == expected_num_conns
