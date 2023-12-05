# -*- coding: utf-8 -*-
#
# test_ignore_and_fire_neuron.py
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

# This script tests the ignore_and_fire neuron in NEST.

import math
import unittest

import nest
import numpy


@nest.ll_api.check_stack
class IgnoreAndFireNeuronTestCase(unittest.TestCase):
    """Check ignore_and_fire neuron spike properties"""

    def setUp(self):
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()

        # set up source spike generator, as well as parrot neurons
        self.rate = 5.0  # firing rate (spikes/s)
        self.phase = 0.4  # firing phase
        self.dt = 2**-3  # simulation time resolution (ms)
        self.T = 1000.0  # simulation time (ms)

        nest.SetKernelStatus({"resolution": self.dt})

        self.neuron = nest.Create("ignore_and_fire", 1)
        self.neuron.set({"rate": self.rate, "phase": self.phase})
        self.spike_recorder = nest.Create("spike_recorder")

        # record ignore-and-fire neuron spikes
        nest.Connect(self.neuron, self.spike_recorder)

    def test_IgnoreAndFireSpikeTimes(self):
        """Check ignore_and_fire neuron spikes at expected times"""

        nest.Simulate(self.T)

        # theoretical spike times
        period = 1.0 / self.rate * 1e3
        first_spike_time = self.phase * period + self.dt
        spike_times_target = numpy.arange(first_spike_time, self.T, period)

        # spike times of NEST model
        spike_times_nest = nest.GetStatus(self.spike_recorder, "events")[0]["times"]

        # assert spike times match the expected values
        assert (spike_times_nest == spike_times_target).all()
