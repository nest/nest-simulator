# -*- coding: utf-8 -*-
#
# astrocyte_brunel.py
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
Random balanced network with astrocytes
---------------------------------------

This script simulates a random balanced network with excitatory and inhibitory
neurons and astrocytes. The astrocytes are modeled with ``astrocyte_lr_1994``,
implemented according to [1]_, [2]_, and [3]_. The neurons are modeled with
``aeif_cond_alpha_astro``, an adaptive exponential integrate-and-fire neuron
supporting neuron-astrocyte interactions.

The simulation results show how astrocytes affect neuronal excitability. The
astrocytic dynamics, the slow inward current in the neurons induced by the
astrocytes, and the raster plot of neuronal firings are shown in the created
figures.

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

:doc:`astrocyte_small_network`

"""

###############################################################################
# Import all necessary modules for simulation and plotting.

import random

import matplotlib.pyplot as plt
import nest
import numpy as np

###############################################################################
# Set simulation parameters.

sim_params = {
    "dt": 0.1,  # simulation resolution in ms
    "pre_sim_time": 100.0,  # pre-simulation time in ms (data not recorded)
    "sim_time": 1000.0,  # simulation time in ms
    "N_rec_spk": 100,  # number of neurons to record from with spike recorder
    "N_rec_mm": 50,  # number of nodes (neurons, astrocytes) to record from with multimeter
    "n_threads": 4,  # number of threads for NEST
    "seed": 100,  # seed for the random module
}

###############################################################################
# Set network parameters.

network_params = {
    "N_ex": 8000,  # number of excitatory neurons
    "N_in": 2000,  # number of inhibitory neurons
    "N_astro": 10000,  # number of astrocytes
    "p_primary": 0.1,  # connection probability between neurons
    "p_third_if_primary": 0.5,  # probability of each created neuron-neuron connection to be paired with one astrocyte
    "pool_size": 10,  # astrocyte pool size for each target neuron
    "pool_type": "random",  # astrocyte pool will be chosen randomly for each target neuron
    "poisson_rate": 2000,  # Poisson input rate for neurons
}

syn_params = {
    "w_a2n": 0.01,  # weight of astrocyte-to-neuron connection
    "w_e": 1.0,  # weight of excitatory connection in nS
    "w_i": -4.0,  # weight of inhibitory connection in nS
    "d_e": 2.0,  # delay of excitatory connection in ms
    "d_i": 1.0,  # delay of inhibitory connection in ms
}

###############################################################################
# Set astrocyte parameters.

astrocyte_model = "astrocyte_lr_1994"
astrocyte_params = {
    "IP3": 0.4,  # IP3 initial value in µM
    "delta_IP3": 0.5,  # Parameter determining the increase in astrocytic IP3 concentration induced by synaptic input
    "tau_IP3": 2.0,  # Time constant of the exponential decay of astrocytic IP3
}

###############################################################################
# Set neuron parameters.

neuron_model = "aeif_cond_alpha_astro"
tau_syn_ex = 2.0
tau_syn_in = 4.0

neuron_params_ex = {
    "tau_syn_ex": tau_syn_ex,  # excitatory synaptic time constant in ms
    "tau_syn_in": tau_syn_in,  # inhibitory synaptic time constant in ms
}

neuron_params_in = {
    "tau_syn_ex": tau_syn_ex,  # excitatory synaptic time constant in ms
    "tau_syn_in": tau_syn_in,  # inhibitory synaptic time constant in ms
}

###############################################################################
# This function creates the nodes and build the network. The astrocytes only
# respond to excitatory synaptic inputs; therefore, only the excitatory
# neuron-neuron connections are paired with the astrocytes. The
# ``TripartiteConnect()`` function and the ``tripartite_bernoulli_with_pool`` rule
# are used to create the connectivity of the network.


def create_astro_network(scale=1.0):
    """Create nodes for a neuron-astrocyte network.

    Nodes in a neuron-astrocyte network are created according to the give scale
    of the model. The nodes created include excitatory and inhibitory neruons,
    astrocytes, and a Poisson generator.

    Parameters
    ---------
    scale
        Scale of the model.

    Return values
    -------------
        Created nodes and Poisson generator.

    """
    print("Creating nodes ...")
    assert scale >= 1.0, "scale must be >= 1.0"
    nodes_ex = nest.Create(neuron_model, int(network_params["N_ex"] * scale), params=neuron_params_ex)
    nodes_in = nest.Create(neuron_model, int(network_params["N_in"] * scale), params=neuron_params_in)
    nodes_astro = nest.Create(astrocyte_model, int(network_params["N_astro"] * scale), params=astrocyte_params)
    nodes_noise = nest.Create("poisson_generator", params={"rate": network_params["poisson_rate"]})
    return nodes_ex, nodes_in, nodes_astro, nodes_noise


def connect_astro_network(nodes_ex, nodes_in, nodes_astro, nodes_noise, scale=1.0):
    """Connect the nodes in a neuron-astrocyte network.

    Nodes in a neuron-astrocyte network are connected. The connection
    probability between neurons is divided by a the given scale to preserve
    the expected number of connections for each node. The astrocytes are paired
    with excitatory connections only.

    Parameters
    ---------
    nodes_ex
        Nodes of excitatory neurons.
    nodes_in
        Nodes of inhibitory neurons.
    nodes_astro
        Nodes of astrocytes.
    node_noise
        Poisson generator.
    scale
        Scale of the model.

    """
    print("Connecting Poisson generator ...")
    assert scale >= 1.0, "scale must be >= 1.0"
    nest.Connect(nodes_noise, nodes_ex + nodes_in, syn_spec={"weight": syn_params["w_e"]})
    print("Connecting neurons and astrocytes ...")
    # excitatory connections are paired with astrocytes
    # conn_spec and syn_spec according to the "tripartite_bernoulli_with_pool" rule
    conn_params_e = {
        "rule": "tripartite_bernoulli_with_pool",
        "p_primary": network_params["p_primary"] / scale,
        "p_third_if_primary": network_params[
            "p_third_if_primary"
        ],  # "p_third_if_primary" is scaled along with "p_primary", so no further scaling is required
        "pool_size": network_params["pool_size"],
        "pool_type": network_params["pool_type"],
    }
    syn_params_e = {
        "primary": {
            "synapse_model": "tsodyks_synapse",
            "weight": syn_params["w_e"],
            "tau_psc": tau_syn_ex,
            "delay": syn_params["d_e"],
        },
        "third_in": {
            "synapse_model": "tsodyks_synapse",
            "weight": syn_params["w_e"],
            "tau_psc": tau_syn_ex,
            "delay": syn_params["d_e"],
        },
        "third_out": {"synapse_model": "sic_connection", "weight": syn_params["w_a2n"]},
    }
    nest.TripartiteConnect(nodes_ex, nodes_ex + nodes_in, nodes_astro, conn_spec=conn_params_e, syn_specs=syn_params_e)
    # inhibitory connections are not paired with astrocytes
    conn_params_i = {"rule": "pairwise_bernoulli", "p": network_params["p_primary"] / scale}
    syn_params_i = {
        "synapse_model": "tsodyks_synapse",
        "weight": syn_params["w_i"],
        "tau_psc": tau_syn_in,
        "delay": syn_params["d_i"],
    }
    nest.Connect(nodes_in, nodes_ex + nodes_in, conn_params_i, syn_params_i)


###############################################################################
# This function plots the dynamics in the astrocytes and their resultant output
# to the neurons. The IP3 and calcium in the astrocytes and the SIC in neurons
# are plotted. Means and standard deviations across sampled nodes are indicated
# by lines and shaded areas, respectively.


def plot_dynamics(astro_data, neuron_data, start):
    """Plot the dynamics in neurons and astrocytes.

    The dynamics in the given neuron and astrocyte nodes are plotted. The
    dynamics in clude IP3 and calcium in the astrocytes, and the SIC input to
    the neurons.

    Parameters
    ---------
    astro_data
        Data of IP3 and calcium dynamics in the astrocytes.
    neuron_data
        Data of SIC input to the neurons.
    start
        Start time of the plotted dynamics.

    """
    print("Plotting dynamics ...")
    # astrocyte data
    astro_mask = astro_data["times"] > start
    astro_ip3 = astro_data["IP3"][astro_mask]
    astro_cal = astro_data["Ca"][astro_mask]
    astro_times = astro_data["times"][astro_mask]
    astro_times_set = list(set(astro_times))
    ip3_means = np.array([np.mean(astro_ip3[astro_times == t]) for t in astro_times_set])
    ip3_sds = np.array([np.std(astro_ip3[astro_times == t]) for t in astro_times_set])
    cal_means = np.array([np.mean(astro_cal[astro_times == t]) for t in astro_times_set])
    cal_sds = np.array([np.std(astro_cal[astro_times == t]) for t in astro_times_set])
    # neuron data
    neuron_mask = neuron_data["times"] > start
    neuron_sic = neuron_data["I_SIC"][neuron_mask]
    neuron_times = neuron_data["times"][neuron_mask]
    neuron_times_set = list(set(neuron_times))
    sic_means = np.array([np.mean(neuron_sic[neuron_times == t]) for t in neuron_times_set])
    sic_sds = np.array([np.std(neuron_sic[neuron_times == t]) for t in neuron_times_set])
    # set plots
    fig, axes = plt.subplots(2, 1, sharex=True)
    color_ip3 = "tab:blue"
    color_cal = "tab:green"
    color_sic = "tab:purple"
    # astrocyte plot
    axes[0].set_title(f"{r'IP$_{3}$'} and {r'Ca$^{2+}$'} in astrocytes (n={len(set(astro_data['senders']))})")
    axes[0].set_ylabel(r"IP$_{3}$ ($\mu$M)")
    axes[0].tick_params(axis="y", labelcolor=color_ip3)
    axes[0].fill_between(
        astro_times_set, ip3_means + ip3_sds, ip3_means - ip3_sds, alpha=0.3, linewidth=0.0, color=color_ip3
    )
    axes[0].plot(astro_times_set, ip3_means, linewidth=2, color=color_ip3)
    ax = axes[0].twinx()
    ax.set_ylabel(r"Ca$^{2+}$ ($\mu$M)")
    ax.tick_params(axis="y", labelcolor=color_cal)
    ax.fill_between(
        astro_times_set, cal_means + cal_sds, cal_means - cal_sds, alpha=0.3, linewidth=0.0, color=color_cal
    )
    ax.plot(astro_times_set, cal_means, linewidth=2, color=color_cal)
    # neuron plot
    axes[1].set_title(f"SIC in neurons (n={len(set(neuron_data['senders']))})")
    axes[1].set_ylabel("SIC (pA)")
    axes[1].set_xlabel("Time (ms)")
    axes[1].fill_between(
        neuron_times_set, sic_means + sic_sds, sic_means - sic_sds, alpha=0.3, linewidth=0.0, color=color_sic
    )
    axes[1].plot(neuron_times_set, sic_means, linewidth=2, color=color_sic)


###############################################################################
# This is the main function for simulation. The network is created and the
# neurons and astrocytes are randomly chosen for recording. After simulation,
# recorded data of neurons and astrocytes are plotted.


def run_simulation():
    """Run simulation of a neuron-astrocyte network."""
    # NEST configuration
    nest.ResetKernel()
    nest.resolution = sim_params["dt"]
    nest.local_num_threads = sim_params["n_threads"]
    nest.print_time = True
    nest.overwrite_files = True

    # use random seed for reproducible sampling
    random.seed(sim_params["seed"])

    # simulation settings
    pre_sim_time = sim_params["pre_sim_time"]
    sim_time = sim_params["sim_time"]

    # create and connect nodes
    exc, inh, astro, noise = create_astro_network()
    connect_astro_network(exc, inh, astro, noise)

    # create and connect recorders (multimeter default resolution = 1 ms)
    sr_neuron = nest.Create("spike_recorder")
    mm_neuron = nest.Create("multimeter", params={"record_from": ["I_SIC"]})
    mm_astro = nest.Create("multimeter", params={"record_from": ["IP3", "Ca"]})

    # select nodes randomly and connect them with recorders
    print("Connecting recorders ...")
    neuron_list = (exc + inh).tolist()
    astro_list = astro.tolist()
    n_neuron_rec_spk = min(len(neuron_list), sim_params["N_rec_spk"])
    n_neuron_rec_mm = min(len(neuron_list), sim_params["N_rec_mm"])
    n_astro_rec = min(len(astro), sim_params["N_rec_mm"])
    neuron_list_for_sr = neuron_list[: min(len(neuron_list), n_neuron_rec_spk)]
    neuron_list_for_mm = sorted(random.sample(neuron_list, n_neuron_rec_mm))
    astro_list_for_mm = sorted(random.sample(astro_list, n_astro_rec))
    nest.Connect(neuron_list_for_sr, sr_neuron)
    nest.Connect(mm_neuron, neuron_list_for_mm)
    nest.Connect(mm_astro, astro_list_for_mm)

    # run pre-simulation
    print("Running pre-simulation ...")
    nest.Simulate(pre_sim_time)

    # run simulation
    print("Running simulation ...")
    nest.Simulate(sim_time)

    # read out recordings
    neuron_spikes = sr_neuron.events
    neuron_data = mm_neuron.events
    astro_data = mm_astro.events

    # make raster plot
    nest.raster_plot.from_device(
        sr_neuron, hist=True, title=f"Raster plot of neuron {neuron_list_for_sr[0]} to {neuron_list_for_sr[-1]}"
    )

    # plot dynamics in astrocytes and neurons
    plot_dynamics(astro_data, neuron_data, 0.0)

    # show plots
    plt.show()


###############################################################################
# Run simulation.

run_simulation()
