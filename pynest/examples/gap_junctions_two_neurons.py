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

"""Gap Junctions: Two neuron example
--------------------------------------

This script simulates two Hodgkin-Huxley neurons of type ``hh_psc_alpha_gap``
connected by a gap junction. Both neurons receive a constant current of
100.0 pA. The neurons are initialized with different membrane potentials and
synchronize over time due to the gap-junction connection.

"""

import nest
import pylab as pl
import numpy

nest.ResetKernel()

###############################################################################
# First we set the resolution of the simulation, create two neurons and
# create a ``voltmeter`` for recording.

nest.SetKernelStatus({'resolution': 0.05})

neuron = nest.Create('hh_psc_alpha_gap', 2)

vm = nest.Create('voltmeter', params={'interval': 0.1})

###############################################################################
# Then we set the constant current input, modify the inital membrane
# potential of one of the neurons and connect the neurons to the ``voltmeter``.

nest.SetStatus(neuron, {'I_e': 100.})
nest.SetStatus(neuron[0], {'V_m': -10.})

nest.Connect(vm, neuron, 'all_to_all')

###############################################################################
# In order to create the ``gap_junction`` connection we employ the
# ``all_to_all`` connection rule: Gap junctions are bidirectional connections,
# therefore we need to connect `neuron[0]` to `neuron[1]` and `neuron[1]` to
# `neuron[0]`:

nest.Connect(neuron, neuron,
             {'rule': 'all_to_all', 'allow_autapses': False},
             {'synapse_model': 'gap_junction', 'weight': 0.5})

###############################################################################
# Finally we start the simulation and plot the membrane potentials of both
# neurons.

nest.Simulate(351.)

senders = nest.GetStatus(vm, 'events')[0]['senders']
times = nest.GetStatus(vm, 'events')[0]['times']
V = nest.GetStatus(vm, 'events')[0]['V_m']

pl.figure(1)
pl.plot(times[numpy.where(senders == 1)],
        V[numpy.where(senders == 1)], 'r-')
pl.plot(times[numpy.where(senders == 2)],
        V[numpy.where(senders == 2)], 'g-')
pl.xlabel('time (ms)')
pl.ylabel('membrane potential (mV)')
pl.show()
