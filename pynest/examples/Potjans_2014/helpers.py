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

"""PyNEST Microcircuit: Helper Functions
-------------------------------------------

Helper functions for network construction, simulation and evaluation of the
microcircuit.

"""

import os
import sys
import numpy as np
if 'DISPLAY' not in os.environ:
    import matplotlib
    matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon


def dc_input_compensating_poisson(net_dict, w_ext):
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


def weight_as_current_from_potential(PSP_val, net_dict):
    """ Computes weight as postsynaptic current from postsynaptic potential.

    The weight is calculated as a postsynaptic current that is large enough to
    elicit a change in the membrane potential of size ``PSP_val``.

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


def total_num_synapses_populations(net_dict):
    """ Returns the total number of synapses between all populations.

    The first index (rows) of the output matrix is the target population
    and the second (columns) the source population.

    This function accounts for potentially scaled population sizes
    (``N_scaling``) but a scaling of the synapse numbers is not included.
    Instead, the main script ``network.py`` applies the corresponding
    ``K_scaling`` to the result of this function.

    Parameters
    ----------
    net_dict
        Dictionary containing parameters of the microcircuit.

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
    N_full_matrix = np.tile(N_full, (number_N, 1)).T
    K = (((n_syn_temp * (
        N_full_matrix * scaling).astype(int)) / N_full_matrix).astype(int))
    return K


def total_num_synapses_thalamus(net_dict, stim_dict):
    """ Computes number of synapses between thalamus and microcircuit.

    The first index (rows) of the output matrix is the target population
    and the second (columns) is the thalamus.

    This function accounts for potentially scaled population sizes
    (``N_scaling``) but a scaling of the synapse numbers is not included.
    Instead, the main script ``network.py`` applies the corresponding
    ``K_scaling`` to the result of this function.

    Parameters
    ----------
    net_dict
        Dictionary containing parameters of the microcircuit.
    stim_dict
        Dictionary containing stimulation parameters.

    Returns
    -------
    K
        Total number of synapses from the thalamus to the populations with
        dimensions[len(populations), 1]

    """
    N_full = net_dict['N_full']
    scaling = net_dict['N_scaling']
    conn_probs = stim_dict['conn_probs_th']
    T_full = stim_dict['n_thal']
    prod = (T_full * N_full).astype(float)
    n_syn_temp = np.log(1. - conn_probs)/np.log((prod - 1.)/prod)
    K = ((n_syn_temp * (N_full * scaling).astype(int))/N_full).astype(int)
    return K


def adjust_weights_and_input_to_synapse_scaling(
        K_full, K_scaling, w, w_from_PSP, DC, net_dict, stim_dict):
    """ Adjusts weights and external input to scaling of synapse numbers.

    The recurrent and external weights are adjusted to the scaling
    of the synapse numbers. Extra DC input is added to compensate for the
    scaling in order to preserve the mean and variance of the input.

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


def plot_raster(path, name, begin, end):
    """ Creates a spike raster plot of the network activity.

    Parameters
    -----------
    path
        Path where the spike times are stored.
    name
        Name of the spike detector.
    begin
        Time point to start plotting spikes (included).
    end
        Time point to stop plotting spikes (included).

    Returns
    -------
    None

    """
    fs = 18
    ylabels = ['L2/3', 'L4', 'L5', 'L6']
    color_list = np.tile(['#000000', '#888888'], 4)

    sd_names, node_ids, data = __load_spike_times(path, name, begin, end)
    last_node_id = node_ids[-1,-1]
    mod_node_ids = np.abs(node_ids - last_node_id) + 1

    label_pos = [(mod_node_ids[i,0] + mod_node_ids[i+1,1])/2. for i in np.arange(0,8,2)]

    fig = plt.figure(1, figsize=(8, 6))
    for i,n in enumerate(sd_names):
        times = data[i]['time_ms']
        neurons = np.abs(data[i]['sender'] - last_node_id) + 1
        plt.plot(times, neurons, '.', color=color_list[i])
    plt.xlabel('time [ms]', fontsize=fs)
    plt.xticks(fontsize=fs)
    plt.yticks(label_pos, ylabels, fontsize=fs)
    plt.savefig(os.path.join(path, 'raster_plot.png'), dpi=300)


def firing_rates(path, name, begin, end):
    """ Computes mean and standard deviation of firing rates per population.

    The firing rate of each neuron in each population is computed and stored
    in a .dat file in the directory of the spike detectors. The mean firing
    rate and its standard deviation are printed out for each population.

    Parameters
    -----------
    path
        Path where the spike times are stored.
    name
        Name of the spike detector.
    begin
        Time point to start calculating the firing rates (included).
    end
        Time point to stop calculating the firing rates (included).

    Returns
    -------
    None

    """
    sd_names, node_ids, data = __load_spike_times(path, name, begin, end)
    all_mean_rates = []
    all_std_rates = []
    for i,n in enumerate(sd_names):
        senders = data[i]['sender']
        # 1 more bin than node ids per population
        bins = np.arange(node_ids[i,0], node_ids[i,1]+2) 
        spike_count_per_neuron, _ = np.histogram(senders, bins=bins)
        rate_per_neuron = spike_count_per_neuron * 1000. / (end - begin)
        np.savetxt(os.path.join(path, ('rate' + str(i) + '.dat')),
                   rate_per_neuron)
        # zeros are included    
        all_mean_rates.append(float('%.3f' % np.mean(rate_per_neuron)))
        all_std_rates.append(float('%.3f' % np.std(rate_per_neuron)))
    print('Mean rates: %r Hz' % all_mean_rates)
    print('Standard deviation of rates: %r Hz' % all_std_rates)


def boxplot(net_dict, path):
    """ Creates a boxblot of the firing rates of all populations.

    To create the boxplot, the firing rates of each neuron in each population
    need to be computed with the function ``fire_rate()``.

    Parameters
    -----------
    net_dict
        Dictionary containing parameters of the microcircuit.
    path
        Patnp.mean(rate_per_neuron)h were the firing rates are stored.

    Returns
    -------
    None

    """
    fs = 18
    pops = net_dict['N_full']
    pop_names = [string.replace('23', '2/3') for string in net_dict['populations']]
    label_pos = list(range(len(pops), 0, -1))
    color_list = ['#888888', '#000000']
    medianprops = dict(linestyle='-', linewidth=2.5, color='green')
    meanprops = dict(linestyle='--', linewidth= 2.5, color='blue')
    
    rates_per_neuron_rev = []
    for i in np.arange(len(pops))[::-1]:
        rates_per_neuron_rev.append(
            np.loadtxt(os.path.join(path, ('rate' + str(i) + '.dat'))))

    fig, ax1 = plt.subplots(figsize=(10, 6))
    bp = plt.boxplot(rates_per_neuron_rev, 0, 'rs', 0, medianprops=medianprops,
        meanprops=meanprops, meanline=True, showmeans=True)
    plt.setp(bp['boxes'], color='black')
    plt.setp(bp['whiskers'], color='black')
    plt.setp(bp['fliers'], color='red', marker='+')

    # boxcolors
    for i in np.arange(len(pops)):
        boxX = []
        boxY = []
        box = bp['boxes'][i]
        for j in list(range(5)):
            boxX.append(box.get_xdata()[j])
            boxY.append(box.get_ydata()[j])
        boxCoords = list(zip(boxX, boxY))
        k = i % 2
        boxPolygon = Polygon(boxCoords, facecolor=color_list[k])
        ax1.add_patch(boxPolygon)
    plt.xlabel('firing rate [Hz]', fontsize=18)
    plt.yticks(label_pos, pop_names, fontsize=18)
    plt.xticks(fontsize=18)
    plt.savefig(os.path.join(path, 'box_plot.png'), dpi=300)


def __gather_metadata(path, name):
    """ Reads names and ids of spike detectors and first and last ids of
    neurons in each population.

    If the simulation was run on several threads or MPI-processes, one name per
    spike detector per MPI-process/thread is extracted.

    Parameters
    ------------
    path
        Path where the spike detector files are stored.
    name
        Name of the spike detector, typically ``spike_detector``.

    Returns
    -------
    sd_files
        Names of all files written by spike detectors.
    sd_names
        Names of all spike detectors.
    node_ids
        Lowest and highest id of nodes in each population.

    """
    # load filenames
    sd_files = []
    sd_names = []
    for fn in sorted(os.listdir(path)):
        if fn.startswith(name):
            sd_files.append(fn)
            # spike detector name and its ID
            fnsplit = '-'.join(fn.split('-')[:-1])
            if fnsplit not in sd_names:
                sd_names.append(fnsplit)

    # load node IDs
    node_idfile = open(path + 'population_nodeids.dat', 'r')
    node_ids = []
    for l in node_idfile:
        a = l.split()
        node_ids.append([int(a[0]), int(a[1])])
    return sd_files, sd_names, np.array(node_ids)


def __load_spike_times(path, name, begin, end):
    """ Loads spike times of each spike detector.

    Parameters
    ----------
    path
        Path where the files with the spike times are stored.
    name
        Name of the spike detector.
    begin
        Time point to start loading spike times (included).
    end
        Time point to stop loading spike times (included).

    Returns
    -------
    data
        Dictionary containing spike times in the interval from ``begin``
        to ``end``.

    """
    sd_files, sd_names, node_ids = __gather_metadata(path, name)
    data = {}
    dtype = {'names': ('sender', 'time_ms'), # as in header
             'formats': ('i4', 'f8')}
    for i,name in enumerate(sd_names):
        data_i_raw = np.array([[]], dtype=dtype)
        for j,f in enumerate(sd_files):
            if name in f:
                # skip header while loading
                ld = np.loadtxt(os.path.join(path, f), skiprows=3, dtype=dtype)
                data_i_raw = np.append(data_i_raw, ld)

        data_i_raw = np.sort(data_i_raw, order='time_ms')
        # begin and end are included if they exist
        low = np.searchsorted(data_i_raw['time_ms'], v=begin, side='left')
        high = np.searchsorted(data_i_raw['time_ms'], v=end, side='right')
        data[i] = data_i_raw[low:high]
    return sd_names, node_ids, data

