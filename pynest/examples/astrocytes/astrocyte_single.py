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
A model using a single astrocyte with calcium dynamics
-------------------------------------------------------

This script simulates an astrocyte with the model ``astrocyte_lr_1994``, which
implements the dynamics in the astrocyte based on [1]_, [2]_, and
[3]_. Recordings are made for two variables in the astrocyte,
inositol 1,4,5-trisphosphate (IP3) and cytosolic calcium. The astrocyte is driven
by a Poissonian spike train which induces the
generation of IP3 in the astrocyte, which in turn influences the calcium dynamics in
the astrocyte.

See Also
~~~~~~~~

:doc:`astrocyte_interaction`

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

"""

###############################################################################
# Import all necessary modules for simulation and plotting.

import matplotlib.pyplot as plt
import nest

###############################################################################
# Set parameters for the simulation.

# simulation time
sim_time = 60000
# astrocyte parameters
params_astro = {"IP3_0": 0.16}
# Poisson input for the astrocyte
poisson_rate = 1.0
poisson_weight = 0.1

###############################################################################
# Create astrocyte and devices and connect them.

astrocyte = nest.Create("astrocyte_lr_1994", params=params_astro)
ps_astro = nest.Create("poisson_generator", params={"rate": poisson_rate})
mm_astro = nest.Create("multimeter", params={"record_from": ["IP3", "Ca"]})
nest.Connect(ps_astro, astrocyte, syn_spec={"weight": poisson_weight})
nest.Connect(mm_astro, astrocyte)

###############################################################################
# Run simulation and get results.

nest.Simulate(sim_time)
data = mm_astro.events

###############################################################################
# Create and show plots.

fig, axes = plt.subplots(2, 1, sharex=True, figsize=(6.4, 4.8), dpi=100)
axes[0].plot(data["times"], data["IP3"])
axes[1].plot(data["times"], data["Ca"])
axes[0].set_ylabel(r"[IP$_{3}$] ($\mu$M)")
axes[1].set_ylabel(r"[Ca$^{2+}$] ($\mu$M)")
axes[1].set_xlabel("Time (ms)")
plt.tight_layout()
plt.show()
plt.close()
