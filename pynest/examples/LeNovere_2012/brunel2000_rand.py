# -*- coding: utf-8 -*-
#
# brunel2000_rand.py
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
import numpy

# Network parameters. These are given in Brunel (2000) J.Comp.Neuro.
g       = 5.0    # Ratio of IPSP to EPSP amplitude: J_I/J_E
eta     = 2.0    # rate of external population in multiples of threshold rate
delay   = 1.5    # synaptic delay in ms
tau_m   = 20.0   # Membrane time constant in mV
V_th    = 20.0   # Spike threshold in mV

N_E = 8000
N_I = 2000
N_neurons = N_E+N_I

C_E    = N_E/10 # number of excitatory synapses per neuron
C_I    = N_I/10 # number of inhibitory synapses per neuron  

J_E  = 0.1
J_I  = -g*J_E

nu_ex  = eta* V_th/(J_E*C_E*tau_m) # rate of an external neuron in ms^-1
p_rate = 1000.0*nu_ex*C_E          # rate of the external population in s^-1


# Set parameters of the NEST simulation kernel
nest.SetKernelStatus({"print_time": True,
                      "local_num_threads": 2})

# Create and seed RNGs
ms = 1000 # master seed
n_vp = nest.GetKernelStatus('total_num_virtual_procs')
pyrngs = [numpy.random.RandomState(s) for s in range(ms, ms+n_vp)]
nest.SetKernelStatus({'grng_seed': ms+n_vp,
                      'rng_seeds': range(ms+n_vp+1, ms+1+2*n_vp)})

nest.SetDefaults("iaf_psc_delta", 
                 {"C_m": 1.0,
                  "tau_m": tau_m,
                  "t_ref": 2.0,
                  "E_L": 0.0,
                  "V_th": V_th,
                  "V_reset": 10.0})

nodes   = nest.Create("iaf_psc_delta",N_neurons)
nodes_E= nodes[:N_E]
nodes_I= nodes[N_E:]

# randomize membrane potential
node_info   = nest.GetStatus(nodes, ['global_id','vp','local'])
local_nodes = [(gid,vp) for gid,vp,islocal in node_info if islocal]
for gid,vp in local_nodes: 
  nest.SetStatus([gid], {'V_m': pyrngs[vp].uniform(-V_th,V_th)})

nest.CopyModel("static_synapse", "excitatory")
for tgt_gid, tgt_vp in local_nodes:
  weights = pyrngs[tgt_vp].uniform(0.5*J_E, 1.5*J_E, C_E)
  nest.RandomConvergentConnect(nodes_E, [tgt_gid], C_E,
                               weight = list(weights), delay = delay,
                               model="excitatory")

nest.CopyModel("static_synapse_hom_wd",
               "inhibitory",
               {"weight":J_I, 
                "delay":delay})
nest.RandomConvergentConnect(nodes_I, nodes, C_I,model="inhibitory")

noise=nest.Create("poisson_generator",1,{"rate": p_rate})

nest.CopyModel("static_synapse_hom_wd",
               "excitatory-input",
               {"weight":J_E, 
                "delay":delay})
nest.DivergentConnect(noise,nodes,model="excitatory-input")

spikes=nest.Create("spike_detector",2, 
                   [{"label": "brunel-py-ex"},
                    {"label": "brunel-py-in"}])
spikes_E=spikes[:1]
spikes_I=spikes[1:]

N_rec = 50    # Number of neurons to record from
nest.ConvergentConnect(nodes_E[:N_rec],spikes_E)
nest.ConvergentConnect(nodes_I[:N_rec],spikes_I)

# Visualization of initial membrane potential and initial weight
# distribution only if we run on single MPI process
if nest.NumProcesses() == 1:
  pylab.figure()
  V_E = nest.GetStatus(nodes_E[:N_rec], 'V_m')
  pylab.hist(V_E, bins=10)
  pylab.xlabel('Membrane potential V_m [mV]')
  #pylab.savefig('../figures/rand_Vm.eps')

  pylab.figure()
  w = nest.GetStatus(nest.GetConnections(nodes_E[:N_rec],
                                          synapse_model='excitatory'),
                     'weight')
  pylab.hist(w, bins=100)
  pylab.xlabel('Synaptic weight [pA]')
  #pylab.title('Distribution of synaptic weights (%d synapses)' % len(w))
  #pylab.savefig('../figures/rand_w.eps')
else:
  print "Multiple MPI processes, skipping graphical output"

simtime   = 300.  # how long shall we simulate [ms]
nest.Simulate(simtime)

events = nest.GetStatus(spikes,"n_events")

# Before we compute the rates, we need to know how many of the recorded
# neurons are on the local MPI process
N_rec_local_E = sum(nest.GetStatus(nodes_E[:N_rec], 'local'))
rate_ex= events[0]/simtime*1000.0/N_rec_local_E
print "Excitatory rate   : %.2f Hz" % rate_ex

N_rec_local_I = sum(nest.GetStatus(nodes_I[:N_rec], 'local'))
rate_in= events[1]/simtime*1000.0/N_rec_local_I
print "Inhibitory rate   : %.2f Hz" % rate_in

if nest.NumProcesses() == 1:
  nest.raster_plot.from_device(spikes_E, hist=True, title='')
  #pylab.savefig('../figures/rand_raster.eps')
else:
  print "Multiple MPI processes, skipping graphical output"

#pylab.show()
