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
neurons and astrocytes. The ``astrocyte_lr_1994`` model is according to the
articles [1]_, [2]_, and [3]_. The ``aeif_cond_alpha_astro`` model is an
adaptive exponential integrate and fire neuron supporting neuron-astrocyte
interactions.

The network is created using the ``pairwise_bernoulli_astro`` rule, with the
``tsodyks_synapse`` as the connections from neurons to neruons and astrocytes,
and the ``sic_connection`` as the connections from astrocytes to neurons.

This network is an example of an astrocytic effect on neuronal excitabitlity.
With the slow inward current (SIC) delivered from the astrocytes through the
``sic_connection``, the neurons show higher firing rates and more synchronized
bursting actitivity. The degrees of local and global synchrony are quantitized
by pairwise spike count correlations and a measure of global synchrony in [4]_
respectively, as shown in a plot made in this script (neuron_synchrony.png).
Plots of astrocytic dynamics and SIC in neurons are also made in this script.

References
~~~~~~~~~~

.. [1] De Young, G. W., & Keizer, J. (1992). A single-pool inositol
       1,4,5-trisphosphate-receptor-based model for agonist-stimulated
       oscillations in Ca2+ concentration. Proceedings of the National Academy
       of Sciences, 89(20), 9895-9899. DOI:
       https://doi.org/10.1073/pnas.89.20.9895

.. [2] Li, Y. X., & Rinzel, J. (1994). Equations for InsP3 receptor-mediated
       [Ca2+]i oscillations derived from a detailed kinetic model: a
       Hodgkin-Huxley like formalism. Journal of theoretical Biology, 166(4),
       461-473. DOI: https://doi.org/10.1006/jtbi.1994.1041

.. [3] Nadkarni S, and Jung P. Spontaneous oscillations of dressed neurons: A
       new mechanism for epilepsy? Physical Review Letters, 91:26. DOI:
       https://doi.org/10.1103/PhysRevLett.91.268101

.. [4] Golomb, D. (2007). Neuronal synchrony measures. Scholarpedia, 2(1), 1347.
       DOI: http://dx.doi.org/10.4249/scholarpedia.1347

See Also
~~~~~~~~

:doc:`astrocyte_connect`

"""

###############################################################################
# Import all necessary modules for simulation, analysis and plotting. Scipy
# should be imported before nest.

import os
import sys
import time
import numpy as np
import random
import hashlib
import json

import nest
import nest.raster_plot
import matplotlib.pyplot as plt
plt.rcParams.update({'font.size': 13})

###############################################################################
# Simulation parameters.

sim_params = {
    "dt": 0.1,  # simulation resolution in ms
    "pre_sim_time": 100.0,  # pre-simulation time in ms
    "sim_time": 1000.0,  # simulation time in ms
    "N_rec_spk": 100,  # number of samples (neuron) for spike detector
    "N_rec_mm": 50,  # number of samples (neuron, astrocyte) for multimeter
    }

###############################################################################
# Network parameters.

network_params = {
    "N_ex": 8000,  # number of excitatory neurons
    "N_in": 2000,  # number of inhibitory neurons
    "N_astro": 10000,  # number of astrocytes
    "p": 0.1,  # neuron-neuron connection probability.
    "p_syn_astro": 0.5,  # synapse-astrocyte pairing probability
    "max_astro_per_target": 10,  # max number of astrocytes per target neuron
    "astro_pool_by_index": False,  # Astrocyte pool selection by index
    "poisson_rate": 2000,  # rate of Poisson input
    }

syn_params = {
    "synapse_model": "tsodyks_synapse",  # model of neuron-to-neuron and neuron-to-astrocyte connections
    "astro2post": "sic_connection",  # model of astrocyte-to-neuron connection
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
    'IP3': 0.4,  # IP3 initial value in uM
    'delta_IP3': 0.5,  # Step increase in IP3 concentration with each unit synaptic weight received by the astrocyte in uM
    'tau_IP3': 2.0,  # Time constant of astrocytic IP3 degradation in ms
    }

###############################################################################
# Neuron parameters.

neuron_model = "aeif_cond_alpha_astro"
tau_syn_ex = 2.0
tau_syn_in = 4.0

neuron_params_ex = {
    "tau_syn_ex": tau_syn_ex, # excitatory synaptic time constant in ms
    "tau_syn_in": tau_syn_in, # inhibitory synaptic time constant in ms
    }

neuron_params_in = {
    "tau_syn_ex": tau_syn_ex, # excitatory synaptic time constant in ms
    "tau_syn_in": tau_syn_in, # inhibitory synaptic time constant in ms
    }

###############################################################################
# Function for network building.

def create_astro_network(scale=1.0):
    """Create nodes for a neuron-astrocyte network."""
    print("Creating nodes ...")
    nodes_ex = nest.Create(
        neuron_model, int(network_params["N_ex"]*scale), params=neuron_params_ex)
    nodes_in = nest.Create(
        neuron_model, int(network_params["N_in"]*scale), params=neuron_params_in)
    nodes_astro = nest.Create(
        astrocyte_model, int(network_params["N_astro"]*scale), params=astrocyte_params)
    nodes_noise = nest.Create(
        "poisson_generator", params={"rate": network_params["poisson_rate"]}
        )
    return nodes_ex, nodes_in, nodes_astro, nodes_noise

def connect_astro_network(nodes_ex, nodes_in, nodes_astro, nodes_noise, scale=1.0):
    """Connect the nodes in a neuron-astrocyte network.
    The astrocytes are paired with excitatory connections only.
    """
    print("Connecting Poisson generator ...")
    assert scale >= 1.0, "scale must be >= 1.0"
    syn_params_noise = {
        "synapse_model": "static_synapse", "weight": syn_params["w_e"]
        }
    nest.Connect(
        nodes_noise, nodes_ex + nodes_in, syn_spec=syn_params_noise
        )
    print("Connecting neurons and astrocytes ...")
    conn_params_e = {
        "rule": "pairwise_bernoulli_astro",
        "astrocyte": nodes_astro,
        "p": network_params["p"]/scale,
        "p_syn_astro": network_params["p_syn_astro"],
        "max_astro_per_target": network_params["max_astro_per_target"],
        "astro_pool_by_index": network_params["astro_pool_by_index"],
        }
    syn_params_e = {
        "synapse_model": syn_params["synapse_model"],
        "weight_pre2post": syn_params["w_e"],
        "tau_psc": tau_syn_ex,
        "astro2post": syn_params["astro2post"],
        "weight_astro2post": syn_params["w_a2n"],
        "delay_pre2post": syn_params["d_e"],
        "delay_pre2astro": syn_params["d_e"],
        }
    conn_params_i = {"rule": "pairwise_bernoulli", "p": network_params["p"]/scale}
    syn_params_i = {
        "synapse_model": syn_params["synapse_model"],
        "weight": syn_params["w_i"],
        "tau_psc": tau_syn_in,
        "delay": syn_params["d_i"],
        }
    nest.Connect(nodes_ex, nodes_ex + nodes_in, conn_params_e, syn_params_e)
    nest.Connect(nodes_in, nodes_ex + nodes_in, conn_params_i, syn_params_i)

    return nodes_ex, nodes_in, nodes_astro, nodes_noise

###############################################################################
# Function for calculating correlation.

def get_corr(hlist):
    """Calculate pairwise correlation coefficients for a list of histograms."""
    coef_list = []
    n_pass = 0
    n_fail = 0
    for i, hist1 in enumerate(hlist):
        idxs = list(range(i + 1, len(hlist)))
        for j in idxs:
            hist2 = hlist[j]
            if np.sum(hist1) != 0 and np.sum(hist2) != 0:
                coef = np.corrcoef(hist1, hist2)[0, 1]
                coef_list.append(coef)
                n_pass += 1
            else:
                n_fail += 1

    return coef_list, n_pass, n_fail

###############################################################################
# Function for calculating and plotting neuronal synchrony.

def plot_synchrony(neuron_spikes, data_path, start, end, N=100, bw=10):
    # get data
    senders = neuron_spikes["senders"][neuron_spikes["times"]>start]
    times = neuron_spikes["times"][neuron_spikes["times"]>start]
    rate = len(senders) / (end - start) * 1000.0 / len(set(senders))
    print(f"Mean neuronal firing rate = {rate} (n = {len(set(senders))})")
    # sample neurons
    n_sample = min(len(set(senders)), N)
    print(f"Sampling {n_sample} neurons for synchrony analysis ...")
    sampled = random.sample(list(set(senders)), n_sample)
    times = times[np.isin(senders, sampled)]
    senders = senders[np.isin(senders, sampled)]
    # make histogram
    bins = np.arange(start,end+0.1, bw) # time bins
    hists = [np.histogram(times[senders==x], bins)[0].tolist() for x in set(senders)] # spiking histograms of individual neurons
    hist_global = (np.histogram(times, bins)[0]/len(set(senders))).tolist() # spiking histogram of all neurons sampled
    # calculate local and global synchrony of neurons sampled
    print("Calculating neuronal local and global synchrony ...")
    coefs, n_pass_, n_fail_ = get_corr(hists) # local (spike count correlation)
    lsync_mu, lsync_sd = np.mean(coefs), np.std(coefs)
    gsync = np.var(hist_global)/np.mean(np.var(hists, axis=1)) # global (variance of all/variance of individual)
    # make plot
    plt.hist(coefs)
    title = f"Firing rate={rate:.2f} spikes/s (n={len(set(senders))}) \n" \
        + f"Local sync.={lsync_mu:.3f}$\pm${np.std(coefs):.3f}, Global sync.={gsync:.3f}\n" \
        + f"(n={n_sample}, total n of pairs={n_pass_})\n"
    plt.title(title)
    plt.xlabel("Pairwise spike count correlation (Pearson's r)")
    plt.ylabel("n of pairs")
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "neuron_synchrony.png"))
    plt.close()
    print(f"Local synchrony = {lsync_mu:.3f}+-{lsync_sd:.3f}")
    print(f"Global synchrony = {gsync:.3f}")
    print(f"n of neurons sampled for synchrony analysis = {len(hists)}")
    print(f"n of pairs included/excluded = {n_pass_}/{n_fail_}\n")

###############################################################################
# Function for plotting dynamics.

def plot_dynamics(astro_data, neuron_data, data_path, start):
    # plot dynamics
    print("Plotting dynamics ...")
    # astrocyte data
    a = astro_data
    a_mask = (a["times"]>start)
    a_ip3 = a["IP3"][a_mask]
    a_cal = a["Ca"][a_mask]
    a_t = a["times"][a_mask]
    t_astro = list(set(a_t))
    m_ip3 = np.array([np.mean(a_ip3[a_t==t]) for t in t_astro])
    s_ip3 = np.array([np.std(a_ip3[a_t==t]) for t in t_astro])
    m_cal = np.array([np.mean(a_cal[a_t==t]) for t in t_astro])
    s_cal = np.array([np.std(a_cal[a_t==t]) for t in t_astro])
    # neuron data
    b = neuron_data
    b_mask = (b["times"]>start)
    b_sic = b["SIC"][b["times"]>start]
    b_t = b["times"][b["times"]>start]
    t_neuro = list(set(b_t))
    m_sic = np.array([np.mean(b_sic[b_t==t]) for t in t_neuro])
    s_sic = np.array([np.std(b_sic[b_t==t]) for t in t_neuro])
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
    axes[0].fill_between(
        t_astro, m_ip3+s_ip3, m_ip3-s_ip3, alpha=0.3, linewidth=0.0,
        color=color_ip3)
    axes[0].plot(t_astro, m_ip3, linewidth=2, color=color_ip3)
    ax = axes[0].twinx()
    ax.set_ylabel(r"Ca$^{2+}$ ($\mu$M)")
    ax.tick_params(axis="y", labelcolor=color_cal)
    ax.fill_between(
        t_astro, m_cal+s_cal, m_cal-s_cal, alpha=0.3, linewidth=0.0,
        color=color_cal)
    ax.plot(t_astro, m_cal, linewidth=2, color=color_cal)
    # neuron plot
    axes[1].set_title(f"SIC in neurons (n={len(set(a['senders']))})")
    axes[1].set_ylabel("SIC (pA)")
    axes[1].set_xlabel("Time (ms)")
    axes[1].fill_between(
        t_neuro, m_sic+s_sic, m_sic-s_sic, alpha=0.3, linewidth=0.0,
        color=color_sic)
    axes[1].plot(t_neuro, m_sic, linewidth=2, color=color_sic)
    # save
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "dynamics.png"))
    plt.close()

###############################################################################
# Main function for simulation.

def run_simulation(data_path):
    # NEST configuration
    nest.ResetKernel()
    nest.resolution = sim_params["dt"]
    nest.print_time = True
    nest.overwrite_files = True
    try:
        nest.local_num_threads = int(sys.argv[1])
    except:
        nest.local_num_threads = 4

    # Simulation settings
    pre_sim_time = sim_params["pre_sim_time"]
    sim_time = sim_params["sim_time"]
    # Time before building
    startbuild = time.time()

    # Create and connect nodes
    exc, inh, astro, noise = \
        create_astro_network()
    connect_astro_network(exc, inh, astro, noise)

    # Create and connect recorders (multimeter default resolution = 1 ms)
    sr_neuron = nest.Create("spike_recorder")
    mm_neuron = nest.Create("multimeter", params={"record_from": ["SIC"]})
    mm_astro = nest.Create(
        "multimeter", params={"record_from": ["IP3", "Ca"]})

    # Time after building
    endbuild = time.time()

    # Run pre-simulation and simulation
    print("Running pre-simulation ...")
    nest.Simulate(pre_sim_time)
    print("Connecting recorders ...")
    n_spk_neuro = min(len(exc + inh), sim_params["N_rec_spk"])
    n_mm_neuro = min(len(exc + inh), sim_params["N_rec_mm"])
    n_mm_astro = min(len(astro), sim_params["N_rec_mm"])
    nest.Connect((exc + inh)[:n_spk_neuro], sr_neuron)
    nest.Connect(mm_neuron, (exc + inh)[:n_mm_neuro])
    nest.Connect(mm_astro, astro[:n_mm_astro])
    print("Running simulation ...")
    nest.Simulate(sim_time)

    # Time after simulation
    endsimulate = time.time()

    # Read out recordings
    neuron_spikes = sr_neuron.events
    neuron_data = mm_neuron.events
    astro_data = mm_astro.events

    # Report firing rates and building/running time
    build_time = endbuild - startbuild
    run_time = endsimulate - endbuild
    print("Brunel network with astrocytes:")
    print(f"Building time = {build_time:.2f} s")
    print(f"Simulation time = {run_time:.2f} s\n")

    # Data saving
    os.system(f"mkdir -p {data_path}")
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
    with open(os.path.join(data_path, 'params.json'), 'w') as f:
        json.dump(params, f, indent=4)

    # Make raster plot and histogram
    nest.raster_plot.from_device(sr_neuron, hist=True)
    plt.title("")
    plt.savefig(os.path.join(data_path, "neuron_raster.png"), bbox_inches="tight")
    plt.close()

    # Calculate and plot neuronal spiking synchrony
    plot_synchrony(neuron_spikes, data_path, pre_sim_time, pre_sim_time+sim_time)

    # Plot dynamics in astrocytes and neurons
    plot_dynamics(astro_data, neuron_data, data_path, pre_sim_time)

    # Copy files
    os.system(f"cp run.jdf out.* err*. {data_path}")

    print("Done!")

###############################################################################
# Run simulation.

hash = hashlib.md5(os.urandom(16)).hexdigest()
run_simulation(os.path.join("astrocyte_brunel", hash))
