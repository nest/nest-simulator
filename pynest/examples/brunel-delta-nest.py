# -*- coding: utf-8 -*-
#
# brunel-delta-nest.py
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

# This version uses NEST's Connect functions.

import nest
import nest.raster_plot

import time
from numpy import exp

nest.ResetKernel()

startbuild = time.time()

dt      = 0.1    # the resolution in ms
simtime = 1000.0 # Simulation time in ms
delay   = 1.5    # synaptic delay in ms

# Parameters for asynchronous irregular firing
g       = 5.0
eta     = 2.0  # external rate relative to threshold rate
epsilon = 0.1  # connection probability

order     = 2500
NE        = 4*order
NI        = 1*order
N_neurons = NE+NI
N_rec     = 50 # record from 50 neurons

CE    = int(epsilon*NE) # number of excitatory synapses per neuron
CI    = int(epsilon*NI) # number of inhibitory synapses per neuron  
C_tot = int(CI+CE)      # total number of synapses per neuron

# Initialize the parameters of the integrate and fire neuron
tauMem = 20.0
theta  = 20.0
J      = 0.1 # postsynaptic amplitude in mV

J_ex  = J
J_in  = -g*J_ex

nu_th  = theta/(J*CE*tauMem)
nu_ex  = eta*nu_th
p_rate = 1000.0*nu_ex*CE

nest.SetKernelStatus({"resolution": dt, "print_time": True})

print("Building network")

neuron_params= {"C_m":        1.0,
                "tau_m":      tauMem,
                "t_ref":      2.0,
                "E_L":        0.0,
                "V_reset":    0.0,
                "V_m":        0.0,
                "V_th":       theta}


nest.SetDefaults("iaf_psc_delta", neuron_params)

nodes_ex=nest.Create("iaf_psc_delta",NE)
nodes_in=nest.Create("iaf_psc_delta",NI)

nest.SetDefaults("poisson_generator",{"rate": p_rate})
noise=nest.Create("poisson_generator")

espikes=nest.Create("spike_detector")
ispikes=nest.Create("spike_detector")

nest.SetStatus(espikes,[{"label": "brunel-py-ex",
                   "withtime": True,
                   "withgid": True,
                   "to_file": True}])

nest.SetStatus(ispikes,[{"label": "brunel-py-in",
                   "withtime": True,
                   "withgid": True,
                   "to_file": True}])

print("Connecting devices")

nest.CopyModel("static_synapse","excitatory",{"weight":J_ex, "delay":delay})
nest.CopyModel("static_synapse","inhibitory",{"weight":J_in, "delay":delay})
 
nest.Connect(noise,nodes_ex, syn_spec="excitatory")
nest.Connect(noise,nodes_in, syn_spec="excitatory")

nest.Connect(nodes_ex[:N_rec], espikes, syn_spec="excitatory")
nest.Connect(nodes_in[:N_rec], ispikes, syn_spec="excitatory")

print("Connecting network")

# We now iterate over all neuron IDs, and connect the neuron to
# the sources from our array. The first loop connects the excitatory neurons
# and the second loop the inhibitory neurons.

print("Excitatory connections")

conn_params_ex = {'rule': 'fixed_indegree', 'indegree': CE}
nest.Connect(nodes_ex, nodes_ex+nodes_in, conn_params_ex, "excitatory")

print("Inhibitory connections")

conn_params_in = {'rule': 'fixed_indegree', 'indegree': CI}
nest.Connect(nodes_in, nodes_ex+nodes_in, conn_params_in, "inhibitory")

endbuild=time.time()

print("Simulating")

nest.Simulate(simtime)

endsimulate= time.time()

events_ex = nest.GetStatus(espikes,"n_events")[0]
rate_ex   = events_ex/simtime*1000.0/N_rec
events_in = nest.GetStatus(ispikes,"n_events")[0]
rate_in   = events_in/simtime*1000.0/N_rec

num_synapses = nest.GetDefaults("excitatory")["num_connections"]+\
nest.GetDefaults("inhibitory")["num_connections"]

build_time = endbuild-startbuild
sim_time   = endsimulate-endbuild

print("Brunel network simulation (Python)")
print("Number of neurons : {0}".format(N_neurons))
print("Number of synapses: {0}".format(num_synapses))
print("       Exitatory  : {0}".format(int(CE * N_neurons) + N_neurons))
print("       Inhibitory : {0}".format(int(CI * N_neurons)))
print("Excitatory rate   : %.2f Hz" % rate_ex)
print("Inhibitory rate   : %.2f Hz" % rate_in)
print("Building time     : %.2f s" % build_time)
print("Simulation time   : %.2f s" % sim_time)

nest.raster_plot.from_device(espikes, hist=True)
