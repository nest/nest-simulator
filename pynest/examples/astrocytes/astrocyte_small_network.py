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
--------------------------------

This script shows how to create a neuron-astrocyte network in NEST. The network
in this script includes 20 neurons and five astrocytes. The astrocytes are modeled
with ``astrocyte_lr_1994``, implemented according to [1]_, [2]_, and [3]_. The
neurons are modeled with ``aeif_cond_alpha_astro``, an adaptive exponential
integrate-and-fire neuron supporting neuron-astrocyte interactions.

The network is created with the ``TripartiteConnect()`` function and the
``tripartite_bernoulli_with_pool`` rule (see :ref:`tripartite_connectivity` for
detailed descriptions). This rule creates a tripartite Bernoulli connectivity
with the following principles:

1. For each pair of neurons, a Bernoulli trial with a probability ``p_primary``
determines if a ``tsodyks_synapse`` will be created between them.

2. For each neuron-neuron connection created, a Bernoulli trial with a
probability ``p_third_if_primary`` determines if it will be paired with one astrocyte.
The selection of this particular astrocyte is confined by ``pool_size`` and
``pool_type`` (see below).

3. If a neuron-neuron connection is to be paired with an astrocyte, a
``tsodyks_synapse`` from the presynaptic (source) neuron to the astrocyte
is created, and a ``sic_connection`` from the astrocyte to the postsynaptic
(target) neuron is created.

The available connectivity parameters are as follows:

* ``conn_spec`` parameters

  * ``p_primary``: Connection probability between neurons.

  * ``p_third_if_primary``: Probability of each created neuron-neuron connection to be
    paired with one astrocyte.

  * ``pool_size``: The size of astrocyte pool for each target neuron. The
    astrocyte pool of each target neuron is determined before making
    connections. Each target neuron can only be connected to astrocytes
    in its pool.

  * ``pool_type``: The way to determine the astrocyte pool for each target
    neuron. If ``"random"``, a number (``pool_size``) of astrocytes are
    randomly chosen from all astrocytes (without replacement) and assigned as
    the pool. If ``"block"``, the astrocytes are evenly distributed to the
    neurons in blocks without overlapping, and the specified ``pool_size`` has
    to be compatible with this arrangement. See :ref:`tripartite_connectivity`
    for more details about ``pool_type``.

* ``syn_specs`` parameters

  * ``primary``: ``syn_spec`` specifications for the connections between neurons.

  * ``third_in``: ``syn_spec`` specifications for the connections from neurons to astrocytes.

  * ``third_out``: ``syn_spec`` specifications for the connections from astrocytes to neurons.

In this script, the network is created with the ``pool_type`` being ``"block"``.
``p_primary`` and ``p_third_if_primary`` are both set to one to include as many
connections as possible. One of the created figures shows the connections between
neurons and astrocytes as a result (note that multiple connections may exist
between a pair of nodes; this is not obvious in the figure since connections
drawn later cover previous ones). It can be seen from the figure that ``"block"``
results in astrocytes being connected to postsynaptic neurons in non-overlapping
blocks. The ``pool_size`` should be compatible with this arrangement; in the case
here, a ``pool_size`` of one is required. Users can try different parameters
(e.g. ``p_primary`` = 0.5 and ``p_third_if_primary`` = 0.5) to see changes in
connections.

With the created network, neuron-astrocyte interactions can be observed. The
presynaptic spikes induce the generation of IP3, which then changes the calcium
concentration in the astrocytes. This change in calcium then induces the slow
inward current (SIC) in the neurons through the ``sic_connection``. The changes
in membrane potential of the presynaptic and postsynaptic neurons are also
recorded. These data are shown in the created figures.

The ``pool_type`` can be changed to "random" to see the results with random
astrocyte pools. In that case, the ``pool_size`` can be any from one to the
total number of astrocytes.

See :ref:`tripartite_connectivity` for more details about the
``TripartiteConnect()`` function and the ``tripartite_bernoulli_with_pool``
rule.

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

import matplotlib.pyplot as plt
import nest
import numpy as np

###############################################################################
# Initialize NEST kernel.

nest.ResetKernel()

###############################################################################
# Set network parameters.

n_neurons = 10  # number of source and target neurons
n_astrocytes = 5  # number of astrocytes
p_primary = 1.0  # connection probability between neurons
p_third_if_primary = 1.0  # probability of each created neuron-neuron connection to be paired with one astrocyte
pool_size = 1  # astrocyte pool size for each target neuron
pool_type = "block"  # the way to determine the astrocyte pool for each target neuron

###############################################################################
# Set astrocyte parameters.

astrocyte_model = "astrocyte_lr_1994"
astrocyte_params = {
    "IP3": 0.4,  # IP3 initial value in µM
    "delta_IP3": 2.0,  # parameter determining the increase in astrocytic IP3 concentration induced by synaptic input
    "tau_IP3": 10.0,  # time constant of the exponential decay of astrocytic IP3
}

###############################################################################
# Set neuron parameters.

neuron_model = "aeif_cond_alpha_astro"
neuron_params = {
    "tau_syn_ex": 2.0,  # excitatory synaptic time constant in ms
    "I_e": 1000.0,  # external current input in pA
}

###############################################################################
# Functions for plotting.


def plot_connections(conn_n2n, conn_n2a, conn_a2n, pre_id_list, post_id_list, astro_id_list):
    """Plot all connections between neurons and astrocytes.

    Parameters
    ---------
    conn_n2n
        Data of neuron-to-neuron connections.
    conn_n2a
        Data of neuron-to-astrocyte connections.
    conn_a2n
        Data of astrocyte-to-neuron connections.
    pre_id_list
        ID list of presynaptic neurons.
    post_id_list
        ID list of postsynaptic neurons.
    astro_id_list
        ID list of astrocytes.

    """
    print("Plotting connections ...")

    # helper function to create lists of connection positions
    def get_conn_positions(dict_in, source_center, target_center):
        source_list = np.array(dict_in["source"]) - source_center
        target_list = np.array(dict_in["target"]) - target_center
        return source_list.tolist(), target_list.tolist()

    # prepare data (lists of node positions, list of connection positions)
    pre_id_center = (pre_id_list[0] + pre_id_list[-1]) / 2
    post_id_center = (post_id_list[0] + post_id_list[-1]) / 2
    astro_id_center = (astro_id_list[0] + astro_id_list[-1]) / 2
    slist_n2n, tlist_n2n = get_conn_positions(conns_n2n.get(), pre_id_center, post_id_center)
    slist_n2a, tlist_n2a = get_conn_positions(conns_n2a.get(), pre_id_center, astro_id_center)
    slist_a2n, tlist_a2n = get_conn_positions(conns_a2n.get(), astro_id_center, post_id_center)
    # initialize figure
    fig, axs = plt.subplots(1, 1, figsize=(10, 8))
    # plot nodes and connections
    # source neuron nodes
    axs.scatter(
        np.array(pre_id_list) - pre_id_center,
        [2] * len(pre_id_list),
        s=300,
        color="gray",
        marker="^",
        label="pre_neurons",
        zorder=3,
    )
    # neuron-to-neuron connections
    for i, (sx, tx) in enumerate(zip(slist_n2n, tlist_n2n)):
        label = "neuron-to-neuron" if i == 0 else None
        axs.plot([sx, tx], [2, 0], linestyle=":", color="b", alpha=0.3, linewidth=2, label=label)
    # target neuron nodes
    axs.scatter(
        np.array(post_id_list) - post_id_center,
        [0] * len(post_id_list),
        s=300,
        color="k",
        marker="^",
        label="post_neurons",
        zorder=3,
    )
    # neuron-to-astrocyte connections
    for i, (sx, tx) in enumerate(zip(slist_n2a, tlist_n2a)):
        label = "neuron-to-astrocyte" if i == 0 else None
        axs.plot([sx, tx], [2, 1], linestyle="-", color="orange", alpha=0.5, linewidth=2, label=label)
    # astrocyte nodes
    axs.scatter(
        np.array(astro_id_list) - astro_id_center,
        [1] * len(astro_id_list),
        s=300,
        color="g",
        marker="o",
        label="astrocytes",
        zorder=3,
    )
    # astrocyte-to-neuron connections
    for i, (sx, tx) in enumerate(zip(slist_a2n, tlist_a2n)):
        label = "astrocyte-to-neuron" if i == 0 else None
        axs.plot([sx, tx], [1, 0], linestyle="-", color="g", linewidth=4, label=label)
    # set legends
    legend = axs.legend(bbox_to_anchor=(0.5, 1.15), loc="upper center", ncol=3, labelspacing=1.5)
    legend.set_frame_on(False)
    # set axes and frames invisible
    axs.get_xaxis().set_visible(False)
    axs.get_yaxis().set_visible(False)
    axs.spines["top"].set_visible(False)
    axs.spines["bottom"].set_visible(False)
    axs.spines["left"].set_visible(False)
    axs.spines["right"].set_visible(False)


def get_plot_data(data_in, variable):
    """Helper function to get times, means, and standard deviations of data for plotting.

    Parameters
    ---------
    data_in
        Data containing the variable to be plotted.
    Variable
        Variable to be plotted.

    Return values
    -------------
        Times, means, and standard deviations of the variable to be plotted.

    """
    times_all = data_in["times"]
    ts = list(set(data_in["times"]))
    means = np.array([np.mean(data_in[variable][times_all == t]) for t in ts])
    sds = np.array([np.std(data_in[variable][times_all == t]) for t in ts])
    return ts, means, sds


def plot_vm(pre_data, post_data, start):
    """Plot membrane potentials of presynaptic and postsynaptic neurons.

    Parameters
    ---------
    pre_data
        Data of the presynaptic neurons.
    post_data
        Data of the postsynaptic neurons.
    start
       Start time of the data to be plotted.

    """

    print("Plotting V_m ...")
    # get presynaptic data
    pre_times, pre_vm_mean, pre_vm_sd = get_plot_data(pre_data, "V_m")
    # get postsynaptic data
    post_times, post_vm_mean, post_vm_sd = get_plot_data(post_data, "V_m")
    # set plots
    fig, axes = plt.subplots(2, 1, sharex=True)
    color_pre = color_post = "tab:blue"
    # plot presynaptic membrane potential
    axes[0].set_title(f"membrane potential of presynaptic neurons (n={len(set(pre_data['senders']))})")
    axes[0].set_ylabel(r"$V_{m}$ (mV)")
    axes[0].fill_between(
        pre_times, pre_vm_mean + pre_vm_sd, pre_vm_mean - pre_vm_sd, alpha=0.3, linewidth=0.0, color=color_pre
    )
    axes[0].plot(pre_times, pre_vm_mean, linewidth=2, color=color_pre)
    # plot postsynaptic  membrane potential
    axes[1].set_title(f"membrane potential of postsynaptic neurons (n={len(set(post_data['senders']))})")
    axes[1].set_ylabel(r"$V_{m}$ (mV)")
    axes[1].set_xlabel("Time (ms)")
    axes[1].fill_between(
        post_times, post_vm_mean + post_vm_sd, post_vm_mean - post_vm_sd, alpha=0.3, linewidth=0.0, color=color_post
    )
    axes[1].plot(post_times, post_vm_mean, linewidth=2, color=color_post)


def plot_dynamics(astro_data, neuron_data, start):
    """Plot dynamics in astrocytes and SIC in neurons.

    Parameters
    ---------
    astro_data
        Data of the astrocytes.
    neuron_data
        Data of the neurons.
    start
       Start time of the data to be plotted.

    """
    print("Plotting dynamics ...")
    # get astrocyte data
    astro_times, astro_ip3_mean, astro_ip3_sd = get_plot_data(astro_data, "IP3")
    astro_times, astro_ca_mean, astro_ca_sd = get_plot_data(astro_data, "Ca")
    # get neuron data
    neuron_times, neuron_sic_mean, neuron_sic_sd = get_plot_data(neuron_data, "I_SIC")
    # set plots
    fig, axes = plt.subplots(2, 1, sharex=True)
    color_ip3 = "tab:blue"
    color_cal = "tab:green"
    color_sic = "tab:purple"
    # plot astrocyte data
    n_astro = len(set(astro_data["senders"]))
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
    n_neuron = len(set(neuron_data["senders"]))
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
        "p_third_if_primary": p_third_if_primary,
        "pool_size": pool_size,
        "pool_type": pool_type,
    },
    syn_specs={
        "primary": {"synapse_model": "tsodyks_synapse"},
        "third_in": {"synapse_model": "tsodyks_synapse"},
        "third_out": {"synapse_model": "sic_connection"},
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
# Run simulation.

nest.Simulate(1000.0)
plot_connections(conns_n2n, conns_n2a, conns_a2n, pre_neurons.tolist(), post_neurons.tolist(), astrocytes.tolist())
plot_vm(mm_pre_neurons.events, mm_post_neurons.events, 0.0)
plot_dynamics(mm_astrocytes.events, mm_post_neurons.events, 0.0)
plt.show()
