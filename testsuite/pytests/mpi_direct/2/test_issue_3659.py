# -*- coding: utf-8 -*-
#
# test_issue_3659.py
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

"""
Preliminary regression tests for issue #3659.
"""


def test_ensure_parallel_conn_array_weight_blocked():
    """
    Since setting connection parameters with arrays works incorrectly under MPI, it must be blocked for now.
    """

    assert nest.NumProcesses() > 1, "Test is only relevant if we use multiple MPI ranks."

    nrns = nest.Create("iaf_psc_alpha", 2)
    nest.Connect(nrns, nrns)
    conns = nest.GetConnections()

    with pytest.raises(NotImplementedError):
        conns.set({"weight": [21.0, 22.0, 23.0, 24.0]})
