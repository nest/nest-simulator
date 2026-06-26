# -*- coding: utf-8 -*-
#
# test_issue_1366.py
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
Regression test for Issue #1366 (GitHub).

This test checks that we are able to connect with `fixed_total_number` equal
to 1 when we have more than 1 virtual process and more than 1 node per process.
"""

import nest
import pytest


@pytest.mark.skipif_missing_threads
def test_fixed_total_number_multithreaded():
    """Ensure it is possible to connect with `fixed_total_number` equal to 1."""

    nest.ResetKernel()
    nest.total_num_virtual_procs = 4

    nodes = nest.Create("iaf_psc_alpha", 4)
    nest.Connect(nodes, nodes, conn_spec={"rule": "fixed_total_number", "N": 1})

    expected_conns = 1
    actual_conns = nest.GetKernelStatus("num_connections")

    assert actual_conns == expected_conns
