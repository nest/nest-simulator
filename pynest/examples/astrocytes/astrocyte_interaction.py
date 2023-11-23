# -*- coding: utf-8 -*-
#
# astrocyte_interaction.py
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
A tripartite interaction between two neurons and one astrocyte
--------------------------------------------------------------

This script simulates a tripartite interaction between two neurons and one
astrocyte. This interaction is part of the astrocyte biology described in
[1]_ that involves the neuron-astrocyte glutamate signaling and the astrocytic
calcium dynamics.

``astrocyte_lr_1994`` is used to model the astrocyte, which implements the
dynamics in the astrocyte based on the articles [2]_, [3]_, and [4]_.
``tsodyks_synapse`` is used to create connections from the presynaptic neuron
to the postsynaptic neuron, and from the presynaptic neuron to the astrocyte.
``sic_connection`` is used to create a connection from the astrocyte to the
postsynaptic neuron. Recordings are made for the following variables: membrance
voltage of the presynaptic neuron, inositol 1,4,5-trisphosphate (IP3), and
calcium in the astrocyte, and slow inward current (SIC) in the postsynaptic
neuron. The result demonstrates a tripartite interaction where the presynaptic
spikes induce changes in IP3 and calcium in the astrocyte, which then induces
the generation of SIC in the postsynaptic neuron.

See Also
~~~~~~~~

:doc:`astrocyte_single`

References
~~~~~~~~~~

.. [1] Bazargani, N., & Attwell, D. (2016). Astrocyte calcium signaling: the
       third wave. Nature neuroscience, 19(2), 182-189. DOI:
       https://doi.org/10.1038/nn.4201

.. [2] Li, Y. X., & Rinzel, J. (1994). Equations for InsP3 receptor-mediated
       [Ca2+]i oscillations derived from a detailed kinetic model: a
       Hodgkin-Huxley like formalism. Journal of theoretical Biology, 166(4),
       461-473. DOI: https://doi.org/10.1006/jtbi.1994.1041

.. [3] De Young, G. W., & Keizer, J. (1992). A single-pool inositol
       1,4,5-trisphosphate-receptor-based model for agonist-stimulated
       oscillations in Ca2+ concentration. Proceedings of the National Academy
       of Sciences, 89(20), 9895-9899. DOI:
       https://doi.org/10.1073/pnas.89.20.9895

.. [4] Nadkarni, S., & Jung, P. (2003). Spontaneous oscillations of dressed
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
# Poisson input for the presynaptic neuron
poisson_rate_neuro = 1500.0
# neuron parameters
params_neuro = {"tau_syn_ex": 2.0}
# astrocyte parameters
params_astro = {"IP3_0": 0.16}
# weights of connections
w_pre2astro = 1.0
w_pre2post = 1.0
w_astro2post = 1.0

###############################################################################
# Create and connect the astrocyte and its devices.

astrocyte = nest.Create("astrocyte_lr_1994", params=params_astro)
mm_astro = nest.Create("multimeter", params={"record_from": ["IP3", "Ca"]})
nest.Connect(mm_astro, astrocyte)

###############################################################################
# Create and connect the neurons and their devices.

pre_neuron = nest.Create("aeif_cond_alpha_astro", params=params_neuro)
post_neuron = nest.Create("aeif_cond_alpha_astro", params=params_neuro)
ps_pre = nest.Create("poisson_generator", params={"rate": poisson_rate_neuro})
mm_pre = nest.Create("multimeter", params={"record_from": ["V_m"]})
mm_post = nest.Create("multimeter", params={"record_from": ["I_SIC"]})
nest.Connect(ps_pre, pre_neuron)
nest.Connect(mm_pre, pre_neuron)
nest.Connect(mm_post, post_neuron)

###############################################################################
# Create tripartite connectivity.

nest.Connect(pre_neuron, post_neuron, syn_spec={"weight": w_pre2post})
nest.Connect(pre_neuron, astrocyte, syn_spec={"weight": w_pre2astro})
nest.Connect(astrocyte, post_neuron, syn_spec={"synapse_model": "sic_connection", "weight": w_astro2post})

###############################################################################
# Run simulation and get results.

nest.Simulate(sim_time)
data_pre = mm_pre.events
data_post = mm_post.events
data_astro = mm_astro.events

###############################################################################
# Create and show plots.

fig, ax = plt.subplots(2, 2, sharex=True, figsize=(6.4, 4.8), dpi=100)
axes = ax.flat
axes[0].plot(data_pre["times"], data_pre["V_m"])
axes[1].plot(data_astro["times"], data_astro["IP3"])
axes[2].plot(data_post["times"], data_post["I_SIC"])
axes[3].plot(data_astro["times"], data_astro["Ca"])
axes[0].set_title(f"Presynaptic neuron\n(Poisson rate = {poisson_rate_neuro} Hz)")
axes[0].set_ylabel("Membrane potential (mV)")
axes[2].set_title("Postsynaptic neuron")
axes[2].set_ylabel("Slow inward current (pA)")
axes[2].set_xlabel("Time (ms)")
axes[1].set_title("Astrocyte")
axes[1].set_ylabel(r"[IP$_{3}$] ($\mu$M)")
axes[3].set_ylabel(r"[Ca$^{2+}$] ($\mu$M)")
axes[3].set_xlabel("Time (ms)")
plt.tight_layout()
plt.show()
plt.close()
