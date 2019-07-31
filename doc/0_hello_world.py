# -*- coding: utf-8 -*-
#
# 0_hello_world.py
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
#
#  PyNEST - Hello World!
#
# **Based on Jupyter Notebook created by Alexander van Meegen**

# This is a basic 'Hello World!' example. Enjoy!

import nest
import nest.voltage_trace
nest.ResetKernel()

# Create the models you want to simulate
neuron = nest.Create('iaf_psc_exp')
# Create the device to stimulate or measure the simulation
spikegenerator = nest.Create('spike_generator')
voltmeter = nest.Create('voltmeter')
# Modify properties of the neuron and device
nest.SetStatus(spikegenerator, {'spike_times': [10., 50.]})
# Connect neurons to devices and specify synapse (connection) properties
nest.Connect(spikegenerator, neuron, syn_spec={'weight': 1e3})
nest.Connect(voltmeter, neuron)
# Simulate  the network for a specific timeframe.
nest.Simulate(100.)
# Display the voltage graph from device
nest.voltage_trace.from_device(voltmeter)
nest.voltage_trace.show()
