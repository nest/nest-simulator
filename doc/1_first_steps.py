# -*- coding: utf-8 -*-
#
# 1_first_steps.py
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
####################################################################################################
# PyNEST - First Steps
#
# **Based on Jupyter Notebook created by Alexander van Meegen**

# import the NEST module and voltage trace so we can plot the membrane potential
import nest
import nest.voltage_trace

# Before creating a new network, reset the simulation kernel. This ensures we start
# in a clean state.
nest.ResetKernel()

#####################################################################################################
# Create neurons

# You can find all models for neurons as well as devices and synapses in our
# Model directory

# Create the neuron. The neuron model we are creating is a leaky
# integrate-and-fire neuron with exponential shaped post-synaptic currents
neuron = nest.Create('iaf_psc_exp')
# Create() just returns a list (tuple) with handles to the new nodes
# (handles = integer numbers called ids)

# Investigate the neuron, this returns the id as a list (in this case we only
# have one neuron)
neuron

# We can now get the current dynamical state/parameters of the neuron.
# Note that the membrane voltage is at -70 mV.
nest.GetStatus(neuron)

# Some of the parameters can be updated. To find parameter info and much more,
# check out the model documentation
nest.help('iaf_psc_exp')

#####################################################################################################
# Create devices

# Devices are network nodes which provide input to the network or record its
# output. They encapsulate the stimulation and measurement process. If you want
# to extract certain information from a simulation, you need a device which is
# able to deliver this information. Likewise, if you want to send specific input
# to the network, you need a device which delivers this input.
# 
# Devices have a built-in timer which controls the period they are active. Outside
# this interval, a device will remain silent. The timer can be configured using
# the command ``SetStatus``.

# Create a spike generator. Devices are created in the same way as neurons.
spikegenerator = nest.Create('spike_generator')

# Check out 'spike_times' in its parameters
nest.GetStatus(spikegenerator)

# Set the spike times at 10 and 50 ms. Note the decimal is required.
nest.SetStatus(spikegenerator, {'spike_times': [10., 50.]})

# Create a voltmeter for recording
voltmeter = nest.Create('voltmeter')

# See that it records membrane voltage, senders, times
nest.GetStatus(voltmeter)

#####################################################################################################
# Connect neuron to devices

# Connections between nodes (neurons, devices or synapses) define possible channels for interactions between
# them. A connection between two nodes is established, using the command
# ``Connect``.
#
# Each connection has two basic parameters, *weight* and *delay*. The weight
# determines the strength of the connection, the delay determines how long an
# event needs to travel from the sending to the receiving node. The delay must be
# a positive number greater or equal to the simulation stepsize and is given in
# ms.

# Investigate Connect() function. If you are running this in a python prompt and
help('nest.Connect')
# If you are running this in ipython you can use nest.Connect?

# You can see there are several options to specify synapse and connection
# specifications

# Connect spike generator and voltmeter to the neuron. We can specify
# the synaptic specifications at this step.
nest.Connect(spikegenerator, neuron, syn_spec={'weight': 1e3})
nest.Connect(voltmeter, neuron)

#####################################################################################################
# Simulate our network

# NEST simulations are time driven. The simulation time proceeds in discrete steps
# of size ``dt``, set using the property ``resolution`` of the root node. In each time
# slice, all nodes in the system are updated and pending events are delivered.
#
# The simulation is run by calling the command ``Simulate(t)``, where ``t`` is the
# simulation time in milliseconds.
#
# Now we can run our simulation. We will set it for 100 ms. Note that the
# decimal is required.
nest.Simulate(100.)

# Let's look at nest's KernelStatus:
# network_size (root node, neuron, spike generator, voltmeter)
# num_connections
# time (simulation duration)
nest.GetKernelStatus()

# Note that voltmeter has recorded 99 events
nest.GetStatus(voltmeter)

# Read out recording time and voltage from voltmeter
times = nest.GetStatus(voltmeter)[0]['events']['times']
voltages = nest.GetStatus(voltmeter)[0]['events']['V_m']

#####################################################################################################
# Plot the output

# Create the plot with NEST's built-in plotting function
nest.voltage_trace.from_device(voltmeter)

# To view the plot use
nest.voltage_trace.show()
