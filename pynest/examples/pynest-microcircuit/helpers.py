# -*- coding: utf-8 -*-
#
# helpers.py
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
pynest microcicuit helpers
----------------------------------------------

Helpers file for the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016
'''

import numpy as np
from network_params import *
from stimulus_params import *


def get_weight(PSP_val):
    """This function computes the weight which elicits a change in the membrane
    potential of size PSP_val.

    Arguements:
    PSP_val: evoked postsynaptic potential

    Returns: weight value

    """
    C_m = net_dict['neuron_params']['C_m']
    tau_m = net_dict['neuron_params']['tau_m']
    tau_ex = net_dict['neuron_params']['tau_syn_ex']
    tau_in = net_dict['neuron_params']['tau_syn_in']

    PSC_e_over_PSP_e = (((C_m)**(-1) * tau_m * tau_ex / (tau_ex - tau_m) * (
        (tau_m / tau_ex)**(- tau_m / (tau_m-tau_ex)) - (
            tau_m/tau_ex)**(- tau_ex/(tau_m-tau_ex))))**(-1))
    PSC_e = (PSC_e_over_PSP_e * PSP_val)
    return PSC_e


def get_total_number_of_synapses():
    """This function returns the total number of synapses between all populations.
    The first index (rows) of the matrix is the target population
    and the second (columns) the source population.
    This function depends on the following parameters specified in net_dict:
    number of populations, Number of Neurons (N_full),
    connection probability (conn_probs), scaling factor (N_scaling)

    Returns: Matrix with total number of synapses,
        dimension: (len(populations), len(populations)).
    """
    N_full = net_dict['N_full']
    conn_probs = net_dict['conn_probs']
    scaling = net_dict['N_scaling']
    prod = np.outer(N_full, N_full)
    n_syn_temp = np.log(1. - conn_probs)/np.log((prod - 1.) / prod)
    N_full_matrix = np.column_stack((N_full for i in range(len(N_full))))
    # if the network is scaled the indegrees are calculated in the same
    # fashion as in the original version of the circuit, which is
    # written in sli
    K = (((n_syn_temp * (
        N_full_matrix * scaling).astype(int)) / N_full_matrix).astype(int))
    return K


def synapses_th_matrix():
    """This function returns the total number of synapses,
        between the thalamus and all target populations.

    Parameters: length of populations, number of neurons (N_full), connection
        probability (conn_probs), scaling factor (N_scaling)

    Returns: Vector with total number of synapses.
    """
    scaling = net_dict['N_scaling']
    N_full = net_dict['N_full']
    conn_probs = stim_dict['conn_probs_th']
    scaling = net_dict['N_scaling']
    prod = (stim_dict['n_thal'] * net_dict['N_full']).astype(float)
    n_syn_temp = np.log(1. - conn_probs)/np.log((prod - 1.)/prod)
    K = (((n_syn_temp * (N_full * scaling).astype(int))/N_full).astype(int))
    return K


def adjust_w_and_ext_to_K_new(K_full, K_scaling, w, w_from_PSP, DC):
    """With this funtion the recurrent and external weights are adjusted
    to the scaling of the indegrees. Extra DC input is added to
    compensate the scaling and preserve the mean and variance of the input.

    Parameters:
    Time constant of the external postsynaptic excitatory current (tau_syn_E),
    mean rates of the populations in the non-scaled version (full_mean_rates),
    weight calcuted with function 'create_weight_from_PSP' (w_from_PSP),
    number of external connections to the different populations (K_ext),
    rate of the poissonian spike generator (bg_rate),
    scaling factor for the connections (K_scaling),
    weight matrix, created by the function 'weight_mat' (w),
    DC input, vector with zeros (DC).

    Returns: Weight matrix, weight for external input, extra DC input.
    """
    tau_syn_E = net_dict['neuron_params']['tau_syn_E']
    full_mean_rates = net_dict['full_mean_rates']
    w_mean = w_from_PSP
    K_ext = net_dict['K_ext']
    bg_rate = net_dict['bg_rate']
    internal_scaling = K_scaling
    w_new = w / np.sqrt(internal_scaling)
    I_ext = np.zeros(len(net_dict['populations']))
    x1_all = w * K_full * full_mean_rates
    x1_sum = np.sum(x1_all, axis=1)
    if net_dict['poisson_input']:
        x1_ext = w_mean * K_ext * bg_rate
        external_scaling = K_scaling
        w_ext_new = w_mean / np.sqrt(external_scaling)
        I_ext = 0.001 * tau_syn_E * (
            (1. - np.sqrt(internal_scaling)) * x1_sum + (
                1. - np.sqrt(external_scaling)) * x1_ext) + DC
    else:
        w_ext_new = np.nan
        I_ext = 0.001 * tau_syn_E * (
            (1. - np.sqrt(internal_scaling)) * x1_sum) + DC
    return w_new, w_ext_new, I_ext
