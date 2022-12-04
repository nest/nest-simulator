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
nest.set(local_num_threads=4)

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
simtime = 1000.0  # Simulation time in ms
delay = dt    # synaptic delay in ms

###############################################################################
# Definition of the parameters crucial for asynchronous irregular firing of
# the neurons.

epsilon = 0.02  # connection probability
J_ex = 6.0  # amplitude of excitatory synaptic weight in nS
J_in = -67.0  # amplitude of inhibitory synaptic weight in nS

###############################################################################
# Definition of the number of neurons in the network and the number of neurons
# recorded from

order = 400
NE = 4 * order  # number of excitatory neurons
NI = 1 * order  # number of inhibitory neurons
N_neurons = NE + NI   # number of neurons in total
N_rec = 100      # record from 50 neurons

###############################################################################
# Definition of connectivity parameters

CE = int(epsilon * NE)  # number of excitatory synapses per neuron
CI = int(epsilon * NI)  # number of inhibitory synapses per neuron
C_tot = int(CI + CE)      # total number of synapses per neuron

###############################################################################
# Initialization of the parameters of the integrate and fire neuron and the
# synapses. The parameters of the neuron are stored in a dictionary. The
# synaptic currents are normalized such that the amplitude of the PSP is J.

tau_syn_ex = 5.0  # excitatory synaptic time constant in ms
tau_syn_in = 10.0  # inhibitory synaptic time constant in ms
C_m = 200.0  # capacitance of membrane in pF
E_L = -60.0  # leak reverse potential in mV
g_L = 10.0  # leak conductance in nS
t_ref = 2.5  # duration of refractory period in ms
tau_w = 600.0  # adaptation time constant in ms
V_reset = -60.0  # reset value for V_m in mV
a = 1.0  # subthreshold adaptation in nS
b_ex = 40.0  # spike-triggered adaptation in pA (excitatory neurons)
b_in = 0.0  # spike-triggered adaptation in pA (inhibitory neurons)
E_ex = 0.0  # excitatory reversal potential in mV
E_in = -80.0  # inhibitory reversal potential in mV
Delta_T = 2.5  # slope factor in mV
V_th = -50.0  # spike initiation threshold in mV
V_peak = -50.0 # spike detection threshold in mV

neuron_params_ex = {"C_m": C_m,
                 "tau_syn_ex": tau_syn_in,
                 "tau_syn_in": tau_syn_in,
                 "t_ref": t_ref,
                 "E_L": E_L,
                 "g_L": g_L,
                 "V_reset": V_reset,
                 "V_m": E_L,
                 "V_th": V_th,
                 "tau_w": tau_w,
                 "a": a,
                 "b": b_ex,
                 "E_ex": E_ex,
                 "E_in": E_in,
                 "Delta_T": Delta_T,
                 "V_peak": V_peak,
                 }
neuron_params_in = copy.deepcopy(neuron_params_ex)
neuron_params_in['b'] = b_in

###############################################################################
# Poisson generator rate

p_rate = 200.0

################################################################################
# Configuration of the simulation kernel by the previously defined time
# resolution used in the simulation. Setting ``print_time`` to `True` prints the
# already processed simulation time as well as its percentage of the total
# simulation time.

nest.resolution = dt
nest.print_time = True
nest.overwrite_files = True

print("Building network")

###############################################################################
# Creation of the nodes using ``Create``. We store the returned handles in
# variables for later reference. Here the excitatory and inhibitory, as well
# as the poisson generator and two spike recorders. The spike recorders will
# later be used to record excitatory and inhibitory spikes. Properties of the
# nodes are specified via ``params``, which expects a dictionary.

nodes_ex = nest.Create(neuron_model, NE, params=neuron_params_ex)
nodes_in = nest.Create(neuron_model, NI, params=neuron_params_in)
noise = nest.Create("poisson_generator", params={"rate": p_rate, "start": 0.0, "stop": 50.0})
espikes = nest.Create("spike_recorder")
ispikes = nest.Create("spike_recorder")

###############################################################################
# Configuration of the spike recorders recording excitatory and inhibitory
# spikes by sending parameter dictionaries to ``set``. Setting the property
# `record_to` to *"ascii"* ensures that the spikes will be recorded to a file,
# whose name starts with the string assigned to the property `label`.

espikes.set(label="brunel-py-ex", record_to="ascii")
ispikes.set(label="brunel-py-in", record_to="ascii")

print("Connecting devices")

###############################################################################
# Definition of a synapse using ``CopyModel``, which expects the model name of
# a pre-defined synapse, the name of the customary synapse and an optional
# parameter dictionary. The parameters defined in the dictionary will be the
# default parameter for the customary synapse. Here we define one synapse for
# the excitatory and one for the inhibitory connections giving the
# previously defined weights and equal delays.

nest.CopyModel("static_synapse", "excitatory",
               {"weight": J_ex, "delay": delay})
nest.CopyModel("static_synapse", "inhibitory",
               {"weight": J_in, "delay": delay})

#################################################################################
# Connecting the previously defined poisson generator to the excitatory and
# inhibitory neurons using the excitatory synapse. Since the poisson
# generator is connected to all neurons in the population the default rule
# (``all_to_all``) of ``Connect`` is used. The synaptic properties are inserted
# via ``syn_spec`` which expects a dictionary when defining multiple variables or
# a string when simply using a pre-defined synapse.

nest.Connect(noise, nodes_ex[:int(len(nodes_ex)/50)], syn_spec="excitatory")
nest.Connect(noise, nodes_in[:int(len(nodes_in)/50)], syn_spec="excitatory")

###############################################################################
# Connecting the first ``N_rec`` nodes of the excitatory and inhibitory
# population to the associated spike recorders using excitatory synapses.
# Here the same shortcut for the specification of the synapse as defined
# above is used.

nest.Connect(nodes_ex[:N_rec], espikes, syn_spec="excitatory")
nest.Connect(nodes_in[:N_rec], ispikes, syn_spec="excitatory")

print("Connecting network")

print("Excitatory connections")

###############################################################################
# Connecting the excitatory population to all neurons using the pre-defined
# excitatory synapse. Beforehand, the connection parameter are defined in a
# dictionary. Here we use the connection rule ``fixed_indegree``,
# which requires the definition of the indegree. Since the synapse
# specification is reduced to assigning the pre-defined excitatory synapse it
# suffices to insert a string.

conn_params_ex = {'rule': 'fixed_indegree', 'indegree': CE}
nest.Connect(nodes_ex, nodes_ex + nodes_in, conn_params_ex, "excitatory")

print("Inhibitory connections")

###############################################################################
# Connecting the inhibitory population to all neurons using the pre-defined
# inhibitory synapse. The connection parameter as well as the synapse
# parameter are defined analogously to the connection from the excitatory
# population defined above.

conn_params_in = {'rule': 'fixed_indegree', 'indegree': CI}
nest.Connect(nodes_in, nodes_ex + nodes_in, conn_params_in, "inhibitory")

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
# Reading out the number of connections established using the excitatory and
# inhibitory synapse model. The numbers are summed up resulting in the total
# number of synapses.

num_synapses_ex = nest.GetDefaults("excitatory")["num_connections"]
num_synapses_in = nest.GetDefaults("inhibitory")["num_connections"]
num_synapses = num_synapses_ex + num_synapses_in

###############################################################################
# Establishing the time it took to build and simulate the network by taking
# the difference of the pre-defined time variables.

build_time = endbuild - startbuild
sim_time = endsimulate - endbuild

###############################################################################
# Printing the network properties, firing rates and building times.

print("Brunel network simulation (Python)")
print(f"Number of neurons : {N_neurons}")
print(f"Number of synapses: {num_synapses}")
print(f"       Excitatory : {num_synapses_ex}")
print(f"       Inhibitory : {num_synapses_in}")
print(f"Excitatory rate   : {rate_ex:.2f} Hz")
print(f"Inhibitory rate   : {rate_in:.2f} Hz")
print(f"Building time     : {build_time:.2f} s")
print(f"Simulation time   : {sim_time:.2f} s")

###############################################################################
# Plot a raster of the excitatory neurons and a histogram.

nest.raster_plot.from_device(espikes, hist=True)
plt.show()
