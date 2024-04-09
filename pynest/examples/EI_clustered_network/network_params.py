# -*- coding: utf-8 -*-
#
# network_params.py
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

"""PyNEST EI-clustered network: Network Parameters
------------------------------------------------

A dictionary with parameters defining the network and neuron parameters.

"""

import numpy as np

net_dict = {
    ############################################
    # neuron parameters
    ############################################
    # neuron model
    "neuron_type": "iaf_psc_exp",
    # Resting potential [mV]
    "E_L": 0.0,
    # Membrane capacitance [pF]
    "C_m": 1.0,
    # Membrane time constant for excitatory neurons [ms]
    "tau_E": 20.0,
    # Membrane time constant for inhibitory neurons [ms]
    "tau_I": 10.0,
    # Refractory period [ms]
    "t_ref": 5.0,
    # Threshold for excitatory neurons [mV]
    "V_th_E": 20.0,
    # Threshold for inhibitory neurons [mV]
    "V_th_I": 20.0,
    # Reset potential [mV]
    "V_r": 0.0,
    # synaptic time constant for excitatory synapses [ms]
    "tau_syn_ex": 5.0,
    # synaptic time constant for inhibitory synapses [ms]
    "tau_syn_in": 5.0,
    # synaptic delay [ms]
    "delay": 0.1,
    # Feed forward excitatory input [rheobase current]
    "I_th_E": 1.25,
    # Feed forward inhibitory input [rheobase current]
    "I_th_I": 0.78,
    # distribution of feed forward input,
    # I_th*[1-delta_I_../2, 1+delta_I_../2]
    "delta_I_xE": 0.0,  # excitatory
    "delta_I_xI": 0.0,  # inhibitory
    # initial membrane potential: either a float (in mV) to initialize all neurons to a fixed value
    # or "rand" for randomized values: "V_th_{E,I}" - 20 * nest.random.lognormal(0, 1)
    "V_m": "rand",
    ############################################
    # network parameters
    ############################################
    # number of excitatory neurons in the network
    # Neurons per cluster N_E/n_clusters
    "N_E": 4000,
    # number of inhibitory neurons in the network
    "N_I": 1000,
    # Number of clusters
    "n_clusters": 20,
    # connection probabilities
    # baseline_conn_prob[0, 0] E to E, baseline_conn_prob[0, 1] I to E,
    # baseline_conn_prob[1, 0] E to I, baseline_conn_prob[1, 1] I to I
    "baseline_conn_prob": np.array([[0.2, 0.5], [0.5, 0.5]]),
    # inhibitory weight ratios - scaling like random balanced network
    "gei": 1.2,  # I to E
    "gie": 1.0,  # E to I
    "gii": 1.0,  # I to I
    # additional scaling factor for all weights
    # - can be used to scale weights with network size
    "s": 1.0,
    # fixed indegree - otherwise established with probability ps
    "fixed_indegree": False,
    # cluster network by "weight" or "probabilities"
    "clustering": "weight",
    # ratio excitatory to inhibitory clustering,
    # rj = 0 means no clustering, which resembles a clustered network
    # with a blanket of inhibition
    "rj": 0.82,
    # excitatory clustering factor,
    # rep = 1 means no clustering, reselmbles a balanced random network
    "rep": 6.0,
}
