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
neurons and astrocytes. The ``astrocyte`` model is according to Nadkarni & Jung
(2003) [1]_. The ``aeif_cond_alpha_astro`` model is an adaptive exponential
integrate and fire neuron supporting neuron-astrocyte interactions.

The network is created using the ``pairwise_bernoulli_astro`` rule, with the
``tsodyks_synapse`` as the connections from neurons to neruons and astrocytes,
and the ``sic_connection`` as the connections from astrocytes to neurons.

This network is an example of an astrocytic effect on neuronal excitabitlity.
With the slow inward current (SIC) delivered from the astrocytes through the
``sic_connection``, the neurons show higher firing rates and more synchronized
bursting actitivity. The degrees of local and global synchrony are quantitized
by pairwise spike count correlations and a measure of global synchrony in [2]_
respectively, as shown in a plot made in this script (neuron_synchrony.png).
Plots of astrocytic dynamics and SIC in neurons are also made in this script.

References
~~~~~~~~~~

.. [1] Nadkarni S, and Jung P. Spontaneous oscillations of dressed neurons: A
       new mechanism for epilepsy? Physical Review Letters, 91:26. DOI:
       10.1103/PhysRevLett.91.268101

.. [2] Golomb, D. (2007). Neuronal synchrony measures. Scholarpedia, 2(1), 1347.

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

import nest
import nest.raster_plot
import matplotlib.pyplot as plt
plt.rcParams.update({'font.size': 13})

###############################################################################
# Simulation parameters.

sim_params = {
    "dt": 0.1,  # simulation resolution in ms
    "pre_sim_time": 0.0,  # pre-simulation time in ms
    "sim_time": 1000.0,  # simulation time in ms
    "N_rec": 1000,  # number of neurons recorded
    "N_ana": 100,  # number of neurons sampled for analysis
    }

###############################################################################
# Network parameters.

network_params = {
    "N_ex": 8000,  # number of excitatory neurons
    "N_in": 2000,  # number of inhibitory neurons
    "N_astro": 10000,  # number of astrocytes
    "p": 0.1,  # neuron-neuron connection probability.
    "p_syn_astro": 1.0,  # synapse-astrocyte pairing probability
    "max_astro_per_target": 10,  # max number of astrocytes per target neuron
    "astro_pool_by_index": False,  # Astrocyte pool selection by index
    "poisson_rate": 2000,  # rate of Poisson input
    }

syn_params = {
    "synapse_model": "tsodyks_synapse",  # model of neuron-to-neuron and neuron-to-astrocyte connections
    "astro2post": "sic_connection",  # model of astrocyte-to-neuron connection
    "w_e": 1.0,  # excitatory synaptic weight in nS
    "w_i": -4.0,  # inhibitory synaptic weight in nS
    "w_a2n": 1.0,  # weight of astrocyte-to-neuron connection
    }

###############################################################################
# Astrocyte parameters.

astrocyte_model = "astrocyte"
astro_params = {
    'IP3': 0.4,  # IP3 initial value in uM
    'incr_IP3': 0.2,  # Step increase in IP3 concentration with each unit synaptic weight received by the astrocyte in uM
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

def create_astro_network(scale):
    """Create nodes for a neuron-astrocyte network."""
    print("Creating nodes")
    nodes_ex = nest.Create(
        neuron_model, int(network_params["N_ex"]*scale), params=neuron_params_ex)
    nodes_in = nest.Create(
        neuron_model, int(network_params["N_in"]*scale), params=neuron_params_in)
    nodes_astro = nest.Create(
        astrocyte_model, int(network_params["N_astro"]*scale), params=astro_params)
    nodes_noise = nest.Create(
        "poisson_generator", params={"rate": network_params["poisson_rate"]}
        )
    return nodes_ex, nodes_in, nodes_astro, nodes_noise

def connect_astro_network(nodes_ex, nodes_in, nodes_astro, nodes_noise):
    """Connect the nodes in a neuron-astrocyte network.
    The astrocytes are paired with excitatory connections only.
    """
    print("Connecting Poisson generator")
    syn_params_noise = {
        "synapse_model": "static_synapse", "weight": syn_params["w_e"]
        }
    nest.Connect(
        nodes_noise, nodes_ex + nodes_in, syn_spec=syn_params_noise
        )
    print("Connecting neurons and astrocytes")
    conn_params_e = {
        "rule": "pairwise_bernoulli_astro",
        "astrocyte": nodes_astro,
        "p": network_params["p"],
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
        "delay": nest.random.uniform(2.0, 4.0)
        }
    conn_params_i = {"rule": "pairwise_bernoulli", "p": network_params["p"]}
    syn_params_i = {
        "synapse_model": syn_params["synapse_model"],
        "weight": syn_params["w_i"],
        "tau_psc": tau_syn_in,
        "delay": nest.random.uniform(0.1, 2.0)
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

def plot_synchrony(neuron_spikes, data_path, start, end, bw=10):
    print("Sampling neurons and prepare data for analysis")
    # get data
    senders = neuron_spikes["senders"][neuron_spikes["times"]>start]
    times = neuron_spikes["times"][neuron_spikes["times"]>start]
    rate = len(senders) / (end - start) * 1000.0 / sim_params["N_rec"]
    # sample neurons
    n_sample = np.minimum(len(set(senders)), sim_params["N_ana"])
    sampled = random.sample(list(set(senders)), n_sample)
    times = times[np.isin(senders, sampled)]
    senders = senders[np.isin(senders, sampled)]
    # make histogram
    bins = np.arange(start,end+0.1, bw) # time bins
    hists = [np.histogram(times[senders==x], bins)[0].tolist() for x in set(senders)] # spiking histograms of individual neurons
    hist_global = (np.histogram(times, bins)[0]/len(set(senders))).tolist() # spiking histogram of all neurons sampled
    # calculate local and global synchrony of neurons sampled
    print("Calculating neuronal local and global synchrony")
    coefs, n_pass_, n_fail_ = get_corr(hists) # local (spike count correlation)
    lsync_mu, lsync_sd = np.mean(coefs), np.std(coefs)
    gsync = np.var(hist_global)/np.mean(np.var(hists, axis=1)) # global (variance of all/variance of individual)
    # make plot
    plt.hist(coefs)
    plt.title(f"Local synchrony={lsync_mu:.3f} (s.d.={np.std(coefs):.3f}), total n={n_pass_}\nGlobal synchrony={gsync:.3f}, firing rate={rate:.2f} spikes/s\n")
    plt.xlabel("Pairwise spike count correlation (Pearson's r)")
    plt.ylabel("n of pairs")
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "neuron_synchrony.png"))
    plt.close()
    print(f"n of neurons sampled = {len(hists)}")
    print(f"n of neuron pairs included/excluded = {n_pass_}/{n_fail_}")
    print(f"Local and global synchrony = {lsync_mu:.3f} (s.d.={lsync_sd:.3f}) and {gsync:.3f}")

###############################################################################
# Function for plotting dynamics.

def plot_dynamics(astro_data, neuron_data, data_path, start):
    # plot dynamics
    print("Plotting dynamics")
    # astrocyte data
    d = astro_data
    d_t = d["times"]
    d_ip3 = d["IP3"]
    d_cal = d["Ca"]
    d_ip3 = d_ip3[d_t>start]
    d_cal = d_cal[d_t>start]
    d_t = d_t[d_t>start]
    m_ip3 = np.array([np.mean(d_ip3[d_t==t]) for t in set(d_t)])
    s_ip3 = np.array([np.std(d_ip3[d_t==t]) for t in set(d_t)])
    m_cal = np.array([np.mean(d_cal[d_t==t]) for t in set(d_t)])
    s_cal = np.array([np.std(d_cal[d_t==t]) for t in set(d_t)])
    t_astro = list(set(d_t))
    # neuron data
    d = neuron_data
    d_t = d["times"]
    d_sic = d["SIC"]
    d_sic = d_sic[d_t>start]
    d_t = d_t[d_t>start]
    m_sic = np.array([np.mean(d_sic[d_t==t]) for t in set(d_t)])
    s_sic = np.array([np.std(d_sic[d_t==t]) for t in set(d_t)])
    t_neuro = list(set(d_t))
    # plots
    str_ip3 = r"IP$_{3}$"
    str_cal = r"Ca$^{2+}$"
    color_ip3 = "tab:blue"
    color_cal = "tab:green"
    color_sic = "tab:purple"
    fig, axes = plt.subplots(2, 1, sharex=True)
    # astrocyte plot
    axes[0].set_title(f"{str_ip3} and {str_cal} in astrocytes (n={len(set(astro_data['senders']))})")
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
    axes[1].set_title(f"SIC in neurons (n={len(set(neuron_data['senders']))})")
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

def run_simulation(data_path='data'):
    # NEST configuration
    nest.ResetKernel()
    nest.resolution = sim_params["dt"]
    nest.print_time = True
    nest.overwrite_files = True
    try:
        nest.local_num_threads = int(sys.argv[1])
    except:
        nest.local_num_threads = 8

    # Simulation settings
    pre_sim_time = sim_params["pre_sim_time"]
    sim_time = sim_params["sim_time"]
    os.system(f"mkdir -p {data_path}")
    os.system(f"cp astrocyte_brunel.py {data_path}")

    # Time before building
    startbuild = time.time()

    # Create and connect nodes
    exc, inh, astro, noise = \
        create_astro_network(scale=1)
    connect_astro_network(exc, inh, astro, noise)

    # Create and connect recorders (multimeter default resolution = 1 ms)
    sr_neuron = nest.Create("spike_recorder")
    mm_neuron = nest.Create("multimeter", params={"record_from": ["SIC"]})
    mm_astro = nest.Create(
        "multimeter", params={"record_from": ["IP3", "Ca"]})

    # Time after building
    endbuild = time.time()

    # Run pre-simulation and simulation
    print("Run pre-simulation")
    nest.Simulate(pre_sim_time)
    print("Connect recorders and run simulation")
    nest.Connect((exc + inh)[:sim_params["N_rec"]], sr_neuron)
    nest.Connect(mm_neuron, (exc + inh)[:sim_params["N_rec"]])
    nest.Connect(mm_astro, astro)
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
    rate = sr_neuron.n_events / sim_time * 1000.0 / sim_params["N_rec"]
    print("Brunel network with astrocytes")
    print(f"Firing rate = {rate:.2f} spikes/s (n={sim_params['N_rec']})")
    print(f"Building time = {build_time:.2f} s")
    print(f"Simulation time = {run_time:.2f} s")

    # Make raster plot and histogram
    nest.raster_plot.from_device(sr_neuron, hist=True)
    plt.title("")
    plt.savefig(os.path.join(data_path, "neuron_raster.png"), bbox_inches="tight")
    plt.close()

    # Calculate and plot neuronal spiking synchrony
    plot_synchrony(neuron_spikes, data_path, pre_sim_time, pre_sim_time+sim_time)

    # Plot dynamics in astrocytes and neurons
    plot_dynamics(astro_data, neuron_data, data_path, pre_sim_time)

    print("Done!")

###############################################################################
# Run simulation.

run_simulation()
