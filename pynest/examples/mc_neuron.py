# -*- coding: utf-8 -*-
#
# mc_neuron.py
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
Simple example of how to use the three-compartment
iaf_cond_alpha_mc model neuron.

Three stimulation paradigms are illustrated:
 - externally applied current, one compartment at a time
 - spikes impinging on each compartment, one at a time
 - rheobase current injected to soma causing output spikes

Voltage and synaptic conductance traces are shown for all compartments.
'''

import nest
import matplotlib
import pylab as pl

nest.ResetKernel()

# Obtain receptor dictionary
syns = nest.GetDefaults('iaf_cond_alpha_mc')['receptor_types']
print "iaf_cond_alpha_mc receptor_types: ", syns

# Obtain list of recordable quantities
rqs = nest.GetDefaults('iaf_cond_alpha_mc')['recordables']
print "iaf_cond_alpha_mc recordables   : ", rqs

# Change some default values:
#  - threshold potential
#  - reset potential
#  - refractory period
#  - somato-proximal coupling conductance
#  - somatic leak conductance
#  - proximal synaptic time constants
#  - distal capacitance
nest.SetDefaults('iaf_cond_alpha_mc',
                 { 'V_th' : -60.0,
                   'V_reset': -65.0,
                   't_ref': 10.0,
                   'g_sp' : 5.0,
                   'soma'    : { 'g_L': 12.0 },
                   'proximal': { 'tau_syn_ex': 1.0, 
                                 'tau_syn_in': 5.0 },
                   'distal'  : { 'C_m': 90.0 }
                 }) 

# Create neuron
n = nest.Create('iaf_cond_alpha_mc')

# Create multimeter recording everything, connect
mm = nest.Create('multimeter', 
                 params = {'record_from': rqs, 
                           'interval': 0.1})
nest.Connect(mm, n)

# Create one current generator per compartment and configure
# stimulus regime that drives distal, proximal and soma, in that order
cgs = nest.Create('dc_generator', 3)
nest.SetStatus(cgs, 
               [{'start': 250.0, 'stop': 300.0, 'amplitude':  50.0},  # soma
                {'start': 150.0, 'stop': 200.0, 'amplitude': -50.0},  # proximal
                {'start':  50.0, 'stop': 100.0, 'amplitude': 100.0}]) # distal

# Connect generators to correct compartments
nest.Connect([cgs[0]], n, params = {'receptor_type': syns['soma_curr']})
nest.Connect([cgs[1]], n, params = {'receptor_type': syns['proximal_curr']})
nest.Connect([cgs[2]], n, params = {'receptor_type': syns['distal_curr']})

# Create one excitatory and one inhibitory spike generator per compartment,
# configure regime that drives distal, proximal and soma, in that order,
# excitation and inhibition alternating 
sgs = nest.Create('spike_generator', 6)
nest.SetStatus(sgs,
               [{'spike_times': [600.0, 620.0]}, # soma excitatory
                {'spike_times': [610.0, 630.0]}, # soma inhibitory
                {'spike_times': [500.0, 520.0]}, # proximal excitatory
                {'spike_times': [510.0, 530.0]}, # proximal inhibitory
                {'spike_times': [400.0, 420.0]}, # distal excitatory
                {'spike_times': [410.0, 430.0]}]) # distal inhibitory

# Connect generators to correct compartments
nest.Connect([sgs[0]], n, params = {'receptor_type': syns['soma_exc']})
nest.Connect([sgs[1]], n, params = {'receptor_type': syns['soma_inh']})
nest.Connect([sgs[2]], n, params = {'receptor_type': syns['proximal_exc']})
nest.Connect([sgs[3]], n, params = {'receptor_type': syns['proximal_inh']})
nest.Connect([sgs[4]], n, params = {'receptor_type': syns['distal_exc']})
nest.Connect([sgs[5]], n, params = {'receptor_type': syns['distal_inh']})

# Simulate 
nest.Simulate(700)

# Now turn on intrinsic current in soma to make neuron spike
nest.SetStatus(n, {'soma': {'I_e': 150.0}})

nest.Simulate(300)


# Retrieve data
rec = nest.GetStatus(mm)[0]['events']
t = rec['times']

# Plot potential traces
pl.figure()
pl.subplot(211)
pl.plot(t, rec['V_m.s'], t, rec['V_m.p'], t, rec['V_m.d'])
pl.legend(('Soma', 'Proximal dendrite', 'Distal dendrite'),loc='lower right')
pl.axis([0, 1000, -76, -59])
pl.ylabel('Membrane potential [mV]')
pl.title('Responses of iaf_cond_alpha_mc neuron')

# Plot conductance traces
pl.subplot(212)
pl.plot(t, rec['g_ex.s'], 'b-', t, rec['g_ex.p'], 'g-', t, rec['g_ex.d'], 'r-')
pl.plot(t, rec['g_in.s'], 'b--', t, rec['g_in.p'], 'g--', t, rec['g_in.d'], 'r--')
pl.legend(('g_ex.s', 'g_ex.p', 'g_in.d','g_in.s', 'g_in.p', 'g_in.d'))
pl.axis([350, 700, 0, 1.15])
pl.xlabel('Time [ms]')
pl.ylabel('Synaptic conductance [nS]')
