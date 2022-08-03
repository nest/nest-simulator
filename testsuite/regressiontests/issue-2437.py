# -*- coding: utf-8 -*-
#
# issue-2437.py
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
This script ensures that the STDP synapses work correctly even for some edge cases.

This is a regression test for GitHub issue 2437.
"""

import nest
from math import exp
import numpy as np


class TestSTDPPlSynapse:
    """
    Compare the STDP power-law synaptic plasticity model against a self-contained Python reference.

    Random pre and post spike times are generated according to a Poisson distribution; some hard-coded spike times are
    added to make sure to test for edge cases such as simultaneous pre and post spike.
    """

    def __init__(self):
        self.resolution = 0.1  # [ms]
        self.simulation_duration = 1E2  # [ms]
        self.synapse_model = "stdp_pl_synapse_hom"
        self.nest_neuron_model = "iaf_psc_delta"
        self.tau_pre = 20.0
        self.tau_post = 33.7
        self.init_weight = .5
        self.dendritic_delay = 1.0
        self.synapse_common_properties = {
            "lambda": 0.1,
            "alpha": 1.0,
            "mu": 0.4,
            "tau_plus": self.tau_pre,
        }
        self.synapse_parameters = {
            "synapse_model": self.synapse_model,
            "receptor_type": 0,
            "delay": self.dendritic_delay,
            "weight": self.init_weight
        }
        self.neuron_parameters = {
            "tau_minus": self.tau_post,
            "t_ref": 1.0
        }

        self.hardcoded_pre_times = np.array([1.5, 3.1], dtype=float)
        self.hardcoded_post_times = np.array([0.2, 2.1, 3.4], dtype=float)

    def do_nest_simulation_and_compare_to_reproduced_weight(self):
        pre_spikes, post_spikes, t_weight_by_nest, weight_by_nest = self.do_the_nest_simulation()

        weight_reproduced = self.reproduce_weight_drift(pre_spikes, post_spikes, self.init_weight)
        np.testing.assert_allclose(weight_by_nest, weight_reproduced)

    def do_the_nest_simulation(self):
        """
        This function is where calls to NEST reside. Returns the generated pre- and post spike sequences and the
        resulting weight established by STDP.
        """
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': self.resolution})

        # presynaptic_neuron = nest.Create(self.nest_neuron_model, 1, params=self.neuron_parameters)
        presynaptic_neuron = nest.Create("parrot_neuron", 1)
        postsynaptic_neuron = nest.Create(self.nest_neuron_model, 1, params=self.neuron_parameters)

        wr = nest.Create('weight_recorder')
        nest.CopyModel(self.synapse_model, self.synapse_model + "_rec", {"weight_recorder": wr})

        spike_senders = nest.Create("spike_generator", 2, params=({"spike_times": self.hardcoded_pre_times}, {"spike_times": self.hardcoded_post_times}) )
        pre_spike_generator = spike_senders[0]
        post_spike_generator = spike_senders[1]

        # The recorder is to save the randomly generated spike trains.
        spike_recorder = nest.Create("spike_recorder")

        nest.Connect(pre_spike_generator, presynaptic_neuron, syn_spec={"synapse_model": "static_synapse", "weight": 9999.})
        nest.Connect(post_spike_generator, postsynaptic_neuron, syn_spec={"synapse_model": "static_synapse", "weight": 9999.})
        nest.Connect(presynaptic_neuron + postsynaptic_neuron, spike_recorder, syn_spec={"synapse_model": "static_synapse"})

        nest.SetDefaults(self.synapse_model + "_rec", self.synapse_common_properties)

        # The synapse of interest itself
        self.synapse_parameters["synapse_model"] += "_rec"
        nest.Connect(presynaptic_neuron, postsynaptic_neuron, syn_spec=self.synapse_parameters)
        self.synapse_parameters["synapse_model"] = self.synapse_model

        nest.Simulate(self.simulation_duration)

        all_spikes = nest.GetStatus(spike_recorder, keys='events')[0]
        pre_spikes = all_spikes['times'][all_spikes['senders'] == presynaptic_neuron.tolist()[0]]
        post_spikes = all_spikes['times'][all_spikes['senders'] == postsynaptic_neuron.tolist()[0]]

        t_hist = nest.GetStatus(wr, "events")[0]["times"]
        weight = nest.GetStatus(wr, "events")[0]["weights"]

        return pre_spikes, post_spikes, t_hist, weight

    def reproduce_weight_drift(self, pre_spikes, post_spikes, initial_weight):
        """Independent, self-contained model of STDP with power-law"""

        def facilitate(w, Kpre):
            return w + self.synapse_common_properties["lambda"] * pow(w, self.synapse_common_properties["mu"]) * Kpre

        def depress(w, Kpost):
            new_weight = w - self.synapse_common_properties["alpha"] * self.synapse_common_properties["lambda"] * w * Kpost
            return new_weight if new_weight > 0.0 else 0.0

        def Kpost_at_time(t, spikes, inclusive=True):
            t_curr = 0.
            Kpost = 0.
            for spike_idx, t_sp in enumerate(spikes):
                if t < t_sp:
                    # integrate to t
                    Kpost *= exp(-(t - t_curr) / self.tau_post)
                    return Kpost
                # integrate to t_sp
                Kpost *= exp(-(t_sp - t_curr) / self.tau_post)
                if inclusive:
                    Kpost += 1.
                if t == t_sp:
                    return Kpost
                if not inclusive:
                    Kpost += 1.
                t_curr = t_sp
            # if we get here, t > t_last_spike
            # integrate to t
            Kpost *= exp(-(t - t_curr) / self.tau_post)
            return Kpost

        eps = 1e-6
        t = 0.
        idx_next_pre_spike = 0
        idx_next_post_spike = 0
        t_last_pre_spike = -1
        t_last_post_spike = -1
        Kpre = 0.
        weight = initial_weight

        w_log = []

        post_spikes_delayed = post_spikes + self.dendritic_delay

        while t < self.simulation_duration:
            if idx_next_pre_spike >= pre_spikes.size:
                t_next_pre_spike = -1
            else:
                t_next_pre_spike = pre_spikes[idx_next_pre_spike]

            if idx_next_post_spike >= post_spikes.size:
                t_next_post_spike = -1
            else:
                t_next_post_spike = post_spikes_delayed[idx_next_post_spike]

            if t_next_post_spike == -1:
                a = 1

            if t_next_post_spike >= 0 and (t_next_post_spike + eps < t_next_pre_spike or t_next_pre_spike < 0):
                handle_pre_spike = False
                handle_post_spike = True
                idx_next_post_spike += 1
            elif t_next_pre_spike >= 0 and (t_next_post_spike > t_next_pre_spike + eps or t_next_post_spike < 0):
                handle_pre_spike = True
                handle_post_spike = False
                idx_next_pre_spike += 1
            else:
                # simultaneous spikes (both true) or no more spikes to process (both false)
                handle_pre_spike = t_next_pre_spike >= 0
                handle_post_spike = t_next_post_spike >= 0
                idx_next_pre_spike += 1
                idx_next_post_spike += 1

            # integrate to min(t_next_pre_spike, t_next_post_spike)
            t_next = t
            if handle_pre_spike:
                t_next = max(t, t_next_pre_spike)
            if handle_post_spike:
                t_next = max(t, t_next_post_spike)

            if t_next == t:
                # no more spikes to process
                t_next = self.simulation_duration

            h = t_next - t
            Kpre *= exp(-h / self.tau_pre)
            t = t_next

            if handle_post_spike:
                if not handle_pre_spike or abs(t_next_post_spike - t_last_post_spike) > eps:
                    if abs(t_next_post_spike - t_last_pre_spike) > eps:
                        weight = facilitate(weight, Kpre)

            if handle_pre_spike:
                Kpre += 1.
                if not handle_post_spike or abs(t_next_pre_spike - t_last_pre_spike) > eps:
                    if abs(t_next_pre_spike - t_last_post_spike) > eps:
                        _Kpost = Kpost_at_time(t - self.dendritic_delay, post_spikes, inclusive=False)
                        weight = depress(weight, _Kpost)
                t_last_pre_spike = t_next_pre_spike
                w_log.append(weight)

            if handle_post_spike:
                t_last_post_spike = t_next_post_spike

        return w_log


if __name__ == "__main__":
    TestSTDPPlSynapse().do_nest_simulation_and_compare_to_reproduced_weight()
