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

'''
Plot weight matrices example
----------------------------

This example demonstrates how to extract the connection strength
for all the synapses among two populations of neurons and gather
these values in weight matrices for further analysis and visualization.

All connection types between these populations are considered, i.e.,
four weight matrices are created and plotted.
'''

'''
First, we import all necessary modules to extract, handle and plot
the connectivity matrices
'''

import numpy as np
import pylab
import nest
import matplotlib.gridspec as gridspec
from mpl_toolkits.axes_grid1 import make_axes_locatable

'''
We now specify a function which takes as arguments lists of neuron gids
corresponding to each population
'''


def plot_weight_matrices(E_neurons, I_neurons):
    '''
    Function to extract and plot weight matrices for all connections
    among E_neurons and I_neurons
    '''

    '''
    First, we initialize all the matrices, whose dimensionality is
    determined by the number of elements in each population
    Since in this example, we have 2 populations (E/I), 2^2 possible
    synaptic connections exist (EE, EI, IE, II)
    '''

    W_EE = np.zeros([len(E_neurons), len(E_neurons)])
    W_EI = np.zeros([len(I_neurons), len(E_neurons)])
    W_IE = np.zeros([len(E_neurons), len(I_neurons)])
    W_II = np.zeros([len(I_neurons), len(I_neurons)])

    '''
    Using `GetConnections`, we extract the information about all the
    connections involving the populations of interest. `GetConnections`
    returns a list of arrays (connection objects), one per connection.
    Each array has the following elements:
    [source-gid target-gid target-thread synapse-model-id port]
    '''

    a_EE = nest.GetConnections(E_neurons, E_neurons)

    '''
    Using `get`, we can extract the value of the connection weight,
    for all the connections between these populations
    '''

    c_EE = a_EE.get('weight')

    '''
    Repeat the two previous steps for all other connection types
    '''

    a_EI = nest.GetConnections(I_neurons, E_neurons)
    c_EI = a_EI.get('weight')
    a_IE = nest.GetConnections(E_neurons, I_neurons)
    c_IE = a_IE.get('weight')
    a_II = nest.GetConnections(I_neurons, I_neurons)
    c_II = a_II.get('weight')

    '''
    We now iterate through the range of all connections of each type.
    To populate the corresponding weight matrix, we begin by identifying
    the source-gid (by using .get('source')) and the target-gid.
    For each gid, we subtract the minimum gid within the corresponding
    population, to assure the matrix indices range from 0 to the size of
    the population.

    After determining the matrix indices [i, j], for each connection
    object, the corresponding weight is added to the entry W[i,j].
    The procedure is then repeated for all the different connection types.
    '''
    a_EE_src = a_EE.get('source')
    a_EE_trg = a_EE.get('target')
    a_EI_src = a_EI.get('source')
    a_EI_trg = a_EI.get('target')
    a_IE_src = a_IE.get('source')
    a_IE_trg = a_IE.get('target')
    a_II_src = a_II.get('source')
    a_II_trg = a_II.get('target')

    for idx in range(len(a_EE)):
        W_EE[a_EE_src[idx] - min(E_neurons),
             a_EE_trg[idx] - min(E_neurons)] += c_EE[idx]
    for idx in range(len(a_EI)):
        W_EI[a_EI_src[idx] - min(I_neurons),
             a_EI_trg[idx] - min(E_neurons)] += c_EI[idx]
    for idx in range(len(a_IE)):
        W_IE[a_IE_src[idx] - min(E_neurons),
             a_IE_trg[idx] - min(I_neurons)] += c_IE[idx]
    for idx in range(len(a_II)):
        W_II[a_II_src[idx] - min(I_neurons),
             a_II_trg[idx] - min(I_neurons)] += c_II[idx]

    '''
    We can now specify the figure and axes properties. For this specific
    example, we wish to display all the weight matrices in a single
    figure, which requires us to use ``GridSpec`` (for example)
    to specify the spatial arrangement of the axes.
    A subplot is subsequently created for each connection type.
    '''

    fig = pylab.figure()
    fig.suptitle('Weight matrices', fontsize=14)
    gs = gridspec.GridSpec(4, 4)
    ax1 = pylab.subplot(gs[:-1, :-1])
    ax2 = pylab.subplot(gs[:-1, -1])
    ax3 = pylab.subplot(gs[-1, :-1])
    ax4 = pylab.subplot(gs[-1, -1])

    '''
    Using ``imshow``, we can visualize the weight matrix in the corresponding
    axis. We can also specify the colormap for this image.
    '''

    plt1 = ax1.imshow(W_EE, cmap='jet')

    '''
    Using the ``axis_divider`` module from ``mpl_toolkits``, we can
    allocate a small extra space on the right of the current axis,
    which we reserve for a colorbar.
    '''

    divider = make_axes_locatable(ax1)
    cax = divider.append_axes("right", "5%", pad="3%")
    pylab.colorbar(plt1, cax=cax)

    '''
    We now set the title of each axis and adjust the axis subplot parameters
    '''

    ax1.set_title('W_{EE}')
    pylab.tight_layout()

    '''
    Finally, the last three steps are repeated for each synapse type
    '''

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

    pylab.show()
