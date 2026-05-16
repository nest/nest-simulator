# -*- coding: utf-8 -*-
#
# test_multiple_random_source_stepping.py
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
Test NEST's behavior when using multiple random sources and different stepping schemes.

The test checks whether simulations of a network with more than one node
consuming random numbers creates identical output under different stepping
regimes, e.g., 1x10.0ms vs 100x0.1ms.

.. note::
    This test works well only if the stepping interval is a multiple of the
    minimal delay. Otherwise, random numbers will be fed to consumers in
    different orders, as nodes are updated for parts of minimal delay periods
    only.
"""

import nest
import numpy.testing as nptest


def run_sim(interval, steppings):
    """Single simulation run."""

    nest.ResetKernel()

    pg1 = nest.Create("poisson_generator_ps", params={"rate": 1000.0})
    pg2 = nest.Create("poisson_generator_ps", params={"rate": 1000.0})
    sr1 = nest.Create("spike_recorder")
    sr2 = nest.Create("spike_recorder")
    nest.Connect(pg1, sr1)
    nest.Connect(pg2, sr2)

    for _ in range(steppings):
        nest.Simulate(interval)

    st1 = sr1.events["times"]
    st2 = sr2.events["times"]

    return st1, st2


def test_multiple_random_source_stepping():
    st1_ref, st2_ref = run_sim(10.0, 1)
    st1_run1, st2_run1 = run_sim(5.0, 2)
    st1_run2, st2_run2 = run_sim(1.0, 10)

    nptest.assert_array_equal(st1_run1, st1_ref)
    nptest.assert_array_equal(st2_run1, st2_ref)
    nptest.assert_array_equal(st1_run2, st1_ref)
    nptest.assert_array_equal(st2_run2, st2_ref)
