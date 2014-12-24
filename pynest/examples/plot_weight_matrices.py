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

import numpy as np
import pylab
import nest
import matplotlib.gridspec as gridspec
from mpl_toolkits.axes_grid1 import make_axes_locatable

def plot_weight_matrices(E_neurons, I_neurons):
    '''Extracts and plots weight matrices'''

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

    for idx,n in enumerate(a_EE):
        W_EE[n[0]-min(E_neurons), n[1]-min(E_neurons)] += c_EE[idx]
    for idx,n in enumerate(a_EI):
        W_EI[n[0]-min(I_neurons), n[1]-min(E_neurons)] += c_EI[idx]
    for idx,n in enumerate(a_IE):
        W_IE[n[0]-min(E_neurons), n[1]-min(I_neurons)] += c_IE[idx]
    for idx,n in enumerate(a_II):
        W_II[n[0]-min(I_neurons), n[1]-min(I_neurons)] += c_II[idx]

    fig = pylab.figure()

    fig.suptitle('Weight matrices', fontsize=14)
    gs = gridspec.GridSpec(4,4)
    ax1 = pylab.subplot(gs[:-1,:-1])
    ax2 = pylab.subplot(gs[:-1,-1])
    ax3 = pylab.subplot(gs[-1,:-1])
    ax4 = pylab.subplot(gs[-1,-1])

    plt1 = ax1.imshow(W_EE)
    plt1.set_cmap('jet')
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
