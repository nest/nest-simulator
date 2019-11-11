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

"""
Pynest microcircuit network
---------------------------

Main file for the microcircuit.

Authors
~~~~~~~~

Hendrik Rothe, Hannah Bos, Sacha van Albada; May 2016
"""

import nest
import numpy as np
import os
from helpers import adj_w_ext_to_K
from helpers import synapses_th_matrix
from helpers import get_total_number_of_synapses
from helpers import get_weight
from helpers import plot_raster
from helpers import fire_rate
from helpers import boxplot
from helpers import compute_DC


class Network:
    """ Handles the setup of the network parameters and
    provides functions to connect the network and devices.

    Arguments
    ---------
    sim_dict
        dictionary containing all parameters specific to the simulation
        such as the directory the data is stored in and the seeds
        (see: sim_params.py)
    net_dict
         dictionary containing all parameters specific to the neurons
         and the network (see: network_params.py)

    Keyword Arguments
    -----------------
    stim_dict
        dictionary containing all parameter specific to the stimulus
        (see: stimulus_params.py)

    """
    def __init__(self, sim_dict, net_dict, stim_dict=None):
        self.sim_dict = sim_dict
        self.net_dict = net_dict
        if stim_dict is not None:
            self.stim_dict = stim_dict
        else:
            self.stim_dict = None
        self.data_path = sim_dict['data_path']
        if nest.Rank() == 0:
            if os.path.isdir(self.sim_dict['data_path']):
                print('data directory already exists')
            else:
                os.mkdir(self.sim_dict['data_path'])
                print('data directory created')
            print('Data will be written to %s' % self.data_path)

    def setup_nest(self):
        """ Hands parameters to the NEST-kernel.

        Resets the NEST-kernel and passes parameters to it.
        The number of seeds for the NEST-kernel is computed, based on the
        total number of MPI processes and threads of each.

        """
        nest.ResetKernel()
        master_seed = self.sim_dict['master_seed']
        if nest.Rank() == 0:
            print('Master seed: %i ' % master_seed)
        nest.SetKernelStatus(
            {'local_num_threads': self.sim_dict['local_num_threads']}
            )
        N_tp = nest.GetKernelStatus(['total_num_virtual_procs'])[0]
        if nest.Rank() == 0:
            print('Number of total processes: %i' % N_tp)
        rng_seeds = list(
            range(
                master_seed + 1 + N_tp,
                master_seed + 1 + (2 * N_tp)
                )
            )
        grng_seed = master_seed + N_tp
        if nest.Rank() == 0:
            print(
                'Seeds for random number generators of virtual processes: %r'
                % rng_seeds
                )
            print('Global random number generator seed: %i' % grng_seed)
        self.pyrngs = [np.random.RandomState(s) for s in list(range(
            master_seed, master_seed + N_tp))]
        self.sim_resolution = self.sim_dict['sim_resolution']
        kernel_dict = {
            'resolution': self.sim_resolution,
            'grng_seed': grng_seed,
            'rng_seeds': rng_seeds,
            'overwrite_files': self.sim_dict['overwrite_files'],
            'print_time': self.sim_dict['print_time'],
            }
        nest.SetKernelStatus(kernel_dict)

    def create_populations(self):
        """ Creates the neuronal populations.

        The neuronal populations are created and the parameters are assigned
        to them. The initial membrane potential of the neurons is drawn from a
        normal distribution. Scaling of the number of neurons and of the
        synapses is performed. If scaling is performed extra DC input is added
        to the neuronal populations.

        """
        self.N_full = self.net_dict['N_full']
        self.N_scaling = self.net_dict['N_scaling']
        self.K_scaling = self.net_dict['K_scaling']
        self.synapses = get_total_number_of_synapses(self.net_dict)
        self.synapses_scaled = self.synapses * self.K_scaling
        self.nr_neurons = self.N_full * self.N_scaling
        self.K_ext = self.net_dict['K_ext'] * self.K_scaling
        self.w_from_PSP = get_weight(self.net_dict['PSP_e'], self.net_dict)
        self.weight_mat = get_weight(
            self.net_dict['PSP_mean_matrix'], self.net_dict
            )
        self.weight_mat_std = self.net_dict['PSP_std_matrix']
        self.w_ext = self.w_from_PSP
        if self.net_dict['poisson_input']:
            self.DC_amp_e = np.zeros(len(self.net_dict['populations']))
        else:
            if nest.Rank() == 0:
                print(
                    """
                    no poisson input provided
                    calculating dc input to compensate
                    """
                    )
            self.DC_amp_e = compute_DC(self.net_dict, self.w_ext)

        v0_type_options = ['original', 'optimized']
        if self.net_dict['V0_type'] not in v0_type_options:
            print(
                '''
                '{0}' is not a valid option, replacing it with '{1}'
                Valid options are {2}
                '''.format(self.net_dict['V0_type'],
                           v0_type_options[0],
                           v0_type_options)
                )
            self.net_dict['V0_type'] = v0_type_options[0]
        if nest.Rank() == 0:
            print(
                'The number of neurons is scaled by a factor of: %.2f'
                % self.N_scaling
                )
            print(
                'The number of synapses is scaled by a factor of: %.2f'
                % self.K_scaling
                )

        # Scaling of the synapses.
        if self.K_scaling != 1:
            synapses_indegree = self.synapses / (
                self.N_full.reshape(len(self.N_full), 1) * self.N_scaling)
            self.weight_mat, self.w_ext, self.DC_amp_e = adj_w_ext_to_K(
                synapses_indegree, self.K_scaling, self.weight_mat,
                self.w_from_PSP, self.DC_amp_e, self.net_dict, self.stim_dict
                )

        # Create cortical populations.
        self.pops = []
        pop_file = open(
            os.path.join(self.data_path, 'population_GIDs.dat'), 'w+'
            )
        for i, pop in enumerate(self.net_dict['populations']):
            population = nest.Create(
                self.net_dict['neuron_model'], int(self.nr_neurons[i])
                )
            population.set({
                'tau_syn_ex': self.net_dict['neuron_params']['tau_syn_ex'],
                'tau_syn_in': self.net_dict['neuron_params']['tau_syn_in'],
                'E_L': self.net_dict['neuron_params']['E_L'],
                'V_th': self.net_dict['neuron_params']['V_th'],
                'V_reset':  self.net_dict['neuron_params']['V_reset'],
                't_ref': self.net_dict['neuron_params']['t_ref'],
                'I_e': self.DC_amp_e[i]
                }
            )
            if self.net_dict['V0_type'] == 'optimized':
                population.set(V_m=nest.random.normal(mean=self.net_dict['neuron_params']['V0_mean']['optimized'][i],
                                                      std=self.net_dict['neuron_params']['V0_sd']['optimized'][i]))
            elif self.net_dict['V0_type'] == 'original':
                population.set(V_m=nest.random.normal(mean=self.net_dict['neuron_params']['V0_mean']['original'],
                                                      std=self.net_dict['neuron_params']['V0_sd']['original']))
            self.pops.append(population)
            pop_file.write('%d  %d \n' % (
                population[0].get('global_id'),
                population[-1].get('global_id')))
        pop_file.close()

    def create_devices(self):
        """ Creates the recording devices.

        Only devices which are given in net_dict['rec_dev'] are created.

        """
        self.spike_detector = []
        self.voltmeter = []
        for i, pop in enumerate(self.pops):
            if 'spike_detector' in self.net_dict['rec_dev']:
                recdict = {
                    'record_to': 'ascii',
                    'label': os.path.join(self.data_path, 'spike_detector')
                    }
                dummy = nest.Create('spike_detector', params=recdict)
                self.spike_detector.append(dummy)
            if 'voltmeter' in self.net_dict['rec_dev']:
                recdictmem = {
                    'interval': self.sim_dict['rec_V_int'],
                    'record_to': 'ascii',
                    'label': os.path.join(self.data_path, 'voltmeter'),
                    'record_from': ['V_m'],
                    }
                volt = nest.Create('voltmeter', params=recdictmem)
                self.voltmeter.append(volt)

        if 'spike_detector' in self.net_dict['rec_dev']:
            if nest.Rank() == 0:
                print('Spike detectors created')
        if 'voltmeter' in self.net_dict['rec_dev']:
            if nest.Rank() == 0:
                print('Voltmeters created')

    def create_thalamic_input(self):
        """ This function creates the thalamic neuronal population if this
        is specified in stimulus_params.py.

        """
        if self.stim_dict['thalamic_input']:
            if nest.Rank() == 0:
                print('Thalamic input provided')
            self.thalamic_population = nest.Create(
                'parrot_neuron', self.stim_dict['n_thal']
                )
            self.thalamic_weight = get_weight(
                self.stim_dict['PSP_th'], self.net_dict
                )
            self.stop_th = (
                self.stim_dict['th_start'] + self.stim_dict['th_duration']
                )
            self.poisson_th = nest.Create('poisson_generator')
            self.poisson_th.set({
                'rate': self.stim_dict['th_rate'],
                'start': self.stim_dict['th_start'],
                'stop': self.stop_th
                }
            )
            nest.Connect(self.poisson_th, self.thalamic_population)
            self.nr_synapses_th = synapses_th_matrix(
                self.net_dict, self.stim_dict
                )
            if self.K_scaling != 1:
                self.thalamic_weight = self.thalamic_weight / (
                    self.K_scaling ** 0.5)
                self.nr_synapses_th = (self.nr_synapses_th * self.K_scaling)
        else:
            if nest.Rank() == 0:
                print('Thalamic input not provided')

    def create_poisson(self):
        """ Creates the Poisson generators.

        If Poissonian input is provided, the Poissonian generators are created
        and the parameters needed are passed to the Poissonian generator.

        """
        if self.net_dict['poisson_input']:
            if nest.Rank() == 0:
                print('Poisson background input created')
            rate_ext = self.net_dict['bg_rate'] * self.K_ext
            self.poisson = []
            for i, target_pop in enumerate(self.pops):
                poisson = nest.Create('poisson_generator')
                poisson.set({'rate': rate_ext[i]})
                self.poisson.append(poisson)

    def create_dc_generator(self):
        """ Creates a DC input generator.

        If DC input is provided, the DC generators are created and the
        necessary parameters are passed to them.

        """
        if self.stim_dict['dc_input']:
            if nest.Rank() == 0:
                print('DC generator created')
            dc_amp_stim = self.net_dict['K_ext'] * self.stim_dict['dc_amp']
            self.dc = []
            if nest.Rank() == 0:
                print('DC_amp_stim', dc_amp_stim)
            for i, target_pop in enumerate(self.pops):
                dc = nest.Create(
                    'dc_generator', params={
                        'amplitude': dc_amp_stim[i],
                        'start': self.stim_dict['dc_start'],
                        'stop': (
                            self.stim_dict['dc_start'] +
                            self.stim_dict['dc_dur']
                            )
                        }
                    )
                self.dc.append(dc)

    def create_connections(self):
        """ Creates the recurrent connections.

        The recurrent connections between the neuronal populations are created.

        """
        if nest.Rank() == 0:
            print('Recurrent connections are established')
        mean_delays = self.net_dict['mean_delay_matrix']
        std_delays = self.net_dict['std_delay_matrix']
        for i, target_pop in enumerate(self.pops):
            for j, source_pop in enumerate(self.pops):
                synapse_nr = int(self.synapses_scaled[i][j])
                if synapse_nr >= 0.:
                    weight = self.weight_mat[i][j]
                    w_sd = abs(weight * self.weight_mat_std[i][j])
                    conn_dict_rec = {
                        'rule': 'fixed_total_number', 'N': synapse_nr
                        }
                    syn_dict = {
                        'synapse_model': 'static_synapse',
                        'weight': {
                            'distribution': 'normal_clipped', 'mu': weight,
                            'sigma': w_sd
                            },
                        'delay': {
                            'distribution': 'normal_clipped',
                            'mu': mean_delays[i][j], 'sigma': std_delays[i][j],
                            'low': self.sim_resolution
                            }
                        }
                    if weight < 0:
                        syn_dict['weight']['high'] = 0.0
                    else:
                        syn_dict['weight']['low'] = 0.0
                    nest.Connect(
                        source_pop, target_pop,
                        conn_spec=conn_dict_rec,
                        syn_spec=syn_dict
                        )

    def connect_poisson(self):
        """ Connects the Poisson generators to the microcircuit."""
        if nest.Rank() == 0:
            print('Poisson background input is connected')
        for i, target_pop in enumerate(self.pops):
            conn_dict_poisson = {'rule': 'all_to_all'}
            syn_dict_poisson = {
                'synapse_model': 'static_synapse',
                'weight': self.w_ext,
                'delay': self.net_dict['poisson_delay']
                }
            nest.Connect(
                self.poisson[i], target_pop,
                conn_spec=conn_dict_poisson,
                syn_spec=syn_dict_poisson
                )

    def connect_thalamus(self):
        """ Connects the thalamic population to the microcircuit."""
        if nest.Rank() == 0:
            print('Thalamus connection established')
        for i, target_pop in enumerate(self.pops):
            conn_dict_th = {
                'rule': 'fixed_total_number',
                'N': int(self.nr_synapses_th[i])
                }
            syn_dict_th = {
                'weight': {
                    'distribution': 'normal_clipped',
                    'mu': self.thalamic_weight,
                    'sigma': (
                        self.thalamic_weight * self.net_dict['PSP_sd']
                        ),
                    'low': 0.0
                    },
                'delay': {
                    'distribution': 'normal_clipped',
                    'mu': self.stim_dict['delay_th'][i],
                    'sigma': self.stim_dict['delay_th_sd'][i],
                    'low': self.sim_resolution
                    }
                }
            nest.Connect(
                self.thalamic_population, target_pop,
                conn_spec=conn_dict_th, syn_spec=syn_dict_th
                )

    def connect_dc_generator(self):
        """ Connects the DC generator to the microcircuit."""
        if nest.Rank() == 0:
            print('DC Generator connection established')
        for i, target_pop in enumerate(self.pops):
            if self.stim_dict['dc_input']:
                nest.Connect(self.dc[i], target_pop)

    def connect_devices(self):
        """ Connects the recording devices to the microcircuit."""
        if nest.Rank() == 0:
            if ('spike_detector' in self.net_dict['rec_dev'] and
                    'voltmeter' not in self.net_dict['rec_dev']):
                print('Spike detector connected')
            elif ('spike_detector' not in self.net_dict['rec_dev'] and
                    'voltmeter' in self.net_dict['rec_dev']):
                print('Voltmeter connected')
            elif ('spike_detector' in self.net_dict['rec_dev'] and
                    'voltmeter' in self.net_dict['rec_dev']):
                print('Spike detector and voltmeter connected')
            else:
                print('no recording devices connected')
        for i, target_pop in enumerate(self.pops):
            if 'voltmeter' in self.net_dict['rec_dev']:
                nest.Connect(self.voltmeter[i], target_pop)
            if 'spike_detector' in self.net_dict['rec_dev']:
                nest.Connect(target_pop, self.spike_detector[i])

    def setup(self):
        """ Execute subfunctions of the network.

        This function executes several subfunctions to create neuronal
        populations, devices and inputs, connects the populations with
        each other and with devices and input nodes.

        """
        self.setup_nest()
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

    def simulate(self):
        """ Simulates the microcircuit."""
        nest.Simulate(self.sim_dict['t_sim'])

    def evaluate(self, raster_plot_time_idx, fire_rate_time_idx):
        """ Displays output of the simulation.

        Calculates the firing rate of each population,
        creates a spike raster plot and a box plot of the
        firing rates.

        """
        if nest.Rank() == 0:
            print(
                'Interval to compute firing rates: %s ms'
                % np.array2string(fire_rate_time_idx)
                )
            fire_rate(
                self.data_path, 'spike_detector',
                fire_rate_time_idx[0], fire_rate_time_idx[1]
                )
            print(
                'Interval to plot spikes: %s ms'
                % np.array2string(raster_plot_time_idx)
                )
            plot_raster(
                self.data_path, 'spike_detector',
                raster_plot_time_idx[0], raster_plot_time_idx[1]
                )
            boxplot(self.net_dict, self.data_path)
