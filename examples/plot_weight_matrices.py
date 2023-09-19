# -*- coding: utf-8 -*-
#
# plot_weight_matrices.py
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
Plot weight matrices example
----------------------------

This example demonstrates how to extract the connection strength
for all the synapses among two populations of neurons and gather
these values in weight matrices for further analysis and visualization.

All connection types between these populations are considered, i.e.,
four weight matrices are created and plotted.
"""

###############################################################################
# First, we import all necessary modules to extract, handle and plot
# the connectivity matrices

import matplotlib.gridspec as gridspec
import matplotlib.pyplot as plt
import nest
import numpy as np
from mpl_toolkits.axes_grid1 import make_axes_locatable

###############################################################################
# We now specify a function to extract and plot weight matrices for all
# connections among `E_neurons` and `I_neurons`.
#
# We initialize all the matrices, whose dimensionality is determined by the
# number of elements in each population.
# Since in this example, we have 2 populations (E/I), :math:`2^2` possible
# synaptic connections exist (EE, EI, IE, II).
#
# Note the use of "post-pre" notation when referring to synaptic connections.
# As a matter of convention in computational neuroscience, we refer to the
# connection from inhibitory to excitatory neurons (I->E) as EI (post-pre) and
# connections from excitatory to inhibitory neurons (E->I) as IE (post-pre).
#
# The script iterates through the list of all connections of each type.
# To populate the corresponding weight matrix, we identify the source-node_id
# and the target-node_id.
# For each `node_id`, we subtract the minimum `node_id` within the corresponding
# population, to assure the matrix indices range from 0 to the size of the
# population.
#
# After determining the matrix indices `[i, j]`, for each connection object, the
# corresponding weight is added to the entry `W[i,j]`. The procedure is then
# repeated for all the different connection types.
#
# We then plot the figure, specifying the properties we want. For example, we
# can display all the weight matrices in a single figure, which requires us to
# use ``GridSpec`` to specify the spatial arrangement of the axes.
# A subplot is subsequently created for each connection type. Using ``imshow``,
# we can visualize the weight matrix in the corresponding axis. We can also
# specify the colormap for this image.
# Using the ``axis_divider`` module from ``mpl_toolkits``, we can allocate a small
# extra space on the right of the current axis, which we reserve for a
# colorbar.
# We can set the title of each axis and adjust the axis subplot parameters.
# Finally, the last three steps are repeated for each synapse type.


def plot_weight_matrices(E_neurons, I_neurons):
    W_EE = np.zeros([len(E_neurons), len(E_neurons)])
    W_EI = np.zeros([len(I_neurons), len(E_neurons)])
    W_IE = np.zeros([len(E_neurons), len(I_neurons)])
    W_II = np.zeros([len(I_neurons), len(I_neurons)])

    a_EE = nest.GetConnections(E_neurons, E_neurons)

    # We extract the value of the connection weight for all the connections between these populations
    c_EE = a_EE.weight

    # Repeat the two previous steps for all other connection types
    a_EI = nest.GetConnections(I_neurons, E_neurons)
    c_EI = a_EI.weight
    a_IE = nest.GetConnections(E_neurons, I_neurons)
    c_IE = a_IE.weight
    a_II = nest.GetConnections(I_neurons, I_neurons)
    c_II = a_II.weight

    # We now iterate through the range of all connections of each type.
    # To populate the corresponding weight matrix, we begin by identifying
    # the source-node_id (by using .source) and the target-node_id.
    # For each node_id, we subtract the minimum node_id within the corresponding
    # population, to assure the matrix indices range from 0 to the size of
    # the population.

    # After determining the matrix indices [i, j], for each connection
    # object, the corresponding weight is added to the entry W[i,j].
    # The procedure is then repeated for all the different connection types.
    a_EE_src = a_EE.source
    a_EE_trg = a_EE.target
    a_EI_src = a_EI.source
    a_EI_trg = a_EI.target
    a_IE_src = a_IE.source
    a_IE_trg = a_IE.target
    a_II_src = a_II.source
    a_II_trg = a_II.target

    min_E = min(E_neurons.tolist())
    min_I = min(I_neurons.tolist())

    for idx in range(len(a_EE)):
        W_EE[a_EE_src[idx] - min_E, a_EE_trg[idx] - min_E] += c_EE[idx]
    for idx in range(len(a_EI)):
        W_EI[a_EI_src[idx] - min_I, a_EI_trg[idx] - min_E] += c_EI[idx]
    for idx in range(len(a_IE)):
        W_IE[a_IE_src[idx] - min_E, a_IE_trg[idx] - min_I] += c_IE[idx]
    for idx in range(len(a_II)):
        W_II[a_II_src[idx] - min_I, a_II_trg[idx] - min_I] += c_II[idx]

    fig = plt.figure()
    fig.suptitle("Weight matrices", fontsize=14)
    gs = gridspec.GridSpec(4, 4)
    ax1 = plt.subplot(gs[:-1, :-1])
    ax2 = plt.subplot(gs[:-1, -1])
    ax3 = plt.subplot(gs[-1, :-1])
    ax4 = plt.subplot(gs[-1, -1])

    plt1 = ax1.imshow(W_EE, cmap="jet")

    divider = make_axes_locatable(ax1)
    cax = divider.append_axes("right", "5%", pad="3%")
    plt.colorbar(plt1, cax=cax)

    ax1.set_title("$W_{EE}$")
    plt.tight_layout()

    plt2 = ax2.imshow(W_IE)
    plt2.set_cmap("jet")
    divider = make_axes_locatable(ax2)
    cax = divider.append_axes("right", "5%", pad="3%")
    plt.colorbar(plt2, cax=cax)
    ax2.set_title("$W_{EI}$")
    plt.tight_layout()

    plt3 = ax3.imshow(W_EI)
    plt3.set_cmap("jet")
    divider = make_axes_locatable(ax3)
    cax = divider.append_axes("right", "5%", pad="3%")
    plt.colorbar(plt3, cax=cax)
    ax3.set_title("$W_{IE}$")
    plt.tight_layout()

    plt4 = ax4.imshow(W_II)
    plt4.set_cmap("jet")
    divider = make_axes_locatable(ax4)
    cax = divider.append_axes("right", "5%", pad="3%")
    plt.colorbar(plt4, cax=cax)
    ax4.set_title("$W_{II}$")
    plt.tight_layout()


#################################################################################
# Create a simple population to demonstrate the plotting function.
#
# Two populations are created, one with excitatory neurons and one with
# inhibitory ones. Excitatory connections are created with synaptic weights
# distributed with a normal distribution with a mean of 20. The inhibitory
# connections have a synaptic weight proportional to the excitatory weights.

# Create populations
NE = 100  # number of excitatory neurons
NI = 25  # number of inhibitory neurons
E_neurons = nest.Create("iaf_psc_alpha", NE)
I_neurons = nest.Create("iaf_psc_alpha", NI)

# Definition of connectivity parameters
CE = int(0.1 * NE)  # number of excitatory synapses per neuron
CI = int(0.1 * NI)  # number of inhibitory synapses per neuron

delay = 1.5  # synaptic delay in ms
g = 5.0  # ratio inhibitory weight/excitatory weight

w_ex = nest.random.normal(20, 0.5)
w_in = -g * w_ex

# Create connections
nest.Connect(
    E_neurons,
    E_neurons + I_neurons,
    conn_spec={"rule": "fixed_indegree", "indegree": CE},
    syn_spec={"synapse_model": "static_synapse", "weight": w_ex, "delay": delay},
)

nest.Connect(
    I_neurons,
    E_neurons + I_neurons,
    conn_spec={"rule": "fixed_indegree", "indegree": CI},
    syn_spec={"synapse_model": "static_synapse", "weight": w_in, "delay": delay},
)

# Use plotting function
plot_weight_matrices(E_neurons, I_neurons)

plt.show()
