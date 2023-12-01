# -*- coding: utf-8 -*-
#
# test_stdp_pl_synapse_hom.py
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

from math import exp

import nest
import numpy as np
import pytest

DEBUG_PLOTS = False

if DEBUG_PLOTS:
    try:
        import matplotlib as mpl  # noqa: F401
        import matplotlib.pyplot as plt

        DEBUG_PLOTS = True
    except Exception:
        DEBUG_PLOTS = False


@nest.ll_api.check_stack
class TestSTDPPlSynapse:
    """
    Compare the STDP power-law synaptic plasticity model against a self-contained Python reference.

    Random pre and post spike times are generated according to a Poisson distribution; some hard-coded spike times are
    added to make sure to test for edge cases such as simultaneous pre and post spike.
    """

    def init_params(self):
        self.resolution = 0.1  # [ms]
        self.simulation_duration = 1e3  # [ms]
        self.synapse_model = "stdp_pl_synapse_hom"
        self.presynaptic_firing_rate = 100.0  # [ms^-1]
        self.postsynaptic_firing_rate = 100.0  # [ms^-1]
        self.tau_pre = 20.0
        self.tau_post = 33.7
        self.init_weight = 0.5
        self.synapse_common_properties = {
            "lambda": 0.1,
            "alpha": 1.0,
            "mu": 0.4,
            "tau_plus": self.tau_pre,
        }
        self.synapse_parameters = {"synapse_model": self.synapse_model, "receptor_type": 0, "weight": self.init_weight}
        self.neuron_parameters = {"tau_minus": self.tau_post}

        # While the random sequences, fairly long, would supposedly reveal small differences in the weight change
        # between NEST and ours, some low-probability events (say, coinciding spikes) can well not have occurred. To
        # generate and test every possible combination of pre/post order, we append some hardcoded spike sequences:
        # pre: 1       5 6 7   9    11 12 13    15    17 18 19   20.1
        # post:  2 3 4       8 9 10    12    14    16    18 19.1 20
        self.hardcoded_pre_times = np.array(
            [1, 5, 6, 7, 9, 11, 12, 13, 14.5, 16.1, 21, 25, 26, 27, 29, 31, 32, 33, 34.5, 36.1], dtype=float
        )
        self.hardcoded_post_times = np.array(
            [2, 3, 4, 8, 9, 10, 12, 12.2, 14.1, 15.4, 22, 23, 24, 28, 29, 30, 32, 33.2, 35.1, 36.4], dtype=float
        )
        self.hardcoded_trains_length = 5.0 + max(np.amax(self.hardcoded_pre_times), np.amax(self.hardcoded_post_times))

    def do_nest_simulation_and_compare_to_reproduced_weight(self, fname_snip):
        pre_spikes, post_spikes, t_weight_by_nest, weight_by_nest = self.do_the_nest_simulation()

        (
            t_weight_reproduced_independently,
            weight_reproduced_independently,
            Kpre_log,
            Kpost_log,
        ) = self.reproduce_weight_drift(pre_spikes, post_spikes, self.init_weight, fname_snip=fname_snip)

        # ``weight_by_nest`` contains only weight values at pre spike times, ``weight_reproduced_independently``
        # contains the weight at pre *and* post times: check that weights are equal only for pre spike times
        assert len(weight_by_nest) > 0

        difference_matrix = t_weight_by_nest[t_weight_by_nest < self.simulation_duration].reshape(
            1, -1
        ) - t_weight_reproduced_independently.reshape(-1, 1)
        pre_spike_reproduced_indices = np.abs(difference_matrix).argmin(axis=0)
        time_differences = np.diagonal(difference_matrix[pre_spike_reproduced_indices])
        # make sure all spike times are equal
        np.testing.assert_allclose(time_differences, 0, atol=1e-07)
        # make sure the weights after the pre_spikes times are equal
        np.testing.assert_allclose(
            weight_by_nest[t_weight_by_nest < self.simulation_duration],
            weight_reproduced_independently[pre_spike_reproduced_indices],
        )

        if DEBUG_PLOTS:
            self.plot_weight_evolution(
                pre_spikes,
                post_spikes,
                t_weight_by_nest,
                weight_by_nest,
                fname_snip=fname_snip,
                title_snip=self.nest_neuron_model + " (NEST)",
            )

            self.plot_weight_evolution(
                pre_spikes,
                post_spikes,
                t_weight_reproduced_independently,
                weight_reproduced_independently,
                Kpre_log,
                Kpost_log,
                pre_indices=pre_spike_reproduced_indices,
                fname_snip=fname_snip + "_ref",
                title_snip="Reference",
            )

    def do_the_nest_simulation(self):
        """
        This function is where calls to NEST reside. Returns the generated pre- and post spike sequences and the
        resulting weight established by STDP.
        """
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus(
            {
                "resolution": self.resolution,
                "min_delay": min(self.min_delay, self.dendritic_delay),
                "max_delay": max(self.max_delay, self.dendritic_delay),
            }
        )

        presynaptic_neuron, postsynaptic_neuron = nest.Create(self.nest_neuron_model, 2, params=self.neuron_parameters)

        generators = nest.Create(
            "poisson_generator",
            2,
            params=(
                {
                    "rate": self.presynaptic_firing_rate,
                    "stop": (self.simulation_duration - self.hardcoded_trains_length),
                },
                {
                    "rate": self.postsynaptic_firing_rate,
                    "stop": (self.simulation_duration - self.hardcoded_trains_length),
                },
            ),
        )
        presynaptic_generator = generators[0]
        postsynaptic_generator = generators[1]

        wr = nest.Create("weight_recorder")
        nest.CopyModel(self.synapse_model, self.synapse_model + "_rec", {"weight_recorder": wr})

        spike_senders = nest.Create(
            "spike_generator",
            2,
            params=(
                {"spike_times": self.hardcoded_pre_times + self.simulation_duration - self.hardcoded_trains_length},
                {"spike_times": self.hardcoded_post_times + self.simulation_duration - self.hardcoded_trains_length},
            ),
        )
        pre_spike_generator = spike_senders[0]
        post_spike_generator = spike_senders[1]

        # The recorder is to save the randomly generated spike trains.
        spike_recorder = nest.Create("spike_recorder")

        nest.Connect(
            presynaptic_generator + pre_spike_generator,
            presynaptic_neuron,
            syn_spec={"synapse_model": "static_synapse", "weight": 3000.0},
        )
        nest.Connect(
            postsynaptic_generator + post_spike_generator,
            postsynaptic_neuron,
            syn_spec={"synapse_model": "static_synapse", "weight": 3000.0},
        )
        nest.Connect(
            presynaptic_neuron + postsynaptic_neuron, spike_recorder, syn_spec={"synapse_model": "static_synapse"}
        )

        # The synapse of interest itself
        nest.SetDefaults(self.synapse_model + "_rec", self.synapse_common_properties)
        self.synapse_parameters["synapse_model"] += "_rec"
        nest.Connect(presynaptic_neuron, postsynaptic_neuron, syn_spec=self.synapse_parameters)
        self.synapse_parameters["synapse_model"] = self.synapse_model

        nest.Simulate(self.simulation_duration)

        all_spikes = nest.GetStatus(spike_recorder, keys="events")[0]
        pre_spikes = all_spikes["times"][all_spikes["senders"] == presynaptic_neuron.tolist()[0]]
        post_spikes = all_spikes["times"][all_spikes["senders"] == postsynaptic_neuron.tolist()[0]]

        t_hist = nest.GetStatus(wr, "events")[0]["times"]
        weight = nest.GetStatus(wr, "events")[0]["weights"]

        return pre_spikes, post_spikes, t_hist, weight

    def reproduce_weight_drift(self, pre_spikes, post_spikes, initial_weight, fname_snip=""):
        """Independent, self-contained model of STDP with power-law"""

        def facilitate(w, Kplus):
            return w + self.synapse_common_properties["lambda"] * pow(w, self.synapse_common_properties["mu"]) * Kplus

        def depress(w, Kminus):
            new_weight = (
                w - self.synapse_common_properties["alpha"] * self.synapse_common_properties["lambda"] * w * Kminus
            )
            return new_weight if new_weight > 0.0 else 0.0

        eps = 1e-6
        t = 0.0
        idx_next_pre_spike = 0
        idx_next_post_spike = 0
        t_last_pre_spike = -1
        t_last_post_spike = -1
        Kplus = 0.0
        Kminus = 0.0
        weight = initial_weight

        t_log = []
        w_log = []
        Kplus_log = []
        Kminus_log = []

        # Make sure only spikes that were relevant for simulation are actually considered in the test
        # For pre-spikes that will be all spikes with: t_pre < sim_duration
        pre_spikes = pre_spikes[pre_spikes + eps < self.simulation_duration]
        # For post-spikes that will be all spikes with: t_post + d_dend <= latest_pre_spike
        post_spikes = post_spikes[post_spikes + self.dendritic_delay <= pre_spikes[-1] + eps]
        post_spikes_delayed = post_spikes + self.dendritic_delay

        while idx_next_pre_spike < len(pre_spikes) or idx_next_post_spike < len(post_spikes_delayed):
            if idx_next_pre_spike >= pre_spikes.size:
                t_next_pre_spike = -1
            else:
                t_next_pre_spike = pre_spikes[idx_next_pre_spike]

            if idx_next_post_spike >= post_spikes_delayed.size:
                t_next_post_spike = -1
            else:
                t_next_post_spike = post_spikes_delayed[idx_next_post_spike]

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
            Kplus *= exp(-h / self.tau_pre)
            Kminus *= exp(-h / self.tau_post)
            t = t_next

            if handle_post_spike:
                if not handle_pre_spike or abs(t_next_post_spike - t_last_post_spike) > eps:
                    if abs(t_next_post_spike - t_last_pre_spike) > eps:
                        weight = facilitate(weight, Kplus)

            if handle_pre_spike:
                if not handle_post_spike or abs(t_next_pre_spike - t_last_pre_spike) > eps:
                    if abs(t_next_pre_spike - t_last_post_spike) > eps:
                        weight = depress(weight, Kminus)
                t_last_pre_spike = t_next_pre_spike
                Kplus += 1.0

            if handle_post_spike:
                t_last_post_spike = t_next_post_spike
                Kminus += 1.0

            # logging
            t_log.append(t)
            w_log.append(weight)
            Kplus_log.append(Kplus)
            Kminus_log.append(Kminus)

        if DEBUG_PLOTS:
            self.plot_weight_evolution(
                pre_spikes,
                post_spikes,
                t_log,
                w_log,
                Kplus_log,
                Kminus_log,
                fname_snip=fname_snip + "_ref",
                title_snip="Reference",
            )

        return np.array(t_log), np.array(w_log), Kplus_log, Kminus_log

    def plot_weight_evolution(
        self,
        pre_spikes,
        post_spikes,
        t_log,
        w_log,
        Kpre_log=None,
        Kpost_log=None,
        pre_indices=slice(-1),
        fname_snip="",
        title_snip="",
    ):
        if not DEBUG_PLOTS:  # make pylint happy if no matplotlib
            return

        # pylint: disable=E0601
        fig, ax = plt.subplots(nrows=3)

        n_spikes = len(pre_spikes)
        for i in range(n_spikes):
            ax[0].plot(2 * [pre_spikes[i]], [0, 1], linewidth=2, color="blue", alpha=0.4)
        ax[0].set_ylabel("Pre spikes")
        ax0_ = ax[0].twinx()
        if Kpre_log:
            ax0_.plot(t_log, Kpre_log)

        n_spikes = len(post_spikes)
        for i in range(n_spikes):
            ax[1].plot(2 * [post_spikes[i]], [0, 1], linewidth=2, color="red", alpha=0.4)
        ax1_ = ax[1].twinx()
        ax[1].set_ylabel("Post spikes")
        if Kpost_log:
            ax1_.plot(t_log, Kpost_log)

        ax[2].plot(t_log[pre_indices], w_log[pre_indices], marker="o", label="nestml")
        ax[2].set_ylabel("w")

        ax[2].set_xlabel("Time [ms]")
        for _ax in ax:
            _ax.grid(which="major", axis="both")
            _ax.grid(which="minor", axis="x", linestyle=":", alpha=0.4)
            _ax.minorticks_on()
            _ax.set_xlim(0.0, self.simulation_duration)

        fig.suptitle(title_snip)
        fig.savefig("./tmp/nest_stdp_pl_synapse_hom_test" + fname_snip + ".png", dpi=300)
        plt.close(fig)

    def test_stdp_synapse(self):
        self.init_params()
        for self.dendritic_delay in (1.0, 0.5, self.resolution):
            self.synapse_parameters["delay"] = self.dendritic_delay
            for self.min_delay in (1.0, 0.4, self.resolution):
                for self.max_delay in (3.0, 1.0):
                    self.min_delay = min(self.min_delay, self.max_delay)
                    self.max_delay = max(self.min_delay, self.max_delay)
                    for self.nest_neuron_model in ("iaf_psc_exp", "iaf_cond_exp"):
                        for self.neuron_parameters["t_ref"] in (self.resolution, 0.5, 1.0, 1.1, 2.5):
                            fname_snip = "_[nest_neuron_mdl=" + self.nest_neuron_model + "]"
                            fname_snip += "_[dend_delay=" + str(self.dendritic_delay) + "]"
                            fname_snip += "_[t_ref=" + str(self.neuron_parameters["t_ref"]) + "]"
                            self.do_nest_simulation_and_compare_to_reproduced_weight(fname_snip=fname_snip)
