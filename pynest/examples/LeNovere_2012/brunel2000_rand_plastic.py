# -*- coding: utf-8 -*-
#
# brunel2000_rand_plastic.py
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

# Synaptic parameters
STDP_alpha   = 2.02     # relative strength of STDP depression w.r.t potentiation 
STDP_Wmax = 3*J_E       #maximum weight of plastic synapse

# Simulation parameters
N_vp      = 8     # number of virtual processes to use
base_seed = 10000  # increase in intervals of at least 2*n_vp+1
N_rec     = 50    # Number of neurons to record from
data2file = True # whether to record data to file
simtime   = 300.  # how long shall we simulate [ms]

# reset kernel so that we avoid problems when running skript several times
# in interactive session
nest.ResetKernel()

# Set parameters of the NEST simulation kernel
nest.SetKernelStatus({"print_time": True,
                      "total_num_virtual_procs": N_vp,
                      "overwrite_files": True})

# Create and seed RNGs
n_vp = nest.GetKernelStatus('total_num_virtual_procs')
bs = base_seed # abbreviation to make following code more compact

# Python RNGs for parameter randomization, one per vp
pyrngs = [numpy.random.RandomState(s) for s in range(bs, bs+n_vp)]

# seed NEST internal RNGs
nest.SetKernelStatus({'grng_seed': bs+n_vp,
                      'rng_seeds': range(bs+n_vp+1, bs+1+2*n_vp)})

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
# we first obtain information about which nodes are local and to
# which VPs they belong, separately for excitatory and inhibitory nodes
node_E_info = nest.GetStatus(nodes_E, ['local','global_id','vp'])
node_I_info = nest.GetStatus(nodes_I, ['local','global_id','vp'])
local_E_nodes = [(gid,vp) for islocal,gid,vp in node_E_info if islocal]
local_I_nodes = [(gid,vp) for islocal,gid,vp in node_I_info if islocal]
for gid,vp in local_E_nodes + local_I_nodes: 
  nest.SetStatus([gid], {'V_m': pyrngs[vp].uniform(-V_th,0.95*V_th)})

# We create a plastic excitatory synapse model for the 
# excitatory-excitatory weights and a static excitatory model for
# the excitatory-inhibitory weights
nest.CopyModel("stdp_synapse_hom",
               "excitatory-plastic",
               {"alpha":STDP_alpha,
                "Wmax":STDP_Wmax})
nest.CopyModel("static_synapse", "excitatory-static")

for tgt_gid, tgt_vp in local_E_nodes:
  # We call RandomConvergentConnect once per exc. target neuron. For each target neuron,
  # we get a random weight vector from the RNG for the VP to which the target belongs.
  weights = pyrngs[tgt_vp].uniform(0.5*J_E, 1.5*J_E, C_E)
  nest.RandomConvergentConnect(nodes_E, [tgt_gid], C_E,
                               weight = list(weights), delay = delay,
                               model="excitatory-plastic")


for tgt_gid, tgt_vp in local_I_nodes:
  # We call RandomConvergentConnect once per inh. target neuron. For each target neuron,
  # we get a random weight vector from the RNG for the VP to which the target belongs.
  weights = pyrngs[tgt_vp].uniform(0.5*J_E, 1.5*J_E, C_E)
  nest.RandomConvergentConnect(nodes_E, [tgt_gid], C_E,
                               weight = list(weights), delay = delay,
                               model="excitatory-static")



nest.CopyModel("static_synapse",
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
                   [{"label": "brunel-py-ex", "to_file": data2file},
                    {"label": "brunel-py-in", "to_file": data2file}])
spikes_E=spikes[:1]
spikes_I=spikes[1:]

nest.ConvergentConnect(nodes_E[:N_rec],spikes_E)
nest.ConvergentConnect(nodes_I[:N_rec],spikes_I)

# Visualization of initial membrane potential and initial weight
# distribution only if we run on single MPI process
if nest.NumProcesses() == 1:

  pylab.figure()

  # membrane potential
  V_E = nest.GetStatus(nodes_E[:N_rec], 'V_m')
  V_I = nest.GetStatus(nodes_I[:N_rec], 'V_m')
  pylab.subplot(2,1,1)
  pylab.hist([V_E, V_I],bins=10)
  pylab.xlabel('Initial membrane potential V_m [mV]')
  pylab.legend(('Excitatory', 'Inibitory'))
  pylab.title('Distribution of initial membrane potentials')
  pylab.draw()

  # weight of excitatory connections
  w = nest.GetStatus(nest.GetConnections(nodes_E[:N_rec],
                                          synapse_model='excitatory-plastic'),
                     'weight')

  pylab.subplot(2,1,2)
  pylab.hist(w, bins=100)
  pylab.xlabel('Synaptic weight w [pA]')
  pylab.title('Distribution of excitatory synaptic weights')
  pylab.draw()

else:
  print "Multiple MPI processes, skipping graphical output"

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
  nest.raster_plot.from_device(spikes_E, hist=True)
  pylab.draw()

  # weight of excitatory connections
  w = nest.GetStatus(nest.GetConnections(nodes_E[:N_rec],
                                          synapse_model='excitatory-plastic'),
                     'weight')
  pylab.figure()
  pylab.hist(w, bins=100)
  pylab.xlabel('Synaptic weight [pA]')
  #pylab.savefig('../figures/rand_plas_w.eps')
  #pylab.show()

else:
  print "Multiple MPI processes, skipping graphical output"
