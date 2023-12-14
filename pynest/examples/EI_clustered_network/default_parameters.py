# -*- coding: utf-8 -*-
#
# default_parameters.py
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

"""PyNEST EI-clustered network: default parameters for EI-clustered network
-------------------------------------------

default parameters for EI-clustered network simulation
"""
import numpy as np

############################################
# general parameters
############################################
eps = np.finfo(float).eps
n_jobs = 1

dt = 0.1
simtime = 1000.
warmup = 0.
record_voltage = False
record_from = 'all'
recording_interval = dt
return_weights = False

############################################
# neuron parameters
############################################
neuron_type = 'iaf_psc_exp'  # 'gif_psc_exp'
E_L = 0.
C_m = 1.
tau_E = 20.
tau_I = 10.
t_ref = 5.
V_th_E = 20.
V_th_I = 20.
V_r = 0.
I_xE = 1.
I_xI = 2.
delta_I_xE = 0.
delta_I_xI = 0.
I_th_E = 1.25
I_th_I = 0.78
V_m = 'rand'

# synapse parameters
tau_syn_ex = 3.
tau_syn_in = 2.
delay = 0.1  # synaptic delay

# Distribution of synaptic weights
# available distributions= https://nest-simulator.readthedocs.io/en/stable/guides/connection_management.html#dist-params

DistParams = {'distribution': 'normal', 'sigma': 0.0, 'fraction': False}

syn_params = {"U": 0.2, "u": 0.0, "tau_rec": 120.0,
              "tau_fac": 0.0}

############################################
# network parameters
############################################

# number of units
N_E = 1200
N_I = 300

# cluster number
Q = 6
# cluster weight ratios
jplus = np.ones((2, 2))

# connection probabilities
ps = np.array([[0.2, 0.5], [0.5, 0.5]])

# connections strengths
# weights are js/sqrt(N)
# nan means they are calculated
js = np.ones((2, 2)) * np.nan
# factors for inhibitory weights
ge = 1.2
gi = 1.
gie = 1.

# factor multiplied with weights
s = 1.
fixed_indegree = False

############################################
# stimulation parameters
############################################
stim_clusters = None  # clusters to be stimulated
stim_amp = 0.  # amplitude of the stimulation current in pA
stim_starts = []  # list of stimulation start times
stim_ends = []  # list of stimulation end times
