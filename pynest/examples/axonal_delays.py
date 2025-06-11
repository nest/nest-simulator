# -*- coding: utf-8 -*-
#
# axonal_delays.py
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
Example of using axonal delays in NEST
--------------------------------------

This script demonstrates the use of axonal delays, by comparing neuronal simulations with
and without axonal delays (i.e., only dendritic delays). It analyzes the impact of axonal delays
on synaptic weight distributions using the Jensen-Shannon divergence
and the number of corrections required for different fractions of axonal delays.
It uses the synapse model ``stdp_pl_synapse_hom_ax_delay``, which supports the
use of axonal delays.

For more information see :ref:`delays`.
"""


########################################
# First, we import all necessary modules

import os.path

import matplotlib.pyplot as plt
import nest
import numpy as np
import seaborn as sns
from scipy.spatial.distance import jensenshannon

########################################
# Next we set the parameters


tau_syn = 0.32582722403722841
neuron_params = {
    "E_L": 0.0,
    "C_m": 250.0,
    "tau_m": 10.0,
    "t_ref": 0.5,
    "V_th": 20.0,
    "V_reset": 0.0,
    "tau_syn_ex": tau_syn,
    "tau_syn_in": tau_syn,
    "tau_minus": 30.0,
    "V_m": 5.7,
}

stdp_params = {"alpha": 0.0513, "mu": 0.4, "tau_plus": 15.0, "weight": 45.0}

NE = 9000
eta = 4.92

single_column_in = 3.348


#################################################################################
# And then we set up the simulation


def run(T, axonal_delay, dendritic_delay, enable_stdp, use_ax_delay):
    # Enable or disable STDP
    if enable_stdp:
        stdp_params["lambda"] = 1.0
    else:
        stdp_params["lambda"] = 0.0
    # Reset the NEST kernel and set simulation parameters
    nest.ResetKernel()
    nest.local_num_threads = 8
    nest.resolution = 0.1
    nest.rng_seed = 42
    nest.set_verbosity("M_ERROR")

    # Create devices and neurons
    E_ext = nest.Create("poisson_generator", 1, {"rate": eta * 1000.0})
    E_pg = nest.Create("poisson_generator", 1, params={"rate": 10.0})
    I_pg = nest.Create("poisson_generator", 1, params={"rate": 10.0 * NE / 5})
    E_neurons = nest.Create("parrot_neuron", NE)
    I_neurons = nest.Create("parrot_neuron", 1)
    post_neuron = nest.Create("iaf_psc_alpha", 1, params=neuron_params)
    sr = nest.Create("spike_recorder")

    # Set synapse parameters with axonal and dendritic delays
    syn_params_copy = stdp_params.copy()
    if use_ax_delay:
        syn_params_copy["dendritic_delay"] = dendritic_delay
        syn_params_copy["axonal_delay"] = axonal_delay
        # Use the new synapse model with axonal delays
        syn_model = "stdp_pl_synapse_hom_ax_delay"
    else:
        syn_params_copy["delay"] = dendritic_delay + axonal_delay
        syn_model = "stdp_pl_synapse_hom"

    nest.SetDefaults(syn_model, syn_params_copy)

    # Connect neurons and devices
    nest.Connect(E_pg, E_neurons)
    nest.Connect(I_pg, I_neurons)
    nest.Connect(E_neurons, post_neuron, syn_spec={"synapse_model": syn_model})
    nest.Connect(I_neurons, post_neuron, syn_spec={"weight": 45.0 * -5})
    nest.Connect(E_ext, post_neuron, syn_spec={"weight": 45.0})
    nest.Connect(post_neuron, sr)

    # Run the simulation
    nest.Simulate(T)

    return sr, post_neuron


#####################################################################
# Set the font sizes for graph


def set_font_sizes(small=8, medium=10, large=12, family="Arial"):
    # plt.rc('text', usetex=True)
    plt.rc("font", size=small)  # controls default text sizes
    plt.rc("axes", titlesize=small)  # fontsize of the axes title
    plt.rc("axes", labelsize=medium)  # fontsize of the x and y labels
    plt.rc("xtick", labelsize=small)  # fontsize of the tick labels
    plt.rc("ytick", labelsize=small)  # fontsize of the tick labels
    plt.rc("legend", fontsize=small)  # legend fontsize
    plt.rc("figure", titlesize=large)  # fontsize of the figure title
    plt.rc("font", family=family)


#####################################################################
# Measure the similarity between two probability distributions using
# Jensen-Shannon divergence


def jensen_shannon_divergence(weights_correction, weights_no_correction):
    p = np.histogram(weights_correction, bins=100000, density=True)[0]
    q = np.histogram(weights_no_correction, bins=100000, density=True)[0]
    return round(jensenshannon(p, q) ** 2, 3)


####################################################################################
# Get different weight distributions of purely dendritic and axonal+dendritic delays


_, post_neuron = run(10000.0, 5.0, 0.0, enable_stdp=True, use_ax_delay=True)
weights_correction = nest.GetConnections(target=post_neuron, synapse_model="stdp_pl_synapse_hom_ax_delay").get("weight")
_, post_neuron = run(10000.0, 5.0, 0.0, enable_stdp=True, use_ax_delay=False)
weights_no_correction = nest.GetConnections(target=post_neuron, synapse_model="stdp_pl_synapse_hom").get("weight")

print(
    "Jensen-Shannon divergence of weight distributions:",
    jensen_shannon_divergence(weights_correction, weights_no_correction),
)

#########################################################################
#  Plot the weight distributions

fig, ax = plt.subplots(figsize=(single_column_in, 3.5))
set_font_sizes()
sns.distplot(
    weights_correction,
    hist=False,
    kde=True,
    kde_kws={"fill": True, "linewidth": 3},
    label="axonal delay and correction",
    ax=ax,
)
sns.distplot(
    weights_no_correction,
    hist=False,
    kde=True,
    kde_kws={"fill": True, "linewidth": 3},
    label="purely dendritic delay",
    ax=ax,
)
plt.setp(ax.spines.values(), linewidth=2)
ax.set_xlabel("Synaptic weight [pA]")
ax.tick_params(width=2)
plt.legend()
fig.tight_layout()
plt.show()


#########################################################################
# Measure number of corrections for increasing fractions of axonal delays


ax_perc = np.arange(0.0, 1.01, 0.01, dtype=np.float32)
num_corrections = np.empty(101, dtype=np.int32)
for i, p in enumerate(ax_perc):
    run(500.0, 5.0 * p, 5.0 * (1 - p), enable_stdp=False, use_ax_delay=True)
    num_corrections[i] = nest.kernel_status["num_corrections"]

##########################################################################
# Plot the number of corrections

fig, ax = plt.subplots(figsize=(single_column_in, 3.5))
set_font_sizes()
plt.plot(ax_perc, num_corrections, color="black")
plt.setp(ax.spines.values(), linewidth=2)
ax.set_xlabel("Fraction of axonal delay")
ax.set_ylabel("Number of corrections")  # average per neuron
ax.tick_params(width=2)
fig.tight_layout()
plt.show()
