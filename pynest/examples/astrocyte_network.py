# -*- coding: utf-8 -*-
#
# astrocyte_network.py
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
Neuron-astrocyte network connected with NEST
------------------------------------------------------------

This script simulates a network of neurons and astrocytes connected with a
pairwise Bernoulli rule.

See Also
~~~~~~~~

:doc:`astrocyte_connect`

"""

###############################################################################
# Import all necessary modules for simulation and plotting.

import multiprocessing as mp

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns

import nest

###############################################################################
# Definition of parameters used in this example.

params = {
    'neuron_model': 'aeif_cond_alpha_astro',
    'astro_model': 'astrocyte',
    'n_cells': 100,
    'poisson_rate': 20000.,
    'conn_rule': 'pairwise_bernoulli_astro',
    'p': 0.1,
    'p_syn_astro': [0.1, 0.5],
    'max_astro_per_target': 5,
    'synapse_model': 'tsodyks_synapse',
    'synapse_model_astro': 'sic_connection',
    'weight': 10.,
    'weight_sic': 2.11,
    'c_spill': 0.3,
    'sim_time': 1000.,
}

###############################################################################
# Data structure used in this example.

df_neuron = pd.DataFrame()
df_astro = pd.DataFrame()

###############################################################################
# Simulation of the network. Testing 2 levels of p_syn_astro.

for i, p_syn_astro in enumerate(params['p_syn_astro']):
    nest.ResetKernel()
    nest.SetKernelStatus({'local_num_threads': mp.cpu_count()})

    neurons = nest.Create(params['neuron_model'], params['n_cells'])
    astrocytes = nest.Create(params['astro_model'], params['n_cells'])
    pgen = nest.Create(
        "poisson_generator", params={"rate": params['poisson_rate']})
    recorder = nest.Create("spike_recorder")
    meter = nest.Create(
        "multimeter", params={"record_from": ["IP3_astro", "Ca_astro"]})

    nest.Connect(
        neurons, neurons,
        conn_spec=dict(
            rule=params['conn_rule'], astrocyte=astrocytes, p=params['p'],
            p_syn_astro=p_syn_astro,
            max_astro_per_target=params['max_astro_per_target']),
        syn_spec=dict(
            synapse_model=params['synapse_model'],
            synapse_model_astro=params['synapse_model_astro'],
            weight=params['weight'], weight_sic=params['weight_sic'],
            c_spill=params['c_spill'])
    )
    nest.Connect(pgen, neurons)
    nest.Connect(neurons, recorder)
    nest.Connect(meter, astrocytes)
    nest.Simulate(params['sim_time'])

    neuron_data = nest.GetStatus(recorder, "events")[0]
    neuron_data['p_syn_astro'] = [p_syn_astro]*len(neuron_data['times'])
    df_neuron = pd.concat(
        (df_neuron, pd.DataFrame(data=neuron_data)), ignore_index=True)
    astro_data = nest.GetStatus(meter, "events")[0]
    astro_data['p_syn_astro'] = [p_syn_astro]*len(astro_data['times'])
    df_astro = pd.concat(
        (df_astro, pd.DataFrame(data=astro_data)), ignore_index=True)

###############################################################################
# Plot settings.

bw = 10.
plt.rcParams.update({'font.size': 20})
palette=["#1f77b4", "#ff7f0e"]

###############################################################################
# Neuron raster plot.

fig, axs = plt.subplots(len(params['p_syn_astro'])+1, 1)
for i, p_syn_astro in enumerate(params['p_syn_astro']):
    sns.scatterplot(
        data=df_neuron[df_neuron["p_syn_astro"]==params['p_syn_astro'][i]],
        x="times", y="senders", ax=axs[i], linewidth=0, s=10, c=[palette[i]])
    axs[i].set(title="p_syn_astro={}".format(p_syn_astro))
    axs[i].set(xlabel=None)
    axs[i].set(ylabel="Neuron ID")

###############################################################################
# Neuron histogram.

for i, p_syn_astro in enumerate(params['p_syn_astro']):
    times = \
        df_neuron[df_neuron["p_syn_astro"]==p_syn_astro]["times"].values
    if len(times) == 0:
        continue
    hist, bins = np.histogram(
        times, bins=np.arange(times.min(), times.max()+bw, bw))
    axs[-1].bar(
        bins[:-1], 1000.*hist/(bw*params['n_cells']), width=bw*0.8, alpha=0.5,
        label="p_syn_astro={}".format(p_syn_astro), zorder=3-i,
        color=palette[i])

###############################################################################
# Save neuron plots.

axs[-1].legend(fontsize="small", ncol=2, loc="upper center")
axs[-1].set(xlabel="Time (ms)")
axs[-1].set(ylabel="Firing rate\n(spikes/s)")
yspan = axs[-1].get_ylim()[1] - axs[-1].get_ylim()[0]
axs[-1].set(ylim=(axs[-1].get_ylim()[0], axs[-1].get_ylim()[1]+yspan/2))
fig.set_size_inches(10, 10)
plt.tight_layout()
plt.savefig('neurons.svg')
plt.savefig('neurons.png')
plt.close()

###############################################################################
# Astrocyte IP3 plot.

fig, axs = plt.subplots(2, 1)
sns.lineplot(
    data=df_astro, x='times', y='IP3_astro', hue='p_syn_astro', ax=axs[0],
    dashes=False, ci="sd", linewidth=4, palette=palette)
plt.setp(axs[0].get_legend().get_texts(), fontsize='small')
plt.setp(axs[0].get_legend().get_title(), fontsize='small')
plt.setp(axs[0].get_legend().get_lines(), linewidth=4)
axs[0].set_ylabel(r'IP$_{3}$ ($\mu$M)')
axs[0].set_xlabel('Time (ms)')
axs[0].set_xlim(
    -params['sim_time']/20, params['sim_time']+params['sim_time']/20)

###############################################################################
# Astrocyte calcium plot.

sns.lineplot(
    data=df_astro, x='times', y='Ca_astro', hue='p_syn_astro', ax=axs[1],
    dashes=False, ci="sd", linewidth=4, palette=palette)

###############################################################################
# Save astrocyte plots.

plt.setp(axs[1].get_legend().get_texts(), fontsize='small')
plt.setp(axs[1].get_legend().get_title(), fontsize='small')
plt.setp(axs[1].get_legend().get_lines(), linewidth=4)
xlims = plt.xlim()
axs[1].hlines(
    0.19669, 0., params['sim_time'], colors="k", linestyles="dashed", zorder=3,
    label="threshold")
axs[1].set_ylabel(r'Ca$^{2+}$ ($\mu$M)')
axs[1].set_xlabel('Time (ms)')
axs[1].set_xlim(
    -params['sim_time']/20, params['sim_time']+params['sim_time']/20)
fig.set_size_inches(10, 10)
plt.tight_layout()
plt.savefig('astrocytes.svg')
plt.savefig('astrocytes.png')
plt.close()
