# -*- coding: utf-8 -*-
#
# networks.py
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

r"""Classes to encapsulate the neuronal networks.
----------------------------------------------------------------
Two types of network capable of playing pong are implemented. PongNetRSTDP
can solve the problem by updating the weights of static synapses after
every simulation step according to the R-STDP rules defined in [1]_.

PongNetDopa uses the actor-critic model described in [2]_ to determine the
amount of reward to send to the dopaminergic synapses between input and motor
neurons. In this framework, the motor neurons represent the actor, while a
secondary network of three populations (termed striatum, VP, and dopaminergic
neurons) form the critic which modulates dopamine concentration based on
temporal difference error.

Both of them inherit some functionality from the abstract base class PongNet.

See Also
---------
`Original implementation <https://github.com/electronicvisions/model-sw-pong>`_

References
----------
.. [1] Wunderlich T., et al (2019). Demonstrating advantages of
       neuromorphic computation: a pilot study. Frontiers in neuroscience, 13,
       260. https://doi.org/10.3389/fnins.2019.00260

.. [2] Potjans W., Diesmann M.  and Morrison A. (2011). An imperfect
       dopaminergic error signal can drive temporal-difference learning. PLoS
       Computational Biology, 7(5), e1001133.
       https://doi.org/10.1371/journal.pcbi.1001133

:Authors: J Gille, T Wunderlich, Electronic Vision(s)
"""

from abc import ABC, abstractmethod
from copy import copy
import logging

import numpy as np

import nest

# Simulation time per iteration in milliseconds.
POLL_TIME = 200
# Number of spikes in an input spiketrain per iteration.
N_INPUT_SPIKES = 20
# Inter-spike interval of the input spiketrain.
ISI = 10.
# Standard deviation of Gaussian current noise in picoampere.
BG_STD = 220.
# Reward to be applied depending on distance to target neuron.
REWARDS_DICT = {0: 1., 1: 0.7, 2: 0.4, 3: 0.1}


class PongNet(ABC):

    def __init__(self, apply_noise=True, num_neurons=20):
        """Abstract base class for network wrappers that learn to play pong.
        Parts of the network that are required for both types of inheriting
        class are created here. Namely, spike_generators and their connected
        parrot_neurons, which serve as input, as well as iaf_psc_exp neurons
        and their corresponding spike_recorders which serve as output. The
        connection between input and output is not established here because it
        is dependent on the plasticity rule used.

        Args:
            num_neurons (int, optional): Number of neurons in both the input and
            output layer. Changes here need to be matched in the game
            simulation in pong.py. Defaults to 20.
            apply_noise (bool, optional): If True, Poisson noise is applied
            to the motor neurons of the network. Defaults to True.
        """
        self.apply_noise = apply_noise
        self.num_neurons = num_neurons

        self.weight_history = []
        self.mean_reward = np.array([0. for _ in range(self.num_neurons)])
        self.mean_reward_history = []
        self.winning_neuron = 0

        self.input_generators = nest.Create("spike_generator", self.num_neurons)
        self.input_neurons = nest.Create("parrot_neuron", self.num_neurons)
        nest.Connect(self.input_generators, self.input_neurons,
                     {'rule': 'one_to_one'})

        self.motor_neurons = nest.Create("iaf_psc_exp", self.num_neurons)
        self.spike_recorders = nest.Create("spike_recorder", self.num_neurons)
        nest.Connect(self.motor_neurons, self.spike_recorders,
                     {'rule': 'one_to_one'})

    def get_all_weights(self):
        """Returns all synaptic weights between input and motor neurons.

        Returns:
            numpy.array: 2D array of shape (n_neurons, n_neurons). Input
            neurons are on the first axis, motor neurons on the second axis.
        """
        x_offset = self.input_neurons[0].get("global_id")
        y_offset = self.motor_neurons[0].get("global_id")
        weight_matrix = np.zeros((self.num_neurons, self.num_neurons))
        conns = nest.GetConnections(self.input_neurons, self.motor_neurons)
        for conn in conns:
            source, target, weight = conn.get(
                ["source", "target", "weight"]).values()
            weight_matrix[source - x_offset, target - y_offset] = weight

        return weight_matrix

    def set_all_weights(self, weights):
        """Set synaptic weights between input and motor neurons of the network.

        Args:
            weights (numpy.array): 2D array of shape (n_neurons, n_neurons).
            Input neurons are on the first axis, motor neurons on the second
            axis. See get_all_weights().
        """
        for i in range(self.num_neurons):
            for j in range(self.num_neurons):
                connection = nest.GetConnections(
                    self.input_neurons[i], self.motor_neurons[j])
                connection.set({"weight": weights[i, j]})

    def get_spike_counts(self):
        """Returns the spike counts of all motor neurons from the
        spike_recorders.

        Returns:
            numpy.array: Array of spike counts of all motor neurons.
        """
        events = self.spike_recorders.get("n_events")
        return np.array(events)

    def reset(self):
        """Reset the network for a new iteration by clearing all spike
        recorders.
        """
        self.spike_recorders.set({"n_events": 0})

    def set_input_spiketrain(self, input_cell, biological_time):
        """Set a spike train to the input neuron specified by an index.

        Args:
            input_cell (int): Index of the input neuron to be stimulated.
            biological_time (float): Current biological time within the NEST
            simulator (in ms).
        """
        self.target_index = input_cell
        self.input_train = [
            biological_time + self.input_t_offset + i * ISI
            for i in range(N_INPUT_SPIKES)]
        # Round spike timings to 0.1ms to avoid conflicts with simulation time
        self.input_train = [np.round(x, 1) for x in self.input_train]

        # clear all input generators
        for input_neuron in range(self.num_neurons):
            nest.SetStatus(self.input_generators[input_neuron],
                           {'spike_times': []})

        nest.SetStatus(self.input_generators[input_cell],
                       {'spike_times': self.input_train})

    def get_max_activation(self):
        """Find the motor neuron with the highest activation (number of spikes).

        Returns:
            int: Index of the motor neuron with the highest activation.
        """
        spikes = self.get_spike_counts()
        logging.debug(f"Got spike counts: {spikes}")

        # If multiple neurons have the same activation, one is chosen at random
        return int(np.random.choice(np.flatnonzero(spikes == spikes.max())))

    def calculate_reward(self):
        """Calculates the reward to be applied to the network based on
        performance in the previous simulation (distance between target and
        actual output). For R-STDP this reward informs the learning rule,
        for dopaminergic plasticity this is just a metric of fitness used for
        plotting the simulation.

        Returns:
            float: Reward between 0 and 1.
        """
        self.winning_neuron = self.get_max_activation()
        distance = np.abs(self.winning_neuron - self.target_index)

        if distance in REWARDS_DICT:
            bare_reward = REWARDS_DICT[distance]
        else:
            bare_reward = 0

        reward = bare_reward - self.mean_reward[self.target_index]

        self.mean_reward[self.target_index] = float(
            self.mean_reward[self.target_index] + reward / 2.0)

        logging.debug(f"Applying reward: {reward}")
        logging.debug(
            f"Average reward across all neurons: {np.mean(self.mean_reward)}")

        self.weight_history.append(self.get_all_weights())
        self.mean_reward_history.append(copy(self.mean_reward))

        return reward

    def get_performance_data(self):
        """Retrieve the performance data of the network across all simulations.

        Returns:
            tuple: A Tuple of 2 numpy.arrays containing reward history and
            weight history.
        """
        return self.mean_reward_history, self.weight_history

    @abstractmethod
    def apply_synaptic_plasticity(self, biological_time):
        """Apply weight changes to the synapses according to a given learning
        rule.

        Args:
            biological_time (float): Current NEST simulation time in ms.
        """
        pass


class PongNetDopa(PongNet):
    # Base reward current that is applied regardless of performance
    baseline_reward = 100.
    # Maximum reward current to be applied to the dopaminergic neurons
    max_reward = 1000
    # Constant scaling factor for determining the current to be applied to the
    # dopaminergic neurons
    dopa_signal_factor = 4800
    # Offset for input spikes at every iteration in milliseconds. This offset
    # reserves the first part of every simulation step for the application of
    # the dopaminergic reward signal, avoiding interference between it and the
    # spikes caused by the input of the following iteration
    input_t_offset = 32

    # Neuron and synapse parameters:
    # Initial mean weight for synapses between input- and motor neurons
    mean_weight = 1275.0
    # Standard deviation for starting weights
    weight_std = 8
    # Number of neurons per population in the critic-network
    n_critic = 8
    # Synaptic weights from striatum and VP to the dopaminergic neurons
    w_da = -1150
    # Synaptic weight between striatum and VP
    w_str_vp = -250
    # Synaptic delay for the direct connection between striatum and
    # dopaminergic neurons
    d_dir = 200
    # Rate (Hz) for the background poisson generators
    poisson_rate = 15

    def __init__(self, apply_noise=True, num_neurons=20):
        super().__init__(apply_noise, num_neurons)

        self.vt = nest.Create("volume_transmitter")
        nest.SetDefaults("stdp_dopamine_synapse",
                         {"vt": self.vt.get("global_id"), "tau_c": 70,
                          "tau_n": 30, "tau_plus": 45, "Wmin": 1220,
                          "Wmax": 1550, "b": 0.028, "A_plus": 0.85})

        if apply_noise:
            nest.Connect(self.input_neurons, self.motor_neurons,
                         {'rule': 'all_to_all'},
                         {"synapse_model": "stdp_dopamine_synapse",
                          "weight": nest.random.normal(self.mean_weight,
                                                       self.weight_std)})
            self.poisson_noise = nest.Create("poisson_generator",
                                             self.num_neurons,
                                             params={"rate":
                                                     self.poisson_rate})
            nest.Connect(self.poisson_noise,
                         self.motor_neurons, {'rule': 'one_to_one'},
                         {"weight": self.mean_weight})
        else:
            # Because the poisson_generators cause adtitional spikes in the
            # motor neurons, it is necessary to compensate for their absence by
            # slightly increasing the mean of the weights between input and
            # motor neurons
            nest.SetDefaults("stdp_dopamine_synapse", {"Wmax": 1750})
            nest.Connect(self.input_neurons, self.motor_neurons,
                         {'rule': 'all_to_all'},
                         {"synapse_model": "stdp_dopamine_synapse",
                          "weight": nest.random.normal(
                              self.mean_weight * 1.3, self.weight_std)})

        # Setup the 'critic' as a network of three populations, consisting of
        # the striatum, ventral pallidum (vp) and dopaminergic neurons (dopa)
        self.striatum = nest.Create("iaf_psc_exp", self.n_critic)
        nest.Connect(self.input_neurons, self.striatum, {'rule': 'all_to_all'},
                     {"synapse_model": "stdp_dopamine_synapse",
                      "weight": nest.random.normal(self.mean_weight,
                                                   self.weight_std)})
        self.vp = nest.Create("iaf_psc_exp", self.n_critic)
        nest.Connect(self.striatum, self.vp, syn_spec={"weight": self.w_str_vp})
        self.dopa = nest.Create("iaf_psc_exp", self.n_critic)
        nest.Connect(self.vp, self.dopa, syn_spec={"weight": self.w_da})
        nest.Connect(self.striatum, self.dopa,
                     syn_spec={"weight": self.w_da, "delay": self.d_dir})
        nest.Connect(self.dopa, self.vt)

        # Current generator to stimulate dopaminergic neurons based on
        # network performance
        self.dopa_current = nest.Create("dc_generator")
        nest.Connect(self.dopa_current, self.dopa)

    def apply_synaptic_plasticity(self, biological_time):
        """ Inject a current into the dopaminergic neurons based on how much of
        the motor neurons' activity stems from the target output neuron.
        """
        spike_counts = self.get_spike_counts()
        target_n_spikes = spike_counts[self.target_index]
        # avoid zero division if none of the neurons fired.
        total_n_spikes = max(sum(spike_counts), 1)

        reward_current = self.dopa_signal_factor * target_n_spikes / \
            total_n_spikes + self.baseline_reward

        # Clip the dopaminergic signal to avoid runaway synaptic weights
        reward_current = min(reward_current, self.max_reward)

        self.dopa_current.stop = biological_time + self.input_t_offset
        self.dopa_current.start = biological_time
        self.dopa_current.amplitude = reward_current

        self.calculate_reward()

    def __repr__(self) -> str:
        return ("noisy " if self.apply_noise else "clean ") + "TD"


class PongNetRSTDP(PongNet):
    # Offset for input spikes in every iteration in milliseconds
    input_t_offset = 1
    # Learning rate to use in weight updates
    learning_rate = 0.7
    # Amplitude of STDP curve in arbitrary units
    stdp_amplitude = 36.0
    # Time constant of STDP curve in milliseconds
    stdp_tau = 64.
    # Satuation value for accumulated STDP
    stdp_saturation = 128
    # Initial mean weight for synapses between input- and motor neurons
    mean_weight = 1300.0

    def __init__(self, apply_noise=True, num_neurons=20):

        super().__init__(apply_noise, num_neurons)

        if apply_noise:
            self.background_generator = nest.Create("noise_generator",
                                                    self.num_neurons,
                                                    params={"std": BG_STD})
            nest.Connect(self.background_generator,
                         self.motor_neurons, {'rule': 'one_to_one'})
            nest.Connect(self.input_neurons, self.motor_neurons,
                         {'rule': 'all_to_all'},
                         {"weight": nest.random.normal(self.mean_weight, 1)})
        else:
            # Because the noise_generators cause additional spikes in the motor
            # neurons, it is necessary to compensate for their absence by
            # slightly increasing the mean of the weights between input and
            # motor neurons
            nest.Connect(self.input_neurons, self.motor_neurons,
                         {'rule': 'all_to_all'},
                         {"weight": nest.random.normal(
                             self.mean_weight * 1.22, 5)})

    def apply_synaptic_plasticity(self, biological_time):
        """ Reward network based on how close target and winning neuron are.
        """
        reward = self.calculate_reward()
        self.apply_rstdp(reward)

    def apply_rstdp(self, reward):
        """Apply the previously calculated reward to all relevant synapses
        according to R-STDP principle.

        Args:
            reward (float): reward to be passed on to the synapses.
        """
        # Store spike timings of all motor neurons
        post_events = {}
        offset = self.motor_neurons[0].get("global_id")
        for index, event in enumerate(self.spike_recorders.get("events")):
            post_events[offset + index] = event["times"]

        # Iterate over all connections from the stimulated neuron and change
        # their weights dependent on spike time correlation and reward
        for connection in nest.GetConnections(
                self.input_neurons[self.target_index]):
            motor_neuron = connection.get("target")
            motor_spikes = post_events[motor_neuron]
            correlation = self.calculate_stdp(self.input_train, motor_spikes)
            old_weight = connection.get("weight")
            new_weight = old_weight + self.learning_rate * correlation * reward
            connection.set({"weight": new_weight})

    def calculate_stdp(self, pre_spikes, post_spikes,
                       only_causal=True, next_neighbor=True):
        """Calculates the STDP trace for given spike trains.

        Args:
            pre_spikes (list, numpy.array): Presynaptic spike times in ms.
            post_spikes (list, numpy.array): Postsynaptic spike times in ms.
            only_causal (bool, optional): Use only facilitation and not
            depression. Defaults to True.
            next_neighbor (bool, optional): Use only next-neighbor
            coincidences. Defaults to True.

        Returns:
            [float]: Scalar that corresponds to accumulated STDP trace.
        """

        pre_spikes, post_spikes = np.sort(pre_spikes), np.sort(post_spikes)
        facilitation = 0
        depression = 0
        positions = np.searchsorted(pre_spikes, post_spikes)
        last_position = -1
        for spike, position in zip(post_spikes, positions):
            if position == last_position and next_neighbor:
                continue  # Only next-neighbor pairs
            if position > 0:
                before_spike = pre_spikes[position - 1]
                facilitation += self.stdp_amplitude * \
                    np.exp(-(spike - before_spike) / self.stdp_tau)
            if position < len(pre_spikes):
                after_spike = pre_spikes[position]
                depression += self.stdp_amplitude * \
                    np.exp(-(after_spike - spike) / self.stdp_tau)
            last_position = position
        if only_causal:
            return min(facilitation, self.stdp_saturation)
        else:
            return min(facilitation - depression, self.stdp_saturation)

    def __repr__(self) -> str:
        return ("noisy " if self.apply_noise else "clean ") + "R-STDP"
