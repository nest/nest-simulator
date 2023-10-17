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
------------------------------------------------------------

This script simulates a random balanced network with excitatory and inhibitory
neurons and astrocytes. The astrocyte is modeled with ``astrocyte_lr_1994``,
implemented according to [1]_, [2]_, and [3]_. The neuron is modeled with
``aeif_cond_alpha_astro``, an adaptive exponential integrate and fire neuron
supporting neuron-astrocyte interactions.

The simulation results of this network demonstrate an astrocytic effect on
neuronal excitability. With the slow inward current (SIC) induced by the
astrocytes through the ``sic_connection``, the neurons show higher firing rates
and more synchronized actitivity. Here, the local synchrony is quantified by
pairwise spike count correlations, and the global synchrony by an approach
suggested in [4]_. The result of this synchrony analysis is shown in the plot
"neuron_synchrony.png". The dynamics in the astrocytes and the SIC in the
neurons are shown in the plot "dynamics.png".

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

.. [4] Golomb, D. (2007). Neuronal synchrony measures. Scholarpedia, 2(1), 1347.
       DOI: http://dx.doi.org/10.4249/scholarpedia.1347

See Also
~~~~~~~~

:doc:`astrocyte_small_network`

"""

###############################################################################
# Import all necessary modules for simulation, analysis and plotting.

import hashlib
import json
import os
import random

import matplotlib.pyplot as plt
import nest
import nest.raster_plot
import numpy as np

plt.rcParams.update({"font.size": 13})

###############################################################################
# Simulation parameters.

sim_params = {
    "dt": 0.1,  # simulation resolution in ms
    "pre_sim_time": 100.0,  # pre-simulation time in ms (excluded from analysis)
    "sim_time": 100.0,  # simulation time in ms
    "N_rec_spk": 100,  # number of samples (neuron) for spike detector
    "N_rec_mm": 50,  # number of samples (neuron, astrocyte) for multimeter
    "n_threads": 4,  # number of threads for NEST
    "seed": 100,  # seed for the "random" module (not NEST)
}

###############################################################################
# Network parameters.

network_params = {
    "N_ex": 8000,  # number of excitatory neurons
    "N_in": 2000,  # number of inhibitory neurons
    "N_astro": 10000,  # number of astrocytes
    "p_primary": 0.1,  # connection probability between neurons
    "p_cond_third": 0.5,  # probability of each created neuron-neuron connection to be paired with one astrocyte
    "pool_size": 10,  # astrocyte pool size for each target neuron
    "pool_type": "random",  # astrocyte pool will be chosen randomly for each target neuron
    "poisson_rate": 2000,  # Poisson input rate for neurons
}

syn_params = {
    "synapse_model": "tsodyks_synapse",
    "w_a2n": 0.01,  # weight of astrocyte-to-neuron connection
    "w_e": 1.0,  # weight of excitatory connection in nS
    "w_i": -4.0,  # weight of inhibitory connection in nS
    "d_e": 2.0,  # delay of excitatory connection in ms
    "d_i": 1.0,  # delay of inhibitory connection in ms
}

###############################################################################
# Astrocyte parameters.

astrocyte_model = "astrocyte_lr_1994"
astrocyte_params = {
    "IP3": 0.4,  # IP3 initial value in µM
    "delta_IP3": 0.5,  # Parameter determining the increase in astrocytic IP3 concentration induced by synaptic input
    "tau_IP3": 2.0,  # Time constant of the exponential decay of astrocytic IP3
}

###############################################################################
# Neuron parameters.

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
# TripartiteConnect() function and the "tripartite_bernoulli_with_pool" rule are
# used to create the connectivity of the network.


def create_astro_network(scale=1.0):
    """Create nodes for a neuron-astrocyte network."""
    print("Creating nodes ...")
    assert scale >= 1.0, "scale must be >= 1.0"
    nodes_ex = nest.Create(neuron_model, int(network_params["N_ex"] * scale), params=neuron_params_ex)
    nodes_in = nest.Create(neuron_model, int(network_params["N_in"] * scale), params=neuron_params_in)
    nodes_astro = nest.Create(astrocyte_model, int(network_params["N_astro"] * scale), params=astrocyte_params)
    nodes_noise = nest.Create("poisson_generator", params={"rate": network_params["poisson_rate"]})
    return nodes_ex, nodes_in, nodes_astro, nodes_noise


def connect_astro_network(nodes_ex, nodes_in, nodes_astro, nodes_noise, scale=1.0):
    """Connect the nodes in a neuron-astrocyte network.
    The astrocytes are paired with excitatory connections only.
    """
    print("Connecting Poisson generator ...")
    assert scale >= 1.0, "scale must be >= 1.0"
    nest.Connect(
        nodes_noise, nodes_ex + nodes_in, syn_spec={"synapse_model": "static_synapse", "weight": syn_params["w_e"]}
    )
    print("Connecting neurons and astrocytes ...")
    # excitatory connections are paired with astrocytes
    # conn_spec and syn_spec according to the "tripartite_bernoulli_with_pool" rule
    conn_params_e = {
        "rule": "tripartite_bernoulli_with_pool",
        "p_primary": network_params["p_primary"] / scale,
        "p_cond_third": network_params["p_cond_third"],
        "pool_size": network_params["pool_size"],
        "pool_type": network_params["pool_type"],
    }
    syn_params_e = {
        "primary": {
            "synapse_model": syn_params["synapse_model"],
            "weight": syn_params["w_e"],
            "tau_psc": tau_syn_ex,
            "delay": syn_params["d_e"],
        },
        "third_in": {
            "synapse_model": syn_params["synapse_model"],
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
        "synapse_model": syn_params["synapse_model"],
        "weight": syn_params["w_i"],
        "tau_psc": tau_syn_in,
        "delay": syn_params["d_i"],
    }
    nest.Connect(nodes_in, nodes_ex + nodes_in, conn_params_i, syn_params_i)


###############################################################################
# This function calculates the pairwise spike count correlations. For each pair
# of neurons, the correlation coefficient (Pearson's r) of their spike count
# histograms is calculated. The result of all pairs are returned.


def get_corr(hlist):
    """Calculate pairwise correlations for a list of histograms."""
    coef_list = []
    n_pair_pass = 0
    n_pair_fail = 0
    for i, hist1 in enumerate(hlist):
        idxs = list(range(i + 1, len(hlist)))
        for j in idxs:
            hist2 = hlist[j]
            if np.sum(hist1) != 0 and np.sum(hist2) != 0:
                coef = np.corrcoef(hist1, hist2)[0, 1]
                coef_list.append(coef)
                n_pair_pass += 1
            else:
                n_pair_fail += 1
    if n_pair_fail > 0:
        print(f"n_pair_fail = {n_pair_fail}!")

    return coef_list, n_pair_pass, n_pair_fail


###############################################################################
# These two functions calculate and plot the synchrony of neuronal firings.
# Histograms of spike counts of all neurons are obtained to evaluate local and
# global synchrony. The local synchrony is evaluated with average pairwise spike
# count correlation, and the global synchrony is evaluated with the variance of
# average spike count/average of variance of individual spike count.


def calc_synchrony(neuron_spikes, n_neuron_rec_spk, start, end, data_path, N=100, bw=10):
    # get data
    senders = neuron_spikes["senders"][neuron_spikes["times"] > start]
    times = neuron_spikes["times"][neuron_spikes["times"] > start]
    rate = len(senders) / (end - start) * 1000.0 / n_neuron_rec_spk
    print(f"Mean neuronal firing rate = {rate:.2f} spikes/s (n = {n_neuron_rec_spk})")
    # sample neurons
    set_senders = list(set(senders))
    n_for_sync = min(len(set_senders), N)
    print(f"n of neurons for synchrony analysis = {n_for_sync}")
    sampled = random.sample(set_senders, n_for_sync)
    times = times[np.isin(senders, sampled)]
    senders = senders[np.isin(senders, sampled)]
    # make spike count histograms of individual neurons
    bins = np.arange(start, end + 0.1, bw)  # time bins
    hists = [np.histogram(times[senders == x], bins)[0].tolist() for x in set(senders)]
    # make spiking histogram of all sampled neurons
    hist_global = (np.histogram(times, bins)[0] / len(set(senders))).tolist()
    # calculate local and global synchrony
    print("Calculating neuronal local and global synchrony ...")
    coefs, n_pair_pass, n_pair_fail = get_corr(hists)  # local
    lsync_mu, lsync_sd = np.mean(coefs), np.std(coefs)
    gsync = np.var(hist_global) / np.mean(np.var(hists, axis=1))  # global
    return rate, coefs, lsync_mu, lsync_sd, gsync, n_for_sync


def plot_synchrony(coefs, title, data_path):
    print("Plotting pairwise spike count correlations ...")
    plt.hist(coefs)
    plt.title(title)
    plt.xlabel("Pairwise spike count correlation (Pearson's r)")
    plt.ylabel("n of pairs")
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "neuron_synchrony.png"))
    plt.close()


###############################################################################
# This function plots the dynamics in the astrocytes and their resultant output
# to the neurons. The IP3 and calcium in the astrocytes and the SIC in neurons
# are plotted. Means and standard deviations across sampled nodes are indicated
# by lines and shaded areas, respectively.


def plot_dynamics(astro_data, neuron_data, data_path, start):
    # plot dynamics
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
    # plots
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
    # save
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "dynamics.png"))
    plt.close()


###############################################################################
# This is the main function for simulation. Here, the network is created and the
# neurons and astrocytes are randomly chosen to be connected with the recording
# devices. The sample numbers are determined by sim_params. After simulation,
# neuron and astrocyte data are analyzed and plotted.


def run_simulation(data_path):
    # NEST configuration
    nest.ResetKernel()
    nest.resolution = sim_params["dt"]
    nest.local_num_threads = sim_params["n_threads"]
    nest.print_time = True
    nest.overwrite_files = True

    # Create data folder
    os.system(f"mkdir -p {data_path}")

    # Simulation settings
    pre_sim_time = sim_params["pre_sim_time"]
    sim_time = sim_params["sim_time"]

    # Create and connect nodes
    exc, inh, astro, noise = create_astro_network()
    connect_astro_network(exc, inh, astro, noise)

    # Create and connect recorders (multimeter default resolution = 1 ms)
    sr_neuron = nest.Create("spike_recorder")
    mm_neuron = nest.Create("multimeter", params={"record_from": ["I_SIC"]})
    mm_astro = nest.Create("multimeter", params={"record_from": ["IP3", "Ca"]})

    # Run pre-simulation
    print("Running pre-simulation ...")
    nest.Simulate(pre_sim_time)

    # Select nodes randomly and connect them with recorders
    print("Connecting recorders ...")
    neuron_list = (exc + inh).tolist()
    astro_list = astro.tolist()
    n_neuron_rec_spk = min(len(neuron_list), sim_params["N_rec_spk"])
    n_neuron_rec_mm = min(len(neuron_list), sim_params["N_rec_mm"])
    n_astro_rec = min(len(astro), sim_params["N_rec_mm"])
    neuron_list_for_sr = sorted(random.sample(neuron_list, n_neuron_rec_spk))
    neuron_list_for_mm = sorted(random.sample(neuron_list, n_neuron_rec_mm))
    astro_list_for_mm = sorted(random.sample(astro_list, n_astro_rec))
    nest.Connect(neuron_list_for_sr, sr_neuron)
    nest.Connect(mm_neuron, neuron_list_for_mm)
    nest.Connect(mm_astro, astro_list_for_mm)

    # Run simulation
    print("Running simulation ...")
    nest.Simulate(sim_time)

    # Read out recordings
    neuron_spikes = sr_neuron.events
    neuron_data = mm_neuron.events
    astro_data = mm_astro.events

    # Save script and parameters (debug)
    os.system(f"cp astrocyte_brunel.py {data_path}")
    params = {
        "sim_params": sim_params,
        "network_params": network_params,
        "syn_params": syn_params,
        "neuron_model": neuron_model,
        "neuron_params_ex": neuron_params_ex,
        "neuron_params_in": neuron_params_in,
        "astrocyte_model": astrocyte_model,
        "astrocyte_params": astrocyte_params,
    }
    with open(os.path.join(data_path, "params.json"), "w") as f:
        json.dump(params, f, indent=4)

    # Make raster plot and histogram
    nest.raster_plot.from_device(sr_neuron, hist=True, title="")
    plt.savefig(os.path.join(data_path, "neuron_raster.png"), bbox_inches="tight")
    plt.close()

    # Calculate and plot synchrony
    rate, coefs, lsync_mu, lsync_sd, gsync, n_for_sync = calc_synchrony(
        neuron_spikes, n_neuron_rec_spk, pre_sim_time, pre_sim_time + sim_time, data_path
    )
    title = (
        f"Firing rate={rate:.2f} spikes/s (n={n_neuron_rec_spk}) \n"
        + f"Local sync.={lsync_mu:.3f}$\\pm${lsync_sd:.3f}, Global sync.={gsync:.3f}\n"
        + f"(n for synchrony analysis={n_for_sync})\n"
    )
    plot_synchrony(coefs, title, data_path)
    print(f"Local synchrony = {lsync_mu:.3f}+-{lsync_sd:.3f}")
    print(f"Global synchrony = {gsync:.3f}")

    # Plot dynamics in astrocytes and neurons
    plot_dynamics(astro_data, neuron_data, data_path, pre_sim_time)


###############################################################################
# Run simulation.

random.seed(sim_params["seed"])
hash = hashlib.md5(os.urandom(16)).hexdigest()
run_simulation("astrocyte_brunel")
# run_simulation(os.path.join("astrocyte_brunel", hash))
