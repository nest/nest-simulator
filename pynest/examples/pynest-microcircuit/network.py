# -*- coding: utf-8 -*-
#
# network.py
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
pynest microcicuit network
----------------------------------------------

Main file for the microcircuit.

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016
'''

import nest
import numpy as np
from helpers import *
import matplotlib.pyplot as plt
import os


class Network:
    """ Handles the setup of the network parameters and
    provides functions to connect the network and devices.

    Arguments:
    sim_dict: dictionary containing all parameters specific to the simulation
              such as the directory the data is stored in and the seeds
              (see: stimulus_params.py)
    net_dict: dictionary containing all parameters specific to the neurons
              and the network
              (see: network_params.py)

    Keyword Arguments:
    stim_dict: dictionary containing all parameter specific to the stimulus
               (see: stimulus_params.py)
    """
    def __init__(self, sim_dict, net_dict, stim_dict=None):
        self.sim_dict = sim_dict
        self.net_dict = net_dict
        if stim_dict is not None:
            self.stim_dict = stim_dict
        print 'Data will be written to %s' % self.sim_dict['data_path']

    # feed NEST-kernel parameter
    def setup(self):
        nest.ResetKernel()
        master_seed = self.sim_dict['master_seed']
        print 'master_seed ', master_seed
        nest.SetKernelStatus({'local_num_threads': self.sim_dict['local_num_threads']})
        N_tp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]  # number of total processes
        print 'number of total processes',  N_tp
        rng_seeds = range(master_seed + 1 + N_tp, master_seed + 1 + (2 * N_tp))
        grng_seed = master_seed + N_tp
        print 'rng_seeds', rng_seeds
        print 'grng_seed', grng_seed
        self.pyrngs = [np.random.RandomState(s) for s in range(master_seed, master_seed + N_tp)]
        kernel_dict = {'resolution': self.sim_dict['sim_resolution'],
                       'grng_seed': grng_seed,
                       'rng_seeds': rng_seeds,
                       'overwrite_files': self.sim_dict['overwrite_files'],
                       }
        nest.SetKernelStatus(kernel_dict)

    # create neuronal populations
    def create_populations(self):
        self.synapses = get_total_number_of_synapses()
        self.synapses_scaled = self.synapses * net_dict['K_scaling']
        self.nr_neurons = net_dict['N_full'] * net_dict['N_scaling']
        self.K_ext = self.net_dict['K_ext'] * net_dict['K_scaling']
        self.w_from_PSP = get_weight(self.net_dict['PSP_e']) # new
        self.weight_mat_mean = get_weight(self.net_dict['PSP_mean_matrix'])
        self.weight_mat_std = self.net_dict['PSP_std_matrix']
        self.w_ext = self.w_from_PSP
        self.DC_amp_extra = np.zeros(len(self.net_dict['populations']))
        if self.net_dict['N_scaling'] != 1:
            print 'The Number of neurons is scaled by a factor of:', self.net_dict['N_scaling']

        # Scaling of the synapses
        if self.net_dict['K_scaling'] != 1:
            synapses_indegree = self.synapses/(self.net_dict['N_full'].reshape(len(self.net_dict['N_full']), 1)
                                               * self.net_dict['N_scaling'])
            self.weight_mat_mean, self.w_ext, self.DC_amp_extra = adjust_w_and_ext_to_K_new(
                synapses_indegree,
                self.net_dict['K_scaling'],
                self.weight_mat_mean, self.w_from_PSP,
                self.DC_amp_extra)
            print 'The number of synapses is scalled by a factor of:', self.net_dict['K_scaling']
        else:
            self.w_ext = self.w_from_PSP

        # Create cortical populations
        self.pops = []
        pop_file = open(self.sim_dict['data_path'] + 'population_GIDs.dat', 'w+')
        for i, pop in enumerate(self.net_dict['populations']):
            population = nest.Create(self.net_dict['neuron_model'],
                                     int(self.nr_neurons[i]))
            nest.SetStatus(population,
                           {'tau_syn_ex': self.net_dict['neuron_params']['tau_syn_ex'],
                            'tau_syn_in': self.net_dict['neuron_params']['tau_syn_in'],
                            'E_L': self.net_dict['neuron_params']['E_L'],
                            'V_th': self.net_dict['neuron_params']['V_th'],
                            'V_reset':  self.net_dict['neuron_params']['V_reset'],
                            't_ref': self.net_dict['neuron_params']['t_ref'],
                            'I_e': self.DC_amp_extra[i]
                            })
            node_info = nest.GetStatus(population)
            local_nodes = [(ni['global_id'], ni['vp']) for ni in node_info if ni['local']]
            for gid, vp in local_nodes:
                nest.SetStatus([gid], {'V_m': self.pyrngs[vp].uniform(self.net_dict['neuron_params']['V_th'],
                                                                      self.net_dict['neuron_params']['V_reset'])})
            self.pops.append(population)
            pop_file.write('%d  %d \n' % (population[0], population[-1]))
        pop_file.close()

    # recording devices are created in this function
    def create_devices(self):
        self.spikedetector = []
        self.voltmeter = []
        for i, pop in enumerate(self.pops):
            if 'spike_detector' in self.net_dict['recording_dev']:
                recdict = {'withgid': True,
                           'withtime': True,
                           'to_memory': False,
                           'to_file': True,
                           'label': self.sim_dict['data_path'] + 'spike_detector'}
                spikedetector = nest.Create('spike_detector', params=recdict)
                self.spikedetector.append(spikedetector)
            if 'voltmeter' in self.net_dict['recording_dev']:
                rcdictmem = {'interval': self.sim_dict['rec_V_int'],
                             'withgid': True,
                             'withtime': True,
                             'to_memory': False,
                             'to_file': True,
                             'label': self.sim_dict['data_path'] + 'volt',
                             'record_from': ['V_m'],
                             }
                volt = nest.Create('voltmeter', params=rcdictmem)
                self.voltmeter.append(volt)

        if 'spike_detector' in self.net_dict['recording_dev']:
            print 'spike_detectors created'
        if 'voltmeter' in self.net_dict['recording_dev']:
            print 'voltmeter created'

    # thalamic neurons are created here
    def create_thalamic_input(self):
        if self.stim_dict['thalamic_input']:
            print 'thalamic input provided'
            self.thalamic_population = nest.Create('parrot_neuron', self.stim_dict['n_thal'])
            self.thalamic_weight = get_weight(self.stim_dict['PSP_th'])
            self.poisson_th = nest.Create('poisson_generator')
            nest.SetStatus(self.poisson_th, {'rate': self.stim_dict['th_rate'],
                                             'start': self.stim_dict['th_start'],
                                             'stop': self.stim_dict['th_start']
                                             + self.stim_dict['th_duration']})
            nest.Connect(self.poisson_th, self.thalamic_population)
            self.nr_synapses_th = synapses_th_matrix()
            if self.net_dict['K_scaling'] != 1:
                self.thalamic_weight = self.thalamic_weight/(self.net_dict['K_scaling']**0.5)
                self.nr_synapses_th = self.nr_synapses_th * self.net_dict['K_scaling']
        else:
            print 'thalamic input not provided'

    # poisson generators are created in this function
    def create_poisson(self):
        if self.net_dict['poisson_input']:
            rate_ext = self.net_dict['bg_rate'] * self.K_ext
            self.poisson = []
            for i, target_pop in enumerate(self.pops):
                    poisson = nest.Create('poisson_generator')
                    nest.SetStatus(poisson, {'rate': rate_ext[i]})
                    self.poisson.append(poisson)
            print 'Poisson background input created'

    # dc generators are created in this functions
    def create_dc_generator(self):
        if self.stim_dict['dc_input']:
            dc_amp = self.net_dict['K_ext'] * self.stim_dict['dc_amplitude']
            self.dc = []
            print 'DC_amp', dc_amp
            for i, target_pop in enumerate(self.pops):
                dc = nest.Create('dc_generator',
                                 params={'amplitude': dc_amp[i],
                                         'start': self.stim_dict['dc_start'],
                                         'stop': self.stim_dict['dc_start'] +
                                         self.stim_dict['dc_dur']})
                self.dc.append(dc)
            print 'DC generator created'

    # the connections between the neuronal populations are created in this function
    def create_connections(self):
        mean_delays = self.net_dict['mean_delay_matrix']
        std_delays = self.net_dict['std_delay_matrix']
        for i, target_pop in enumerate(self.pops):
            for j, source_pop in enumerate(self.pops):
                synapse_nr = int(self.synapses_scaled[i][j])
                if synapse_nr >= 0.:
                    weight = self.weight_mat_mean[i][j]
                    w_sd = abs(weight * self.weight_mat_std[i][j])
                    conn_dict_poisson = {'rule': 'all_to_all'}
                    syn_dict_poisson = {'model': 'static_synapse',
                                        'weight': self.w_ext,
                                        'delay':
                                        {'distribution': 'normal_clipped',
                                                         'mu': 1.5,
                                                         'sigma': 0.75,
                                                         'low': self.sim_dict['sim_resolution']}}
                    conn_dict_recurrent = {'rule': 'fixed_total_number',
                                           'N': synapse_nr}
                    syn_dict_recurrent_ex = {'model': 'static_synapse',
                                             'weight': {'distribution': 'normal_clipped',
                                                        'mu': weight,
                                                        'sigma': w_sd,
                                                        'low': 0.0},
                                             'delay': {'distribution':
                                                       'normal_clipped',
                                                       'mu': mean_delays[i][j],
                                                       'sigma': std_delays[i][j],
                                                       'low': self.sim_dict['sim_resolution']}}
                    syn_dict_recurrent_in = {'model': 'static_synapse',
                                             'weight': {'distribution': 'normal_clipped',
                                                        'mu': weight,
                                                        'sigma': w_sd,
                                                        'high': 0.0},
                                             'delay': {'distribution': 'normal_clipped',
                                                       'mu': mean_delays[i][j],
                                                       'sigma': std_delays[i][j],
                                                       'low': self.sim_dict['sim_resolution']}}
                    if j % 2:
                        syn_dict_recurrent = syn_dict_recurrent_in
                    else:
                        syn_dict_recurrent = syn_dict_recurrent_ex
                    nest.Connect(source_pop, target_pop,
                                 conn_spec=conn_dict_recurrent,
                                 syn_spec=syn_dict_recurrent)
        print 'Recurrent connections established'

    # the poisson generators are connected to the different populations
    def connect_poisson(self):
        for i, target_pop in enumerate(self.pops):
                conn_dict_poisson = {'rule': 'all_to_all'}
                syn_dict_poisson = {'model': 'static_synapse',
                                    'weight': self.w_ext,
                                    'delay': self.net_dict['poisson_delay']}
                nest.Connect(self.poisson[i], target_pop,
                             conn_spec=conn_dict_poisson,
                             syn_spec=syn_dict_poisson)
        print 'Poisson background input connected'

    # the thalamic neurons are connected to the different populations
    def connect_thalamus(self):
        for i, target_pop in enumerate(self.pops):
                conn_dict_th = {'rule': 'fixed_total_number',
                                'N': int(self.nr_synapses_th[i])}
                syn_dict_th = {'weight': {'distribution': 'normal_clipped',
                                          'mu': self.thalamic_weight,
                                          'sigma': self.thalamic_weight
                                          * self.net_dict['PSP_e_rel_sd'],
                                          'low': 0.0},
                               'delay': {'distribution': 'normal_clipped',
                                         'mu': self.stim_dict['delay_th'][i],
                                         'sigma': stim_dict['delay_th_sd'][i],
                                         'low': self.sim_dict['sim_resolution']}}
                nest.Connect(self.thalamic_population, target_pop,
                             conn_spec=conn_dict_th, syn_spec=syn_dict_th)
        print 'Thalamus connected'

    # the dc generators are connected to the neuronal populations
    def connect_dc_generator(self):
        for i, target_pop in enumerate(self.pops):
            if self.stim_dict['dc_input']:
                nest.Connect(self.dc[i], target_pop)
        print 'DC Generator connected'

    # the recording devices are connected to the neuronal populations
    def connect_devices(self):
        for i, target_pop in enumerate(self.pops):
            if 'voltmeter' in self.net_dict['recording_dev']:
                nest.Connect(self.voltmeter[i], target_pop)
            if 'spike_detector' in self.net_dict['recording_dev']:
                nest.Connect(target_pop, self.spikedetector[i])
        print '%s of 2 Devices connected' % (len(self.net_dict['recording_dev']))

    # this function executes many subfunctions, needed to create populations,
    # devices and inputs and the connect functions which connects everything
    def connect(self):
        self.setup()
        self.create_populations()
        self.create_devices()
        self.create_thalamic_input()
        self.create_poisson()
        self.create_dc_generator()
        self.create_connections()
        if self.net_dict['poisson_input']:
            self.connect_poisson()
        if self.stim_dict['thalamic_input']:
            self.connect_thalamus()
        if self.stim_dict['dc_input']:
            self.connect_dc_generator()
        self.connect_devices()

    # this function simulates the network
    def simulate(self):
        nest.Simulate(self.sim_dict['t_sim'])

    def plot_spikes(self):
        dSD = nest.GetStatus(self.spikedetector, keys='events')[0]
        evs = dSD['senders']
        ts = dSD['times']
        plt.figure(1)
        plt.plot(ts, evs, '.')
        plt.show()
