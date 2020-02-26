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

def num_synapses_from_conn_probs(conn_probs, popsize1, popsize2):
    """Computes the total number of synapses between two populations from
    connection probabilities.

    Here it is irrelevant which population is source and which target.

    Paramters
    ---------
    conn_probs
        Matrix of connection probabilities.
    popsize1
        Size of first poulation.
    popsize2
        Size of second population.

    Returns
    -------
    K
        Matrix of synapse numbers.
    """
    prod = np.outer(popsize1, popsize2)
    K = np.log(1. - conn_probs)/np.log((prod - 1.) / prod)
    return K


def weight_as_current_from_potential(PSP, C_m, tau_m, tau_syn):
    """ Computes weight as postsynaptic current from postsynaptic potential.

    The weight is calculated as a postsynaptic current that is large enough to
    elicit a change in the membrane potential of size ``PSP_e``.

    Parameters
    ----------
    PSP
        Mean amplitude of postsynaptic potential.
    C_m
        Membrane capacitance.
    tau_m
        Membrane time constant.
    tau_syn
        Synaptic time constant.

    Returns
    -------
    PSC
        Amplitude of postynaptic current.

    """
    PSC_over_PSP= (((C_m) ** (-1) * tau_m * tau_syn / (
        tau_syn - tau_m) * ((tau_m / tau_syn) ** (
            - tau_m / (tau_m - tau_syn)) - (tau_m / tau_syn) ** (
                - tau_syn / (tau_m - tau_syn)))) ** (-1))
    PSC = PSC_over_PSP * PSP
    return PSC


def dc_input_compensating_poisson(bg_rate, K_ext, tau_syn, PSC_ext):
    """ Computes DC input if no Poisson input is provided to the microcircuit.

    Parameters
    ----------
    bg_rate
        Rate of external Poisson generators.
    K_ext
        External indegrees.
    tau_syn
        Synaptic time constant in ms.
    PSC_ext
        Weight of external connections in pA.

    Returns
    -------
    DC
        DC input, which compensates lacking Poisson input.
    """
    DC = bg_rate * K_ext * PSC_ext * tau_syn * 0.001
    return DC


def adjust_weights_and_input_to_synapse_scaling(
    N_full, full_num_synapses, K_scaling, mean_PSC_matrix, PSC_ext, tau_syn,
    full_mean_rates, DC_amp, poisson_input, bg_rate, K_ext):
    """ Adjusts weights and external input to scaling of indegrees.

    The recurrent and external weights are adjusted to the scaling
    of the indegrees. Extra DC input is added to compensate for the
    scaling in order to preserve the mean and variance of the input.

    Parameters
    ----------
    N_full
        Total numbers of neurons.
    full_num_synapses
        Total numbers of synapses.
    K_scaling
        Scaling factor for indegrees.
    mean_PSC_matrix
        Weight matrix.
    PSC_ext
        External weight.
    tau_syn
        Synaptic time constant.
    full_mean rates
        Firing rates of the full network.
    DC_amp
        DC input current.
    poisson_input
        True if Poisson input is used.
    bg_rate
        Firing rate of Poisson generators.
    K_ext
        External indegrees.

    Returns
    -------
    PSC_matrix_new
        Adjusted weight matrix.
    PSC_ext_new
        Adjusted external weight.
    DC_amp_new
        Adjusted DC input.

    """
    PSC_matrix_new = mean_PSC_matrix / np.sqrt(K_scaling)
    PSC_ext_new = PSC_ext / np.sqrt(K_scaling)
    
    # recurrent input of full network
    indegree_matrix = \
        full_num_synapses / N_full.reshape(len(K_ext), 1)
    input_rec = np.sum(mean_PSC_matrix * indegree_matrix * full_mean_rates,
                       axis=1)

    DC_amp_new = DC_amp \
        + 0.001 * tau_syn * (1. - np.sqrt(K_scaling)) * input_rec

    if poisson_input:
        input_ext = PSC_ext * K_ext * bg_rate
        DC_amp_new += 0.001 * tau_syn * (1. - np.sqrt(K_scaling)) * input_ext
    return PSC_matrix_new, PSC_ext_new, DC_amp_new


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

