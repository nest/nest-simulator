# -*- coding: utf-8 -*-
#
# astrocyte_single.py
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
An exmple of single-astrocyte dynamics.
------------------------------------------------------------

This script simulates an astrocyte with dynamics of three variables.

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
# Astrocyte parameters
params_astro = {'IP3_0': 0.16}
# Poisson input for astrocyte
poisson_rate = 1.0
poisson_weight = 0.1

###############################################################################
# Setting section (testing only).

plt.rcParams.update({'font.size': 14})
data_path = os.path.join('astrocyte_single', hashlib.md5(os.urandom(16)).hexdigest())

###############################################################################
# Function for simulation.

def simulate():
    # Create astrocyte and its devices
    astrocyte = nest.Create('astrocyte_lr_1994', params=params_astro)
    ps_astro = nest.Create('poisson_generator', params={'rate': poisson_rate})
    mm_astro = nest.Create('multimeter', params={'record_from': ['IP3', 'Ca']})
    nest.Connect(ps_astro, astrocyte, syn_spec={'weight': poisson_weight})
    nest.Connect(mm_astro, astrocyte)

    # Simulate
    nest.Simulate(sim_time)
    return mm_astro.events

###############################################################################
# Main function.

def run():
    # Create data directory and save script (testing only)
    os.system(f'mkdir -p {data_path}')
    os.system(f'cp astrocyte_single.py {data_path}')

    # Save parameters (testing only)
    default = nest.GetDefaults('astrocyte_lr_1994')
    default.update(params_astro)
    out = open(os.path.join(data_path, 'astrocyte_params.json'), 'w')
    json.dump(default, out, indent=4)
    out.close()

    # Run simulation and get results
    data_astro = simulate()

    # Create plots
    fig, axes = plt.subplots(2, 1, sharex=True, figsize=(8, 8))
    axes[0].plot(data_astro["times"], data_astro["IP3"])
    axes[1].plot(data_astro["times"], data_astro["Ca"])

    # Label and show plots
    axes[0].set_title('Astrocyte')
    axes[0].set_ylabel(r"[IP$_{3}$] ($\mu$M)")
    axes[1].set_ylabel(r"[Ca$^{2+}$] ($\mu$M)")
    axes[1].set_xlabel('Time (ms)')
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, 'astrocyte_single.png')) # (testing only)
    plt.show()
    plt.close()

if __name__ == "__main__":
    run()
