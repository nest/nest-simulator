# -*- coding: utf-8 -*-
#
# gap_junctions_two_neurons.py
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
Gap Junctions: Two neurons example
------------------
This is a simple example of two hh_psc_alpha_gap neurons connected
by a gap-junction. Please note that gap junctions are two-way connections:
In order to create an accurate gap-junction connection between two
neurons i and j the use of the flag "make_symmetric" is required.
"""

import nest
import pylab
import numpy

nest.ResetKernel()

nest.SetKernelStatus({'resolution': 0.05,
                      # Settings for waveform relaxation
                      # 'use_wfr': False uses communication in every step
                      # instead of an iterative solution
                      'use_wfr': True,
                      'wfr_comm_interval': 1.0,
                      'wfr_tol': 0.0001,
                      'wfr_max_iterations': 15,
                      'wfr_interpolation_order': 3})

neuron = nest.Create('hh_psc_alpha_gap', 2)

vm = nest.Create('voltmeter', params={'to_file': False,
                                      'withgid': True,
                                      'withtime': True,
                                      'interval': 0.1})

nest.SetStatus(neuron, {'I_e': 100.})
nest.SetStatus([neuron[0]], {'V_m': -10.})

nest.Connect(vm, neuron, 'all_to_all')

"""
Use the 'make_symmetric' flag to connect
neuron[0] -> neuron[1] and neuron[1] -> neuron[0]
with a single call to nest.Connect in order to create
an accurate gap junction between both neurons
"""
nest.Connect([neuron[0]], [neuron[1]],
             {'rule': 'one_to_one', 'make_symmetric': True},
             {'model': 'gap_junction', 'weight': 0.5})

nest.Simulate(351.)

senders = nest.GetStatus(vm, 'events')[0]['senders']
times = nest.GetStatus(vm, 'events')[0]['times']
V = nest.GetStatus(vm, 'events')[0]['V_m']

pylab.figure(1)
pylab.plot(times[numpy.where(senders == 1)],
           V[numpy.where(senders == 1)], 'r-')
pylab.plot(times[numpy.where(senders == 2)],
           V[numpy.where(senders == 2)], 'g-')
pylab.xlabel('time (ms)')
pylab.ylabel('membrane potential (mV)')
pylab.show()
