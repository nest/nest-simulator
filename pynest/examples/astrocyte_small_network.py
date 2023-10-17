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
network. The network in this script includes 20 neurons and 5 astrocytes. The
astrocyte is modeled with ``astrocyte_lr_1994``, implemented according to [1]_,
[2]_, and [3]_. The neuron is modeled with ``aeif_cond_alpha_astro``, an
adaptive exponential integrate and fire neuron supporting neuron-astrocyte
interactions.

The network is created with the TripartiteConnect() function and the
``tripartite_bernoulli_with_pool`` rule. This rule creates a tripartite
Bernoulli connectivity with the following principles:

1. For each pair of neurons, a Bernoulli trial with a probability ``p_primary``
determines if they will be connected.

2. For each neuron-neuron connection created, a Bernoulli trial with a probability
``p_cond_third`` determines if it will be paired with one astrocyte. The
selection of this particular astrocyte is confined by ``pool_size`` and
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
    neuron. If "random", a number (=``pool_size``) of astrocytes are randomly
    chosen from all astrocytes and assigned as the pool. If "block", the
    astrocytes pool is evenly distributed to the neurons in blocks without
    overlapping, and the defined ``pool_size`` has to be compatible with this
    arrangement.

* ``syn_specs`` parameters

  * ``primary``: specifications for the connections between neurons.

  * ``third_in``: specifications for the connections from neurons to astrocytes.

  * ``third_out``: specifications for the connections from astrocytes to neurons.

In this script, the network is created with the ``pool_type`` being "block".
``p_primary`` and ``p_cond_third`` are both chosen to be 1, so that all possible
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
number of all astrocytes.

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
spath = "astrocyte_small_network"
# spath = os.path.join("astrocyte_small_network", hl.md5(os.urandom(16)).hexdigest())
os.system(f"mkdir -p {spath}")
os.system(f"cp astrocyte_small_network.py {spath}") # (debug)

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
    "delta_IP3": 2.0, # parameter determining the increase in astrocytic IP3 concentration induced by synaptic input
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
# Functions for plotting.

def plot_connections(conn_n2n, conn_n2a, conn_a2n):
    # helper function to create lists of positions for source and target nodes, for plotting
    def get_set_and_list(dict_in):
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
    slist_n2n, tlist_n2n = get_set_and_list(conns_n2n.get())
    slist_n2a, alist_n2a = get_set_and_list(conns_n2a.get())
    alist_a2n, tlist_a2n = get_set_and_list(conns_a2n.get())
    # make plot
    fig, axs = plt.subplots(1, 1, figsize=(10, 8))
    # plot nodes (need the sets of node positions)
    axs.scatter(list(set(slist_n2a)), [2] * len(set(slist_n2a)), s=400, color="gray", marker="^", label="pre_neurons", zorder=3)
    axs.scatter(list(set(alist_a2n)), [1] * len(set(alist_a2n)), s=400, color="g", marker="o", label="astrocytes", zorder=3)
    axs.scatter(list(set(tlist_a2n)), [0] * len(set(tlist_a2n)), s=400, color="k", marker="^", label="post_neurons", zorder=3)
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
    plt.savefig(os.path.join(spath, "connections.png"))

def plot_vm(pre_data, post_data, data_path, start):
    print("Plotting V_m ...")
    # presynaptic data
    a = pre_data
    a_mask = a["times"] > start
    a_vm = a["V_m"][a_mask]
    a_t = a["times"][a_mask]
    t_a = list(set(a_t))
    m_pre_vm = np.array([np.mean(a_vm[a_t == t]) for t in t_a])
    s_pre_vm = np.array([np.std(a_vm[a_t == t]) for t in t_a])
    # postsynaptic data
    b = post_data
    b_mask = b["times"] > start
    b_vm = b["V_m"][b_mask]
    b_t = b["times"][b_mask]
    t_b = list(set(b_t))
    m_post_vm = np.array([np.mean(b_vm[b_t == t]) for t in t_b])
    s_post_vm = np.array([np.std(b_vm[b_t == t]) for t in t_b])
    # plots
    color_pre = "tab:blue"
    color_post = "tab:blue"
    fig, axes = plt.subplots(2, 1, sharex=True)
    # presynaptic
    axes[0].set_title(f"presynaptic neurons (n={len(set(a['senders']))})")
    axes[0].set_ylabel(r"$V_{m}$ (mV)")
    axes[0].fill_between(t_a, m_pre_vm + s_pre_vm, m_pre_vm - s_pre_vm, alpha=0.3, linewidth=0.0, color=color_pre)
    axes[0].plot(t_a, m_pre_vm, linewidth=2, color=color_pre)
    # postsynaptic
    axes[1].set_title(f"postsynaptic neurons (n={len(set(b['senders']))})")
    axes[1].set_ylabel(r"$V_{m}$ (mV)")
    axes[1].set_xlabel("Time (ms)")
    axes[1].fill_between(t_b, m_post_vm + s_post_vm, m_post_vm - s_post_vm, alpha=0.3, linewidth=0.0, color=color_post)
    axes[1].plot(t_b, m_post_vm, linewidth=2, color=color_post)
    # save
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "V_m.png"))
    plt.close()

def plot_dynamics(astro_data, neuron_data, data_path, start):
    print("Plotting dynamics ...")
    # astrocyte data
    a = astro_data
    a_mask = a["times"] > start
    a_ip3 = a["IP3"][a_mask]
    a_cal = a["Ca"][a_mask]
    a_t = a["times"][a_mask]
    t_astro = list(set(a_t))
    m_ip3 = np.array([np.mean(a_ip3[a_t == t]) for t in t_astro])
    s_ip3 = np.array([np.std(a_ip3[a_t == t]) for t in t_astro])
    m_cal = np.array([np.mean(a_cal[a_t == t]) for t in t_astro])
    s_cal = np.array([np.std(a_cal[a_t == t]) for t in t_astro])
    # neuron data
    b = neuron_data
    b_mask = b["times"] > start
    b_sic = b["I_SIC"][b_mask]
    b_t = b["times"][b_mask]
    t_neuro = list(set(b_t))
    m_sic = np.array([np.mean(b_sic[b_t == t]) for t in t_neuro])
    s_sic = np.array([np.std(b_sic[b_t == t]) for t in t_neuro])
    # plots
    str_ip3 = r"IP$_{3}$"
    str_cal = r"Ca$^{2+}$"
    color_ip3 = "tab:blue"
    color_cal = "tab:green"
    color_sic = "tab:purple"
    fig, axes = plt.subplots(2, 1, sharex=True)
    # astrocyte plot
    axes[0].set_title(f"{str_ip3} and {str_cal} in astrocytes (n={len(set(a['senders']))})")
    axes[0].set_ylabel(r"IP$_{3}$ ($\mu$M)")
    axes[0].tick_params(axis="y", labelcolor=color_ip3)
    axes[0].fill_between(t_astro, m_ip3 + s_ip3, m_ip3 - s_ip3, alpha=0.3, linewidth=0.0, color=color_ip3)
    axes[0].plot(t_astro, m_ip3, linewidth=2, color=color_ip3)
    ax = axes[0].twinx()
    ax.set_ylabel(r"Ca$^{2+}$ ($\mu$M)")
    ax.tick_params(axis="y", labelcolor=color_cal)
    ax.fill_between(t_astro, m_cal + s_cal, m_cal - s_cal, alpha=0.3, linewidth=0.0, color=color_cal)
    ax.plot(t_astro, m_cal, linewidth=2, color=color_cal)
    # neuron plot
    axes[1].set_title(f"SIC in postsynaptic neurons (n={len(set(a['senders']))})")
    axes[1].set_ylabel("SIC (pA)")
    axes[1].set_xlabel("Time (ms)")
    axes[1].fill_between(t_neuro, m_sic + s_sic, m_sic - s_sic, alpha=0.3, linewidth=0.0, color=color_sic)
    axes[1].plot(t_neuro, m_sic, linewidth=2, color=color_sic)
    # save
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "dynamics.png"))
    plt.close()

###############################################################################
# Run simulation and save results.

nest.Simulate(1000.0)
plot_connections(conns_n2n, conns_n2a, conns_a2n)
plot_vm(mm_pre_neurons.events, mm_post_neurons.events, spath, 0.0)
plot_dynamics(mm_astrocytes.events, mm_post_neurons.events, spath, 0.0)
