# -*- coding: utf-8 -*-
#
# test_poisson_generator_ps.py
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
This basic test checks whether two targets receive different spikes trains
and whether the start and stop properties of the generator are respected.
The test does neither check that spikes indeed have high resolution nor
that grid-constrained neuron models receive consistent spike times.
Remarks:
This test fails for a correct implementation if in the simulation interval both targets
receive no spikes. The seed set in the default configuration of NEST avoids this
problem. Therefore, failure of this script indicates that the configuration is not
portable.
"""


import nest
import numpy as np
import pytest


def test_poisson_generator_ps():
    nest.ResetKernel()

    pg = nest.Create("poisson_generator_ps")

    sr1 = nest.Create("spike_recorder")
    sr2 = nest.Create("spike_recorder")

    pg.set(rate=100.0, start=200.0, stop=500.0)

    nest.Connect(pg, sr1)
    nest.Connect(pg, sr2)

    nest.Simulate(1000.0)

    pg.set(start=1200.0, stop=1800.0)

    nest.Simulate(1000.0)

    times1 = sr1.events["times"]
    times2 = sr2.events["times"]
    assert not np.array_equal(times1, times2)

    # check there are no spikes between stop and start time
    assert not ((times1 > 500.0) & (times1 < 1200.0)).any()
    assert not ((times2 > 500.0) & (times2 < 1200.0)).any()
