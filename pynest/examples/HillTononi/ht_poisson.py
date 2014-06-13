# -*- coding: utf-8 -*-
#
# ht_poisson.py
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
"""
A small example using the ht_neuron.

The neuron is bombarded with spike trains from four
Poisson generators, which are connected to the AMPA,
NMDA, GABA_A, and GABA_B synapses, respectively.

Once simulation is completed, membrane potential and
threshold, as well as synaptic conductances and intrinsic currents
are shown.
"""

import nest
import numpy as np
import matplotlib.pyplot as pl

nest.ResetKernel()

# create neuron
nrn = nest.Create('ht_neuron')

# get receptor ID information, so we can connect to the
# different synapses
receptors = nest.GetDefaults('ht_neuron')['receptor_types']

# create multimeter and configure it to record all information
# we want at 0.1ms resolution
mm = nest.Create('multimeter')
nest.SetStatus(mm, {'interval': 0.1,
                    'record_from': ['V_m', 'Theta',  # membrane potential, threshold
                                    # synaptic conductances
                                    'g_AMPA', 'g_NMDA', 'g_GABAA', 'g_GABAB',
                                    # intrinsic currents
                                    'I_NaP', 'I_KNa', 'I_T', 'I_h']})

# create four Poisson generators
g = nest.Create('poisson_generator', 4, params={'rate': 100.})

# we cannot connect Poisson generators to neurons via dynamic synapses directly,
# so we put parrot neurons in between
p = nest.Create('parrot_neuron', 4)

# connect each generator to synapse with given weight
# using the adapting ht_synapse; connections pass through parrot neuron
for s in zip(g, p, [('AMPA', 500.0), ('NMDA', 50.), ('GABA_A', 250.), ('GABA_B', 100.0)]):
    nest.Connect([s[0]], [s[1]])
    nest.Connect([s[1]], nrn, syn_spec={'weight': s[2][1], 'receptor_type': receptors[s[2][0]],
                 'model': 'ht_synapse'})

# connect multimeter
nest.Connect(mm, nrn)

# simulate
for n in range(10):
    nest.Simulate(100)

# extract data from multimeter
events = nest.GetStatus(mm)[0]['events']
t = events['times'];  # time axis

pl.clf()
pl.subplot(311)
pl.plot(t, events['V_m'], 'b', t, events['Theta'], 'g')
pl.legend(['V_m', 'Theta'])
pl.ylabel('Membrane potential [mV]')
pl.title('ht_neuron driven by Poisson procs through AMPA, NMDA, GABA_A, GABA_B')

pl.subplot(312)
pl.plot(t, events['g_AMPA'], 'b', t, events['g_NMDA'], 'g',
        t, events['g_GABAA'], 'r', t, events['g_GABAB'], 'm')
pl.legend(['AMPA', 'NMDA', 'GABA_A', 'GABA_B'])
pl.ylabel('Synaptic conductance [nS]')

pl.subplot(313)
pl.plot(t, events['I_h'], 'maroon', t, events['I_T'], 'orange',
        t, events['I_NaP'], 'crimson', t, events['I_KNa'], 'aqua')
pl.legend(['I_h', 'I_T', 'I_NaP', 'I_KNa'])
pl.ylabel('Intrinsic current [pA]')
pl.xlabel('Time [ms]')

pl.plot()
