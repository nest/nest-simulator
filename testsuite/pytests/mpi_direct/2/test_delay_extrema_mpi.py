# -*- coding: utf-8 -*-
#
# test_delay_extrema_mpi.py
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
Test that the delay extrema are the same on all MPI ranks.

Each neuron is connected to itself with its own delay, so that the delay
extrema of the connections local to a rank differ from the global ones.
"""

import nest
import pytest

MIN_DELAY = 1.0
MAX_DELAY = 5.0


@pytest.fixture
def neurons():
    assert nest.NumProcesses() > 1, "Test is only relevant if we use multiple MPI ranks."

    nest.ResetKernel()
    nrns = nest.Create("parrot_neuron", 2)
    nest.Connect(nrns[0], nrns[0], syn_spec={"delay": MIN_DELAY})
    nest.Connect(nrns[1], nrns[1], syn_spec={"delay": MAX_DELAY})
    return nrns


def assert_global_delay_extrema():
    assert nest.min_delay == MIN_DELAY
    assert nest.max_delay == MAX_DELAY


def test_delay_extrema_after_prepare(neurons):
    """Querying the kernel status after Prepare must not make the extrema rank-local."""

    nest.Prepare()
    assert_global_delay_extrema()
    nest.Run(10.0)
    nest.Cleanup()
    assert_global_delay_extrema()


def test_delay_extrema_after_get_connections(neurons):
    """Simulate after GetConnections must use the global extrema on all ranks."""

    nest.GetConnections()
    nest.Simulate(10.0)
    assert_global_delay_extrema()
