# -*- coding: utf-8 -*-
#
# soundclick_example.py
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

'''
soundclick recording backend example
------------------------------------

Example PyNest script to demonstrate the soundclick recording backend.

Recorded spike events produce a clicking sound similar to that heard in
electrophysiological recordings. The recording backend makes use of the
SFML (Simple and Fast Multimedia Library) audio module.
This requires libsfml-dev to be installed. To create the illusion of a
realistic sound from an electrophysiological recording, the recording
backend slows down the simulation to biological real time.
'''

import nest
nest.Install("mymodule")

population = nest.Create("izhikevich", 1)
spike_detector = nest.Create("spike_detector", 1)
nest.SetStatus(spike_detector, {"record_to": "soundclick"})
nest.Connect(population, spike_detector)

# regular spiking
nest.SetStatus(population, {"a": 0.02,
                            "b": 0.2,
                            "c": -65.0,
                            "d": 8.0,
                            "U_m": 0.0,
                            "V_m": -75.0,
                            "I_e": 6.0})
nest.Simulate(4000)
nest.SetStatus(population, {"I_e": 0.0})
nest.Simulate(500)

# fast spiking
nest.SetStatus(population, {"a": 0.1,
                            "b": 0.2,
                            "c": -65.0,
                            "d": 2.0,
                            "U_m": 0.0,
                            "V_m": -75.0,
                            "I_e": 6.0})
nest.Simulate(4000)
nest.SetStatus(population, {"I_e": 0.0})
nest.Simulate(500)

# chattering
nest.SetStatus(population, {"a": 0.02,
                            "b": 0.2,
                            "c": -50.0,
                            "d": 2.0,
                            "U_m": 0.0,
                            "V_m": -75.0,
                            "I_e": 6.0})
nest.Simulate(4000)
