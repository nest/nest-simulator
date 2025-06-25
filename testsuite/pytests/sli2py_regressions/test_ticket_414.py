# -*- coding: utf-8 -*-
#
# test_ticket_414.py
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


def test_ticket_414():
    """
    Regression test for Ticket #414.

    This test asserts that neurons can receive spikes even when frozen.

    Author: Hans E Plesser, 2010-04-21
    """
    nest.ResetKernel()

    # Create a spike generator with specific parameters
    spike_generator_params = {"precise_times": False, "spike_times": [1.0]}
    spike_generator = nest.Create("spike_generator", params=spike_generator_params)

    # Create an iaf_psc_alpha neuron and set it to frozen
    neuron = nest.Create("iaf_psc_alpha")
    neuron.set(frozen=True)

    # Connect the spike generator to the neuron
    nest.Connect(spike_generator, neuron)

    # Simulate for 10.0 ms
    nest.Simulate(10.0)

    # The test passes if no exceptions are raised
