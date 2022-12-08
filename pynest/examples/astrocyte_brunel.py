# -*- coding: utf-8 -*-
#
# brunel_astro.py
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
Random balanced network (alpha synapses) with astrocytes
------------------------------------------------------------

This script simulates a network with excitatory and inhibitory neurons and
astrocytes on the basis of the network used in [1]_.

References
~~~~~~~~~~

.. [1] Destexhe, A. (2009). Self-sustained asynchronous irregular states and
       up–down states in thalamic, cortical and thalamocortical networks of
       nonlinear integrate-and-fire neurons. Journal of computational 
       neuroscience, 27(3), 493-506.

"""

###############################################################################
# Import all necessary modules for simulation, analysis and plotting. Scipy
# should be imported before nest.

import copy
import time
import numpy as np
import scipy.special as sp

import nest
import nest.raster_plot
import matplotlib.pyplot as plt


###############################################################################
# Reset kernel

nest.ResetKernel()
# nest.set(local_num_threads=8)

###############################################################################
# Assigning the current time to a variable in order to determine the build
# time of the network.

startbuild = time.time()

###############################################################################
# Neuron model used

neuron_model = "aeif_cond_alpha_astro"

###############################################################################
# Assigning the simulation parameters to variables.

dt = 0.1    # the resolution in ms
simtime = 3000.0  # Simulation time in ms
delay = dt    # synaptic delay in ms

###############################################################################
# Parameters for neuron-neuron connections

p_neuron = 0.02  # connection probability for neurons
J_ex = 6.0  # excitatory synaptic weight in nS
J_in = -67.0  # inhibitory synaptic weight in nS

###############################################################################
# Parameters for astrocytes

p_synapse_astrocyte = 1.0
max_astro_per_target = 3

###############################################################################
# Definition of the number of neurons and astrocytes in the network and the
# number of neurons recorded from.

order = 400
NE = 4 * order  # number of excitatory neurons
NI = 1 * order  # number of inhibitory neurons
N_astro = 400  # number of astrocytes
N_rec = 100  # record from 100 neurons

###############################################################################
# Initialization of the parameters of the integrate and fire neuron and the
# synapses. The parameters of the neuron are stored in a dictionary.

tau_syn_ex = 5.0
tau_syn_in = 10.0
neuron_params_ex = {
                 "V_m": -60.0, # membrane potential in mV
                 "C_m": 200.0, # capacitance of membrane in pF
                 "t_ref": 2.5, # duration of refractory period in ms
                 "V_reset": -60.0,  # reset value for V_m in mV
                 "E_L": -60.0, # leak reverse potential in mV
                 "g_L": 10.0, # leak conductance in nS
                 "a": 1.0, # subthreshold adaptation in nS
                 "b": 40.0, # spike-triggered adaptation in pA
                 "Delta_T": 2.5, # slope factor in mV
                 "tau_w": 600.0, # adaptation time constant in ms
                 "V_th": -50.0, # spike initiation threshold in mV
                 "V_peak": -50.0, # spike detection threshold in mV
                 "E_ex": 0.0, # excitatory reversal potential in mV
                 "E_in": -80.0, # inhibitory reversal potential in mV
                 "tau_syn_ex": tau_syn_ex, # excitatory synaptic time constant in ms
                 "tau_syn_in": tau_syn_in, # inhibitory synaptic time constant in ms
                 }
neuron_params_in = copy.deepcopy(neuron_params_ex)
neuron_params_in["b"] = 0.0 # spike-triggered adaptation for inhibitory neurons

###############################################################################
# Poisson generator rate

poisson_rate = 200.0

################################################################################
# Configuration of the simulation kernel by the previously defined time
# resolution used in the simulation. Setting ``print_time`` to `True` prints the
# already processed simulation time as well as its percentage of the total
# simulation time.

nest.resolution = dt
nest.print_time = True
nest.overwrite_files = True

###############################################################################
# Creation of the nodes using ``Create``. We store the returned handles in
# variables for later reference.

print("Building network")
nodes_ex = nest.Create(neuron_model, NE, params=neuron_params_ex)
nodes_in = nest.Create(neuron_model, NI, params=neuron_params_in)
nodes_astro = nest.Create("astrocyte", N_astro)
noise = nest.Create("poisson_generator", params={"rate": poisson_rate, "start": 0.0, "stop": 50.0})
espikes = nest.Create("spike_recorder")
ispikes = nest.Create("spike_recorder")

###############################################################################
# Configuration of the spike recorders recording excitatory and inhibitory
# spikes by sending parameter dictionaries to ``set``. Setting the property
# `record_to` to *"ascii"* ensures that the spikes will be recorded to a file,
# whose name starts with the string assigned to the property `label`.

espikes.set(label="brunel-py-ex", record_to="ascii")
ispikes.set(label="brunel-py-in", record_to="ascii")

#################################################################################
# Connecting the previously defined poisson generator to the excitatory and
# inhibitory neurons using the excitatory synapse. 

print("Connecting devices")
noise_prob = 0.02 # connection probability with poisson generator
conn_spec_noise_ex = {"rule": "fixed_outdegree", "outdegree": int(len(nodes_ex)*noise_prob)}
conn_spec_noise_in = {"rule": "fixed_outdegree", "outdegree": int(len(nodes_in)*noise_prob)}
syn_params_noise = {"synapse_model": "static_synapse", "weight": J_ex, "delay": delay}
nest.Connect(noise, nodes_ex, conn_spec=conn_spec_noise_ex, syn_spec=syn_params_noise)
nest.Connect(noise, nodes_in, conn_spec=conn_spec_noise_in, syn_spec=syn_params_noise)

###############################################################################
# Connecting the first ``N_rec`` nodes of the excitatory and inhibitory
# population to the associated spike recorders using excitatory synapses.
# Here the same shortcut for the specification of the synapse as defined
# above is used.

nest.Connect(nodes_ex[:N_rec], espikes)
nest.Connect(nodes_in[:N_rec], ispikes)

###############################################################################
# Connecting the excitatory population to all neurons. The connections are
# paired with astrocytes with the "pairwise_bernoulli_astro" rule.

print("Connecting network")
print("Excitatory connections")
conn_params_astro = {
                  "rule": "pairwise_bernoulli_astro",
                  "astrocyte": nodes_astro,
                  "p": p_neuron,
                  "p_syn_astro": p_synapse_astrocyte,
                  "max_astro_per_target": max_astro_per_target
                  }
syn_params_astro = {
                 "synapse_model": "tsodyks_synapse",
                 "weight": J_ex,
                 "delay": delay,
                 "U": 0.5,
                 "tau_psc": tau_syn_ex,
                 "tau_fac": 0.0,
                 "tau_rec": 800.0
                 }
nest.Connect(nodes_ex, nodes_ex + nodes_in, conn_params_astro, syn_params_astro)

###############################################################################
# Connecting the inhibitory population to all neurons.

print("Inhibitory connections")
conn_params_in = {"rule": "pairwise_bernoulli", "p": p_neuron}
syn_params_in = {
              "synapse_model": "tsodyks_synapse",
              "weight": J_in,
              "delay": delay,
              "U": 0.04,
              "tau_psc": tau_syn_in,
              "tau_fac": 1000.0,
              "tau_rec": 100.0
              }
nest.Connect(nodes_in, nodes_ex + nodes_in, conn_params_in, syn_params_in)

###############################################################################
# Storage of the time point after the buildup of the network in a variable.

endbuild = time.time()

###############################################################################
# Simulation of the network.

print("Simulating")

nest.Simulate(simtime)

###############################################################################
# Storage of the time point after the simulation of the network in a variable.

endsimulate = time.time()

###############################################################################
# Reading out the total number of spikes received from the spike recorder
# connected to the excitatory population and the inhibitory population.

events_ex = espikes.n_events
events_in = ispikes.n_events

###############################################################################
# Calculation of the average firing rate of the excitatory and the inhibitory
# neurons by dividing the total number of recorded spikes by the number of
# neurons recorded from and the simulation time. The multiplication by 1000.0
# converts the unit 1/ms to 1/s=Hz.

rate_ex = events_ex / simtime * 1000.0 / N_rec
rate_in = events_in / simtime * 1000.0 / N_rec

###############################################################################
# Establishing the time it took to build and simulate the network by taking
# the difference of the pre-defined time variables.

build_time = endbuild - startbuild
sim_time = endsimulate - endbuild

###############################################################################
# Printing the network properties, firing rates and building times.

print("Brunel network simulation (Python)")
print(f"Excitatory rate   : {rate_ex:.2f} Hz")
print(f"Inhibitory rate   : {rate_in:.2f} Hz")
print(f"Building time     : {build_time:.2f} s")
print(f"Simulation time   : {sim_time:.2f} s")

###############################################################################
# Plot a raster of the excitatory neurons and a histogram.

nest.raster_plot.from_device(espikes, hist=True)
plt.show()
