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
#/
# NEST is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

"""PyNEST Microcircuit: Network Class
----------------------------------------

Main file of the microcircuit defining the ``Network`` class with functions to
build and simulate the network.

"""

import os
import numpy as np
import nest
import helpers

class Network:
    """ Provides functions to setup NEST, to create and connect all nodes of
    the network, to simulate, and to evaluate the resulting spike data.

    Arguments
    ---------
    sim_dict
        dictionary containing all parameters specific to the simulation
        such as the directory the data is stored in and the seeds
        (see: ``sim_params.py``)
    net_dict
         dictionary containing all parameters specific to the neurons
         and the network (see: ``network_params.py``)

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
                print('Data directory already exists.')
            else:
                os.mkdir(self.sim_dict['data_path'])
                print('Data directory created.')
            print('Data will be written to: %s' % self.data_path)

        # check if V0_type is correctly set
        v0_type_options = ['original', 'optimized']
        if self.net_dict['V0_type'] not in v0_type_options:
            print(
                """
                {0} is not a valid option for V0_type, replacing it with {1}.
                Valid options are {2}.
                """.format(self.net_dict['V0_type'],
                           v0_type_options[0],
                           v0_type_options)
                )
            self.net_dict['V0_type'] = v0_type_options[0]


    def setup(self):
        """ Executes subfunctions of the network.

        This function executes several subfunctions to create neuronal
        populations, devices and inputs, and to connect the populations with
        each other and with devices and input nodes.

        """
        self.__setup_nest()

        # create nodes
        self.__create_neuronal_populations()
        self.__create_recording_devices()
        self.__create_poisson_bg_input()
        self.__create_thalamic_stim_input()
        self.__create_dc_stim_input()

        # create connections
        self.__connect_neuronal_populations()
        self.__connect_recording_devices()
        if self.net_dict['poisson_input']:
            self.__connect_poisson_bg_input()
        if self.stim_dict['thalamic_input']:
            self.__connect_thalamic_stim_input()
        if self.stim_dict['dc_input']:
            self.__connect_dc_stim_input()

        
    def simulate(self):
        """ Simulates the microcircuit. """
        nest.Simulate(self.sim_dict['t_sim'])


    def evaluate(self, raster_plot_interval, firing_rates_interval):
        """ Displays simulation results.

        Creates a spike raster plot.
        Calculates the firing rate of each population and displays them as a
        box plot.

        Parameters
        ----------
        raster_plot_interval
            Times to start and stop loading spike times for raster plot
            (included).
        firing_rates_interval
            Times to start and stop lading spike times for computing firing
            rates (included).

        Returns
        -------
            None

        """
        if nest.Rank() == 0:
            print(
                'Interval to plot spikes: %s ms'
                % np.array2string(raster_plot_interval)
                )
            helpers.plot_raster(
                self.data_path, 'spike_detector',
                raster_plot_interval[0], raster_plot_interval[1]
                )

            print(
                'Interval to compute firing rates: %s ms'
                % np.array2string(firing_rates_interval)
                )
            helpers.firing_rates(
                self.data_path, 'spike_detector',
                firing_rates_interval[0], firing_rates_interval[1]
                )
            helpers.boxplot(self.net_dict, self.data_path)


    def __setup_nest(self):
        """ Hands parameters to the NEST kernel.

        Resets the NEST kernel and passes parameters to it.
        The number of seeds for random number generation are computed based on
        the total number of virtual processes
        (number of MPI processes x number of threads per MPI process).
        """
        nest.ResetKernel()

        # compute seeds for random number generation
        master_seed = self.sim_dict['master_seed']
        if nest.Rank() == 0:
            print('Master seed: %i ' % master_seed)
        nest.SetKernelStatus(
            {'local_num_threads': self.sim_dict['local_num_threads']}
            )
        N_tp = nest.GetKernelStatus('total_num_virtual_procs')
        if nest.Rank() == 0:
            print('Total number of virtual processes: %i' % N_tp)
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

        # pass parameters to NEST kernel
        self.sim_resolution = self.sim_dict['sim_resolution']
        kernel_dict = {
            'resolution': self.sim_resolution,
            'grng_seed': grng_seed,
            'rng_seeds': rng_seeds,
            'overwrite_files': self.sim_dict['overwrite_files'],
            'print_time': self.sim_dict['print_time'],
            }
        nest.SetKernelStatus(kernel_dict)


    def __create_neuronal_populations(self):
        """ Creates the neuronal populations.

        The neuronal populations are created and the parameters are assigned
        to them. The initial membrane potential of the neurons is drawn from
        normal distributions dependent on the parameter ``V0_type``.
        The number of neurons and synapses is scaled if the parameters
        ``N_scaling`` and ``K_scaling`` are not 1. Scaling of the synapse number
        leads to an extra DC input to be added to the neuronal populations.
        """
        if nest.Rank() == 0:
            print('Creating neuronal populations.')

        # total number of neurons and synapses and information about weights
        # and external input before scaling
        self.N_full = self.net_dict['N_full']
        self.synapses = helpers.total_num_synapses_populations(self.net_dict)
        
        self.w_from_PSP = helpers.weight_as_current_from_potential(
            self.net_dict['PSP_e'], self.net_dict)
        self.weight_mat = helpers.weight_as_current_from_potential(
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
                    No Poisson input provided.
                    Calculating DC input to compensate.
                    """
                    )
            self.DC_amp_e = helpers.dc_input_compensating_poisson(
                self.net_dict, self.w_ext)

        # scaling factors
        self.N_scaling = self.net_dict['N_scaling']
        self.K_scaling = self.net_dict['K_scaling']

        # compute scaled number of neurons and synapses,
        # adjust weights if synapses are scaled
        self.nr_neurons = self.N_full * self.N_scaling 
        self.synapses_scaled = self.synapses * self.K_scaling
        self.K_ext = self.net_dict['K_ext'] * self.K_scaling
        
        if self.K_scaling != 1:
            synapses_indegree = self.synapses / (
                self.N_full.reshape(len(self.N_full), 1) * self.N_scaling)
            self.weight_mat, self.w_ext, self.DC_amp_e = \
                helpers.adjust_weights_and_input_to_synapse_scaling(
                synapses_indegree, self.K_scaling, self.weight_mat,
                self.w_from_PSP, self.DC_amp_e, self.net_dict, self.stim_dict
                )

        if nest.Rank() == 0:
            print(
                'The number of neurons is scaled by a factor of: %.2f'
                % self.N_scaling
                )
            print(
                'The number of synapses is scaled by a factor of: %.2f'
                % self.K_scaling
                )
            if self.K_scaling != 1:
                print('Weights and external input are adjusted for compensation.')

        # create cortical populations
        self.pops = []
        pop_file = open(
            os.path.join(self.data_path, 'population_nodeids.dat'), 'w+'
            )
        for i, pop in enumerate(self.net_dict['populations']):
            population = nest.Create(
                self.net_dict['neuron_model'], int(self.nr_neurons[i])
                )

            population.set(
                    tau_syn_ex=self.net_dict['neuron_params']['tau_syn_ex'],
                    tau_syn_in=self.net_dict['neuron_params']['tau_syn_in'],
                    E_L=self.net_dict['neuron_params']['E_L'],
                    V_th=self.net_dict['neuron_params']['V_th'],
                    V_reset=self.net_dict['neuron_params']['V_reset'],
                    t_ref=self.net_dict['neuron_params']['t_ref'],
                    I_e=self.DC_amp_e[i]
                    )

            if self.net_dict['V0_type'] == 'optimized':
                population.set(V_m=nest.random.normal(
                    self.net_dict['neuron_params']['V0_mean']['optimized'][i],
                    self.net_dict['neuron_params']['V0_sd']['optimized'][i],
                    ))
            elif self.net_dict['V0_type'] == 'original':
                population.set(V_m=nest.random.normal(
                    self.net_dict['neuron_params']['V0_mean']['original'],
                    self.net_dict['neuron_params']['V0_sd']['original'],
                    ))
            self.pops.append(population)
            pop_file.write('%d  %d \n' % (
                population.global_id[0],
                population.global_id[-1]))
        pop_file.close()


    def __create_recording_devices(self):
        """ Creates the recording devices.

        Only devices which are given in ``net_dict['rec_dev']`` are created.

        """
        if nest.Rank() == 0:
            print('Creating recording devices.')

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
                print('Spike detectors created.')
        if 'voltmeter' in self.net_dict['rec_dev']:
            if nest.Rank() == 0:
                print('Voltmeters created.')


    def __create_poisson_bg_input(self):
        """ Creates the Poisson generators for ongoing background input if
        specified in ``network_params.py``.
        If ``poisson_input`` is ``False``, DC input is applied for compensation
        in ``create_neuronal_populations()``.

        """
        if self.net_dict['poisson_input']:
            if nest.Rank() == 0:
                print('Creating Poisson generators for background input.')

            rate_ext = self.net_dict['bg_rate'] * self.K_ext
            self.poisson = []
            for i, target_pop in enumerate(self.pops):
                poisson = nest.Create('poisson_generator')
                poisson.rate = rate_ext[i]
                self.poisson.append(poisson)


    def __create_thalamic_stim_input(self):
        """ Creates the thalamic neuronal population if specified in
        ``stimulus_params.py``.

        """
        if self.stim_dict['thalamic_input']:
            if nest.Rank() == 0:
                print('Creating Thalamic input for external stimulation.')

            self.thalamic_population = nest.Create(
                'parrot_neuron', self.stim_dict['n_thal']
                )
            self.thalamic_weight = helpers.weight_as_current_from_potential(
                self.stim_dict['PSP_th'], self.net_dict
                )
            self.stop_th = (
                self.stim_dict['th_start'] + self.stim_dict['th_duration']
                )
            self.poisson_th = nest.Create('poisson_generator')
            self.poisson_th.set(
                rate=self.stim_dict['th_rate'],
                start=self.stim_dict['th_start'],
                stop=self.stop_th
            )
            nest.Connect(self.poisson_th, self.thalamic_population)
            self.nr_synapses_th = helpers.total_num_synapses_thalamus(
                self.net_dict, self.stim_dict
                )
            if self.K_scaling != 1:
                self.thalamic_weight = self.thalamic_weight / (
                    self.K_scaling ** 0.5)
                self.nr_synapses_th = (self.nr_synapses_th * self.K_scaling)
        else:
            if nest.Rank() == 0:
                print('Thalamic input not provided.')


    def __create_dc_stim_input(self):
        """ Creates DC generators for external stimulation if specified
        in ``stimulus_params.py``.

        """
        if self.stim_dict['dc_input']:
            if nest.Rank() == 0:
                print('Creating DC generators for external stimulation.')
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


    def __connect_neuronal_populations(self):
        """ Creates the recurrent connections between neuronal populations. """
        if nest.Rank() == 0:
            print('Connecting neuronal populations recurrently.')

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


    def __connect_recording_devices(self):
        """ Connects the recording devices to the microcircuit."""
        if nest.Rank() == 0:
            if ('spike_detector' in self.net_dict['rec_dev'] and
                    'voltmeter' not in self.net_dict['rec_dev']):
                print('Connecting spike detectors.')
            elif ('spike_detector' not in self.net_dict['rec_dev'] and
                    'voltmeter' in self.net_dict['rec_dev']):
                print('Connecting voltmeters.')
            elif ('spike_detector' in self.net_dict['rec_dev'] and
                    'voltmeter' in self.net_dict['rec_dev']):
                print('Connecting spike detectors and voltmeters.')
            else:
                print('No recording devices will be connected.')

        for i, target_pop in enumerate(self.pops):
            if 'voltmeter' in self.net_dict['rec_dev']:
                nest.Connect(self.voltmeter[i], target_pop)
            if 'spike_detector' in self.net_dict['rec_dev']:
                nest.Connect(target_pop, self.spike_detector[i])


    def __connect_poisson_bg_input(self):
        """ Connects the Poisson generators to the microcircuit."""
        if nest.Rank() == 0:
            print('Connecting Poisson generators for background input.')

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


    def __connect_thalamic_stim_input(self):
        """ Connects the Thalamic input to the neuronal populations. """

        if nest.Rank() == 0:
            print('Connecting Thalamic input.')

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


    def __connect_dc_stim_input(self):
        """ Connects the DC generators to the neuronal populations. """
        if nest.Rank() == 0:
            print('Connecting DC generators.')

        for i, target_pop in enumerate(self.pops):
            if self.stim_dict['dc_input']:
                nest.Connect(self.dc[i], target_pop)
