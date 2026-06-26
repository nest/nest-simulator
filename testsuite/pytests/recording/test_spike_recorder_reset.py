# -*- coding: utf-8 -*-
#
# test_spike_recorder_reset.py
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
Test that resetting ``spike_recorder`` works as expected.
"""

import nest
import numpy as np
import numpy.testing as nptest


def test_spike_recorder_reset():
    """
    Test resetting of ``spike_recorder``.

    The test feeds the ``spike_recorder`` with a given set of spikes,
    checks if they are recorded properly to memory, then resets, then
    records a few more and checks.
    """

    nest.ResetKernel()

    spike_times = np.linspace(10.0, 200.0, 20)

    sgen = nest.Create("spike_generator", params={"spike_times": spike_times})
    srec = nest.Create("spike_recorder")

    nest.Connect(sgen, srec)

    nest.Simulate(105.0)  # should record spikes 10..100 -> 10 spikes

    nptest.assert_array_equal(srec.events["times"], spike_times[:10])

    # reset
    srec.n_events = 0

    assert srec.n_events == 0
    assert len(srec.events["times"]) == 0

    # simulate more, till 160
    nest.Simulate(55.0)  # spikes 110 .. 160 -> 6 spikes

    nptest.assert_array_equal(srec.events["times"], spike_times[10:16])
