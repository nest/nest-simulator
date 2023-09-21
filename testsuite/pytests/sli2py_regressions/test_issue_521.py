# -*- coding: utf-8 -*-
#
# test_issue_521.py
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
Regression test for Issue #521 (GitHub).
"""

import nest
import pytest


@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("num_threads", [1, 2, 5, 11])
def test_hpc_synapse_correct_global_target_node_id_multithreaded(num_threads):
    """
    Ensure that *_hpc connections also return correct global target node ID on ``GetConnections``.

    The test connects two neurons from a larger ``NodeCollection``. The chosen node IDs of these
    neurons together with the choice of number of threads ensures variation of thread placement.
    """

    nest.ResetKernel()
    nest.local_num_threads = num_threads

    nc = nest.Create("iaf_psc_alpha", 20)

    # connect two neurons
    n1 = nc[4]
    n2 = nc[12]
    nest.Connect(n1, n2, syn_spec={"synapse_model": "static_synapse"})
    nest.Connect(n1, n2, syn_spec={"synapse_model": "static_synapse_hpc"})

    conns = nest.GetConnections()

    assert conns.get("target")[0] == n2.global_id

    # test for static_synapse_hpc
    assert conns.get("target")[1] == n2.global_id
