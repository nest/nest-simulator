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

from matplotlib.patches import Polygon
import matplotlib.pyplot as plt
import os
import sys
import numpy as np
if 'DISPLAY' not in os.environ:
    import matplotlib
    matplotlib.use('Agg')


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
    num_synapses
        Matrix of synapse numbers.
    """
    prod = np.outer(popsize1, popsize2)
    num_synapses = np.log(1. - conn_probs) / np.log((prod - 1.) / prod)
    return num_synapses


def postsynaptic_potential_to_current(C_m, tau_m, tau_syn):
    """ Computes a factor to convert postsynaptic potentials to currents.

    The time course of the postsynaptic potential ``v`` is computed as
    :math: `v(t)=(i*h)(t)`
    with the exponential postsynaptic current
    :math:`i(t)=J\mathrm{e}^{-t/\tau_\mathrm{syn}}\Theta (t)`,
    the voltage impulse response
    :math:`h(t)=\frac{1}{\tau_\mathrm{m}}\mathrm{e}^{-t/\tau_\mathrm{m}}\Theta (t)`,
    and
    :math:`\Theta(t)=1` if :math:`t\geq 0` and zero otherwise.

    The ``PSP`` is considered as the maximum of ``v``, i.e., it is
    computed by setting the derivative of ``v(t)`` to zero.
    The expression for the time point at which ``v`` reaches its maximum
    can be found in Eq. 5 of [1]_.

    The amplitude of the postsynaptic current ``J`` corresponds to the
    synaptic weight ``PSC``.

    References
    ----------
    .. [1] Hanuschkin A, Kunkel S, Helias M, Morrison A and Diesmann M (2010)
           A general and efficient method for incorporating precise spike times
           in globally time-driven simulations.
           Front. Neuroinform. 4:113.
           DOI: `10.3389/fninf.2010.00113 <https://doi.org/10.3389/fninf.2010.00113>`__.

    Parameters
    ----------
    C_m
        Membrane capacitance (in pF).
    tau_m
        Membrane time constant (in ms).
    tau_syn
        Synaptic time constant (in ms).

    Returns
    -------
    PSC_over_PSP
        Conversion factor to be multiplied to a `PSP` (in mV) to obtain a `PSC`
        (in pA).

    """
    sub = 1. / (tau_syn - tau_m)
    pre = tau_m * tau_syn / C_m * sub
    frac = (tau_m / tau_syn) ** sub

    PSC_over_PSP = 1. / (pre * (frac**tau_m - frac**tau_syn))
    return PSC_over_PSP


def dc_input_compensating_poisson(bg_rate, K_ext, tau_syn, PSC_ext):
    """ Computes DC input if no Poisson input is provided to the microcircuit.

    Parameters
    ----------
    bg_rate
        Rate of external Poisson generators (in spikes/s).
    K_ext
        External indegrees.
    tau_syn
        Synaptic time constant (in ms).
    PSC_ext
        Weight of external connections (in pA).

    Returns
    -------
    DC
        DC input (in pA) which compensates lacking Poisson input.
    """
    DC = bg_rate * K_ext * PSC_ext * tau_syn * 0.001
    return DC


def adjust_weights_and_input_to_synapse_scaling(
        full_num_neurons,
        full_num_synapses,
        K_scaling,
        mean_PSC_matrix,
        PSC_ext,
        tau_syn,
        full_mean_rates,
        DC_amp,
        poisson_input,
        bg_rate,
        K_ext):
    """ Adjusts weights and external input to scaling of indegrees.

    The recurrent and external weights are adjusted to the scaling
    of the indegrees. Extra DC input is added to compensate for the
    scaling in order to preserve the mean and variance of the input.

    Parameters
    ----------
    full_num_neurons
        Total numbers of neurons.
    full_num_synapses
        Total numbers of synapses.
    K_scaling
        Scaling factor for indegrees.
    mean_PSC_matrix
        Weight matrix (in pA).
    PSC_ext
        External weight (in pA).
    tau_syn
        Synaptic time constant (in ms).
    full_mean_rates
        Firing rates of the full network (in spikes/s).
    DC_amp
        DC input current (in pA).
    poisson_input
        True if Poisson input is used.
    bg_rate
        Firing rate of Poisson generators (in spikes/s).
    K_ext
        External indegrees.

    Returns
    -------
    PSC_matrix_new
        Adjusted weight matrix (in pA).
    PSC_ext_new
        Adjusted external weight (in pA).
    DC_amp_new
        Adjusted DC input (in pA).

    """
    PSC_matrix_new = mean_PSC_matrix / np.sqrt(K_scaling)
    PSC_ext_new = PSC_ext / np.sqrt(K_scaling)

    # recurrent input of full network
    indegree_matrix = \
        full_num_synapses / full_num_neurons[:, np.newaxis]
    input_rec = np.sum(mean_PSC_matrix * indegree_matrix * full_mean_rates,
                       axis=1)

    DC_amp_new = DC_amp \
        + 0.001 * tau_syn * (1. - np.sqrt(K_scaling)) * input_rec

    if poisson_input:
        input_ext = PSC_ext * K_ext * bg_rate
        DC_amp_new += 0.001 * tau_syn * (1. - np.sqrt(K_scaling)) * input_ext
    return PSC_matrix_new, PSC_ext_new, DC_amp_new


def plot_raster(path, name, begin, end, N_scaling):
    """ Creates a spike raster plot of the network activity.

    Parameters
    -----------
    path
        Path where the spike times are stored.
    name
        Name of the spike detector.
    begin
        Time point (in ms) to start plotting spikes (included).
    end
        Time point (in ms) to stop plotting spikes (included).
    N_scaling
        Scaling factor for number of neurons.

    Returns
    -------
    None

    """
    fs = 18  # fontsize
    ylabels = ['L2/3', 'L4', 'L5', 'L6']
    color_list = np.tile(['#595289', '#af143c'], 4)

    sd_names, node_ids, data = __load_spike_times(path, name, begin, end)
    last_node_id = node_ids[-1, -1]
    mod_node_ids = np.abs(node_ids - last_node_id) + 1

    label_pos = [(mod_node_ids[i, 0] + mod_node_ids[i + 1, 1]) /
                 2. for i in np.arange(0, 8, 2)]

    stp = 1
    if N_scaling > 0.1:
        stp = int(10. * N_scaling)
        print('  Only spikes of neurons in steps of {} are shown.'.format(stp))

    plt.figure(figsize=(8, 6))
    for i, n in enumerate(sd_names):
        times = data[i]['time_ms']
        neurons = np.abs(data[i]['sender'] - last_node_id) + 1
        plt.plot(times[::stp], neurons[::stp], '.', color=color_list[i])
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
        Time point (in ms) to start calculating the firing rates (included).
    end
        Time point (in ms) to stop calculating the firing rates (included).

    Returns
    -------
    None

    """
    sd_names, node_ids, data = __load_spike_times(path, name, begin, end)
    all_mean_rates = []
    all_std_rates = []
    for i, n in enumerate(sd_names):
        senders = data[i]['sender']
        # 1 more bin than node ids per population
        bins = np.arange(node_ids[i, 0], node_ids[i, 1] + 2)
        spike_count_per_neuron, _ = np.histogram(senders, bins=bins)
        rate_per_neuron = spike_count_per_neuron * 1000. / (end - begin)
        np.savetxt(os.path.join(path, ('rate' + str(i) + '.dat')),
                   rate_per_neuron)
        # zeros are included
        all_mean_rates.append(np.mean(rate_per_neuron))
        all_std_rates.append(np.std(rate_per_neuron))
    print('Mean rates: {} spikes/s'.format(np.around(all_mean_rates, decimals=3)))
    print('Standard deviation of rates: {} spikes/s'.format(
        np.around(all_std_rates, decimals=3)))


def boxplot(path, populations):
    """ Creates a boxblot of the firing rates of all populations.

    To create the boxplot, the firing rates of each neuron in each population
    need to be computed with the function ``firing_rate()``.

    Parameters
    -----------
    path
        Path where the firing rates are stored.
    populations
        Names of neuronal populations.

    Returns
    -------
    None

    """
    fs = 18
    pop_names = [string.replace('23', '2/3') for string in populations]
    label_pos = list(range(len(populations), 0, -1))
    color_list = ['#af143c', '#595289']
    medianprops = dict(linestyle='-', linewidth=2.5, color='black')
    meanprops = dict(linestyle='--', linewidth=2.5, color='lightgray')

    rates_per_neuron_rev = []
    for i in np.arange(len(populations))[::-1]:
        rates_per_neuron_rev.append(
            np.loadtxt(os.path.join(path, ('rate' + str(i) + '.dat'))))

    plt.figure(figsize=(8, 6))
    bp = plt.boxplot(rates_per_neuron_rev, 0, 'rs', 0, medianprops=medianprops,
                     meanprops=meanprops, meanline=True, showmeans=True)
    plt.setp(bp['boxes'], color='black')
    plt.setp(bp['whiskers'], color='black')
    plt.setp(bp['fliers'], color='red', marker='+')

    # boxcolors
    for i in np.arange(len(populations)):
        boxX = []
        boxY = []
        box = bp['boxes'][i]
        for j in list(range(5)):
            boxX.append(box.get_xdata()[j])
            boxY.append(box.get_ydata()[j])
        boxCoords = list(zip(boxX, boxY))
        k = i % 2
        boxPolygon = Polygon(boxCoords, facecolor=color_list[k])
        plt.gca().add_patch(boxPolygon)
    plt.xlabel('firing rate [spikes/s]', fontsize=fs)
    plt.yticks(label_pos, pop_names, fontsize=fs)
    plt.xticks(fontsize=fs)
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
        node_ids.append(l.split())
    node_ids = np.array(node_ids, dtype='i4')
    return sd_files, sd_names, node_ids


def __load_spike_times(path, name, begin, end):
    """ Loads spike times of each spike detector.

    Parameters
    ----------
    path
        Path where the files with the spike times are stored.
    name
        Name of the spike detector.
    begin
        Time point (in ms) to start loading spike times (included).
    end
        Time point (in ms) to stop loading spike times (included).

    Returns
    -------
    data
        Dictionary containing spike times in the interval from ``begin``
        to ``end``.

    """
    sd_files, sd_names, node_ids = __gather_metadata(path, name)
    data = {}
    dtype = {'names': ('sender', 'time_ms'),  # as in header
             'formats': ('i4', 'f8')}
    for i, name in enumerate(sd_names):
        data_i_raw = np.array([[]], dtype=dtype)
        for j, f in enumerate(sd_files):
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
