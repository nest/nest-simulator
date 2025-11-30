# -*- coding: utf-8 -*-
#
# test_tsodyks2_synapse.py
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

import math

import nest
import numpy as np
import pytest


@nest.ll_api.check_stack
class TestTsodyks2Synapse:
    """
    Functional test for the "tsodyks2" synapse: compare NEST implementation to
    a reference generated in the method reproduce_weight_drift(), for a
    sequence of spike times coming from a Poisson generator.
    """

    @pytest.fixture(autouse=True)
    def setUp(self):
        self.resolution = 0.1  # [ms]
        self.presynaptic_firing_rate = 20.0  # [spks/s]
        self.simulation_duration = 1e3  # [ms]
        self.hardcoded_trains_length = 15.0  # [ms]
        self.synapse_parameters = {
            "synapse_model": "tsodyks2_synapse_rec",
            "receptor_type": 1,
            "delay": self.resolution,
            "U": 1.0,
            "u": 1.0,
            "x": 1.0,
            "tau_rec": 100.0,
            "tau_fac": 0.0,
            "weight": 1.0,  # maximal possible response (absolute synaptic efficacy)
        }

    @pytest.mark.parametrize("tau_fac", [0.0, 10.0])
    def test_tsodyk2_synapse(self, tau_fac: float):
        self.synapse_parameters["tau_fac"] = tau_fac

        pre_spikes, weight_by_nest = self.do_the_nest_simulation()
        weight_reproduced_independently = self.reproduce_weight_drift(
            pre_spikes, absolute_weight=self.synapse_parameters["weight"]
        )

        np.testing.assert_allclose(weight_reproduced_independently, weight_by_nest, atol=1e-12)

    def do_the_nest_simulation(self):
        """
        This function is where calls to NEST reside.
        Returns the generated pre- and post spike sequences
        and the resulting weight established by the tsodyks2 synapse.
        """
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.resolution = self.resolution

        neurons = nest.Create("parrot_neuron", 2, params={})
        presynaptic_neuron = neurons[0]
        postsynaptic_neuron = neurons[1]

        presynaptic_generator = nest.Create(
            "poisson_generator",
            params={
                "rate": self.presynaptic_firing_rate,
                "stop": (self.simulation_duration - self.hardcoded_trains_length),
            },
        )

        spike_recorder = nest.Create("spike_recorder")

        nest.Connect(presynaptic_generator, presynaptic_neuron, syn_spec={"synapse_model": "static_synapse"})
        nest.Connect(
            presynaptic_neuron + postsynaptic_neuron, spike_recorder, syn_spec={"synapse_model": "static_synapse"}
        )
        # The synapse of interest itself
        wr = nest.Create("weight_recorder")
        nest.CopyModel("tsodyks2_synapse", "tsodyks2_synapse_rec", {"weight_recorder": wr})
        nest.Connect(presynaptic_neuron, postsynaptic_neuron, syn_spec=self.synapse_parameters)

        nest.Simulate(self.simulation_duration)

        all_spikes = spike_recorder.events
        pre_spikes = all_spikes["times"][all_spikes["senders"] == presynaptic_neuron.get("global_id")]

        weights = wr.get("events", "weights")

        return (pre_spikes, weights)

    def reproduce_weight_drift(self, _pre_spikes, absolute_weight=1.0):
        """
        Returns the total weight change of the synapse
        computed outside of NEST.
        The implementation imitates a step-based simulation: evolving time, we
        trigger a weight update when the time equals one of the spike moments.
        Parameters
        ----------
        absolute_weight : float
            maximal possible response (absolute synaptic efficacy)
        """

        # These are defined just for convenience,
        # STDP is evaluated on exact times nonetheless
        pre_spikes_forced_to_grid = [int(t / self.resolution) for t in _pre_spikes]

        n_steps = 1 + int(np.ceil(self.simulation_duration / self.resolution))
        w_log = []

        t_lastspike = -1.0
        R_ = 1.0  # fraction of synaptic resources available for transmission in the range [0..1]
        u_ = self.synapse_parameters["U"]
        for time_in_simulation_steps in range(n_steps):
            if time_in_simulation_steps in pre_spikes_forced_to_grid:
                # A presynaptic spike occurred now.
                # Adjusting the current time to make it exact.
                t_spike = _pre_spikes[pre_spikes_forced_to_grid.index(time_in_simulation_steps)]

                if t_lastspike >= 0.0:
                    # Evaluating the depression rule.
                    h = t_spike - t_lastspike
                    R_decay = math.exp(-h / self.synapse_parameters["tau_rec"])
                    if self.synapse_parameters["tau_fac"] == 0:  # tau_fac == 0 disables facilitation
                        u_decay = 0.0
                    else:
                        u_decay = math.exp(-h / self.synapse_parameters["tau_fac"])

                    R_ = 1.0 + (R_ - R_ * u_ - 1.0) * R_decay
                    u_ = self.synapse_parameters["U"] + u_ * (1.0 - self.synapse_parameters["U"]) * u_decay

                w = R_ * u_ * absolute_weight
                w_log.append(w)

                t_lastspike = t_spike

        return w_log
