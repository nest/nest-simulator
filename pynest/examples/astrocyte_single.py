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
A model of single astrocyte with calcium dynamics
------------------------------------------------------------

This script simulates an astrocyte with the model ``astrocyte_lr_1994`` and
plots the dynamics of two variables, inositol 1,4,5-trisphosphate (IP3) and
cytosolic calcium. A Poisson input is created and connected to the astrocyte.
This input changes the IP3 dynamics, which then changes the calcium dynamics.
This model is implemented based on the articles [1]_, [2]_, and [3]_.

References
~~~~~~~~~~

.. [1] De Young, G. W., & Keizer, J. (1992). A single-pool inositol
       1,4,5-trisphosphate-receptor-based model for agonist-stimulated
       oscillations in Ca2+ concentration. Proceedings of the National Academy
       of Sciences, 89(20), 9895-9899.
.. [2] Li, Y. X., & Rinzel, J. (1994). Equations for InsP3 receptor-mediated
       [Ca2+]i oscillations derived from a detailed kinetic model: a
       Hodgkin-Huxley like formalism. Journal of theoretical Biology, 166(4),
       461-473.
.. [3] Nadkarni S, and Jung P. Spontaneous oscillations of dressed neurons: A
       new mechanism for epilepsy? Physical Review Letters, 91:26. DOI:
       10.1103/PhysRevLett.91.268101

"""

###############################################################################
# Import all necessary modules for simulation and plotting.

import os
import json
import hashlib

import matplotlib.pyplot as plt
plt.rcParams.update({'font.size': 14})

import nest

###############################################################################
# Set parameters for the simulation.

# Simulation time
sim_time = 60000
# Astrocyte parameters
params_astro = {'IP3_0': 0.16}
# Poisson input for the astrocyte
poisson_rate = 1.0
poisson_weight = 0.1

###############################################################################
# Main function to run the simulation.

def run():
    # Create data directory and save relevant data
    data_path = os.path.join('astrocyte_single', hashlib.md5(os.urandom(16)).hexdigest())
    os.system(f'mkdir -p {data_path}')
    os.system(f'cp astrocyte_single.py {data_path}')
    default = nest.GetDefaults('astrocyte_lr_1994')
    default.update(params_astro)
    out = open(os.path.join(data_path, 'astrocyte_params.json'), 'w')
    json.dump(default, out, indent=4)
    out.close()

    # Create astrocyte and devices and connect them
    astrocyte = nest.Create('astrocyte_lr_1994', params=params_astro)
    ps_astro = nest.Create('poisson_generator', params={'rate': poisson_rate})
    mm_astro = nest.Create('multimeter', params={'record_from': ['IP3', 'Ca']})
    nest.Connect(ps_astro, astrocyte, syn_spec={'weight': poisson_weight})
    nest.Connect(mm_astro, astrocyte)

    # Run simulation and get results
    nest.Simulate(sim_time)
    data_astro = mm_astro.events

    # Create and show plots
    fig, axes = plt.subplots(2, 1, sharex=True, figsize=(8, 8))
    axes[0].plot(data_astro["times"], data_astro["IP3"])
    axes[1].plot(data_astro["times"], data_astro["Ca"])
    axes[0].set_ylabel(r"[IP$_{3}$] ($\mu$M)")
    axes[1].set_ylabel(r"[Ca$^{2+}$] ($\mu$M)")
    axes[1].set_xlabel('Time (ms)')
    plt.tight_layout()
    plt.savefig(os.path.join(data_path, 'astrocyte_single.png'))
    plt.show()
    plt.close()

if __name__ == "__main__":
    run()
