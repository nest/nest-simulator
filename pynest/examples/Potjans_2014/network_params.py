# -*- coding: utf-8 -*-
#
# network_params.py
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

"""PyNEST Microcircuit: Network Parameters
---------------------------------------------

A dictionary with base network and neuron parameters is enhanced with derived
parameters.

"""

import numpy as np


def get_exc_inh_matrix(val_exc, val_inh, num_pops):
    """ Creates a matrix for excitatory and inhibitory values.

    Parameters
    ----------
    val_exc
        Excitatory value.
    val_inh
        Inhibitory value.
    num_pops
        Number of populations.

    Returns
    -------
    matrix
        A matrix of of size (num_pops x num_pops).

    """
    matrix = np.zeros((num_pops, num_pops))
    matrix[:, 0:num_pops:2] = val_exc
    matrix[:, 1:num_pops:2] = val_inh
    return matrix


net_dict = {
    # factor to scale the number of neurons
    'N_scaling': 0.1,
    # factor to scale the indegrees
    'K_scaling': 0.1,
    # neuron model
    'neuron_model': 'iaf_psc_exp',
    # names of the simulated neuronal populations
    'populations': ['L23E', 'L23I', 'L4E', 'L4I', 'L5E', 'L5I', 'L6E', 'L6I'],
    # number of neurons in the different populations (same order as
    # 'populations')
    'full_num_neurons':
        np.array([20683, 5834, 21915, 5479, 4850, 1065, 14395, 2948]),
    # mean rates of the different populations in the non-scaled version of the
    # microcircuit (in spikes/s; same order as in 'populations');
    # necessary for the scaling of the network.
    # The values were optained by running this PyNEST microcircuit with 12 MPI
    # processes and both 'N_scaling' and 'K_scaling' set to 1.
    'full_mean_rates':
        np.array([0.943, 3.026, 4.368, 5.882, 7.733, 8.664, 1.096, 7.851]),
    # connection probabilities (the first index corresponds to the targets
    # and the second to the sources)
    'conn_probs':
        np.array(
            [[0.1009, 0.1689, 0.0437, 0.0818, 0.0323, 0., 0.0076, 0.],
             [0.1346, 0.1371, 0.0316, 0.0515, 0.0755, 0., 0.0042, 0.],
             [0.0077, 0.0059, 0.0497, 0.135, 0.0067, 0.0003, 0.0453, 0.],
             [0.0691, 0.0029, 0.0794, 0.1597, 0.0033, 0., 0.1057, 0.],
             [0.1004, 0.0622, 0.0505, 0.0057, 0.0831, 0.3726, 0.0204, 0.],
             [0.0548, 0.0269, 0.0257, 0.0022, 0.06, 0.3158, 0.0086, 0.],
             [0.0156, 0.0066, 0.0211, 0.0166, 0.0572, 0.0197, 0.0396, 0.2252],
             [0.0364, 0.001, 0.0034, 0.0005, 0.0277, 0.008, 0.0658, 0.1443]]),
    # mean amplitude of excitatory postsynaptic potential (in mV)
    'PSP_exc_mean': 0.15,
    # relative standard deviation of the weight
    'weight_rel_std': 0.1,
    # relative inhibitory weight
    'g': -4,
    # mean delay of excitatory connections (in ms)
    'delay_exc_mean': 1.5,
    # mean delay of inhibitory connections (in ms)
    'delay_inh_mean': 0.75,
    # relative standard deviation of the delay of excitatory and
    # inhibitory connections
    'delay_rel_std': 0.5,

    # turn Poisson input on or off (True or False)
    # if False: DC input is applied for compensation
    'poisson_input': True,
    # indegree of external connections to the different populations (same order
    # as in 'populations')
    'K_ext': np.array([1600, 1500, 2100, 1900, 2000, 1900, 2900, 2100]),
    # rate of the Poisson generator (in spikes/s)
    'bg_rate': 8.,
    # delay from the Poisson generator to the network (in ms)
    'delay_poisson': 1.5,

    # initial conditions for the membrane potential, options are:
    # 'original': uniform mean and standard deviation for all populations as
    #             used in earlier implementations of the model
    # 'optimized': population-specific mean and standard deviation, allowing a
    #              reduction of the initial activity burst in the network
    #              (default)
    'V0_type': 'optimized',
    # parameters of the neuron model
    'neuron_params': {
        # membrane potential average for the neurons (in mV)
        'V0_mean': {'original': -58.0,
                    'optimized': [-68.28, -63.16, -63.33, -63.45,
                                  -63.11, -61.66, -66.72, -61.43]},
        # standard deviation of the average membrane potential (in mV)
        'V0_std': {'original': 10.0,
                   'optimized': [5.36, 4.57, 4.74, 4.94,
                                 4.94, 4.55, 5.46, 4.48]},
        # reset membrane potential of the neurons (in mV)
        'E_L': -65.0,
        # threshold potential of the neurons (in mV)
        'V_th': -50.0,
        # membrane potential after a spike (in mV)
        'V_reset': -65.0,
        # membrane capacitance (in pF)
        'C_m': 250.0,
        # membrane time constant (in ms)
        'tau_m': 10.0,
        # time constant of postsynaptic currents (in ms)
        'tau_syn': 0.5,
        # refractory period of the neurons after a spike (in ms)
        't_ref': 2.0}}

# derive matrix of mean PSPs,
# the mean PSP of the connection from L4E to L23E is doubled
PSP_matrix_mean = get_exc_inh_matrix(
    net_dict['PSP_exc_mean'],
    net_dict['PSP_exc_mean'] * net_dict['g'],
    len(net_dict['populations']))
PSP_matrix_mean[0, 2] = 2. * net_dict['PSP_exc_mean']

updated_dict = {
    # matrix of mean PSPs
    'PSP_matrix_mean': PSP_matrix_mean,

    # matrix of mean delays
    'delay_matrix_mean': get_exc_inh_matrix(
        net_dict['delay_exc_mean'],
        net_dict['delay_inh_mean'],
        len(net_dict['populations']))}

net_dict.update(updated_dict)
