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
Illustration of calcium dynamics in isolated astrocytes. Three astrocytes are shown:
1) Astrocyte that reaches constant calcium level in the steady-state. Astrocyte receives no external inputs.
2) Astrocyte with oscillating calcium dynamics in the absence of external inputs.
3) Astrocyte that receives Poisson inputs and exhibits calcium transients with frequency of xx Hz.
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
import matplotlib
matplotlib.use('TkAgg')

import pdb

###############################################################################
# Simulation parameters

sim_params = {
    "dt": 0.1, # simulation resolution in ms
    "sim_time": 180000.0, # simulation time in ms
    "data_path": "data"
    }

###############################################################################
# Cell population parameters
population_params = {
    "N_astro": 3,       # Generate three astrocytes to illustrate three different dynamical regimes of astrocytic calcium
}

###############################################################################
# Astrocyte parameters

astro_params = {
    'IP3_astro':    [0.16, 0.16, 0.16],     # Initial condition for IP3 concentration in the astrocytic cytosol
    'Ca_astro':     [0.073, 0.073, 0.073],   # Initial condition for calcium concentration in the astrocytic cytosol
    'f_IP3R_astro': [0.792, 0.792, 0.792],   # Initial condition for fraction of IP3 receptors on the astrocytic ER that are not yet inactivated by calcium
    'Ca_tot_astro': 2.0,     # Total free astrocytic calcium concentration in uM
    'IP3_0_astro': [0.16, 0.4, 0.16],     # 0.16 Baseline value of the astrocytic IP3 concentration in uM
    'K_act_astro': 0.08234,  # Astrocytic IP3R dissociation constant of calcium (activation) in uM
    'K_inh_astro': 1.049,    # Astrocytic IP3R dissociation constant of calcium (inhibition) in uM
    'K_IP3_1_astro': 0.13,   # Astrocytic IP3R dissociation constant of IP3 in uM
    'K_IP3_2_astro': 0.9434, # Astrocytic IP3R dissociation constant of IP3 in uM
    'K_SERCA_astro': 0.1,    # Half-activation constant of astrocytic SERCA pump in uM
    'r_ER_cyt_astro': 0.185, # Ratio between astrocytic ER and cytosol volumes
    'r_IP3_astro': 5.,       # Step increase in IP3 concentration at each spike time at each synapse interacting with the same astrocyte, in uM/ms
    'r_IP3R_astro': 0.0002,   # Astrocytic IP3R binding constant for calcium inhibition in 1/(uM*ms)
    'r_L_astro': 0.00011,    # Rate constant for calcium leak from the astrocytic ER to cytosol in 1/ms
    'tau_IP3_astro': 7142.0, # Time constant of astrocytic IP3 degradation
    'v_IP3R_astro': 0.006,   # Maximum rate of calcium release via astrocytic IP3R in 1/ms
    'v_SERCA_astro': 0.0009, # Maximum rate of calcium uptake by astrocytic IP3R in uM/ms
    #'sic_thr_astro': 196.69  # Threshold that determines the minimal level of intracellular astrocytic calcium sufficient to induce SIC 
    }

###############################################################################
# Noise parameters
noise_params = {
    "poisson_rate": [.1] # Frequency of Poisson noise serving as an input to astrocytes
}

###############################################################################
# Function for network building

def build_astro(poisson_rate):
    """Construct three isolated astrocytes"""

    print("Creating nodes")
    nodes_astro = nest.Create(
        "astrocyte", int(population_params["N_astro"]), params=astro_params)
    
    nodes_noise = nest.Create(
        "poisson_generator", 
        params={
            "rate": poisson_rate,
            "start": 0.0, "stop": sim_params["sim_time"]
            }
        )
    
    # Connect Poisson generator (noise) to one astrocyte
    print("Connecting Poisson generator")
    conn_spec_astro = {
        "rule": "fixed_outdegree",
        "outdegree": len(poisson_rate)
        }
    syn_params_astro = {
        "synapse_model": "static_synapse", "weight": 1.
        }
    
    nest.Connect(
        nodes_noise, nodes_astro[2], conn_spec=conn_spec_astro, syn_spec=syn_params_astro
        )
    
    return nodes_astro


###############################################################################
# Main function for simulation.

def run_simulation():

    # Configuration
    nest.ResetKernel()
    nest.resolution = sim_params["dt"]
    nest.print_time = True
    nest.local_num_threads = 8
    nest.overwrite_files = True
    sim_time = sim_params["sim_time"]
    data_path = sim_params["data_path"]
    os.system(f"mkdir -p {data_path}")
    os.system(f"cp single_astrocyte.py {data_path}")

    # time before building
    startbuild = time.time()

    # create and connect nodes
    astro = build_astro(poisson_rate = noise_params["poisson_rate"])

    # create and connect recorders
    # multimeter default resolution = 1 ms
    mm_astro = nest.Create(
        "multimeter", params={"record_from": ["IP3_astro", "Ca_astro"]})
    nest.Connect(mm_astro, astro[:])

    # time after building
    endbuild = time.time()

    # simulation
    print("Simulating")
    nest.Simulate(sim_time)

    # time after simulation
    endsimulate = time.time()

    # read out recordings
    astro_data = mm_astro.events

    # plot astrocyte data
    print("Plotting calcium dynamics in astrocytes ...")

    keys = list(astro_data.keys())
    fig, axes = plt.subplots(2, population_params["N_astro"], sharex=True)

    axes[0][0].set_title("Steady-state")
    axes[0][1].set_title("Oscillations")
    axes[0][2].set_title("Poisson input")
    
    ylabels = [r'Ca$^{2+}$ ($\mu$M)', r'IP$_{3}$ ($\mu$M)']
    for iastro in range(population_params["N_astro"]):
       for i, key in enumerate(["Ca_astro", "IP3_astro"]):
            which_astro = (astro_data["senders"] == iastro+1)
            axes[i][iastro].plot(astro_data["times"][which_astro], astro_data[key][which_astro], linewidth=2)
            axes[i][iastro].set_ylabel(ylabels[i])
    plt.xlabel("Time (ms)")
    plt.tight_layout()
    
    plt.savefig(os.path.join(data_path, "calcium_isolated_astrocyte.png"))
    plt.close()
    print("Done!")
  

if __name__ == "__main__":
    plt.rcParams.update({'font.size': 13})
    run_simulation()
