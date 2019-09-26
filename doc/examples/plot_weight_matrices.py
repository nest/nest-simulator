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

import numpy as np
import pylab
import nest
import matplotlib.gridspec as gridspec
from mpl_toolkits.axes_grid1 import make_axes_locatable

###############################################################################
# We now specify a function to extract and plot weight matrices for all
# connections among `E_neurons` and `I_neurons`.
#
# We initialize all the matrices, whose dimensionality is determined by the
# number of elements in each population.
# Since in this example, we have 2 populations (E/I), :math:`2^2` possible
# synaptic connections exist (EE, EI, IE, II).


def plot_weight_matrices(E_neurons, I_neurons):

    W_EE = np.zeros([len(E_neurons), len(E_neurons)])
    W_EI = np.zeros([len(I_neurons), len(E_neurons)])
    W_IE = np.zeros([len(E_neurons), len(I_neurons)])
    W_II = np.zeros([len(I_neurons), len(I_neurons)])

    a_EE = nest.GetConnections(E_neurons, E_neurons)
    c_EE = nest.GetStatus(a_EE, keys='weight')
    a_EI = nest.GetConnections(I_neurons, E_neurons)
    c_EI = nest.GetStatus(a_EI, keys='weight')
    a_IE = nest.GetConnections(E_neurons, I_neurons)
    c_IE = nest.GetStatus(a_IE, keys='weight')
    a_II = nest.GetConnections(I_neurons, I_neurons)
    c_II = nest.GetStatus(a_II, keys='weight')

    for idx, n in enumerate(a_EE):
        W_EE[n[0] - min(E_neurons), n[1] - min(E_neurons)] += c_EE[idx]
    for idx, n in enumerate(a_EI):
        W_EI[n[0] - min(I_neurons), n[1] - min(E_neurons)] += c_EI[idx]
    for idx, n in enumerate(a_IE):
        W_IE[n[0] - min(E_neurons), n[1] - min(I_neurons)] += c_IE[idx]
    for idx, n in enumerate(a_II):
        W_II[n[0] - min(I_neurons), n[1] - min(I_neurons)] += c_II[idx]

    fig = pylab.figure()
    fig.subtitle('Weight matrices', fontsize=14)
    gs = gridspec.GridSpec(4, 4)
    ax1 = pylab.subplot(gs[:-1, :-1])
    ax2 = pylab.subplot(gs[:-1, -1])
    ax3 = pylab.subplot(gs[-1, :-1])
    ax4 = pylab.subplot(gs[-1, -1])

    plt1 = ax1.imshow(W_EE, cmap='jet')

    divider = make_axes_locatable(ax1)
    cax = divider.append_axes("right", "5%", pad="3%")
    pylab.colorbar(plt1, cax=cax)

    ax1.set_title('W_{EE}')
    pylab.tight_layout()

    plt2 = ax2.imshow(W_IE)
    plt2.set_cmap('jet')
    divider = make_axes_locatable(ax2)
    cax = divider.append_axes("right", "5%", pad="3%")
    pylab.colorbar(plt2, cax=cax)
    ax2.set_title('W_{EI}')
    pylab.tight_layout()

    plt3 = ax3.imshow(W_EI)
    plt3.set_cmap('jet')
    divider = make_axes_locatable(ax3)
    cax = divider.append_axes("right", "5%", pad="3%")
    pylab.colorbar(plt3, cax=cax)
    ax3.set_title('W_{IE}')
    pylab.tight_layout()

    plt4 = ax4.imshow(W_II)
    plt4.set_cmap('jet')
    divider = make_axes_locatable(ax4)
    cax = divider.append_axes("right", "5%", pad="3%")
    pylab.colorbar(plt4, cax=cax)
    ax4.set_title('W_{II}')
    pylab.tight_layout()

#################################################################################
# The script iterates through the list of all connections of each type.
# To populate the corresponding weight matrix, we identify the source-gid
# (first element of each connection object, `n[0]`) and the target-gid (second
# element of each connection object, `n[1]`).
# For each `gid`, we subtract the minimum `gid` within the corresponding
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
