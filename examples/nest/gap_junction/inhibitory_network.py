# -*- coding: utf-8 -*-
#
# inhibitory_network.py
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
This is the inhibitory network used as test case 2 (see figure 9 and 10) in

    Hahne, J., Helias, M., Kunkel, S., Igarashi, J., 
    Bolten, M., Frommer, A. and Diesmann, M.,
    A unified framework for spiking and gap-junction interactions
    in distributed neuronal network simulations, 
    Front. Neuroinform. 9:22. (2015), 
    doi: 10.3389/fninf.2015.00022

The network contains 500 hh_psc_alpha_gap neurons with random initial
membrane potentials between −40 and −80 mV. Each neuron receives 50 
inhibitory synaptic inputs that are randomly selected from all other 
neurons, each with synaptic weight JI = −50.0 pA and synaptic delay 
d = 1.0 ms. Each neuron receives an excitatory external Poissonian 
input of 500.0 Hz with synaptic weight JE = 300.0 pA and the same 
delay d. In addition (60*500)/2 gap junctions are added randomly to the
network resulting in an average of 60 gap-junction connections per neuron.
"""


import pylab
import nest
import random

n_neuron = 500
gap_per_neuron = 60
inh_per_neuron = 50
delay= 1.0
j_exc= 300.
j_inh= -50.
threads= 8
stepsize = 0.05
simtime = 501.

"""
Set gap weight here
"""
gap_weight = 0.32


random.seed(1)

nest.ResetKernel()

nest.SetKernelStatus({'resolution': 0.05, 'total_num_virtual_procs': threads})
nest.SetKernelStatus({'max_num_prelim_iterations': 15, 'prelim_interpolation_order': 3, 'prelim_tol': 0.0001})

neuron = nest.Create('hh_psc_alpha_gap',n_neuron)

sd = nest.Create("spike_detector", params={'to_file': False, 'to_memory': True})
pg = nest.Create("poisson_generator",params={'rate': 500.0})

conn_dict = {'rule': 'fixed_indegree', 'indegree': inh_per_neuron, 'autapses': False, 'multapses': True}
syn_dict = {'model': 'static_synapse', 'weight': j_inh, 'delay': delay}
nest.Connect(neuron, neuron, conn_dict, syn_dict)

nest.Connect(pg,neuron,'all_to_all',syn_spec={'model': 'static_synapse', 'weight': j_exc, 'delay': delay})
nest.Connect(neuron,sd,'all_to_all')

for i in range(n_neuron):
  nest.SetStatus([neuron[i]], { 'V_m': (-40. - 40. * random.random()) })

"""
We must not use the 'fixed_indegree' oder 'fixed_outdegree' functionality of nest.Connect
to create the connections, as gap_junction connections are two-way connections and we 
need to make sure that the same neurons are connected in both ways.
"""

# create gap_junction connections
n_connection = n_neuron * gap_per_neuron / 2
connection = [random.sample(range(n_neuron),2) for i in range(n_connection)]    
for i in range(n_connection):
  nest.Connect([neuron[connection[i][0]]], [neuron[connection[i][1]]], syn_spec={'model': 'gap_junction', 'weight': gap_weight})
  nest.Connect([neuron[connection[i][1]]], [neuron[connection[i][0]]], syn_spec={'model': 'gap_junction', 'weight': gap_weight})

nest.Simulate(simtime)

times_sd = nest.GetStatus(sd, 'events')[0]['times']
spikes = nest.GetStatus(sd, 'events')[0]['senders']
n_spikes = nest.GetStatus(sd, 'n_events')[0]

hz_rate = (1000.0*n_spikes/simtime)/n_neuron

pylab.figure(1)
pylab.plot(times_sd,spikes,'o')
pylab.title('Average spike rate (Hz): %.2f' % hz_rate)
pylab.xlabel('time (ms)')
pylab.ylabel('neuron no')
pylab.show()

