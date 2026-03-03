# -*- coding: utf-8 -*-
#
# test_ticket_689.py
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

"""
Regression test for Ticket #689.

Test ported from SLI regression test.
Ensure GetConnections works correctly with hpc synapses.

Authors: Susanne Kunkel and Maximilian Schmidt, 2012-02-19
"""


def test_ticket_689_getconnections_with_hpc_synapse():
    """
    Ensure retrieving connections with stdp_pl_synapse_hom_hpc succeeds.
    """

    nest.ResetKernel()
    nest.total_num_virtual_procs = 1

    n1 = nest.Create("iaf_psc_alpha")
    n2 = nest.Create("iaf_psc_alpha")

    nest.Connect(n1, n2, conn_spec={"rule": "all_to_all"}, syn_spec="stdp_pl_synapse_hom_hpc")

    connections = nest.GetConnections(target=n2)

    assert len(connections) == 1
