# -*- coding: utf-8 -*-
#
# hpc_benchmark.py
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
Random balanced network HPC benchmark
-------------------------------------

This script produces a balanced random network of `scale*11250` neurons in
which the excitatory-excitatory neurons exhibit STDP with
multiplicative depression and power-law potentiation. A mutual
equilibrium is obtained between the activity dynamics (low rate in
asynchronous irregular regime) and the synaptic weight distribution
(unimodal). The number of incoming connections per neuron is fixed
and independent of network size (indegree=11250).

This is the standard network investigated in [1]_, [2]_, [3]_.

A note on scaling
~~~~~~~~~~~~~~~~~

This benchmark was originally developed for very large-scale simulations on
supercomputers with more than 1 million neurons in the network and
11.250 incoming synapses per neuron. For such large networks, synaptic input
to a single neuron will be little correlated across inputs and network
activity will remain stable over long periods of time.

The original network size corresponds to a scale parameter of 100 or more.
In order to make it possible to test this benchmark script on desktop
computers, the scale parameter is set to 1 below, while the number of
11.250 incoming synapses per neuron is retained. In this limit, correlations
in input to neurons are large and will lead to increasing synaptic weights.
Over time, network dynamics will therefore become unstable and all neurons
in the network will fire in synchrony, leading to extremely slow simulation
speeds.

Therefore, the presimulation time is reduced to 50 ms below and the
simulation time to 250 ms, while we usually use 100 ms presimulation and
1000 ms simulation time.

For meaningful use of this benchmark, you should use a scale > 10 and check
that the firing rate reported at the end of the benchmark is below 10 spikes
per second.

References
~~~~~~~~~~

.. [1] Morrison A, Aertsen A, Diesmann M (2007). Spike-timing-dependent
       plasticity in balanced random networks. Neural Comput 19(6):1437-67
.. [2] Helias et al (2012). Supercomputers ready for use as discovery machines
       for neuroscience. Front. Neuroinform. 6:26
.. [3] Kunkel et al (2014). Spiking network simulation code for petascale
       computers. Front. Neuroinform. 8:78

"""

import numpy as np
import os
import time
import scipy.special as sp

import nest
import nest.raster_plot

M_INFO = 10
M_ERROR = 30

###############################################################################
# Parameter section
# Define all relevant parameters: changes should be made here

params = {
    'num_threads': 64,  # total number of threads per process
    'scale': 4.0,  # scaling factor of the network size
    # total network size = scale*11250 neurons
    'simtime': 1000.,  # total simulation time in ms
    'presimtime': 500.,  # simulation time until reaching equilibrium
    'dt': 0.1,  # simulation step
    'compressed_spikes': True,  # whether to use spike compression
    'record_spikes': False,  # switch to record spikes of excitatory neurons to file
    'rng_seed': 654,  # random number generator seed
    'path_name': '.',  # path where all files will have to be written
    'log_file': 'logfile',  # naming scheme for the log files
    'step_data_keys': 'local_spike_counter'  # metrics to be recorded at each time step
}
step_data_keys = params['step_data_keys'].split(',')


def convert_synapse_weight(tau_m, tau_syn, C_m):
    """
    Computes conversion factor for synapse weight from mV to pA

    This function is specific to the leaky integrate-and-fire neuron
    model with alpha-shaped postsynaptic currents.

    """

    # compute time to maximum of V_m after spike input
    # to neuron at rest
    a = tau_m / tau_syn
    b = 1.0 / tau_syn - 1.0 / tau_m
    t_rise = 1.0 / b * (-lambertwm1(-np.exp(-1.0 / a) / a).real - 1.0 / a)

    v_max = np.exp(1.0) / (tau_syn * C_m * b) * (
            (np.exp(-t_rise / tau_m) - np.exp(-t_rise / tau_syn)) /
            b - t_rise * np.exp(-t_rise / tau_syn))
    return 1. / v_max


###############################################################################
# For compatibility with earlier benchmarks, we require a rise time of
# ``t_rise = 1.700759 ms`` and we choose ``tau_syn`` to achieve this for given
# ``tau_m``. This requires numerical inversion of the expression for ``t_rise``
# in ``convert_synapse_weight``. We computed this value once and hard-code
# it here.


tau_syn = 0.32582722403722841

brunel_params = {
    'NE': int(9000 * params['scale']),  # number of excitatory neurons
    'NI': int(2250 * params['scale']),  # number of inhibitory neurons

    'Nrec': 1000,  # number of neurons to record spikes from

    'model_params': {  # Set variables for iaf_psc_alpha
        'E_L': 0.0,  # Resting membrane potential(mV)
        'C_m': 250.0,  # Capacity of the membrane(pF)
        'tau_m': 10.0,  # Membrane time constant(ms)
        't_ref': 0.5,  # Duration of refractory period(ms)
        'V_th': 20.0,  # Threshold(mV)
        'V_reset': 0.0,  # Reset Potential(mV)
        # time const. postsynaptic excitatory currents(ms)
        'tau_syn_ex': tau_syn,
        # time const. postsynaptic inhibitory currents(ms)
        'tau_syn_in': tau_syn,
        'tau_minus': 30.0,  # time constant for STDP(depression)
        # V can be randomly initialized see below
        'V_m': 5.7  # mean value of membrane potential
    },

    ####################################################################
    # Note that Kunkel et al. (2014) report different values. The values
    # in the paper were used for the benchmarks on K, the values given
    # here were used for the benchmark on JUQUEEN.

    'randomize_Vm': True,
    'mean_potential': 5.7,
    'sigma_potential': 7.2,

    'delay': 5.0 + 0.0,  # synaptic delay, all connections(ms)

    # synaptic weight
    'JE': 0.14,  # peak of EPSP

    'sigma_w': 3.47,  # standard dev. of E->E synapses(pA)
    'g': -5.0,

    'stdp_params': {
        'delay': 5.0,
        'axonal_delay': 0.0,
        'alpha': 0.0513,
        'lambda': 0.0,  # STDP step size
        'mu': 0.4,  # STDP weight dependence exponent(potentiation)
        'tau_plus': 15.0,  # time constant for potentiation
    },

    'eta': 1.685,  # scaling of external stimulus
    'filestem': params['path_name']
}


###############################################################################
# Function Section


def build_network():
    """Builds the network including setting of simulation and neuron
    parameters, creation of neurons and connections
    """

    tic = time.time()  # start timer on construction

    # unpack a few variables for convenience
    NE = brunel_params['NE']
    NI = brunel_params['NI']
    model_params = brunel_params['model_params']
    stdp_params = brunel_params['stdp_params']

    # set global kernel parameters
    nest.SetKernelStatus({'local_num_threads': params['num_threads'],
                          'resolution': params['dt'],
                          'rng_seed': params['rng_seed'],
                          'overwrite_files': True,
                          'use_compressed_spikes': params['compressed_spikes'],
                          'keep_source_table': False})
    extra_params = {}
    if extra_params:
        nest.SetKernelStatus(extra_params)

    nest.message(M_INFO, 'build_network', 'Creating excitatory population.')
    E_neurons = nest.Create('iaf_psc_alpha', NE, params=model_params)

    nest.message(M_INFO, 'build_network', 'Creating inhibitory population.')
    I_neurons = nest.Create('iaf_psc_alpha', NI, params=model_params)

    if brunel_params['randomize_Vm']:
        nest.message(M_INFO, 'build_network',
                     'Randomzing membrane potentials.')

        random_vm = nest.random.normal(brunel_params['mean_potential'],
                                       brunel_params['sigma_potential'])
        nest.GetLocalNodeCollection(E_neurons).V_m = random_vm
        nest.GetLocalNodeCollection(I_neurons).V_m = random_vm

    # number of incoming excitatory connections
    CE = int(1. * NE / params['scale'])
    # number of incomining inhibitory connections
    CI = int(1. * NI / params['scale'])

    nest.message(M_INFO, 'build_network',
                 'Creating excitatory stimulus generator.')

    # Convert synapse weight from mV to pA
    conversion_factor = convert_synapse_weight(
        model_params['tau_m'], model_params['tau_syn_ex'], model_params['C_m'])
    JE_pA = conversion_factor * brunel_params['JE']

    nu_thresh = model_params['V_th'] / (
            CE * model_params['tau_m'] / model_params['C_m'] *
            JE_pA * np.exp(1.) * tau_syn)
    nu_ext = nu_thresh * brunel_params['eta']

    E_stimulus = nest.Create('poisson_generator', 1, {
        'rate': nu_ext * CE * 1000.})

    nest.message(M_INFO, 'build_network',
                 'Creating excitatory spike recorder.')

    if params['record_spikes']:
        recorder_label = os.path.join(
            brunel_params['filestem'],
            'alpha_' + str(stdp_params['alpha']) + '_spikes')
        E_recorder = nest.Create('spike_recorder', params={
            'record_to': 'ascii',
            'label': recorder_label
        })

    BuildNodeTime = time.time() - tic
    node_memory = str(get_vmsize())
    node_memory_rss = str(get_rss())
    node_memory_peak = str(get_vmpeak())

    tic = time.time()

    nest.SetDefaults('static_synapse_hpc', {'delay': brunel_params['delay']})
    nest.CopyModel('static_synapse_hpc', 'syn_ex',
                   {'weight': JE_pA})
    nest.CopyModel('static_synapse_hpc', 'syn_in',
                   {'weight': brunel_params['g'] * JE_pA})

    stdp_params['weight'] = JE_pA
    nest.SetDefaults('stdp_pl_synapse_hom_ax_delay_hpc', stdp_params)

    nest.message(M_INFO, 'build_network', 'Connecting stimulus generators.')

    # Connect Poisson generator to neuron

    nest.Connect(E_stimulus, E_neurons, {'rule': 'all_to_all'},
                 {'synapse_model': 'syn_ex'})
    nest.Connect(E_stimulus, I_neurons, {'rule': 'all_to_all'},
                 {'synapse_model': 'syn_ex'})

    nest.message(M_INFO, 'build_network',
                 'Connecting excitatory -> excitatory population.')

    nest.Connect(E_neurons, E_neurons,
                 {'rule': 'fixed_indegree', 'indegree': CE,
                  'allow_autapses': False, 'allow_multapses': True},
                 {'synapse_model': 'stdp_pl_synapse_hom_ax_delay_hpc'})

    nest.message(M_INFO, 'build_network',
                 'Connecting inhibitory -> excitatory population.')

    nest.Connect(I_neurons, E_neurons,
                 {'rule': 'fixed_indegree', 'indegree': CI,
                  'allow_autapses': False, 'allow_multapses': True},
                 {'synapse_model': 'syn_in'})

    nest.message(M_INFO, 'build_network',
                 'Connecting excitatory -> inhibitory population.')

    nest.Connect(E_neurons, I_neurons,
                 {'rule': 'fixed_indegree', 'indegree': CE,
                  'allow_autapses': False, 'allow_multapses': True},
                 {'synapse_model': 'syn_ex'})

    nest.message(M_INFO, 'build_network',
                 'Connecting inhibitory -> inhibitory population.')

    nest.Connect(I_neurons, I_neurons,
                 {'rule': 'fixed_indegree', 'indegree': CI,
                  'allow_autapses': False, 'allow_multapses': True},
                 {'synapse_model': 'syn_in'})

    if params['record_spikes']:
        if params['num_threads'] != 1:
            local_neurons = nest.GetLocalNodeCollection(E_neurons)
            # GetLocalNodeCollection returns a stepped composite NodeCollection, which
            # cannot be sliced. In order to allow slicing it later on, we're creating a
            # new regular NodeCollection from the plain node IDs.
            local_neurons = nest.NodeCollection(local_neurons.tolist())
        else:
            local_neurons = E_neurons

        if len(local_neurons) < brunel_params['Nrec']:
            nest.message(
                M_ERROR, 'build_network',
                """Spikes can only be recorded from local neurons, but the
                number of local neurons is smaller than the number of neurons
                spikes should be recorded from. Aborting the simulation!""")
            exit(1)

        nest.message(M_INFO, 'build_network', 'Connecting spike recorders.')
        nest.Connect(local_neurons[:brunel_params['Nrec']], E_recorder,
                     'all_to_all', 'static_synapse_hpc')

    # read out time used for building
    BuildEdgeTime = time.time() - tic
    network_memory = str(get_vmsize())
    network_memory_rss = str(get_rss())
    network_memory_peak = str(get_vmpeak())

    d = {'py_time_create': BuildNodeTime,
         'py_time_connect': BuildEdgeTime,
         'node_memory': node_memory,
         'node_memory_rss': node_memory_rss,
         'node_memory_peak': node_memory_peak,
         'network_memory': network_memory,
         'network_memory_rss': network_memory_rss,
         'network_memory_peak': network_memory_peak}
    recorders = E_recorder if params['record_spikes'] else None

    return d, recorders


def run_simulation():
    """Performs a simulation, including network construction"""

    nest.ResetKernel()
    nest.set_verbosity(M_ERROR)

    base_memory = str(get_vmsize())
    base_memory_rss = str(get_rss())
    base_memory_peak = str(get_vmpeak())

    build_dict, sr = build_network()

    tic = time.time()

    nest.Prepare()

    InitTime = time.time() - tic
    init_memory = str(get_vmsize())
    init_memory_rss = str(get_rss())
    init_memory_peak = str(get_vmpeak())

    presim_steps = int(params['presimtime'] // nest.min_delay)
    presim_remaining_time = params['presimtime'] - (presim_steps * nest.min_delay)
    sim_steps = int(params['simtime'] // nest.min_delay)
    sim_remaining_time = params['simtime'] - (sim_steps * nest.min_delay)

    total_steps = presim_steps + sim_steps + (1 if presim_remaining_time > 0 else 0) + (
        1 if sim_remaining_time > 0 else 0)
    times, vmsizes, vmpeaks, vmrsss = (
    np.empty(total_steps), np.empty(total_steps), np.empty(total_steps), np.empty(total_steps))
    step_data = {key: np.empty(total_steps) for key in step_data_keys}
    tic = time.time()

    for d in range(presim_steps):
        nest.Run(nest.min_delay)
        times[d] = time.time() - tic
        vmsizes[presim_steps] = get_vmsize()
        vmpeaks[presim_steps] = get_vmpeak()
        vmrsss[presim_steps] = get_rss()
        for key in step_data_keys:
            step_data[key][d] = getattr(nest, key)

    if presim_remaining_time > 0:
        nest.Run(presim_remaining_time)
        times[presim_steps] = time.time() - tic
        vmsizes[presim_steps + sim_steps] = get_vmsize()
        vmpeaks[presim_steps + sim_steps] = get_vmpeak()
        vmrsss[presim_steps + sim_steps] = get_rss()
        for key in step_data_keys:
            step_data[key][presim_steps] = getattr(nest, key)
        presim_steps += 1

    PreparationTime = time.time() - tic
    tic = time.time()

    for d in range(sim_steps):
        nest.Run(nest.min_delay)
        times[presim_steps + d] = time.time() - tic
        for key in step_data_keys:
            step_data[key][presim_steps + d] = getattr(nest, key)

    if sim_remaining_time > 0:
        nest.Run(sim_remaining_time)
        times[presim_steps + sim_steps] = time.time() - tic
        for key in step_data_keys:
            step_data[key][presim_steps + sim_steps] = getattr(nest, key)
        sim_steps += 1

    SimCPUTime = time.time() - tic
    total_memory = str(get_vmsize())
    total_memory_rss = str(get_rss())
    total_memory_peak = str(get_vmpeak())

    average_rate = 0.0
    if params['record_spikes']:
        average_rate = compute_rate(sr)

    d = {'py_time_init': InitTime,
         'py_time_presimulate': PreparationTime,
         'py_time_simulate': SimCPUTime,
         'base_memory': base_memory,
         'init_memory': init_memory,
         'total_memory': total_memory,
         'base_memory_rss': base_memory_rss,
         'init_memory_rss': init_memory_rss,
         'total_memory_rss': total_memory_rss,
         'base_memory_peak': base_memory_peak,
         'init_memory_peak': init_memory_peak,
         'total_memory_peak': total_memory_peak,
         'average_rate': average_rate}
    d.update(build_dict)
    d.update(nest.kernel_status)
    print(d)

    nest.Cleanup()

    fn = '{fn}_{rank}.dat'.format(fn=params['log_file'], rank=nest.Rank())
    with open(fn, 'w') as f:
        for key, value in d.items():
            f.write(key + ' ' + str(value) + '\n')

    fn = '{fn}_{rank}_steps.dat'.format(fn=params['log_file'], rank=nest.Rank())
    with open(fn, 'w') as f:
        f.write('time ' + ' '.join(step_data_keys) + ' \n')
        for d in range(presim_steps + sim_steps):
            f.write(str(times[d]) + ' ' + ' '.join(str(step_data[key][d]) for key in step_data_keys) + ' \n')


def compute_rate(sr):
    """Compute local approximation of average firing rate

    This approximation is based on the number of local nodes, number
    of local spikes and total time. Since this also considers devices,
    the actual firing rate is usually underestimated.

    """

    n_local_spikes = sr.n_events
    n_local_neurons = brunel_params['Nrec']
    simtime = params['simtime']
    return 1. * n_local_spikes / (n_local_neurons * simtime) * 1e3


def _VmB(VmKey):
    _proc_status = '/proc/%d/status' % os.getpid()
    _scale = {'kB': 1024.0, 'mB': 1024.0 * 1024.0, 'KB': 1024.0, 'MB': 1024.0 * 1024.0}
    # get pseudo file  /proc/<pid>/status
    try:
        t = open(_proc_status)
        v = t.read()
        t.close()
    except:
        return 0.0  # non-Linux?
    # get VmKey line e.g. 'VmRSS:  9999  kB\n ...'
    i = v.index(VmKey)
    v = v[i:].split(None, 3)  # whitespace
    if len(v) < 3:
        return 0.0  # invalid format?
    # convert Vm value to bytes
    return float(v[1]) * _scale[v[2]]


def get_vmsize(since=0.0):
    """Return memory usage in bytes."""
    return _VmB('VmSize:') - since


def get_rss(since=0.0):
    """Return resident memory usage in bytes."""
    return _VmB('VmRSS:') - since


def get_vmpeak(since=0.0):
    """Return peak memory usage in bytes."""
    return _VmB('VmPeak:') - since


def lambertwm1(x):
    """Wrapper for LambertWm1 function"""
    # Using scipy to mimic the gsl_sf_lambert_Wm1 function.
    return sp.lambertw(x, k=-1 if x < 0 else 0).real


if __name__ == '__main__':
    run_simulation()