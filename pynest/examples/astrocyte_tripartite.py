# -*- coding: utf-8 -*-
#
# astrocyte_tripartite.py
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
A tripartite network of two neurons and one astrocyte.
------------------------------------------------------------

This script simulates a tripartite network of two neurons and one astrocyte.

References
~~~~~~~~~~

.. [1] Nadkarni S, and Jung P. Spontaneous oscillations of dressed neurons: A
       new mechanism for epilepsy? Physical Review Letters, 91:26. DOI:
       10.1103/PhysRevLett.91.268101

"""

###############################################################################
# Import all necessary modules for simulation, analysis and plotting.

import os
import json
import hashlib

import matplotlib.pyplot as plt

import nest

###############################################################################
# Parameter section.

# Simulation time
sim_time = 60000
# Poisson rate for neuron
poisson_rate_neuro = 1500.0
# Poisson rate for astrocyte
poisson_rate_astro = 0.0
# Neuron parameters
params_neuro = {'tau_syn_ex': 2.0}
# Astrocyte parameters
params_astro = {'IP3_0': 0.16}
# Connection weights
w_pre2astro = 1.0
w_pre2post = 1.0
w_astro2post = 1.0

###############################################################################
# Setting section (testing only).

plt.rcParams.update({'font.size': 14})
data_path = os.path.join('astrocyte_tripartite', hashlib.md5(os.urandom(16)).hexdigest())

###############################################################################
# Function for simulation.

def simulate():
    # Initialize NEST kernel
    nest.ResetKernel()
    # Create astrocyte and its devices
    astrocyte = nest.Create('astrocyte_lr_1994', params=params_astro)
    ps_astro = nest.Create('poisson_generator', params={'rate': poisson_rate_astro})
    mm_astro = nest.Create('multimeter', params={'record_from': ['IP3', 'Ca']})
    nest.Connect(ps_astro, astrocyte)
    nest.Connect(mm_astro, astrocyte)
    # Create neurons and their devices
    pre_neuron = nest.Create('aeif_cond_alpha_astro', params=params_neuro)
    post_neuron = nest.Create('aeif_cond_alpha_astro', params=params_neuro)
    ps_pre = nest.Create('poisson_generator', params={'rate': poisson_rate_neuro})
    mm_pre = nest.Create('multimeter', params={'record_from': ['V_m']})
    mm_post = nest.Create('multimeter', params={'record_from': ['SIC']})
    nest.Connect(ps_pre, pre_neuron)
    nest.Connect(mm_pre, pre_neuron)
    nest.Connect(mm_post, post_neuron)
    # Create tripartite connectivity
    nest.Connect(pre_neuron, post_neuron, syn_spec={'weight': w_pre2post})
    nest.Connect(pre_neuron, astrocyte, syn_spec={'weight': w_pre2astro})
    nest.Connect(astrocyte, post_neuron, syn_spec={'synapse_model': 'sic_connection', 'weight': w_astro2post})
    # Simulate
    nest.Simulate(sim_time)
    return mm_astro.events, mm_pre.events, mm_post.events

###############################################################################
# Main function.

def run():
    # Create data directory and save script (testing only)
    os.system(f'mkdir -p {data_path}')
    os.system(f'cp astrocyte_tripartite.py {data_path}')

    # Save parameters (testing only)
    default = nest.GetDefaults('astrocyte_lr_1994')
    default.update(params_astro)
    out = open(os.path.join(data_path, 'astrocyte_params.json'), 'w')
    json.dump(default, out, indent=4)
    out.close()

    # Run simulation and get results
    data_astro, data_pre, data_post = simulate()

    # Create plots
    fig, ax = plt.subplots(2, 2, sharex=True, figsize=(10, 8))
    axes = ax.flat
    axes[0].plot(data_pre["times"], data_pre["V_m"])
    axes[1].plot(data_astro["times"], data_astro["IP3"])
    axes[2].plot(data_post["times"], data_post["SIC"])
    axes[3].plot(data_astro["times"], data_astro["Ca"])

    # Label and show plots
    axes[0].set_title(f'Presynaptic neuron\n(Poisson rate = {poisson_rate_neuro} Hz)')
    axes[0].set_ylabel('Membrane potential (mV)')
    axes[1].set_title(f'Astrocyte\n(Poisson rate = {poisson_rate_astro} Hz)')
    axes[1].set_ylabel(r"[IP$_{3}$] ($\mu$M)")
    axes[2].set_title(f'Postsynaptic neuron')
    axes[2].set_ylabel("Slow inward current (pA)")
    axes[2].set_xlabel('Time (ms)')
    axes[3].set_ylabel(r"[Ca$^{2+}$] ($\mu$M)")
    axes[3].set_xlabel('Time (ms)')
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, 'astrocyte_tripartite.png')) # (testing only)
    plt.show()
    plt.close()

if __name__ == "__main__":
    run()
