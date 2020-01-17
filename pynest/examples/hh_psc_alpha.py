# -*- coding: utf-8 -*-
#
# hh_psc_alpha.py
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

"""Example using Hodgkin-Huxley neuron
----------------------------------------

This example produces a rate-response (FI) curve of the Hodgkin-Huxley
neuron ``hh_psc_alpha`` in response to a range of different current (DC) stimulations.
The result is plotted using matplotlib.

Since a DC input affetcs only the neuron's channel dynamics, this routine
does not yet check correctness of synaptic response.
"""

import nest
import numpy as np
import matplotlib.pyplot as plt

nest.set_verbosity('M_WARNING')
nest.ResetKernel()

simtime = 1000

# Amplitude range, in pA
dcfrom = 0
dcstep = 20
dcto = 2000

h = 0.1  # simulation step size in mS

neuron = nest.Create('hh_psc_alpha')
sd = nest.Create('spike_detector')

nest.SetStatus(sd)

nest.Connect(neuron, sd, syn_spec={'weight': 1.0, 'delay': h})

# Simulation loop
n_data = int(dcto / float(dcstep))
amplitudes = np.zeros(n_data)
event_freqs = np.zeros(n_data)
for i, amp in enumerate(range(dcfrom, dcto, dcstep)):
    nest.SetStatus(neuron, {'I_e': float(amp)})
    print("Simulating with current I={} pA".format(amp))
    nest.Simulate(1000)  # one second warm-up time for equilibrium state
    nest.SetStatus(sd, {'n_events': 0})  # then reset spike counts
    nest.Simulate(simtime)  # another simulation call to record firing rate

    n_events = nest.GetStatus(sd, keys={'n_events'})[0][0]
    amplitudes[i] = amp
    event_freqs[i] = n_events / (simtime / 1000.)

plt.plot(amplitudes, event_freqs)
plt.show()
