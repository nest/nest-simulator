# -*- coding: utf-8 -*-
#
# astrocyte_small_network.py
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
A small neuron-astrocyte network
------------------------------------------------------------

This script demonstrates an aproach with NEST to create a neuron-astrocyte
network. The network in this script includes 20 neurons and five astrocytes. The
astrocyte is modeled with ``astrocyte_lr_1994``, implemented according to [1]_,
[2]_, and [3]_. The neuron is modeled with ``aeif_cond_alpha_astro``, an
adaptive exponential integrate and fire neuron supporting neuron-astrocyte
interactions.

The network is created with the TripartiteConnect() function and the
``tripartite_bernoulli_with_pool`` rule. This rule creates a tripartite
Bernoulli connectivity with the following principles:

1. For each pair of neurons, a Bernoulli trial with a probability ``p_primary``
determines if they will be connected.

2. For each neuron-neuron connection created, a Bernoulli trial with a
probability ``p_cond_third`` determines if it will be paired with one astrocyte.
The selection of this particular astrocyte is confined by ``pool_size`` and
``pool_type`` (see below).

3. If a neuron-neuron connection is to be paired with an astrocyte, a connection
from the presynaptic (source) neuron to the astrocyte is created, and a
connection (``sic_connection``) from the astrocyte to the postsynaptic (target)
neuron is created.

The available connectivity parameters are as follows:

* ``conn_spec`` parameters

  * ``p_primary``: Connection probability between neurons.

  * ``p_cond_third``: Probability of each created neuron-neuron connection to be
    paired with one astrocyte.

  * ``pool_size``: The size of astrocyte pool for each target neuron. The
    target neuron can only be connected to astrocytes selected from its pool.

  * ``pool_type``: The way to determine the astrocyte pool for each target
    neuron. If specified "random", a number (``pool_size``) of astrocytes are
    randomly chosen from all astrocytes and assigned as the pool. If specified
    "block", the astrocytes are evenly distributed to the neurons in blocks
    without overlapping, and the specified ``pool_size`` has to be compatible
    with this arrangement.

* ``syn_specs`` parameters

  * ``primary``: specifications for the connections between neurons.

  * ``third_in``: specifications for the connections from neurons to astrocytes.

  * ``third_out``: specifications for the connections from astrocytes to neurons.

In this script, the network is created with the ``pool_type`` being "block".
``p_primary`` and ``p_cond_third`` are both set to one, so that all possible
connections are made. It can be seen from the result plot "connections.png" that
"block" distributes the astrocytes evenly to the postsynaptic neurons in blocks
without overlapping. The ``pool_size`` should be compatible with this
arrangement. In the case here, a ``pool_size`` of one is required.

With the created network, neuron-astrocyte interactions can be observed. The
presynaptic spikes induce the generation of IP3, which then changes the calcium
concentration in the astrocytes. This change in calcium then induces the slow
inward current (SIC) in the neurons through the ``sic_connection``. These
dynamics are shown in the plot "dynamics.png". The changes in membrane potential
in the presynaptic and postsynaptic neurons are shown in the plot "V_m.png".

The ``pool_type`` can be changed to "random" to see the results with random
astrocyte pools. In that case, the ``pool_size`` can be any from one to the
total number of astrocytes.

References
~~~~~~~~~~

.. [1] Li, Y. X., & Rinzel, J. (1994). Equations for InsP3 receptor-mediated
       [Ca2+]i oscillations derived from a detailed kinetic model: a
       Hodgkin-Huxley like formalism. Journal of theoretical Biology, 166(4),
       461-473. DOI: https://doi.org/10.1006/jtbi.1994.1041

.. [2] De Young, G. W., & Keizer, J. (1992). A single-pool inositol
       1,4,5-trisphosphate-receptor-based model for agonist-stimulated
       oscillations in Ca2+ concentration. Proceedings of the National Academy
       of Sciences, 89(20), 9895-9899. DOI:
       https://doi.org/10.1073/pnas.89.20.9895

.. [3] Nadkarni, S., & Jung, P. (2003). Spontaneous oscillations of dressed
       neurons: a new mechanism for epilepsy?. Physical review letters, 91(26),
       268101. DOI: https://doi.org/10.1103/PhysRevLett.91.268101

See Also
~~~~~~~~

:doc:`astrocyte_brunel`

"""

###############################################################################
# Import all necessary modules.

import hashlib as hl
import os

import matplotlib.pyplot as plt
import nest
import numpy as np

plt.rcParams.update({"font.size": 13})

###############################################################################
# Initialize NEST kernel and create folder to save data.

nest.ResetKernel()
save_path = "astrocyte_small_network"
os.system(f"mkdir -p {save_path}")

###############################################################################
# Network parameters.

n_neurons = 10  # number of source and target neurons
n_astrocytes = 5  # number of astrocytes
p_primary = 1.0  # connection probability between neurons
p_cond_third = 1.0  # probability of each created neuron-neuron connection to be paired with one astrocyte
pool_size = 1  # astrocyte pool size for each target neuron
pool_type = "block"  # the way to determine the astrocyte pool for each target neuron (change to "random" to see different results)

###############################################################################
# Astrocyte parameters.

astrocyte_model = "astrocyte_lr_1994"
astrocyte_params = {
    "IP3": 0.4,  # IP3 initial value in µM
    "delta_IP3": 2.0,  # parameter determining the increase in astrocytic IP3 concentration induced by synaptic input
    "tau_IP3": 10.0,  # time constant of the exponential decay of astrocytic IP3
}

###############################################################################
# Neuron parameters.

neuron_model = "aeif_cond_alpha_astro"
neuron_params = {
    "tau_syn_ex": 2.0,  # excitatory synaptic time constant in ms
    "I_e": 1000.0,  # external current input in pA
}

###############################################################################
# Create and connect populations and devices. The neurons and astrocytes are
# connected with multimeters to record their dynamics.

pre_neurons = nest.Create(neuron_model, n_neurons, params=neuron_params)
post_neurons = nest.Create(neuron_model, n_neurons, params=neuron_params)
astrocytes = nest.Create(astrocyte_model, n_astrocytes, params=astrocyte_params)
nest.TripartiteConnect(
    pre_neurons,
    post_neurons,
    astrocytes,
    conn_spec={
        "rule": "tripartite_bernoulli_with_pool",
        "p_primary": p_primary,
        "p_cond_third": p_cond_third,
        "pool_size": pool_size,
        "pool_type": pool_type,
    },
    syn_specs={
        "primary": {"synapse_model": "tsodyks_synapse", "weight": 1.0, "delay": 1.0},
        "third_in": {"synapse_model": "tsodyks_synapse", "weight": 1.0, "delay": 1.0},
        "third_out": {"synapse_model": "sic_connection", "weight": 1.0, "delay": 1.0},
    },
)
mm_pre_neurons = nest.Create("multimeter", params={"record_from": ["V_m"]})
mm_post_neurons = nest.Create("multimeter", params={"record_from": ["V_m", "I_SIC"]})
mm_astrocytes = nest.Create("multimeter", params={"record_from": ["IP3", "Ca"]})
nest.Connect(mm_pre_neurons, pre_neurons)
nest.Connect(mm_post_neurons, post_neurons)
nest.Connect(mm_astrocytes, astrocytes)

###############################################################################
# Get connection data. The data are used to plot the network connectivity.

conns_a2n = nest.GetConnections(astrocytes, post_neurons)
conns_n2n = nest.GetConnections(pre_neurons, post_neurons)
conns_n2a = nest.GetConnections(pre_neurons, astrocytes)

###############################################################################
# Functions for plotting. For neuron and astrocyte data, means and standard
# deviations across sampled nodes are indicated by lines and shaded areas,
# respectively.


# Plot all connections between neurons and astrocytes
def plot_connections(conn_n2n, conn_n2a, conn_a2n, data_path):
    # helper function to create lists of positions for source and target nodes, for plotting
    def get_node_positions(dict_in):
        source_list = dict_in["source"] - np.unique(dict_in["source"]).mean()
        target_list = dict_in["target"] - np.unique(dict_in["target"]).mean()
        return source_list, target_list

    # helper function to set plot frames invisible
    def set_frame_invisible(ax):
        ax.get_xaxis().set_visible(False)
        ax.get_yaxis().set_visible(False)
        ax.spines["top"].set_visible(False)
        ax.spines["bottom"].set_visible(False)
        ax.spines["left"].set_visible(False)
        ax.spines["right"].set_visible(False)

    print("Plotting connections ...")
    # prepare data (lists of node positions)
    slist_n2n, tlist_n2n = get_node_positions(conns_n2n.get())
    slist_n2a, alist_n2a = get_node_positions(conns_n2a.get())
    alist_a2n, tlist_a2n = get_node_positions(conns_a2n.get())
    # make plot
    fig, axs = plt.subplots(1, 1, figsize=(10, 8))
    # plot nodes (need the sets of node positions)
    axs.scatter(
        list(set(slist_n2a)), [2] * len(set(slist_n2a)), s=400, color="gray", marker="^", label="pre_neurons", zorder=3
    )
    axs.scatter(
        list(set(alist_a2n)), [1] * len(set(alist_a2n)), s=400, color="g", marker="o", label="astrocytes", zorder=3
    )
    axs.scatter(
        list(set(tlist_a2n)), [0] * len(set(tlist_a2n)), s=400, color="k", marker="^", label="post_neurons", zorder=3
    )
    # plot connections
    for sx, tx in zip(slist_n2n, tlist_n2n):
        axs.plot([sx, tx], [2, 0], linestyle=":", color="b", alpha=0.5, linewidth=1)
    for sx, tx in zip(slist_n2a, alist_n2a):
        axs.plot([sx, tx], [2, 1], linestyle="-", color="orange", alpha=0.5, linewidth=2)
    for sx, tx in zip(alist_a2n, tlist_a2n):
        axs.plot([sx, tx], [1, 0], linestyle="-", color="g", alpha=0.8, linewidth=4)
    # tweak and save
    axs.legend(bbox_to_anchor=(0.5, 1.1), loc="upper center", ncol=3)
    set_frame_invisible(axs)
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "connections.png"))


# Helper function to mask data by start time
def mask_data_by_start(data_in, start):
    data_out = {}
    times = data_in["times"]
    for key, value in data_in.items():
        data = data_in[key]
        data_out[key] = data[times > start]
    return data_out


# Helper function to get times, means, and standard deviation of data for plotting
def get_plot_data(data_in, variable):
    times_all = data_in["times"]
    ts = list(set(data_in["times"]))
    means = np.array([np.mean(data_in[variable][times_all == t]) for t in ts])
    sds = np.array([np.std(data_in[variable][times_all == t]) for t in ts])
    return ts, means, sds


# Plot membrane potentials of presynaptic and postsynaptic neurons
def plot_vm(pre_data, post_data, data_path, start):
    print("Plotting V_m ...")
    # get presynaptic data
    pre_data_masked = mask_data_by_start(pre_data, start)
    pre_times, pre_vm_mean, pre_vm_sd = get_plot_data(pre_data_masked, "V_m")
    # get postsynaptic data
    post_data_masked = mask_data_by_start(post_data, start)
    post_times, post_vm_mean, post_vm_sd = get_plot_data(post_data_masked, "V_m")
    # set plots
    fig, axes = plt.subplots(2, 1, sharex=True)
    color_pre = color_post = "tab:blue"
    # plot presynaptic membrane potential
    axes[0].set_title(f"presynaptic neurons (n={len(set(pre_data['senders']))})")
    axes[0].set_ylabel(r"$V_{m}$ (mV)")
    axes[0].fill_between(
        pre_times, pre_vm_mean + pre_vm_sd, pre_vm_mean - pre_vm_sd, alpha=0.3, linewidth=0.0, color=color_pre
    )
    axes[0].plot(pre_times, pre_vm_mean, linewidth=2, color=color_pre)
    # plot postsynaptic  membrane potential
    axes[1].set_title(f"postsynaptic neurons (n={len(set(post_data['senders']))})")
    axes[1].set_ylabel(r"$V_{m}$ (mV)")
    axes[1].set_xlabel("Time (ms)")
    axes[1].fill_between(
        post_times, post_vm_mean + post_vm_sd, post_vm_mean - post_vm_sd, alpha=0.3, linewidth=0.0, color=color_post
    )
    axes[1].plot(post_times, post_vm_mean, linewidth=2, color=color_post)
    # save
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "V_m.png"))
    plt.close()


# Plot dynamics in astrocytes and SIC in neurons
def plot_dynamics(astro_data, neuron_data, data_path, start):
    print("Plotting dynamics ...")
    # get astrocyte data
    astro_data_masked = mask_data_by_start(astro_data, start)
    astro_times, astro_ip3_mean, astro_ip3_sd = get_plot_data(astro_data_masked, "IP3")
    astro_times, astro_ca_mean, astro_ca_sd = get_plot_data(astro_data_masked, "Ca")
    # get neuron data
    neuron_data_masked = mask_data_by_start(neuron_data, start)
    neuron_times, neuron_sic_mean, neuron_sic_sd = get_plot_data(neuron_data_masked, "I_SIC")
    # set plots
    fig, axes = plt.subplots(2, 1, sharex=True)
    color_ip3 = "tab:blue"
    color_cal = "tab:green"
    color_sic = "tab:purple"
    # plot astrocyte data
    n_astro = len(set(astro_data_masked["senders"]))
    axes[0].set_title(f"IP$_{{3}}$ and Ca$^{{2+}}$ in astrocytes (n={n_astro})")
    axes[0].set_ylabel(r"IP$_{3}$ ($\mu$M)")
    axes[0].tick_params(axis="y", labelcolor=color_ip3)
    axes[0].fill_between(
        astro_times,
        astro_ip3_mean + astro_ip3_sd,
        astro_ip3_mean - astro_ip3_sd,
        alpha=0.3,
        linewidth=0.0,
        color=color_ip3,
    )
    axes[0].plot(astro_times, astro_ip3_mean, linewidth=2, color=color_ip3)
    ax = axes[0].twinx()
    ax.set_ylabel(r"Ca$^{2+}$ ($\mu$M)")
    ax.tick_params(axis="y", labelcolor=color_cal)
    ax.fill_between(
        astro_times, astro_ca_mean + astro_ca_sd, astro_ca_mean - astro_ca_sd, alpha=0.3, linewidth=0.0, color=color_cal
    )
    ax.plot(astro_times, astro_ca_mean, linewidth=2, color=color_cal)
    # plot neuron data
    n_neuron = len(set(neuron_data_masked["senders"]))
    axes[1].set_title(f"SIC in postsynaptic neurons (n={n_neuron})")
    axes[1].set_ylabel("SIC (pA)")
    axes[1].set_xlabel("Time (ms)")
    axes[1].fill_between(
        neuron_times,
        neuron_sic_mean + neuron_sic_sd,
        neuron_sic_mean - neuron_sic_sd,
        alpha=0.3,
        linewidth=0.0,
        color=color_sic,
    )
    axes[1].plot(neuron_times, neuron_sic_mean, linewidth=2, color=color_sic)
    # save
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "dynamics.png"))
    plt.close()


###############################################################################
# Run simulation and save results.

nest.Simulate(1000.0)
plot_connections(conns_n2n, conns_n2a, conns_a2n, save_path)
plot_vm(mm_pre_neurons.events, mm_post_neurons.events, save_path, 0.0)
plot_dynamics(mm_astrocytes.events, mm_post_neurons.events, save_path, 0.0)
