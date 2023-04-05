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

"""

###############################################################################
# Import all necessary modules for simulation, analysis and plotting. Scipy
# should be imported before nest.

import os
import time
import numpy as np
import multiprocessing as mp
import random

import nest
import nest.raster_plot
import matplotlib.pyplot as plt


###############################################################################
# Simulation parameters.

sim_params = {
    "dt": 0.1,  # simulation resolution
    "pre_sim_time": 1000.0,  # time before simulation
    "sim_time": 1000.0,  # simulation time
    "N_rec": 1000,  # number of neurons recorded
    "N_sample": 100,  # number of neurons sampled for analysis
    "data_path": "data"
    }

###############################################################################
# Network parameters.

network_params = {
    "N_ex": 8000,  # number of excitatory neurons
    "N_in": 2000,  # number of inhibitory neurons
    "N_astro": 1000,  # number of astrocytes
    "p": 0.1,  # neuron-neuron connection probability.
    "p_syn_astro": 0.5,  # synapse-astrocyte pairing probability
    "max_astro_per_target": 5,  # max number of astrocytes per target neuron
    "astro_pool_by_index": False,  # Astrocyte pool selection by index
    "poisson_rate": 100,  # rate of Poisson input
    "poisson_per_neuron": 1,  # average Poisson input per neuron
    }

syn_params = {
    "synapse_model": "tsodyks_synapse",
    "astro2post": "sic_connection",
    "weight_astro2post": 4.0,
    "J_ee": 3.0,  # excitatory-to-excitatory synaptic weight in nS
    "J_ei": 3.0,  # excitatory-to-inhibitory synaptic weight in nS
    "J_ie": -6.0,  # inhibitory-to-excitatory synaptic weight in nS
    "J_ii": -6.0,  # inhibitory-to-inhibitory synaptic weight in nS
    "tau_rec_ee": 2882.9445, # excitatory-to-excitatory depression time constant
    "tau_rec_ei": 5317.747,  # excitatory-to-inhibitory depression time constant
    "tau_rec_ie": 226.859,  # inhibitory-to-excitatory depression time constant
    "tau_rec_ii": 2542.207,  # inhibitory-to-inhibitory depression time constant
    "tau_fac_ee": 0.0,  # excitatory-to-excitatory facilitation time constant
    "tau_fac_ei": 0.0,  # excitatory-to-inhibitory facilitation time constant
    "tau_fac_ie": 0.0,  # inhibitory-to-excitatory facilitation time constant
    "tau_fac_ii": 0.0,  # inhibitory-to-inhibitory facilitation time constant
    "U_ee": 0.928,  # excitatory-to-excitatory release probability parameter
    "U_ei": 0.264,  # excitatory-to-inhibitory release probability parameter
    "U_ie": 0.541,  # inhibitory-to-excitatory release probability parameter
    "U_ii": 0.189,  # inhibitory-to-inhibitory release probability parameter
    }

###############################################################################
# Astrocyte parameters.

astro_params = {
    'Ca_tot': 2.0,  # Total free astrocytic calcium concentration
    'IP3_0': 0.16,  # Baseline value of astrocytic IP3 concentration
    'Kd_act': 0.08234,  # Astrocytic IP3R dissociation constant of calcium (activation)
    'Kd_inh': 1.049,  # Astrocytic IP3R dissociation constant of calcium (inhibition)
    'Kd_IP3_1': 0.13,  # First astrocytic IP3R dissociation constant of IP3
    'Kd_IP3_2': 0.9434,  # Second astrocytic IP3R dissociation constant of IP3
    'Km_SERCA': 0.1,  # Half-activation constant of astrocytic SERCA pump
    'ratio_ER_cyt': 0.185,  # Ratio between astrocytic ER and cytosol volumes
    'incr_IP3': 0.1,  # Step increase in IP3 concentration with each unit synaptic weight received by the astrocyte
    'k_IP3R': 0.001,  # Astrocytic IP3R binding constant for calcium
    'rate_L': 0.00011,  # Rate constant for calcium leak from ER to cytosol
    'tau_IP3': 300.0,  # Time constant of astrocytic IP3 degradation
    'rate_IP3R': 0.006,  # Maximum rate of calcium release via astrocytic IP3R
    'rate_SERCA': 0.0009,  # Maximum rate of calcium uptake by astrocytic SERCA pump
    }

###############################################################################
# Neuron parameters.

neuron_model = "aeif_cond_alpha_astro"
tau_syn_ex = 2.0
tau_syn_in = 5.0

neuron_params_ex = {
    "V_m": -70.0, # membrane potential in mV
    "C_m": 100.0, # capacitance of membrane in pF
    "t_ref": 2.5, # duration of refractory period in ms
    "V_reset": -61.0,  # reset value for V_m in mV
    "E_L": -70.0, # leak reverse potential in mV
    "g_L": 10.0, # leak conductance in nS
    "a": 0.254, # subthreshold adaptation in nS
    "b": 1.38, # spike-triggered adaptation in pA
    "Delta_T": 2.0, # slope factor in mV
    "tau_w": 115.51, # adaptation time constant in ms
    "V_th": -40.0, # spike initiation threshold in mV
    "V_peak": 0.0, # spike detection threshold in mV
    "E_ex": 0.0, # excitatory reversal potential in mV
    "E_in": -80.0, # inhibitory reversal potential in mV
    "tau_syn_ex": tau_syn_ex, # excitatory synaptic time constant in ms
    "tau_syn_in": tau_syn_in, # inhibitory synaptic time constant in ms
    }

neuron_params_in = {
    "V_m": -70.0, # membrane potential in mV
    "C_m": 100.0, # capacitance of membrane in pF
    "t_ref": 2.5, # duration of refractory period in ms
    "V_reset": -47.21,  # reset value for V_m in mV
    "E_L": -70.0, # leak reverse potential in mV
    "g_L": 10.0, # leak conductance in nS
    "a": 0.254, # subthreshold adaptation in nS
    "b": 1.481, # spike-triggered adaptation in pA
    "Delta_T": 2.0, # slope factor in mV
    "tau_w": 202.386, # adaptation time constant in ms
    "V_th": -40.0, # spike initiation threshold in mV
    "V_peak": 0.0, # spike detection threshold in mV
    "E_ex": 0.0, # excitatory reversal potential in mV
    "E_in": -80.0, # inhibitory reversal potential in mV
    "tau_syn_ex": tau_syn_ex, # excitatory synaptic time constant in ms
    "tau_syn_in": tau_syn_in, # inhibitory synaptic time constant in ms
    }

###############################################################################
# Function for network building.

def build_astro(scale, poisson_time):
    """Build a Brunel network with astrocytes."""

    print("Creating nodes")
    nodes_ex = nest.Create(
        neuron_model, int(network_params["N_ex"]*scale), params=neuron_params_ex)
    nodes_in = nest.Create(
        neuron_model, int(network_params["N_in"]*scale), params=neuron_params_in)
    nodes_astro = nest.Create(
        "astrocyte", int(network_params["N_astro"]*scale), params=astro_params)
    nodes_noise = nest.Create(
        "poisson_generator",
        params={
            "rate": network_params["poisson_rate"],
            "start": 0.0, "stop": poisson_time
            }
        )

    # Connect Poisson generator (noise)
    print("Connecting Poisson generator")
    if isinstance(network_params["poisson_per_neuron"], float) or \
       isinstance(network_params["poisson_per_neuron"], int):
        conn_spec_ne = {
            "rule": "fixed_outdegree",
            "outdegree": int(len(nodes_ex)*network_params["poisson_per_neuron"])
            }
        conn_spec_ni = {
            "rule": "fixed_outdegree",
            "outdegree": int(len(nodes_in)*network_params["poisson_per_neuron"])
            }
    else:
        conn_spec_ne = None
        conn_spec_ni = None
    syn_params_ne = {
        "synapse_model": "static_synapse", "weight": syn_params["J_ee"]
        }
    syn_params_ni = {
        "synapse_model": "static_synapse", "weight": syn_params["J_ei"]
        }
    nest.Connect(
        nodes_noise, nodes_ex, conn_spec=conn_spec_ne, syn_spec=syn_params_ne
        )
    nest.Connect(
        nodes_noise, nodes_in, conn_spec=conn_spec_ni, syn_spec=syn_params_ni
        )

    # Connect excitatory and inhibitory neurons
    # Astrocytes are paired with excitatory-to-excitatory connections
    conn_params_astro = {
        "rule": "pairwise_bernoulli_astro",
        "astrocyte": nodes_astro,
        "p": network_params["p"],
        "p_syn_astro": network_params["p_syn_astro"],
        "max_astro_per_target": network_params["max_astro_per_target"],
        "astro_pool_by_index": network_params["astro_pool_by_index"],
        }
    conn_params_neuro = {"rule": "pairwise_bernoulli", "p": network_params["p"]}
    print("Connecting excitatory and inhibitory neurons")
    syn_params_ee = {
        "synapse_model": syn_params["synapse_model"],
        "weight_pre2post": syn_params["J_ee"],
        "U": syn_params["U_ee"],
        "tau_psc": tau_syn_ex,
        "tau_fac": syn_params["tau_fac_ee"],
        "tau_rec": syn_params["tau_rec_ee"],
        "astro2post": syn_params["astro2post"],
        "weight_astro2post": syn_params["weight_astro2post"],
        }
    syn_params_ei = {
        "synapse_model": syn_params["synapse_model"],
        "weight": syn_params["J_ei"],
        "U": syn_params["U_ei"],
        "tau_psc": tau_syn_ex,
        "tau_fac": syn_params["tau_fac_ei"],
        "tau_rec": syn_params["tau_rec_ei"],
        }
    nest.Connect(nodes_ex, nodes_ex, conn_params_astro, syn_params_ee)
    nest.Connect(nodes_ex, nodes_in, conn_params_neuro, syn_params_ei)
    syn_params_ie = {
        "synapse_model": syn_params["synapse_model"],
        "weight": syn_params["J_ie"],
        "U": syn_params["U_ie"],
        "tau_psc": tau_syn_in,
        "tau_fac": syn_params["tau_fac_ie"],
        "tau_rec": syn_params["tau_rec_ie"],
        }
    syn_params_ii = {
        "synapse_model": syn_params["synapse_model"],
        "weight": syn_params["J_ii"],
        "U": syn_params["U_ii"],
        "tau_psc": tau_syn_in,
        "tau_fac": syn_params["tau_fac_ii"],
        "tau_rec": syn_params["tau_rec_ii"],
        }
    nest.Connect(nodes_in, nodes_ex, conn_params_neuro, syn_params_ie)
    nest.Connect(nodes_in, nodes_in, conn_params_neuro, syn_params_ii)

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
# Main function for simulation.

def run_simulation():
    # Configuration
    nest.ResetKernel()
    nest.resolution = sim_params["dt"]
    nest.print_time = True
    nest.local_num_threads = 8
    nest.overwrite_files = True
    pre_sim_time = sim_params["pre_sim_time"]
    sim_time = sim_params["sim_time"]
    data_path = sim_params["data_path"]
    os.system(f"mkdir -p {data_path}")
    os.system(f"cp astrocyte_brunel.py {data_path}")

    # time before building
    startbuild = time.time()

    # create and connect nodes
    exc, inh, astro, noise = \
        build_astro(scale=1, poisson_time=pre_sim_time+sim_time)

    # create and connect recorders
    # multimeter default resolution = 1 ms
    sr_neuron = nest.Create("spike_recorder")
    mm_neuron = nest.Create("multimeter", params={"record_from": ["SIC"]})
    mm_astro = nest.Create(
        "multimeter", params={"record_from": ["IP3", "Ca"]})
    nest.Connect(mm_neuron, (exc + inh)[:sim_params["N_rec"]])
    nest.Connect(mm_astro, astro)

    # time after building
    endbuild = time.time()

    # simulation
    print("Simulating")
    nest.Simulate(pre_sim_time)
    nest.Connect((exc + inh)[:sim_params["N_rec"]], sr_neuron)
    nest.Simulate(sim_time)

    # time after simulation
    endsimulate = time.time()

    # read out recordings
    neuron_spikes = sr_neuron.events
    neuron_data = mm_neuron.events
    astro_data = mm_astro.events

    # print firing rates and building/running time
    build_time = endbuild - startbuild
    run_time = endsimulate - endbuild
    rate = sr_neuron.n_events / sim_time * 1000.0 / sim_params["N_rec"]
    print("Brunel network with astrocytes")
    print(f"n of neurons recorded = {sim_params['N_rec']}")
    print(f"Firing rate = {rate:.2f} spikes/s (n={sim_params['N_rec']})")
    print(f"Building time = {build_time:.2f} s")
    print(f"Simulation time = {run_time:.2f} s")

    # plot a raster and a histogram of neurons recorded
    nest.raster_plot.from_device(sr_neuron, hist=True)
    plt.title("")
    # plt.title(f"n of neurons={sim_params['N_rec']}")
    plt.savefig(os.path.join(data_path, "neuron_raster.png"), bbox_inches="tight")
    plt.close()

    # sample neurons for spiking data
    print("Sampling neurons and prepare data for analysis ...")
    senders = neuron_spikes["senders"][neuron_spikes["times"]>pre_sim_time]
    times = neuron_spikes["times"][neuron_spikes["times"]>pre_sim_time]
    n_sample = np.minimum(len(set(senders)), sim_params["N_sample"])
    sampled = random.sample(list(set(senders)), n_sample) # sample neurons
    times = times[np.isin(senders, sampled)] # spiking times
    senders = senders[np.isin(senders, sampled)] # neuron IDs
    bins = np.arange(pre_sim_time,pre_sim_time+sim_time+0.1, 10) # time bins
    hists = [np.histogram(times[senders==x], bins)[0].tolist() for x in set(senders)] # spiking histograms of individual neurons
    hist_global = (np.histogram(times, bins)[0]/len(set(senders))).tolist() # spiking histogram of all neurons sampled

    # calculate local and global synchrony of neurons sampled
    print("Calculating neuronal local and global synchrony ...")
    coefs, n_pass_, n_fail_ = get_corr(hists) # local (spike count correlation)
    lsync_mu, lsync_sd = np.mean(coefs), np.std(coefs)
    gsync = np.var(hist_global)/np.mean(np.var(hists, axis=1)) # global (variance of all/variance of individual)
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

    # plot dynamics
    print("Plotting dynamics ...")
    # astrocyte data
    d = astro_data
    d_t = d["times"]
    d_ip3 = d["IP3"]
    d_cal = d["Ca"]
    m_ip3 = np.array([np.mean(d_ip3[d_t==t]) for t in set(d_t)])
    s_ip3 = np.array([np.std(d_ip3[d_t==t]) for t in set(d_t)])
    m_cal = np.array([np.mean(d_cal[d_t==t]) for t in set(d_t)])
    s_cal = np.array([np.std(d_cal[d_t==t]) for t in set(d_t)])
    t_astro = list(set(d_t))
    # neuron data
    d = neuron_data
    d_t = d["times"]
    d_sic = d["SIC"]
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
    axes[0].fill_between(
        t_astro, m_cal+s_cal, m_cal-s_cal, alpha=0.3, linewidth=0.0,
        color=color_cal)
    ax.plot(t_astro, m_cal, linewidth=2, color=color_cal)
    # neuron plot
    axes[1].set_title(f"SIC in neurons (n={len(set(neuron_data['senders']))})")
    axes[1].set_ylabel("SIC (pA)")
    axes[1].set_xlabel("Time (ms)")
    axes[0].fill_between(
        t_neuro, m_sic+s_sic, m_sic-s_sic, alpha=0.3, linewidth=0.0,
        color=color_sic)
    axes[1].plot(t_neuro, m_sic, linewidth=2, color=color_sic)
    # save
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, "dynamics.png"))
    plt.close()

    print("Done!")

if __name__ == "__main__":
    plt.rcParams.update({'font.size': 13})
    run_simulation()