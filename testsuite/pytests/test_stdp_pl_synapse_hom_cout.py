# -*- coding: utf-8 -*-
#
# test_stdp_synapse.py
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

import nest
from math import exp
import numpy as np

import pandas as pd
import os
import io
import sys
import threading


class TestSTDPPlSynapse:
    """
    Compare the STDP power-law synaptic plasticity model against a self-contained Python reference.

    Random pre and post spike times are generated according to a Poisson distribution; some hard-coded spike times are
    added to make sure to test for edge cases such as simultaneous pre and post spike.
    """

    def init_params(self):
        self.eps = 1e-6
        self.resolution = 0.1  # [ms]
        self.simulation_duration = 1E3  # [ms]
        self.synapse_model = "stdp_pl_synapse_hom_ax_delay"
        self.presynaptic_firing_rate = 200.  # [ms^-1]
        self.postsynaptic_firing_rate = 200.  # [ms^-1]
        self.tau_pre = 20.0
        self.tau_post = 33.7
        self.init_weight = .5
        self.dendritic_delay = self.delay - self.axonal_delay
        self.synapse_common_properties = {
            "axonal_delay": self.axonal_delay,
            "lambda": 0.1,
            "alpha": 1.0,
            "mu": 0.4,
            "tau_plus": self.tau_pre,
        }
        self.synapse_parameters = {
            "synapse_model": self.synapse_model,
            "receptor_type": 0,
            "delay": self.delay,
            "weight": self.init_weight
        }
        self.neuron_parameters = {
            "tau_minus": self.tau_post
        }

        # While the random sequences, fairly long, would supposedly reveal small differences in the weight change
        # between NEST and ours, some low-probability events (say, coinciding spikes) can well not have occurred. To
        # generate and test every possible combination of pre/post order, we append some hardcoded spike sequences:
        # pre: 1       5 6 7   9    11 12 13    15    17 18 19   20.1
        # post:  2 3 4       8 9 10    12    14    16    18 19.1 20
        self.hardcoded_pre_times = np.array([1, 5, 6, 7, 9, 11, 12, 13, 15, 17, 18, 19, 20.1], dtype=float)
        self.hardcoded_post_times = np.array([2, 3, 4, 8, 9, 10, 12, 14, 16, 18, 19.1, 20], dtype=float)
        self.hardcoded_trains_length = 5. + max(np.amax(self.hardcoded_pre_times), np.amax(self.hardcoded_post_times))

    def do_nest_simulation_and_compare_to_reproduced_weight(self):
        pre_spikes, post_spikes, t_weight_by_nest, weight_by_nest, df_nest = self.do_the_nest_simulation()

        df_repr = self.reproduce_weight_drift(pre_spikes, post_spikes, self.init_weight)

        with pd.option_context('display.max_rows', None, 'display.max_columns', None, 'display.expand_frame_repr',
                               False, 'display.float_format', lambda x: '%.10f' % x):
            if len(df_nest.index) > 0:
                df = pd.concat([df_nest, df_repr], axis=1)
                df = df[["type", "time", "time r", "K nest", "K repr", "w nest", "w repr"]]
                df["w diff"] = np.abs(df["w nest"] - df["w repr"])
                if self.axonal_delay > self.dendritic_delay:
                    df = df.loc[df["type"] == "Post"]
                if np.nanmax(df["w diff"].to_numpy()) > self.eps:
                    print(df)
                np.testing.assert_allclose(df["w diff"], 0, atol=self.eps, rtol=0)

    def do_the_nest_simulation(self):
        """
        This function is where calls to NEST reside. Returns the generated pre- and post spike sequences and the
        resulting weight established by STDP.
        """
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': self.resolution})

        presynaptic_neuron, postsynaptic_neuron = nest.Create(self.nest_neuron_model, 2, params=self.neuron_parameters)

        generators = nest.Create(
            "poisson_generator",
            2,
            params=({"rate": self.presynaptic_firing_rate,
                     "stop": (self.simulation_duration - self.hardcoded_trains_length)},
                    {"rate": self.postsynaptic_firing_rate,
                     "stop": (self.simulation_duration - self.hardcoded_trains_length)}))
        presynaptic_generator = generators[0]
        postsynaptic_generator = generators[1]

        wr = nest.Create('weight_recorder')
        nest.CopyModel(self.synapse_model, self.synapse_model + "_rec", {"weight_recorder": wr})

        spike_senders = nest.Create(
            "spike_generator",
            2,
            params=({"spike_times": self.hardcoded_pre_times
                                    + self.simulation_duration - self.hardcoded_trains_length},
                    {"spike_times": self.hardcoded_post_times
                                    + self.simulation_duration - self.hardcoded_trains_length})
        )
        pre_spike_generator = spike_senders[0]
        post_spike_generator = spike_senders[1]

        # The recorder is to save the randomly generated spike trains.
        spike_recorder = nest.Create("spike_recorder")

        nest.Connect(presynaptic_generator + pre_spike_generator, presynaptic_neuron,
                     syn_spec={"synapse_model": "static_synapse", "weight": 1000.})
        nest.Connect(postsynaptic_generator + post_spike_generator, postsynaptic_neuron,
                     syn_spec={"synapse_model": "static_synapse", "weight": 1000.})
        nest.Connect(presynaptic_neuron + postsynaptic_neuron, spike_recorder,
                     syn_spec={"synapse_model": "static_synapse"})

        # The synapse of interest itself
        nest.SetDefaults(self.synapse_model + "_rec", self.synapse_common_properties)
        self.synapse_parameters["synapse_model"] += "_rec"
        nest.Connect(presynaptic_neuron, postsynaptic_neuron, syn_spec=self.synapse_parameters)
        self.synapse_parameters["synapse_model"] = self.synapse_model

        t, stdout_fileno, stdout_save, stdout_pipe = start_capture_stdout()
        nest.Simulate(self.simulation_duration)
        # debug_df = pd.DataFrame({})
        end_capture_stdout(t, stdout_fileno, stdout_save, stdout_pipe)
        debug_df = pd.read_csv(io.StringIO(captured_stdout), sep=" ", names=["type", "time", "K nest", "w nest"],
                              dtype={"type": str, "time": float, "trace": float, "weight": float})
        # debug_df.drop_duplicates(inplace=True, ignore_index=True)
        all_spikes = nest.GetStatus(spike_recorder, keys='events')[0]
        pre_spikes = all_spikes['times'][all_spikes['senders'] == presynaptic_neuron.tolist()[0]]
        post_spikes = all_spikes['times'][all_spikes['senders'] == postsynaptic_neuron.tolist()[0]]

        t_hist = nest.GetStatus(wr, "events")[0]["times"]
        weight = nest.GetStatus(wr, "events")[0]["weights"]
        return pre_spikes, post_spikes, t_hist, weight, debug_df

    def reproduce_weight_drift(self, pre_spikes, post_spikes, initial_weight):
        """Independent, self-contained model of STDP with power-law"""

        def facilitate(w, Kplus):
            return w + self.synapse_common_properties["lambda"] * pow(w, self.synapse_common_properties["mu"]) * Kplus

        def depress(w, Kminus):
            new_weight = w - self.synapse_common_properties["alpha"] * self.synapse_common_properties[
                "lambda"] * w * Kminus
            return new_weight if new_weight > 0.0 else 0.0

        def Kminus_at_time(t, spikes, inclusive=True):
            t_curr = 0.
            Kminus = 0.
            if t == 985.9:
                a = 2
            for spike_idx, t_sp in enumerate(spikes):
                if t < t_sp:
                    # integrate to t
                    Kminus *= exp(-(t - t_curr) / self.tau_post)
                    return Kminus
                # integrate to t_sp
                Kminus *= exp(-(t_sp - t_curr) / self.tau_post)
                if inclusive:
                    Kminus += 1.
                if t == t_sp:
                    return Kminus
                if not inclusive:
                    Kminus += 1.
                t_curr = t_sp
            # if we get here, t > t_last_spike
            # integrate to t
            Kminus *= exp(-(t - t_curr) / self.tau_post)
            return Kminus

        t = 0.
        idx_next_pre_spike = 0
        idx_next_post_spike = 0
        t_last_pre_spike = -1
        t_last_post_spike = -1
        Kplus = 0.
        weight = initial_weight

        t_log = []
        w_log = []
        K_log = []

        pre_spikes_delayed = pre_spikes + self.axonal_delay
        post_spikes_delayed = post_spikes + self.dendritic_delay
        # Make sure only spikes that were relevant for simulation are actually considered in the test
        # For pre-spikes that will be all spikes with: t_pre < sim_duration
        pre_spikes_delayed = pre_spikes_delayed[pre_spikes + self.eps < self.simulation_duration]
        # For post-spikes that will be all spikes with: t_post + d_dend < latest_pre_spike + d_axon
        post_spikes_delayed = post_spikes_delayed[post_spikes_delayed < pre_spikes_delayed[-1] + self.eps]

        while t < self.simulation_duration:
            if idx_next_pre_spike >= pre_spikes_delayed.size:
                t_next_pre_spike = -1
            else:
                t_next_pre_spike = pre_spikes_delayed[idx_next_pre_spike]

            if idx_next_post_spike >= post_spikes_delayed.size:
                t_next_post_spike = -1
            else:
                t_next_post_spike = post_spikes_delayed[idx_next_post_spike]

            if t_next_post_spike >= 0 and (t_next_post_spike + self.eps < t_next_pre_spike or t_next_pre_spike < 0):
                handle_pre_spike = False
                handle_post_spike = True
                idx_next_post_spike += 1
            elif t_next_pre_spike >= 0 and (t_next_post_spike > t_next_pre_spike + self.eps or t_next_post_spike < 0):
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
            t = t_next

            if handle_post_spike:
                if not handle_pre_spike or abs(t_next_post_spike - t_last_post_spike) > self.eps:
                    if abs(t_next_post_spike - t_last_pre_spike) > self.eps:
                        weight = facilitate(weight, Kplus)
                        K_log.append(Kplus)
                        t_log.append(t_next_post_spike)
                        w_log.append(weight)

            if handle_pre_spike:
                if not handle_post_spike or abs(t_next_pre_spike - t_last_pre_spike) > self.eps:
                    if abs(t_next_pre_spike - t_last_post_spike) > self.eps:
                        _Kminus = Kminus_at_time(t - self.dendritic_delay, post_spikes, inclusive=False)
                        weight = depress(weight, _Kminus)
                        K_log.append(_Kminus)
                        t_log.append(t_next_pre_spike)
                        w_log.append(weight)
                t_last_pre_spike = t_next_pre_spike
                Kplus += 1.

            if handle_post_spike:
                t_last_post_spike = t_next_post_spike

        df_repr = pd.DataFrame({
            "time r": t_log,
            "K repr": K_log,
            "w repr": w_log
        })
        return df_repr

    def test_stdp_synapse_single(self):
        self.delay = 1
        self.axonal_delay = 1
        self.init_params()
        self.nest_neuron_model = "iaf_psc_alpha_ax_delay"
        self.neuron_parameters["t_ref"] = .1
        self.do_nest_simulation_and_compare_to_reproduced_weight()

    def test_stdp_synapse(self):
        self.delay = self.axonal_delay = float('nan')
        self.init_params()
        for self.delay, self.axonal_delay in [(1., 0.), (1., .5), (1., 1.), (self.resolution, 0.),
                                              (self.resolution, self.resolution)]:
            self.init_params()
            for self.nest_neuron_model in ["iaf_psc_alpha_ax_delay"]:  # ["iaf_psc_exp", "iaf_cond_exp"]:
                for self.neuron_parameters["t_ref"] in [.1, .5, 1., 1.5, 2., 2.5]:
                    print(self.dendritic_delay, self.axonal_delay, self.neuron_parameters["t_ref"],
                          self.nest_neuron_model)
                    self.do_nest_simulation_and_compare_to_reproduced_weight()
                    global captured_stdout
                    captured_stdout = ""


captured_stdout = ""


def start_capture_stdout():
    stdout_fileno = sys.stdout.fileno()
    stdout_save = os.dup(stdout_fileno)
    stdout_pipe = os.pipe()
    os.dup2(stdout_pipe[1], stdout_fileno)
    os.close(stdout_pipe[1])

    def drain_pipe():
        global captured_stdout
        while True:
            data = os.read(stdout_pipe[0], 1024)
            if not data:
                break
            captured_stdout += data.decode("utf-8")

    t = threading.Thread(target=drain_pipe)
    t.start()
    return t, stdout_fileno, stdout_save, stdout_pipe


def end_capture_stdout(t, stdout_fileno, stdout_save, stdout_pipe):
    os.close(stdout_fileno)
    t.join()

    # Clean up the pipe and restore the original stdout
    os.close(stdout_pipe[0])
    os.dup2(stdout_save, stdout_fileno)
    os.close(stdout_save)


if __name__ == "__main__":
    TestSTDPPlSynapse().test_stdp_synapse()
