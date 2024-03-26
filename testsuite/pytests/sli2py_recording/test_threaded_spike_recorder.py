# -*- coding: utf-8 -*-
#
# test_threaded_spike_recorder.py
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
Test that ``spike_recorder`` behaves as expected in multithreaded simulations.
"""

import nest
import pandas as pd
import pandas.testing as pdtest
import pytest

pytestmark = pytest.mark.skipif_missing_threads


def simulator(num_threads):
    """
    Simulate the system with provided number of threads and return recorded events.
    """

    nest.ResetKernel()
    nest.local_num_threads = num_threads

    num_nrns = 10  # should not be divisble by thread number

    nrns = nest.Create("iaf_psc_alpha", num_nrns, params={"I_e": 376.0})
    srec = nest.Create("spike_recorder")

    nest.Connect(nrns, srec)

    nest.Simulate(200.0)

    df = pd.DataFrame(srec.get("events"))

    return df


def test_threaded_spike_recorder():
    """
    Test that recorded spike times are independent of number of threads.
    """

    df_t1 = simulator(1)
    df_t3 = simulator(3)

    df_t1.sort_values(by=["times", "senders"], inplace=True, ignore_index=True)
    df_t3.sort_values(by=["times", "senders"], inplace=True, ignore_index=True)

    pdtest.assert_frame_equal(df_t1, df_t3)
