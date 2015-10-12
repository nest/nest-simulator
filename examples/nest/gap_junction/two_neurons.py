# -*- coding: utf-8 -*-
#
# two_neurons.py
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
This is a simple example of two hh_psc_alpha_gap neurons connected
by a gap-junction. Please note that gap junctions are two-way connections:
In order to create an accurate gap-junction connection between two
neurons i and j two connections are required.
"""

import nest
import pylab
import numpy

nest.ResetKernel()

nest.SetKernelStatus({'resolution': 0.05})  
nest.SetKernelStatus({'max_num_prelim_iterations': 15, 'prelim_interpolation_order': 3, 'prelim_tol': 0.0001})

neuron = nest.Create('hh_psc_alpha_gap',2)
vm = nest.Create('voltmeter', params={ "to_file": False, 'withgid': True, 'withtime': True, 'interval': 0.1})

nest.SetStatus(neuron, {'I_e': 100.})
nest.SetStatus([neuron[0]], {'V_m': -10.})

nest.Connect(vm, neuron, 'all_to_all')

"""
Use 'all_to_all' to connect neurons.
This is equivalent to:
nest.Connect([neuron[0]],[neuron[1]], 'one_to_one', syn_spec={'model': 'gap_junction', 'weight': 0.5})
nest.Connect([neuron[1]],[neuron[0]], 'one_to_one', syn_spec={'model': 'gap_junction', 'weight': 0.5})
"""
nest.Connect(neuron,neuron, 'all_to_all', syn_spec={'model': 'gap_junction', 'weight': 0.5})

nest.Simulate(351.)

senders_vm = nest.GetStatus(vm, 'events')[0]['senders']
times_vm = nest.GetStatus(vm, 'events')[0]['times']
V_vm = nest.GetStatus(vm, 'events')[0]['V_m']

V = [[] for i in range(2)]
times = [[] for i in range(2)]
for i in range(len(senders_vm)):
  V[senders_vm[i]-1].append(V_vm[i])
  times[senders_vm[i]-1].append(times_vm[i])
V = numpy.array(V)
times = numpy.array(times)

pylab.figure(1)
pylab.plot(times[0,:],V[0,:],'r-')
pylab.plot(times[0,:],V[1,:],'g-')
pylab.xlabel('time (ms)')
pylab.ylabel('membrane potential (mV)')
pylab.show()