# -*- coding: utf-8 -*-
#
# test_ginzburg_and_mcculloch.py
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


def test_ginzburg_and_mcculloch_pitts_neurons():
    """
    Test `ginzburg_neuron` and `mcculloch_pitts_neuron` in simulation.

    This test ensures that we are able to connect a `ginzburg_neuron` with
    a `mcculloch_pitts_neuron` and that we can then run a simulation.
    """

    ginzburg = nest.Create("ginzburg_neuron")
    mcculloch = nest.Create("mcculloch_pitts_neuron")
    nest.Connect(ginzburg, mcculloch)

    assert nest.num_connections == 1

    nest.Simulate(100.0)

    assert nest.local_spike_counter >= 1
