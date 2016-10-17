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
--------------------------

Helpers file for the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016
'''

import numpy as np
import os
import sys
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon
#from network_params import net_dict
#from stimulus_params import stim_dict


def compute_DC(net_dict, w_ext):
    """ Computes DC input if no Poisson input is provided to the microcircuit.

    Parameters
    ----------
    net_dict
        Parameters of the microcircuit.
    w_ext
        Weight of external connections.

    Returns
    -------
    DC
        DC input, which compensates lacking Poisson input.
    """
    DC = (
        net_dict['bg_rate'] * net_dict['K_ext'] *
        w_ext * net_dict['neuron_params']['tau_syn_E'] * 0.001
        )
    return DC


def get_weight(PSP_val, net_dict):
    """ Function computes weight to elicit a change in the membrane potential.

    This function computes the weight which elicits a change in the membrane
    potential of size PSP_val. To implement this, the weight is calculated to
    elicit a current that is high enough to implement the desired change in the
    membrane potential.

    Parameters
    ----------
    PSP_val
        Evoked postsynaptic potential.
    net_dict
        Dictionary containing parameters of the microcircuit.

    Returns
    -------
    PSC_e
        Weight value(s).

    """
    C_m = net_dict['neuron_params']['C_m']
    tau_m = net_dict['neuron_params']['tau_m']
    tau_syn_ex = net_dict['neuron_params']['tau_syn_ex']

    PSC_e_over_PSP_e = (((C_m) ** (-1) * tau_m * tau_syn_ex / (
        tau_syn_ex - tau_m) * ((tau_m / tau_syn_ex) ** (
            - tau_m / (tau_m - tau_syn_ex)) - (tau_m / tau_syn_ex) ** (
                - tau_syn_ex / (tau_m - tau_syn_ex)))) ** (-1))
    PSC_e = (PSC_e_over_PSP_e * PSP_val)
    return PSC_e


def get_total_number_of_synapses(net_dict):
    """ Function returns the total number of synapses between all populations.

    The first index (rows) of the output matrix is the target population
    and the second (columns) the source population. If a scaling of the
    synapses is intended this is done in the main simulation script and the
    variable is ignored in this function.

    Parameters
    ----------
    net_dict
        Dictionary containing parameters of the microcircuit.
    N_full
        Number of neurons in all populations.
    number_N
        Total number of populations.
    conn_probs
        Connection probabilities of the eight populations.
    scaling
        Factor that scales the number of neurons.

    Returns
    -------
    K
        Total number of synapses with
        dimensions [len(populations), len(populations)].

    """
    N_full = net_dict['N_full']
    number_N = len(N_full)
    conn_probs = net_dict['conn_probs']
    scaling = net_dict['N_scaling']
    prod = np.outer(N_full, N_full)
    n_syn_temp = np.log(1. - conn_probs)/np.log((prod - 1.) / prod)
    N_full_matrix = np.column_stack((
        N_full for i in list(range(number_N))))
    # if the network is scaled the indegrees are calculated in the same
    # fashion as in the original version of the circuit, which is
    # written in sli
    K = (((n_syn_temp * (
        N_full_matrix * scaling).astype(int)) / N_full_matrix).astype(int))
    return K


def synapses_th_matrix(net_dict, stim_dict):
    """ Computes number of synapses between thalamus and microcircuit.

    This function ignores the variable, which scales the number of synapses.
    If this is intended the scaling is performed in the main simulation script.

    Parameters
    ----------
    net_dict
        Dictionary containing parameters of the microcircuit.
    stim_dict
        Dictionary containing parameters of stimulation settings.
    N_full
        Number of neurons in the eight populations.
    number_N
        Total number of populations.
    conn_probs
        Connection probabilities of the thalamus to the eight populations.
    scaling
        Factor that scales the number of neurons.
    T_full
        Number of thalamic neurons.

    Returns
    -------
    K
        Total number of synapses.

    """
    N_full = net_dict['N_full']
    number_N = len(N_full)
    scaling = net_dict['N_scaling']
    conn_probs = stim_dict['conn_probs_th']
    T_full = stim_dict['n_thal']
    prod = (T_full * N_full).astype(float)
    n_syn_temp = np.log(1. - conn_probs)/np.log((prod - 1.)/prod)
    K = (((n_syn_temp * (N_full * scaling).astype(int))/N_full).astype(int))
    return K


def adj_w_ext_to_K(K_full, K_scaling, w, w_from_PSP, DC, net_dict, stim_dict):
    """ Adjustment of weights to scaling is dine in this function.

    With this funtion the recurrent and external weights are adjusted
    to the scaling of the indegrees. Extra DC input is added to
    compensate the scaling and preserve the mean and variance of the input.

    Parameters
    ----------
    K_full
        Total number of connections between the eight populations.
    K_scaling
        Scaling factor for the connections.
    w
        Weight matrix of the connections of the eight populations.
    w_from_PSP
        Weight of the external connections.
    DC
        DC input to the eight populations.
    net_dict
        Dictionary containing parameters of the microcircuit.
    stim_dict
        Dictionary containing stimulation parameters.
    tau_syn_E
        Time constant of the external postsynaptic excitatory current.
    full_mean_rates
        Mean rates of the eight populations in the full scale version.
    K_ext
        Number of external connections to the eight populations.
    bg_rate
        Rate of the Poissonian spike generator.

    Returns
    -------
    w_new
        Adjusted weight matrix.
    w_ext_new
        Adjusted external weight.
    I_ext
        Extra DC input.

    """
    tau_syn_E = net_dict['neuron_params']['tau_syn_E']
    full_mean_rates = net_dict['full_mean_rates']
    w_mean = w_from_PSP
    K_ext = net_dict['K_ext']
    bg_rate = net_dict['bg_rate']
    w_new = w / np.sqrt(K_scaling)
    I_ext = np.zeros(len(net_dict['populations']))
    x1_all = w * K_full * full_mean_rates
    x1_sum = np.sum(x1_all, axis=1)
    if net_dict['poisson_input']:
        x1_ext = w_mean * K_ext * bg_rate
        w_ext_new = w_mean / np.sqrt(K_scaling)
        I_ext = 0.001 * tau_syn_E * (
            (1. - np.sqrt(K_scaling)) * x1_sum + (
                1. - np.sqrt(K_scaling)) * x1_ext) + DC
    else:
        w_ext_new = w_from_PSP / np.sqrt(K_scaling)
        I_ext = 0.001 * tau_syn_E * (
            (1. - np.sqrt(K_scaling)) * x1_sum) + DC
    return w_new, w_ext_new, I_ext


def read_name(path, name):
    """ Reads names and ids of spike detector.

    This function reads out the names of the spike detectors and
    computes the lowest and highest id of each spike detector.

    Arguments
    ---------
    path
        Path where the spike detector files are stored.
    name
        Name of the spike detector.

    Returns
    -------
    files
        Name of all spike detectors, which are located in the path.
    gids
        Lowest and highest ids of the spike detectors.

    """
    # import filenames
    files = []
    for file in os.listdir(path):
        if file.endswith('.gdf') and file.startswith(name):
            files.append(file)
    # import GIDs
    gidfile = open(path + 'population_GIDs.dat', 'r')
    gids = []
    for l in gidfile:
        a = l.split()
        gids.append([int(a[0]), int(a[1])])
    return files, gids


def plot_raster(path, name, begin, end):
    """ This function creates a spike raster plot of the microcircuit.

    Arguments
    ---------
    path
        Path where the spike times are stored.
    name
        Name of the spike detector.
    begin
        Low time value of spike times to plot.
    end
        High time value of spike times to plot.

    Returns
    -------
    Spike raster plot.

    """

    files, gids = read_name(path, name)
    highest_gid = gids[-1][-1]
    gids_numpy = np.asarray(gids)
    gids_numpy_changed = abs(gids_numpy - highest_gid) + 1
    L23_label_pos = (gids_numpy_changed[0][0] + gids_numpy_changed[1][1])/2
    L4_label_pos = (gids_numpy_changed[2][0] + gids_numpy_changed[3][1])/2
    L5_label_pos = (gids_numpy_changed[4][0] + gids_numpy_changed[5][1])/2
    L6_label_pos = (gids_numpy_changed[6][0] + gids_numpy_changed[7][1])/2

    # import data
    data_all = {}
    for i in list(range(len(files))):
        data_raw = np.loadtxt(os.path.join(path, files[i]))
        cut_low_data_idx = np.where(data_raw[:, 1] > begin)
        data_low_cut_away = data_raw[cut_low_data_idx]
        cut_high_data_idx = np.where(data_low_cut_away[:, 1] < end)
        data_cut = data_low_cut_away[cut_high_data_idx]
        times = data_cut[:, 1]
        neurons = data_cut[:, 0]
        neurons_ch_id = abs(neurons - highest_gid) + 1
        data_all[i] = np.vstack((neurons_ch_id, times))

    ylabels = ['L23', 'L4', 'L5', 'L6']
    color_list = [
        '#000000', '#888888', '#000000', '#888888',
        '#000000', '#888888', '#000000', '#888888'
        ]
    Fig1 = plt.figure(1, figsize=(8, 6))
    for i in list(range(len(files))):
        plt.plot(data_all[i][1], data_all[i][0], '.', color=color_list[i])
    plt.xlabel('time [ms]', fontsize=18)
    plt.xticks(fontsize=18)
    plt.yticks(
        [L23_label_pos, L4_label_pos, L5_label_pos, L6_label_pos],
        ylabels, rotation=10, fontsize=18
        )
    plt.show()


def fire_rate(path, name, begin, end):
    """ This function computes firing rate and standard deviation of it.

    The firing rate of each neuron for each population is computed and stored
    in a numpy file in the directory of the spike detectors. The mean firing
    rate and its standard deviation is also displayed for each population.

    Arguments
    ---------
    path
        Path where the spike times are stored.
    name
        Name of the spike detector.
    begin
        Low time value of spike times to calculate the firing rate.
    end
        High time value of spike times to calculate the firing rate.

    Returns
    -------
    None

    """
    files, gids = read_name(path, name)
    rates_averaged_all = []
    rates_std_all = []
    for h in list(range(len(files))):
        data = np.loadtxt(os.path.join(path, files[h]))
        data_sorted = data[np.argsort(data[:, 1])]
        idx_low = data_sorted[:, 1] > begin
        idx_high = data_sorted[:, 1] < end
        idx = idx_low * idx_high
        data_fil = data_sorted[idx]
        n_fil = data_fil[:, 0]
        n_fil = n_fil.astype(int)
        t_fil = data_fil[:, 1]
        count_of_n = np.bincount(n_fil)
        count_of_n_fil = count_of_n[gids[h][0]-1:gids[h][1]]
        rate_each_n = count_of_n_fil * 1000. / (end - begin)
        rate_averaged = np.mean(rate_each_n)
        rate_std = np.std(rate_each_n)
        rates_averaged_all.append(float('%.3f' % rate_averaged))
        rates_std_all.append(float('%.3f' % rate_std))
        np.save(os.path.join(path, ('rate' + str(h) + '.npy')), rate_each_n)
    print('mean rates: %r Hz' % rates_averaged_all)
    print('std of rates: %r Hz' % rates_std_all)


def box_plot(path, net_dict):
    """ Function creates a boxblot of the firing rates.

    Creates a boxplot of the firing rates of populations of the
    microcircuit. Before this can be done the firing rates need to be computed
    with the function 'fire_rate'.

    Arguments
    ---------
    path
        Path were the firing rates are stored.

    Returns
    -------
    None

    """
    pops = net_dict['N_full']
    reversed_order_list = list(range(len(pops) - 1, -1, -1))
    list_rates_rev = []
    for h in reversed_order_list:
        list_rates_rev.append(
            np.load(os.path.join(path, ('rate' + str(h) + '.npy')))
            )
    pop_names = net_dict['populations']
    label_pos = list(range(len(pops), 0, -1))
    color_list = ['#888888', '#000000']
    medianprops = dict(linestyle='-', linewidth=2.5, color='firebrick')
    fig, ax1 = plt.subplots(figsize=(10, 6))
    bp = plt.boxplot(list_rates_rev, 0, 'rs', 0, medianprops=medianprops)
    plt.setp(bp['boxes'], color='black')
    plt.setp(bp['whiskers'], color='black')
    plt.setp(bp['fliers'], color='red', marker='+')
    for h in list(range(len(pops))):
        boxX = []
        boxY = []
        box = bp['boxes'][h]
        for j in list(range(5)):
            boxX.append(box.get_xdata()[j])
            boxY.append(box.get_ydata()[j])
        boxCoords = list(zip(boxX, boxY))
        k = h % 2
        boxPolygon = Polygon(boxCoords, facecolor=color_list[k])
        ax1.add_patch(boxPolygon)
    plt.xlabel('firing rate [Hz]', fontsize=18)
    plt.yticks(label_pos, pop_names, fontsize=18)
    plt.xticks(fontsize=18)
    plt.show()
