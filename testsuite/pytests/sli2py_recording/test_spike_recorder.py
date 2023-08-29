# -*- coding: utf-8 -*-
#
# test_spike_recorder.py
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
Test that ``spike_recorder`` behaves as expected.
"""

import nest
import numpy.testing as nptest
import pytest


def simulator(resolution):
    """
    Simulate the system with provided time resolution and return spike times.
    """

    nest.ResetKernel()
    nest.resolution = resolution

    spike_times = [0.1, 5.0, 5.3, 5.33, 5.4, 5.9, 6.0]

    sgen = nest.Create("spike_generator", params={"spike_times": spike_times, "precise_times": True})
    srec = nest.Create("spike_recorder")

    nest.Connect(sgen, srec)

    nest.Simulate(8.0)

    return srec.events["times"], spike_times


@pytest.mark.parametrize("resolution", [1.0, 0.1, 0.02, 0.01, 0.001])
def test_spike_recorder_different_resolutions(resolution):
    """
    Test that recorded spike times are independent of resolution.
    """

    actual_spikes, expected_spikes = simulator(resolution)
    nptest.assert_almost_equal(actual_spikes, expected_spikes)
