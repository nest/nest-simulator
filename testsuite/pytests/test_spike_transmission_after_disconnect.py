# -*- coding: utf-8 -*-
#
# test_spike_transmission_after_disconnect.py
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


def test_spike_transmission_after_disconnect():
    """
    Confirm that spikes can be transmitted after connections have been removed.
    """

    n = nest.Create("parrot_neuron", 10)
    nest.Connect(n, n)

    # Delete 1/3 of connections
    c = nest.GetConnections()
    c[::3].disconnect()

    # Add spike generator to drive
    g = nest.Create("spike_generator", params={"spike_times": [1]})
    nest.Connect(g, n)

    # Simulate long enough for spikes to be delivered, but not too long
    # since we otherwise will be buried by exponential growth in number
    # of spikes.
    nest.Simulate(3)
