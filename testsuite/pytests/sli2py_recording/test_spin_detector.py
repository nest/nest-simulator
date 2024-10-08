# -*- coding: utf-8 -*-
#
# test_spin_detector.py
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
Test decoding mechanism of binary events by ``spin_detector``.
"""

import nest
import numpy.testing as nptest


def test_spin_detector():
    """
    Test that communication mechanism for binary neurons works as expected.

    The test checks whether the ``spin_detector`` correctly decodes binary
    transitions. Double spikes should be interpreted as up transition and
    a single spike as down transition.
    """

    spike_times = [10.0, 10.0, 15.0]

    spike_gen = nest.Create("spike_generator", params={"spike_times": spike_times})
    spin_det = nest.Create("spin_detector")

    nest.Connect(spike_gen, spin_det)

    nest.Simulate(20.0)

    expected_states = [1, 0]
    actual_states = spin_det.events["state"]

    expected_times = [10.0, 15.0]
    actual_times = spin_det.events["times"]

    nptest.assert_array_equal(actual_states, expected_states)
    nptest.assert_array_equal(actual_times, expected_times)
