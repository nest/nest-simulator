# -*- coding: utf-8 -*-
#
# clopath_synapse_small_network.py
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
Clopath Rule: Bidirectional connections
-----------------------------------------

This script simulates a small network of ten excitatory and three
inhibitory ``aeif_psc_delta_clopath`` neurons. The neurons are randomly connected
and driven by 500 Poisson generators. The synapses from the Poisson generators
to the excitatory population and those among the neurons of the network
are Clopath synapses. The rate of the Poisson generators is modulated with
a Gaussian profile whose center shifts randomly each 100 ms between ten
equally spaced positions.
This setup demonstrates that the Clopath synapse is able to establish
bidirectional connections. The example is adapted from [1]_ (cf. fig. 5).

References
~~~~~~~~~~~

.. [1] Clopath C, Büsing L, Vasilaki E, Gerstner W (2010). Connectivity reflects coding:
       a model of voltage-based STDP with homeostasis.
       Nature Neuroscience 13:3, 344--352
"""

import nest
import numpy as np
import matplotlib.pyplot as pl
import random

##############################################################################
# Set the parameters

simulation_time = 1.0e4
resolution = 0.1
delay = resolution

# Poisson_generator parameters
pg_A = 30.      # amplitude of Gaussian
pg_sigma = 10.  # std deviation

nest.ResetKernel()
nest.SetKernelStatus({'resolution': resolution})

# Create neurons and devices
nrn_model = 'aeif_psc_delta_clopath'
nrn_params = {'V_m': -30.6,
              'g_L': 30.0,
              'w': 0.0,
              'tau_plus': 7.0,
              'tau_minus': 10.0,
              'tau_w': 144.0,
              'a': 4.0,
              'C_m': 281.0,
              'Delta_T': 2.0,
              'V_peak': 20.0,
              't_clamp': 2.0,
              'A_LTP': 8.0e-6,
              'A_LTD': 14.0e-6,
              'A_LTD_const': False,
              'b': 0.0805,
              'u_ref_squared': 60.0**2}

pop_exc = nest.Create(nrn_model, 10, nrn_params)
pop_inh = nest.Create(nrn_model, 3, nrn_params)

##############################################################################
# We need parrot neurons since Poisson generators can only be connected
# with static connections

pop_input = nest.Create('parrot_neuron', 500)  # helper neurons
pg = nest.Create('poisson_generator', 500)
wr = nest.Create('weight_recorder', 1)

##############################################################################
# First connect Poisson generators to helper neurons
nest.Connect(pg, pop_input, 'one_to_one', {'synapse_model': 'static_synapse',
                                           'weight': 1.0, 'delay': delay})

##############################################################################
# Create all the connections

nest.CopyModel('clopath_synapse', 'clopath_input_to_exc',
               {'Wmax': 3.0})
conn_dict_input_to_exc = {'rule': 'all_to_all'}
syn_dict_input_to_exc = {'synapse_model': 'clopath_input_to_exc',
                         'weight': nest.random.uniform(0.5, 2.0),
                         'delay': delay}
nest.Connect(pop_input, pop_exc, conn_dict_input_to_exc,
             syn_dict_input_to_exc)

# Create input->inh connections
conn_dict_input_to_inh = {'rule': 'all_to_all'}
syn_dict_input_to_inh = {'synapse_model': 'static_synapse',
                         'weight': nest.random.uniform(0.0, 0.5),
                         'delay': delay}
nest.Connect(pop_input, pop_inh, conn_dict_input_to_inh, syn_dict_input_to_inh)

# Create exc->exc connections
nest.CopyModel('clopath_synapse', 'clopath_exc_to_exc',
               {'Wmax': 0.75, 'weight_recorder': wr[0]})
syn_dict_exc_to_exc = {'synapse_model': 'clopath_exc_to_exc', 'weight': 0.25,
                       'delay': delay}
conn_dict_exc_to_exc = {'rule': 'all_to_all', 'allow_autapses': False}
nest.Connect(pop_exc, pop_exc, conn_dict_exc_to_exc, syn_dict_exc_to_exc)

# Create exc->inh connections
syn_dict_exc_to_inh = {'synapse_model': 'static_synapse',
                       'weight': 1.0, 'delay': delay}
conn_dict_exc_to_inh = {'rule': 'fixed_indegree', 'indegree': 8}
nest.Connect(pop_exc, pop_inh, conn_dict_exc_to_inh, syn_dict_exc_to_inh)

# Create inh->exc connections
syn_dict_inh_to_exc = {'synapse_model': 'static_synapse',
                       'weight': 1.0, 'delay': delay}
conn_dict_inh_to_exc = {'rule': 'fixed_outdegree', 'outdegree': 6}
nest.Connect(pop_inh, pop_exc, conn_dict_inh_to_exc, syn_dict_inh_to_exc)

##############################################################################
# Randomize the initial membrane potential

pop_exc.V_m = nest.random.normal(-60., 25.)
pop_inh.V_m = nest.random.normal(-60., 25.)

##############################################################################
# Simulation divided into intervals of 100ms for shifting the Gaussian

sim_interval = 100.
for i in range(int(simulation_time/sim_interval)):
    # set rates of poisson generators
    rates = np.empty(500)
    # pg_mu will be randomly chosen out of 25,75,125,...,425,475
    pg_mu = 25 + random.randint(0, 9) * 50
    for j in range(500):
        rates[j] = pg_A * np.exp((-1 * (j - pg_mu)**2) / (2 * pg_sigma**2))
        pg[j].set({'rate': rates[j]*1.75})
    nest.Simulate(sim_interval)

##############################################################################
# Plot results

fig1, axA = pl.subplots(1, sharex=False)

# Plot synapse weights of the synapses within the excitatory population
# Sort weights according to sender and reshape
exc_conns = nest.GetConnections(pop_exc, pop_exc)
exc_conns_senders = np.array(list(exc_conns.sources()))
exc_conns_targets = np.array(list(exc_conns.targets()))
exc_conns_weights = np.array(exc_conns.get('weight'))
idx_array = np.argsort(exc_conns_senders)
targets = np.reshape(exc_conns_targets[idx_array], (10, 10-1))
weights = np.reshape(exc_conns_weights[idx_array], (10, 10-1))

# Sort according to target
for i, (trgs, ws) in enumerate(zip(targets, weights)):
    idx_array = np.argsort(trgs)
    weights[i] = ws[idx_array]

weight_matrix = np.zeros((10, 10))
tu9 = np.triu_indices_from(weights)
tl9 = np.tril_indices_from(weights, -1)
tu10 = np.triu_indices_from(weight_matrix, 1)
tl10 = np.tril_indices_from(weight_matrix, -1)
weight_matrix[tu10[0], tu10[1]] = weights[tu9[0], tu9[1]]
weight_matrix[tl10[0], tl10[1]] = weights[tl9[0], tl9[1]]

# Difference between initial and final value
init_w_matrix = np.ones((10, 10))*0.25
init_w_matrix -= np.identity(10)*0.25

caxA = axA.imshow(weight_matrix - init_w_matrix)
cbarB = fig1.colorbar(caxA, ax=axA)
axA.set_xticks([0, 2, 4, 6, 8])
axA.set_xticklabels(['1', '3', '5', '7', '9'])
axA.set_yticks([0, 2, 4, 6, 8])
axA.set_xticklabels(['1', '3', '5', '7', '9'])
axA.set_xlabel("to neuron")
axA.set_ylabel("from neuron")
axA.set_title("Change of syn weights before and after simulation")
pl.show()
