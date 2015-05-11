# -*- coding: utf-8 -*-
#
# brunel2000_interactive.py
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

import nest
import nest.raster_plot
import pylab

# Network parameters. These are given in Brunel (2000) J.Comp.Neuro.
g       = 5.0    # Ratio of IPSP to EPSP amplitude: J_I/J_E
eta     = 2.0    # rate of external population in multiples of threshold rate
delay   = 1.5    # synaptic delay in ms
tau_m   = 20.0   # Membrane time constant in mV
V_th    = 20.0   # Spike threshold in mV

N_E = 8000
N_I = 2000
N_neurons = N_E + N_I

C_E    = int(N_E / 10) # number of excitatory synapses per neuron
C_I    = int(N_I / 10) # number of inhibitory synapses per neuron

J_E  = 0.1
J_I  = -g * J_E

nu_ex  = eta * V_th / (J_E * C_E * tau_m) # rate of an external neuron in ms^-1
p_rate = 1000.0 * nu_ex * C_E             # rate of the external population in s^-1


# Set parameters of the NEST simulation kernel
nest.SetKernelStatus({'print_time': True,
                      'local_num_threads': 2})

nest.SetDefaults('iaf_psc_delta', 
                 {'C_m': 1.0,
                  'tau_m': tau_m,
                  't_ref': 2.0,
                  'E_L': 0.0,
                  'V_th': V_th,
                  'V_reset': 10.0})

# Create nodes -------------------------------------------------

nodes = nest.Create('iaf_psc_delta', N_neurons)
nodes_E = nodes[:N_E]
nodes_I = nodes[N_E:]

noise=nest.Create('poisson_generator', 1, {'rate': p_rate})

spikes=nest.Create('spike_detector',2, 
                   [{'label': 'brunel_py_ex'},
                    {'label': 'brunel_py_in'}])
spikes_E=spikes[:1]
spikes_I=spikes[1:]

# Connect nodes ------------------------------------------------

nest.CopyModel('static_synapse_hom_w',
               'excitatory',
               {'weight':J_E, 
                'delay':delay})
nest.Connect(nodes_E, nodes,
             {'rule': 'fixed_indegree', 
              'indegree': C_E},
             'excitatory')

nest.CopyModel('static_synapse_hom_w',
               'inhibitory',
               {'weight':J_I, 
                'delay':delay})
nest.Connect(nodes_I, nodes,
             {'rule': 'fixed_indegree', 
              'indegree': C_I},
             'inhibitory')

nest.Connect(noise, nodes, syn_spec='excitatory')

N_rec   = 50    # Number of neurons to record from
nest.Connect(nodes_E[:N_rec], spikes_E)
nest.Connect(nodes_I[:N_rec], spikes_I)

# Simulate -----------------------------------------------------

simtime = 300.
nest.Simulate(simtime)

ex_events, in_events = nest.GetStatus(spikes, 'n_events')
events_to_rate = 1000. / simtime /N_rec

rate_ex = ex_events * events_to_rate
print('Excitatory rate: {:.2f} Hz'.format(rate_ex))

rate_in = in_events * events_to_rate
print('Inhibitory rate: {:.2f} Hz'.format(rate_in))

nest.raster_plot.from_device(spikes_E, hist=True)
#pylab.show()


pylab.savefig('../figures/brunel_interactive.eps')
