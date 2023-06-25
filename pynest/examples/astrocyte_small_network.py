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
Neuron-astrocyte connection
------------------------------------------------------------

This script demonstrates how to use the NEST connection builder and the
"pairwise_bernoulli_astro" rule to create a small neuron-astrocyte network.

See Also
~~~~~~~~

:doc:`astrocyte_brunel`

"""

###############################################################################
# Import all necessary modules.

import os
import hashlib as hl

from mpi4py import MPI
import nest
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
plt.rcParams.update({'font.size': 13})
pd.set_option('display.max_rows', None)

###############################################################################
# Initialize kernel.

nest.ResetKernel()
comm = MPI.COMM_WORLD
rank = comm.Get_rank()

###############################################################################
# Create save path.

spath = os.path.join('astrocyte_small_network', hl.md5(os.urandom(16)).hexdigest())
os.system(f'mkdir -p {spath}')

###############################################################################
# Astrocyte parameters.

astrocyte_model = "astrocyte_lr_1994"
astrocyte_params = {
    'IP3': 0.4,  # IP3 initial value in uM
    'incr_IP3': 0.5,  # Step increase in IP3 concentration with each unit synaptic weight received by the astrocyte in uM
    'tau_IP3': 10.0,  # Time constant of astrocytic IP3 degradation in ms
    }

###############################################################################
# Neuron parameters.

neuron_model = "aeif_cond_alpha_astro"
neuron_params = {
    "tau_syn_ex": 2.0, # excitatory synaptic time constant in ms
    "I_e": 1000.0, # external current input in pA
    }

###############################################################################
# Create and connect populations and devices.

pre_neurons = nest.Create(neuron_model, 10, params=neuron_params)
post_neurons = nest.Create(neuron_model, 10, params=neuron_params)
astrocytes = nest.Create(astrocyte_model, 10, params=astrocyte_params)
nest.Connect(
    pre_neurons, post_neurons,
    conn_spec={
        'rule':'pairwise_bernoulli_astro',
        'astrocyte': astrocytes,
        'p': 1.,
        'p_syn_astro': 1.,
        'max_astro_per_target': 3,
        'astro_pool_by_index': True,
    },
    syn_spec={
        'delay': 1.0,
        'weight_pre2post': 1.0,
        'weight_pre2astro': 1.0,
        'weight_astro2post': 1.0,
    }
)

mm_pre_neurons = nest.Create('multimeter', params={'record_from': ['V_m']})
mm_post_neurons = nest.Create('multimeter', params={'record_from': ['V_m', 'SIC']})
mm_astrocytes = nest.Create('multimeter', params={'record_from': ['IP3', 'Ca']})
nest.Connect(mm_pre_neurons, pre_neurons)
nest.Connect(mm_post_neurons, post_neurons)
nest.Connect(mm_astrocytes, astrocytes)

###############################################################################
# Print and save population and connection data.

pre_loc = np.array(nest.GetLocalNodeCollection(pre_neurons))
print(f'pre_neurons on rank {rank}:\n{pre_loc}')
post_loc = np.array(nest.GetLocalNodeCollection(post_neurons))
print(f'post_neurons on rank {rank}:\n{post_loc}')
astrocytes_loc = np.array(nest.GetLocalNodeCollection(astrocytes))
print(f'astrocytes on rank {rank}:\n{astrocytes_loc}')

conns_a2n = nest.GetConnections(astrocytes, post_neurons)
conns_n2n = nest.GetConnections(pre_neurons, post_neurons)
conns_n2a = nest.GetConnections(pre_neurons, astrocytes)
print_list = ['source', 'target', 'weight', 'delay', 'synapse_model']
pd.DataFrame(conns_n2n.get())[print_list].to_csv(os.path.join(spath,f'connections_n2n_rank={rank}.csv'), index=False)
pd.DataFrame(conns_n2a.get())[print_list].to_csv(os.path.join(spath,f'connections_n2a_rank={rank}.csv'), index=False)
pd.DataFrame(conns_a2n.get())[print_list].to_csv(os.path.join(spath,f'connections_a2n_rank={rank}.csv'), index=False)

###############################################################################
# Functions for plotting.

def plot_connections(conn_n2n, conn_n2a, conn_a2n, rank=0): # Doesn't work with MPI yet
    print("Plotting connections ...")
    # Get data
    dict_n2n = conns_n2n.get()
    dict_n2a = conns_n2a.get()
    dict_a2n = conns_a2n.get()
    # Set of cells
    sset_n2n = np.unique(dict_n2n['source'])
    tset_n2n = np.unique(dict_n2n['target'])
    sset_n2a = np.unique(dict_n2a['source'])
    aset_n2a = np.unique(dict_n2a['target'])
    aset_a2n = np.unique(dict_a2n['source'])
    tset_a2n = np.unique(dict_a2n['target'])
    # List of cells in connections
    slist_n2n = dict_n2n['source'] - sset_n2n.mean()
    tlist_n2n = dict_n2n['target'] - tset_n2n.mean()
    slist_n2a = dict_n2a['source'] - sset_n2a.mean()
    alist_n2a = dict_n2a['target'] - aset_n2a.mean()
    alist_a2n = dict_a2n['source'] - aset_a2n.mean()
    tlist_a2n = dict_a2n['target'] - tset_a2n.mean()
    # Shift sets
    sset_n2a = sset_n2a - sset_n2a.mean()
    aset_n2a = aset_n2a - aset_n2a.mean()
    aset_a2n = aset_a2n - aset_a2n.mean()
    tset_a2n = tset_a2n - tset_a2n.mean()

    def set_frame_invisible(ax):
        ax.get_xaxis().set_visible(False)
        ax.get_yaxis().set_visible(False)
        ax.spines['top'].set_visible(False)
        ax.spines['bottom'].set_visible(False)
        ax.spines['left'].set_visible(False)
        ax.spines['right'].set_visible(False)

    fig, axs = plt.subplots(1, 1, figsize=(10, 8))
    axs.scatter(
        sset_n2a, [2]*len(sset_n2a), s=400, color='gray', marker='^', label='pre_neurons', zorder=3)
    axs.scatter(
        aset_a2n, [1]*len(aset_a2n), s=400, color='g', marker='o', label='astrocyte', zorder=3)
    axs.scatter(
        tset_a2n, [0]*len(tset_a2n), s=400, color='k', marker='^', label='post_neurons', zorder=3)
    for sx, tx in zip(slist_n2n, tlist_n2n):
        axs.plot(
            [sx, tx], [2, 0], linestyle=':', color='b', alpha=0.5, linewidth=1)
    for sx, tx in zip(slist_n2a, alist_n2a):
        axs.plot(
            [sx, tx], [2, 1], linestyle='-', color='orange', alpha=0.5, linewidth=2)
    for sx, tx in zip(alist_a2n, tlist_a2n):
        axs.plot(
            [sx, tx], [1, 0], linestyle='-', color='g', alpha=0.8, linewidth=4)
    axs.legend(bbox_to_anchor=(0.5, 1.1), loc='upper center', ncol=3)
    set_frame_invisible(axs)
    plt.tight_layout()
    plt.savefig(os.path.join(spath, f'connections_rank={rank}.png'))

def plot_vm(pre_data, post_data, data_path, start, rank=0):
    # plot dynamics
    print("Plotting V_m ...")
    # presynaptic data
    a = pre_data
    a_mask = (a["times"]>start)
    a_vm = a["V_m"][a_mask]
    a_t = a["times"][a_mask]
    t_a = list(set(a_t))
    m_pre_vm = np.array([np.mean(a_vm[a_t==t]) for t in t_a])
    s_pre_vm = np.array([np.std(a_vm[a_t==t]) for t in t_a])
    # postsynaptic data
    b = post_data
    b_mask = (b["times"]>start)
    b_vm = b["V_m"][b_mask]
    b_t = b["times"][b_mask]
    t_b = list(set(b_t))
    m_post_vm = np.array([np.mean(b_vm[b_t==t]) for t in t_b])
    s_post_vm = np.array([np.std(b_vm[b_t==t]) for t in t_b])
    # plots
    color_pre = "tab:blue"
    color_post = "tab:blue"
    fig, axes = plt.subplots(2, 1, sharex=True)
    # presynaptic
    axes[0].set_title(f"presynaptic neurons (n={len(set(a['senders']))})")
    axes[0].set_ylabel(r"$V_{m}$ (mV)")
    axes[0].fill_between(
        t_a, m_pre_vm+s_pre_vm, m_pre_vm-s_pre_vm, alpha=0.3, linewidth=0.0,
        color=color_pre)
    axes[0].plot(t_a, m_pre_vm, linewidth=2, color=color_pre)
    # postsynaptic
    axes[1].set_title(f"postsynaptic neurons (n={len(set(b['senders']))})")
    axes[1].set_ylabel(r"$V_{m}$ (mV)")
    axes[1].set_xlabel("Time (ms)")
    axes[1].fill_between(
        t_b, m_post_vm+s_post_vm, m_post_vm-s_post_vm, alpha=0.3, linewidth=0.0,
        color=color_post)
    axes[1].plot(t_b, m_post_vm, linewidth=2, color=color_post)
    # save
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, f"Vm_rank={rank}.png"))
    plt.close()

def plot_dynamics(astro_data, neuron_data, data_path, start, rank=0):
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
    b_sic = b["SIC"][b_mask]
    b_t = b["times"][b_mask]
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
    axes[1].set_title(f"SIC in postsynaptic neurons (n={len(set(a['senders']))})")
    axes[1].set_ylabel("SIC (pA)")
    axes[1].set_xlabel("Time (ms)")
    axes[1].fill_between(
        t_neuro, m_sic+s_sic, m_sic-s_sic, alpha=0.3, linewidth=0.0,
        color=color_sic)
    axes[1].plot(t_neuro, m_sic, linewidth=2, color=color_sic)
    # save
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, f"dynamics_rank={rank}.png"))
    plt.close()

###############################################################################
# Run simulation and save results.

nest.Simulate(1000.0)
os.system(f'cp astrocyte_small_network.py {spath}')
plot_connections(conns_n2n, conns_n2a, conns_a2n, rank)
plot_vm(mm_pre_neurons.events, mm_post_neurons.events, spath, 0.0, rank)
plot_dynamics(mm_astrocytes.events, mm_post_neurons.events, spath, 0.0, rank)
