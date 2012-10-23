#!/usr/bin/python
#
# event_delivery_interference.py
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
Event interference during spike delivery.

This test demonstrates that if the send() method of a Connection
manipulates the multiplicity of the Event object that is used
for all spike deliveries during a call to Scheduler::deliver_events(),
errors results, as this changed multiplicity will affect all 
subsequent deliveries during the same time step.

See also #426.

This test requires static_synapse_mult0 from developer.

Originally reported by Thomas Pfeil.
This test by Hans Ekkehard Plesser, 2010-05-06.
"""

import nest

nest.ResetKernel()
nest.SetKernelStatus({'print_time': True, 'resolution': 0.1})

# create spike generator that sends two spikes in a min_delay period
sg = nest.Create('spike_generator', params={'precise_times': false,
                                            'spike_times': [0.2, 0.3]})
a = nest.Create('parrot_neuron')
c = nest.Create('parrot_neuron')
spike_det_a = nest.Create('spike_detector')

nest.Connect(sg, a)   
nest.Connect(a, spike_det_a)   # spike_detector fails on multiplicity 0
nest.Connect(a, c, model='static_synapse_mult0')  # sets event multiplicity to 0

nest.Simulate(3.0)
