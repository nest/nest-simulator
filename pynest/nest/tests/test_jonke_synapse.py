# -*- coding: utf-8 -*-
#
# test_jonke_synapse.py
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
Test functionality of the Tetzlaff stdp synapse
"""

import unittest
import nest
import numpy as np


@nest.ll_api.check_stack
class JonkeSynapseTest(unittest.TestCase):
    """
    Test the weight change by STDP.
    The test is performed by generating two Poisson spike trains,
    feeding them to NEST as presynaptic and postsynaptic,
    then reconstructing the expected weight change outside of NEST
    and comparing the actual weight change to the expected one.
    """

    resolution = 0.1  # [ms]
    presynaptic_firing_rate = 20.0  # [Hz]
    postsynaptic_firing_rate = 20.0  # [Hz]
    simulation_duration = 1e+4  # [ms]
    hardcoded_trains_length = 15.  # [ms]
    synapse_parameters = {
        "synapse_model": "jonke_synapse",
        "receptor_type": 1,
        "delay": resolution,
        # initial weight
        "weight": 2.0
    }
    synapse_constants = {
        # STDP constants
        "lambda": 0.1 * np.e,
        "alpha": 1.0 / np.e,
        "beta": 0.0,
        "mu_plus": -1.0,
        "mu_minus": 0.0,
        "tau_plus": 36.8,
        "Wmax": 100.0,
    }
    neuron_parameters = {
        "tau_minus": 33.7
    }

    def test_weight_drift(self):
        """
        Runs NEST simulation, records the spikes and weight changes.
        Based on the recorded spikes, the weight changes are confirmed in Python.
        """
        pre_spikes, post_spikes, weight_by_nest = self.do_the_nest_simulation()
        weight_reproduced_independently = self.reproduce_weight_drift(
            pre_spikes, post_spikes,
            self.synapse_parameters["weight"])
        self.assertAlmostEqual(
            weight_reproduced_independently,
            weight_by_nest,
            msg=f"{self.synapse_parameters['synapse_model']} test:\n" +
                f"Resulting synaptic weight {weight_by_nest} " +
                f"differs from expected {weight_reproduced_independently}")

    def do_the_nest_simulation(self):
        """
        This function is where calls to NEST reside.
        Returns the generated pre- and post spike sequences
        and the resulting weight established by STDP.
        """
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': self.resolution})

        neurons = nest.Create(
            "parrot_neuron",
            2,
            params=self.neuron_parameters)
        presynaptic_neuron = neurons[0]
        postsynaptic_neuron = neurons[1]

        generators = nest.Create(
            "poisson_generator",
            2,
            params=({"rate": self.presynaptic_firing_rate,
                     "stop": (self.simulation_duration - self.hardcoded_trains_length)},
                    {"rate": self.postsynaptic_firing_rate,
                     "stop": (self.simulation_duration - self.hardcoded_trains_length)}))
        presynaptic_generator = generators[0]
        postsynaptic_generator = generators[1]

        # While the random sequences, fairly long, would supposedly
        # reveal small differences in the weight change between NEST
        # and ours, some low-probability events (say, coinciding
        # spikes) can well not have occurred. To generate and
        # test every possible combination of pre/post precedence, we
        # append some hardcoded spike sequences:
        # pre: 1       5 6 7   9    11 12 13
        # post:  2 3 4       8 9 10    12
        hardcoded_pre_times, hardcoded_post_times = [[self.simulation_duration - self.hardcoded_trains_length + t
                                                      for t in train]
                                                     for train in ((1, 5, 6, 7, 9, 11, 12, 13),
                                                                   (2, 3, 4, 8, 9, 10, 12))]

        spike_senders = nest.Create(
            "spike_generator",
            2,
            params=({"spike_times": hardcoded_pre_times},
                    {"spike_times": hardcoded_post_times})
        )
        pre_spike_generator = spike_senders[0]
        post_spike_generator = spike_senders[1]

        # The recorder is to save the randomly generated spike trains.
        spike_recorder = nest.Create("spike_recorder")

        nest.Connect(presynaptic_generator + pre_spike_generator, presynaptic_neuron,
                     syn_spec={"synapse_model": "static_synapse"})
        nest.Connect(postsynaptic_generator + post_spike_generator, postsynaptic_neuron,
                     syn_spec={"synapse_model": "static_synapse"})
        nest.Connect(presynaptic_neuron + postsynaptic_neuron, spike_recorder,
                     syn_spec={"synapse_model": "static_synapse"})

        # The synapse of interest itself
        nest.SetDefaults(
            self.synapse_parameters["synapse_model"], self.synapse_constants)
        nest.Connect(presynaptic_neuron, postsynaptic_neuron,
                     syn_spec=self.synapse_parameters)
        plastic_synapse_of_interest = nest.GetConnections(
            synapse_model=self.synapse_parameters["synapse_model"])

        nest.Simulate(self.simulation_duration)

        all_spikes = spike_recorder.events
        pre_spikes = all_spikes['times'][all_spikes['senders'] == presynaptic_neuron.tolist()[0]]
        post_spikes = all_spikes['times'][all_spikes['senders'] == postsynaptic_neuron.tolist()[0]]

        weight = plastic_synapse_of_interest.weight
        return (pre_spikes, post_spikes, weight)

    def reproduce_weight_drift(self, _pre_spikes, _post_spikes, _initial_weight):
        """
        Returns the total weight change of the synapse computed outside of NEST.
        The implementation imitates a step-based simulation: evolving time, we
        trigger a weight update when the time equals one of the spike moments.
        """
        # These are defined just for convenience,
        # STDP is evaluated on exact times nonetheless
        pre_spikes_forced_to_grid = [int(t / self.resolution) for t in _pre_spikes]
        post_spikes_forced_to_grid = [int(t / self.resolution) for t in _post_spikes]

        t_previous_pre = -1
        t_previous_post = -1
        syn_trace_pre = 0.0
        syn_trace_post = 0.0
        weight = _initial_weight
        n_steps = int(self.simulation_duration / self.resolution)
        for time_in_simulation_steps in range(n_steps):
            if time_in_simulation_steps in pre_spikes_forced_to_grid:
                # A presynaptic spike occurred now.

                # Adjusting the current time to make it exact.
                t = _pre_spikes[pre_spikes_forced_to_grid.index(
                    time_in_simulation_steps)]

                if t_previous_post != -1:
                    # Otherwise, if == -1, there have been no post-spikes yet.
                    weight = self.depress(t_previous_post - t, weight, syn_trace_post)

                # Memorizing the current pre-spike and the presynaptic trace
                # to account it further with the next post-spike.
                syn_trace_pre = (
                    syn_trace_pre * np.exp(
                        (t_previous_pre - t) /
                        self.synapse_constants["tau_plus"]
                    ) + 1.0
                )
                t_previous_pre = t

            if time_in_simulation_steps in post_spikes_forced_to_grid:
                # A postsynaptic spike occurred now.

                # Adjusting the current time to make it exact.
                t = _post_spikes[post_spikes_forced_to_grid.index(
                    time_in_simulation_steps)]

                # A post-spike is actually accounted in STDP only after
                # it backpropagates through the dendrite.
                t += self.synapse_parameters["delay"]

                # Evaluating the facilitation rule.
                if t_previous_pre != -1:  # otherwise nothing to pair with
                    weight = self.facilitate(t_previous_pre - t, weight, syn_trace_pre)

                # Memorizing the current post-spike and the postsynaptic trace
                # to account it further with the next pre-spike.
                syn_trace_post = (
                    syn_trace_post * np.exp(
                        (t_previous_post - t) /
                        self.neuron_parameters["tau_minus"]
                    ) + 1
                )
                t_previous_post = t

        return weight

    def facilitate(self, _delta_t, weight, Kplus):
        if _delta_t == 0:
            return weight
        weight += (
            self.synapse_constants["lambda"] / np.e *
            np.exp(1 - weight) *
            Kplus * np.exp(_delta_t / self.synapse_constants["tau_plus"])
        )
        if weight > self.synapse_constants["Wmax"]:
            weight = self.synapse_constants["Wmax"]
        return weight

    def depress(self, _delta_t, weight, Kminus):
        if _delta_t == 0:
            return weight
        weight -= (
            self.synapse_constants["lambda"] / np.e *
            Kminus * np.exp(_delta_t / self.neuron_parameters["tau_minus"])
        )
        if weight < 0:
            weight = 0
        return weight


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(JonkeSynapseTest)
    return unittest.TestSuite([suite])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
