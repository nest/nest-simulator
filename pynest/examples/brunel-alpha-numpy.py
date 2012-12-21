# -*- coding: utf-8 -*-
#
# brunel-alpha-numpy.py
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

# This version uses numpy to draw the random connections.

from scipy.optimize import fsolve

import nest
import nest.raster_plot

import numpy
from numpy import exp

import time

def LambertWm1(x):
    nest.sli_push(x); nest.sli_run('LambertWm1'); y=nest.sli_pop()
    return y

def ComputePSPnorm(tauMem, CMem, tauSyn):
  """Compute the maximum of postsynaptic potential
     for a synaptic input current of unit amplitude
     (1 pA)"""

  a = (tauMem / tauSyn)
  b = (1.0 / tauSyn - 1.0 / tauMem)

  # time of maximum
  t_max = 1.0/b * ( -LambertWm1(-exp(-1.0/a)/a) - 1.0/a )

  # maximum of PSP for current of unit amplitude
  return exp(1.0)/(tauSyn*CMem*b) * ((exp(-t_max/tauMem) - exp(-t_max/tauSyn)) / b - t_max*exp(-t_max/tauSyn))

nest.ResetKernel()

startbuild= time.time()

dt      = 0.1    # the resolution in ms
simtime = 1000.0 # Simulation time in ms
delay   = 1.5    # synaptic delay in ms

# Parameters for asynchronous irregular firing
g       = 5.0
eta     = 2.0
epsilon = 0.1    # connection probability

order     = 50
NE        = 4*order
NI        = 1*order
N_neurons = NE+NI
N_rec     = 50 # record from 50 neurons

CE    = epsilon*NE   # number of excitatory synapses per neuron
CI    = epsilon*NI   # number of inhibitory synapses per neuron  
C_tot = int(CI+CE)  # total number of synapses per neuron

# Initialize the parameters of the integrate and fire neuron
tauSyn = 0.5
CMem = 250.0
tauMem = 20.0
theta  = 20.0
J      = 0.1 # postsynaptic amplitude in mV

# normalize synaptic current so that amplitude of a PSP is J
J_ex  = J / ComputePSPnorm(tauMem, CMem, tauSyn)
J_in  = -g*J_ex

# threshold rate, equivalent rate of events needed to
# have mean input current equal to threshold
nu_th  = (theta * CMem) / (J_ex*CE*numpy.exp(1)*tauMem*tauSyn)
nu_ex  = eta*nu_th
p_rate = 1000.0*nu_ex*CE

nest.SetKernelStatus({"resolution": dt, "print_time": True})

print "Building network"

neuron_params= {"C_m"       : CMem,
                "tau_m"     : tauMem,
                "tau_syn_ex": tauSyn,
                "tau_syn_in": tauSyn,
                "t_ref"     : 2.0,
                "E_L"       : 0.0,
                "V_reset"   : 0.0,
                "V_m"       : 0.0,
                "V_th"      : theta}

nest.SetDefaults("iaf_psc_alpha", neuron_params)

nodes_ex=nest.Create("iaf_psc_alpha",NE)
nodes_in=nest.Create("iaf_psc_alpha",NI)

nest.SetDefaults("poisson_generator",{"rate": p_rate})
noise=nest.Create("poisson_generator")

espikes=nest.Create("spike_detector")
ispikes=nest.Create("spike_detector")

nest.SetStatus(espikes,[{"label": "brunel-py-ex",
                   "withtime": True,
                   "withgid": True}])

nest.SetStatus(ispikes,[{"label": "brunel-py-in",
                   "withtime": True,
                   "withgid": True}])

print "Connecting devices."

nest.CopyModel("static_synapse","excitatory",{"weight":J_ex, "delay":delay})
nest.CopyModel("static_synapse","inhibitory",{"weight":J_in, "delay":delay})
 
nest.DivergentConnect(noise,nodes_ex,model="excitatory")
nest.DivergentConnect(noise,nodes_in,model="excitatory")

nest.ConvergentConnect(range(1,N_rec+1),espikes,model="excitatory")
nest.ConvergentConnect(range(NE+1,NE+1+N_rec),ispikes,model="excitatory")

print "Connecting network."

# Here, we create the connections from the excitatory neurons to all other
# neurons. We exploit that the neurons have consecutive IDs, running from
# 1,...,NE           for the excitatory neurons and from
# (NE+1),...,(NE+NI) for the inhibitory neurons.

numpy.random.seed(1234)

sources_ex = numpy.random.random_integers(1,NE,(N_neurons,CE))
sources_in = numpy.random.random_integers(NE+1,N_neurons,(N_neurons,CI))

# We now iterate over all neuron IDs, and connect the neuron to
# the sources from our array. The first loop connects the excitatory neurons
# and the second loop the inhibitory neurons.


for n in xrange(N_neurons):
    nest.ConvergentConnect(list(sources_ex[n]),[n+1],model="excitatory")

for n in xrange(N_neurons):
    nest.ConvergentConnect(list(sources_in[n]),[n+1],model="inhibitory")

endbuild=time.time()

print "Simulating."

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

print "Brunel network simulation (Python)"
print "Number of neurons :", N_neurons
print "Number of synapses:", num_synapses
print "       Exitatory  :", int(CE*N_neurons)+N_neurons
print "       Inhibitory :", int(CI*N_neurons)
print "Excitatory rate   : %.2f Hz" % rate_ex
print "Inhibitory rate   : %.2f Hz" % rate_in
print "Building time     : %.2f s" % build_time
print "Simulation time   : %.2f s" % sim_time

nest.raster_plot.from_device(espikes, hist=True)
