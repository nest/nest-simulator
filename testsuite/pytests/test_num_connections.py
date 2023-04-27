# -*- coding: utf-8 -*-
#
# test_num_connections.py
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
Test that the kernel variable num_connections reports the correct total number of VP-local connections and that
synapse-type specific num_connections report the correct numbers of VP-local synapses of the specific type.
"""

import pytest
import nest


@pytest.mark.skipif_missing_threads
def test_num_connections():
    """
    Set the total number of VPs to N_VP, create N neurons and connect them using static and STDP synapses and a
    fixed-indegree rule with indegrees K_static and K_stdp, respectively. Query the number of connections from the
    kernel and the synapse types and compare to the expected values.
    """

    N = 100  # total number of neurons
    K_static = 20   # number of incoming static connections per neuron
    K_stdp = 80   # number of incoming STDP connections per neuron
    N_VP = 4    # number of virtual processes
    expected_num_static_connections_per_vp = N * K_static
    expected_num_stdp_connections_per_vp = N * K_stdp
    expected_num_connections_per_vp = expected_num_static_connections_per_vp + expected_num_stdp_connections_per_vp

    nest.ResetKernel()
    nest.SetKernelStatus({"total_num_virtual_procs": N_VP})
    neurons = nest.Create("iaf_psc_alpha", N)
    nest.Connect(neurons, neurons,
                 conn_spec={"rule": "fixed_indegree", "indegree": K_static},
                 syn_spec={"synapse_model": "static_synapse"})
    nest.Connect(neurons, neurons,
                 conn_spec={"rule": "fixed_indegree", "indegree": K_stdp},
                 syn_spec={"synapse_model": "stdp_synapse"})
    actual_num_connections_per_vp = nest.GetKernelStatus("num_connections")
    actual_num_static_connections_per_vp = nest.GetDefaults("static_synapse", ["num_connections"])[0]
    actual_num_stdp_connections_per_vp = nest.GetDefaults("stdp_synapse", ["num_connections"])[0]

    assert(actual_num_connections_per_vp == expected_num_connections_per_vp)
    assert(actual_num_static_connections_per_vp == expected_num_static_connections_per_vp)
    assert(actual_num_stdp_connections_per_vp == expected_num_stdp_connections_per_vp)
