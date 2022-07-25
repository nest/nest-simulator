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

    def __init__(self):
        self.resolution = 0.1  # [ms]
        self.simulation_duration = 1E2  # [ms]
        self.synapse_model = "stdp_pl_synapse_hom_ax_delay"
        self.nest_neuron_model = "iaf_psc_exp"
        self.tau_pre = 20.0
        self.tau_post = 33.7
        self.init_weight = .5
        self.dendritic_delay = 0.1
        self.axonal_delay = 0.
        self.synapse_parameters = {
            "synapse_model": self.synapse_model,
            "receptor_type": 0,
            "delay": self.dendritic_delay + self.axonal_delay,
            # "axonal_delay": self.axonal_delay,
            # STDP constants
            "lambda": 0.1,
            "alpha": 1.0,
            "mu": 0.4,
            "tau_plus": self.tau_pre,
            "weight": self.init_weight
        }
        self.neuron_parameters = {
            "tau_minus": self.tau_post,
            "t_ref": 1.0
        }

        # While the random sequences, fairly long, would supposedly reveal small differences in the weight change
        # between NEST and ours, some low-probability events (say, coinciding spikes) can well not have occurred. To
        # generate and test every possible combination of pre/post order, we append some hardcoded spike sequences:
        # pre: 1       5 6 7   9    11 12 13    15    17
        # post:  2 3 4       8 9 10    12    14    16    18
        self.hardcoded_pre_times = np.array([2, 5, 6, 7, 9, 11, 12, 13, 15, 17, 18, 19], dtype=float)
        self.hardcoded_post_times = np.array([3, 4, 8, 9, 10, 12, 14, 16, 18, 19], dtype=float)

    def do_nest_simulation_and_compare_to_reproduced_weight(self):
        pre_spikes, post_spikes, t_weight_by_nest, weight_by_nest, df_nest = self.do_the_nest_simulation()

        df_repr = self.reproduce_weight_drift(pre_spikes, post_spikes, self.init_weight)

        with pd.option_context('display.max_rows', None, 'display.max_columns', None, 'display.expand_frame_repr', False, 'display.float_format', lambda x: '%.10f' % x):
            if len(df_nest.index) > 0:
                df = pd.concat([df_nest, df_repr], axis=1)
                df = df[["type", "time", "time r", "K nest", "K repr", "w nest", "w repr"]]
                df["w diff"] = np.abs(df["w nest"] - df["w repr"])
                print(df)

    def do_the_nest_simulation(self):
        """
        This function is where calls to NEST reside. Returns the generated pre- and post spike sequences and the
        resulting weight established by STDP.
        """
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': self.resolution})

        presynaptic_neuron = nest.Create(self.nest_neuron_model, 1, params=self.neuron_parameters)
        # presynaptic_neuron = nest.Create("parrot_neuron", 1)
        postsynaptic_neuron = nest.Create(self.nest_neuron_model, 1, params=self.neuron_parameters)

        wr = nest.Create('weight_recorder')
        nest.CopyModel(self.synapse_model, self.synapse_model + "_rec", {"weight_recorder": wr})

        spike_senders = nest.Create("spike_generator", 2, params=({"spike_times": self.hardcoded_pre_times - 1}, {"spike_times": self.hardcoded_post_times - 2}) )
        pre_spike_generator = spike_senders[0]
        post_spike_generator = spike_senders[1]

        # The recorder is to save the randomly generated spike trains.
        spike_recorder = nest.Create("spike_recorder")

        nest.Connect(pre_spike_generator, presynaptic_neuron, syn_spec={"synapse_model": "static_synapse", "weight": 9999.})
        nest.Connect(post_spike_generator, postsynaptic_neuron, syn_spec={"synapse_model": "static_synapse", "weight": 9999.})
        nest.Connect(presynaptic_neuron + postsynaptic_neuron, spike_recorder, syn_spec={"synapse_model": "static_synapse"})
        # The synapse of interest itself
        self.synapse_parameters["synapse_model"] += "_rec"
        nest.Connect(presynaptic_neuron, postsynaptic_neuron, syn_spec=self.synapse_parameters)
        self.synapse_parameters["synapse_model"] = self.synapse_model

        t, stdout_fileno, stdout_save, stdout_pipe = start_capture_stdout()
        nest.Simulate(self.simulation_duration)
        end_capture_stdout(t, stdout_fileno, stdout_save, stdout_pipe)
        # print(captured_stdout)
        # exit()
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

        def facilitate(w, Kpre):
            return w + self.synapse_parameters["lambda"] * pow(w, self.synapse_parameters["mu"]) * Kpre

        def depress(w, Kpost):
            new_weight = w - self.synapse_parameters["alpha"] * self.synapse_parameters["lambda"] * w * Kpost
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

        t_log = []
        w_log = []
        K_log = []

        # logging
        # t_log.append(t)
        # w_log.append(weight)
        # Kpre_log.append(Kpre)

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

            if t_next_post_spike >= 0 and t_next_post_spike + eps < t_next_pre_spike or t_next_pre_spike < 0:
                handle_pre_spike = False
                handle_post_spike = True
                idx_next_post_spike += 1
            elif t_next_pre_spike >= 0 and t_next_post_spike > t_next_pre_spike + eps or t_next_post_spike < 0:
                handle_pre_spike = True
                handle_post_spike = False
                idx_next_pre_spike += 1
            else:
                # simultaneous spikes (both true) or no more spikes to process (both false)
                handle_pre_spike = idx_next_pre_spike >= 0
                handle_post_spike = idx_next_post_spike >= 0
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
                        K_log.append(Kpre)
                        t_log.append(t_next_post_spike)
                        w_log.append(weight)

            if handle_pre_spike:
                Kpre += 1.
                if not handle_post_spike or abs(t_next_pre_spike - t_last_pre_spike) > eps:
                    if abs(t_next_pre_spike - t_last_post_spike) > eps:
                        _Kpost = Kpost_at_time(t - self.dendritic_delay, post_spikes, inclusive=False)
                        weight = depress(weight, _Kpost)
                        K_log.append(_Kpost)
                        t_log.append(t_next_pre_spike)
                        w_log.append(weight)
                t_last_pre_spike = t_next_pre_spike

            if handle_post_spike:
                t_last_post_spike = t_next_post_spike

        df_repr = pd.DataFrame({
            "time r": np.sort(np.array(t_log)),
            "K repr": K_log,
            "w repr": w_log
        })
        return df_repr

    def test_stdp_synapse(self):
        self.do_nest_simulation_and_compare_to_reproduced_weight()

    def test_stdp_synapse2(self):
        self.dendritic_delay = self.axonal_delay = float('nan')
        self.init_params()
        for self.dendritic_delay in [1., self.resolution]:
            for self.axonal_delay in [0.]:
                self.init_params()
                for self.nest_neuron_model in ["iaf_psc_exp", "iaf_cond_exp"]:
                    for self.neuron_parameters["t_ref"] in [.1, .5, 1., 1.5, 2., 2.5]:
                        print(self.dendritic_delay, self.axonal_delay, self.neuron_parameters["t_ref"],
                              self.nest_neuron_model)
                        self.do_nest_simulation_and_compare_to_reproduced_weight()


captured_stdout = ''
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
