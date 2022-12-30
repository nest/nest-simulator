# -*- coding: utf-8 -*-
#
# astrocyte_connect.py
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
Neuron-astrocyte connection
------------------------------------------------------------

This script tests the neuron-astrocyte connection functions.

See Also
~~~~~~~~

:doc:`astrocyte_network`

"""

###############################################################################
# Import all necessary modules.

from mpi4py import MPI
import matplotlib.pyplot as plt
import numpy as np

import nest

###############################################################################
# Initialize kernel.

nest.ResetKernel()
comm = MPI.COMM_WORLD
rank = comm.Get_rank()

###############################################################################
# Create and connect populations.

pre_neurons = nest.Create('aeif_cond_alpha_astro', 20)
post_neurons = nest.Create('aeif_cond_alpha_astro', 20)
astrocytes = nest.Create('astrocyte', 10)
nest.Connect(
    pre_neurons, post_neurons,
    conn_spec={
        'rule':'pairwise_bernoulli_astro',
        'astrocyte': astrocytes,
        'p': 1.,
        'p_syn_astro': 1.,
        'astro_pool_by_index': True,
        'max_astro_per_target': 3,
    }
)

###############################################################################
# Print populations and connections.

pre_loc = np.array(nest.GetLocalNodeCollection(pre_neurons))
print('pre_neurons on rank {}:\n{}'.format(rank, pre_loc))
post_loc = np.array(nest.GetLocalNodeCollection(post_neurons))
print('post_neurons on rank {}:\n{}'.format(rank, post_loc))
astrocytes_loc = np.array(nest.GetLocalNodeCollection(astrocytes))
print('astrocytes on rank {}:\n{}'.format(rank, astrocytes_loc))
conns_a2n = nest.GetConnections(astrocytes, post_neurons)
conns_n2n = nest.GetConnections(pre_neurons, post_neurons)
conns_n2a = nest.GetConnections(pre_neurons, astrocytes)
print('astrocytes => post_neurons on rank {}:'.format(rank))
print(conns_a2n)
print('pre_neurons => post_neurons on rank {}:'.format(rank))
print(conns_n2n)
print('pre_neurons => astrocytes on rank {}:'.format(rank))
print(conns_n2a)

###############################################################################
# Make connectivity plot.

def normalize(list_in):
    list_out = np.array(list_in)
    list_out -= list_out.min()
    list_out = list_out/float(list_out.max())
    return list_out

dict_a2n = conns_a2n.get()
slist = normalize(dict_a2n['source'])
tlist = normalize(dict_a2n['target'])
sset = normalize(list(set(dict_a2n['source'])))
tset = normalize(list(set(dict_a2n['target'])))

def set_frame_invisible(ax):
    ax.get_xaxis().set_visible(False)
    ax.get_yaxis().set_visible(False)
    ax.spines['top'].set_visible(False)
    ax.spines['bottom'].set_visible(False)
    ax.spines['left'].set_visible(False)
    ax.spines['right'].set_visible(False)

fig, axs = plt.subplots(1, 1, figsize=(10, 5))
axs.scatter(
   sset, [1]*len(sset), s=100, color='g', marker='o', label='astrocyte')
axs.scatter(
   tset, [0]*len(tset), s=100, color='k', marker='^', label='post_neurons')
for sx, tx in zip(slist, tlist):
    axs.plot(
        [sx, tx], [1, 0], linestyle='-', color='g', alpha=0.5, linewidth=2)
axs.legend(loc='right')
set_frame_invisible(axs)
plt.tight_layout()
plt.savefig('astrocyte_connect_rank={}.png'.format(rank))
