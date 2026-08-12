# -*- coding: utf-8 -*-
#
# test_stdp_dopamine_synapse.py
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
import unittest
from math import exp
import numpy as np

try:
    import matplotlib as mpl
    import matplotlib.pyplot as plt

    DEBUG_PLOTS = True
except Exception:
    DEBUG_PLOTS = False


@nest.ll_api.check_stack
class STDPSynapseTest(unittest.TestCase):
    """
    Compare the STDP dopamine synaptic plasticity model against a self-contained Python reference.

    Random pre, post, dopa spike times are generated according to a Poisson distribution; some hard-coded spike times
    are added to make sure to test for edge cases such as simultaneous pre- and post spike.
    """

    def init_params(self):
        self.resolution = 0.1  # [ms]
        self.sim_dura = 100.  # 1E3    # [ms]
        self.synapse_model = "stdp_dopamine_synapse"
        self.presynaptic_firing_rate = 20.  # [ms^-1]
        self.postsynaptic_firing_rate = 20.  # [ms^-1]
        self.dopa_rate = 20.
        self.tau_pre = 20.
        self.tau_post = 33.7
        self.init_weight = 35.
        self.init_c = 0.
        self.init_n = 0.
        self.J = 3000.  # 9999.
        self.synapse_parameters = {
            "receptor_type": 0,
            "delay": self.dendritic_delay,
            # STDP dopamine parameters
            "A_plus": 1.,
            "A_minus": 1.5,
            "tau_plus": self.tau_pre,
            "tau_c": 1000.,
            "tau_n": 200.,
            "b": 0.,
            "Wmin": 0.,
            "Wmax": 200.,
            "weight": self.init_weight,
            "c": self.init_c,
            "n": self.init_n
        }
        self.neuron_parameters = {
            "tau_minus": self.tau_post,
        }

        # While the random sequences, fairly long, would supposedly
        # reveal small differences in the weight change between NEST
        # and ours, some low-probability events (say, coinciding
        # spikes) can well not have occured. To generate and
        # test every possible combination of pre/post precedence, we
        # append some hardcoded spike sequences:
        # pre: 1       5 6 7   9    11 12 13
        # post:  2 3 4       8 9 10    12
        # self.hardcoded_pre_times = np.array([1, 5], dtype=float)
        self.hardcoded_pre_times = np.array([1, 5], dtype=float)
        # self.hardcoded_pre_times = np.array([1, 5, 6, 7, 9, 11, 12, 13], dtype=float)
        self.hardcoded_post_times = np.array([2, 3], dtype=float)
        # self.hardcoded_post_times = np.array([2, 3, 4, 8, 9, 10, 12], dtype=float)
        self.hardcoded_trains_length = 2. + max(np.amax(self.hardcoded_pre_times), np.amax(self.hardcoded_post_times))

    def do_nest_simulation_and_compare_to_reproduced_weight(self, fname_snip):
        pre_spikes, post_spikes, dopa_spikes, t_weight_by_nest, weight_by_nest = self.do_the_nest_simulation()
        if DEBUG_PLOTS:
            self.plot_weight_evolution(pre_spikes, post_spikes, dopa_spikes,
                                       t_weight_by_nest,
                                       weight_by_nest,
                                       fname_snip=fname_snip,
                                       title_snip="NEST")

        t_weight_reproduced_independently, weight_reproduced_independently = self.reproduce_weight_drift(
            pre_spikes, post_spikes, dopa_spikes,
            fname_snip=fname_snip)

        # ``weight_by_nest`` containts only weight values at pre spike times, ``weight_reproduced_independently``
        # contains the weight at pre *and* post times: check that weights are equal only for pre spike times
        assert len(weight_by_nest) > 0
        for idx_pre_spike_nest, t_pre_spike_nest in enumerate(t_weight_by_nest):
            idx_pre_spike_reproduced_independently = \
                np.argmin((t_pre_spike_nest - t_weight_reproduced_independently) ** 2)
            np.testing.assert_allclose(t_pre_spike_nest,
                                       t_weight_reproduced_independently[idx_pre_spike_reproduced_independently])
            np.testing.assert_allclose(weight_by_nest[idx_pre_spike_nest],
                                       weight_reproduced_independently[idx_pre_spike_reproduced_independently])

    def do_the_nest_simulation(self):
        """
        This function is where calls to NEST reside. Returns the generated pre-, post spike, dopa spikes
        sequences, and the resulting weight established by STDP dopamine.
        """
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': self.resolution})

        # create pre and postsynaptic neurons
        presynaptic_neuron, postsynaptic_neuron = nest.Create(
            self.nest_neuron_model,
            2,
            params=self.neuron_parameters)

        # create poisson generators for the pre and postsynaptic neurons
        generators = nest.Create(
            "poisson_generator",
            2,
            params=({"rate": self.presynaptic_firing_rate,
                     "stop": (self.sim_dura - self.hardcoded_trains_length)},
                    {"rate": self.postsynaptic_firing_rate,
                     "stop": (self.sim_dura - self.hardcoded_trains_length)}))
        presynaptic_generator = generators[0]
        postsynaptic_generator = generators[1]

        # print('stop', self.sim_dura - self.hardcoded_trains_length)

        # create poisson generator for the dopamine release
        pg_dopa = nest.Create("poisson_generator", params={"rate": self.dopa_rate})
        parrot_neuron = nest.Create("parrot_neuron")

        spike_senders = nest.Create(
            "spike_generator",
            2,
            params=(
                {"spike_times": self.hardcoded_pre_times + self.sim_dura - self.hardcoded_trains_length},
                {"spike_times": self.hardcoded_post_times + self.sim_dura - self.hardcoded_trains_length}))
        pre_spike_generator = spike_senders[0]
        post_spike_generator = spike_senders[1]

        # create volume transmitter and weight recorder
        vt = nest.Create("volume_transmitter")
        wr = nest.Create("weight_recorder")

        # The recorder is to save the randomly generated spike trains.
        spike_recorder = nest.Create("spike_recorder")
        spike_recorder_dopa = nest.Create("spike_recorder")

        nest.Connect(presynaptic_generator + pre_spike_generator, presynaptic_neuron,
                     syn_spec={"synapse_model": "static_synapse", "weight": self.J})
        nest.Connect(postsynaptic_generator + post_spike_generator, postsynaptic_neuron,
                     syn_spec={"synapse_model": "static_synapse", "weight": self.J})
        nest.Connect(presynaptic_neuron + postsynaptic_neuron, spike_recorder,
                     syn_spec={"synapse_model": "static_synapse"})

        nest.Connect(pg_dopa, parrot_neuron)
        nest.Connect(parrot_neuron, vt)

        nest.Connect(parrot_neuron, spike_recorder_dopa)

        # creating a link from the synapse to volume transmitter
        self.synapse_parameters["weight_recorder"] = wr
        self.synapse_parameters["vt"] = vt.get('global_id')
        nest.CopyModel(self.synapse_model, self.synapse_model + "_rec", self.synapse_parameters)

        nest.Connect(presynaptic_neuron, postsynaptic_neuron, syn_spec={"synapse_model": self.synapse_model + "_rec"})

        # simulate network
        nest.Simulate(self.sim_dura)

        all_spikes = nest.GetStatus(spike_recorder, keys='events')[0]
        pre_spikes = all_spikes['times'][all_spikes['senders'] == presynaptic_neuron.tolist()[0]]
        post_spikes = all_spikes['times'][all_spikes['senders'] == postsynaptic_neuron.tolist()[0]]
        dopa_spikes = nest.GetStatus(spike_recorder_dopa, keys='events')[0]['times']

        t_hist = nest.GetStatus(wr, "events")[0]["times"]
        weight = nest.GetStatus(wr, "events")[0]["weights"]

        a = nest.GetConnections(synapse_model=self.synapse_model + "_rec")

        return pre_spikes, post_spikes, dopa_spikes, t_hist, weight

    def reproduce_weight_drift(self, pre_spikes, post_spikes, dopa_spikes, fname_snip=""):
        """Independent, self-contained model of STDP dopamine
        """

        def update_dopamine(n):
            n += 1 / self.synapse_parameters["tau_n"]
            return n

        def update_weight(w, c, n, h):

            taus = (self.synapse_parameters["tau_c"] + self.synapse_parameters["tau_n"]) / \
                    (self.synapse_parameters["tau_c"] * self.synapse_parameters["tau_n"])

            w = w - c * (n / taus * np.expm1(-taus * h)
                         - self.synapse_parameters["b"] * self.synapse_parameters["tau_c"]
                         * np.expm1(-h / self.synapse_parameters["tau_c"]))

            w = max(0., w)

            return w

        def facilitate(c, Kpre):
            c += self.synapse_parameters["A_plus"] * Kpre
            return c

        def depress(c, Kpost):
            c -= self.synapse_parameters["A_minus"] * Kpost
            return c

        t = 0.
        Kpre = 0.
        Kpost = 0.
        e = 0.00000001
        weight = self.init_weight
        c = self.init_c
        n = self.init_n

        t_log = []
        w_log = []
        Kpre_log = []

        # logging
        t_log.append(t)
        w_log.append(weight)
        Kpre_log.append(Kpre)

        post_spikes_delayed = post_spikes + self.dendritic_delay
        pre_spikes_delayed = pre_spikes
        # post_spikes_delayed = post_spikes
        dopa_spikes_delayed = dopa_spikes + self.dendritic_delay

        print('post', post_spikes_delayed)
        print('pre', pre_spikes_delayed)
        print('dopa', dopa_spikes_delayed)

        while t < self.sim_dura:
            # idx_next_pre_spike = -1
            t_next_pre_spike = np.inf
            if np.where(abs(pre_spikes_delayed - t) < e)[0].size > 0:
                idx_next_pre_spike = np.where(abs(pre_spikes_delayed - t) < e)[0][0]
                t_next_pre_spike = pre_spikes_delayed[idx_next_pre_spike]

            # idx_next_post_spike = -1
            t_next_post_spike = np.inf
            if np.where(abs(post_spikes_delayed - t) < e)[0].size > 0:
                idx_next_post_spike = np.where(abs(post_spikes_delayed - t) < e)[0][0]
                t_next_post_spike = post_spikes_delayed[idx_next_post_spike]

            # idx_next_dopa_spike = -1
            t_next_dopa_spike = np.inf
            if np.where(abs(dopa_spikes_delayed - t) < e)[0].size > 0:
                idx_next_dopa_spike = np.where(abs(dopa_spikes_delayed - t) < e)[0][0]
                t_next_dopa_spike = dopa_spikes_delayed[idx_next_dopa_spike]

            handle_dopa_spike = False
            handle_post_spike = False
            handle_pre_spike = False

            t_next = min(t_next_dopa_spike, t_next_pre_spike, t_next_post_spike)
            if t_next != np.inf:
                if t_next == t_next_dopa_spike:
                    handle_dopa_spike = True
                if t_next == t_next_post_spike:
                    handle_post_spike = True
                if t_next == t_next_pre_spike:
                    handle_pre_spike = True

            # update weight
            weight = update_weight(weight, c, n, self.resolution)

            c = c * exp(-self.resolution / self.synapse_parameters["tau_c"])
            n = n * exp(-self.resolution / self.synapse_parameters["tau_n"])

            # decay pre and post traces
            Kpre *= exp(-self.resolution / self.tau_pre)
            Kpost *= exp(-self.resolution / self.tau_post)

            if handle_pre_spike:
                Kpre += 1.
                c = depress(c, Kpost)
                print('handle_pre_spike c=%f, n=%f, weight=%f' % (c, n, weight))
                print('t_next', t_next)

            # compute c
            if handle_post_spike:
                Kpost += 1.
                c = facilitate(c, Kpre)
                print('handle_post_spike c=%f, n=%f, weight=%f' % (c, n, weight))
                print('t_next', t_next)

            # compute n
            if handle_dopa_spike:
                n = update_dopamine(n)
                print('handle_dopa_spike c=%f, n=%f, weight=%f' % (c, n, weight))
                print('t_next', t_next)

            # logging
            t_log.append(t)
            w_log.append(weight)
            Kpre_log.append(Kpre)

            t += self.resolution

        if DEBUG_PLOTS:
            self.plot_weight_evolution(pre_spikes, post_spikes, dopa_spikes, t_log, w_log,
                                       fname_snip=fname_snip, title_snip="reference")

        return t_log, w_log

    def plot_weight_evolution(self, pre_spikes, post_spikes, dopa_spikes, t_log, w_log, Kpre_log=None, Kpost_log=None,
                              fname_snip="", title_snip=""):
        """ Plot weight evolution
        """

        # plot data
        fig, ax = plt.subplots(nrows=4)
        mew = 1
        ms = 10
        ms_w = 2
        alpha_spikes = 0.4
        alpha_grid = 0.4

        ax[0].plot(pre_spikes, np.ones(len(pre_spikes)), '|', ms=ms, mew=mew, color="blue", alpha=alpha_spikes)
        ax[0].set_ylabel("Pre spikes")

        ax[1].plot(post_spikes, np.ones(len(post_spikes)), '|', ms=ms, mew=mew, color="blue", alpha=alpha_spikes)
        ax[1].set_ylabel("Post spikes")

        ax[2].plot(dopa_spikes, np.ones(len(dopa_spikes)), '|', ms=ms, mew=mew, color='red')
        ax[2].set_ylabel("Dopa spikes")

        ax[3].plot(t_log, w_log, marker="o", ms=ms_w)
        ax[3].set_ylabel("Weight (pA)")
        ax[3].set_xlabel("Time [ms]")

        for i, _ax in enumerate(ax):
            _ax.grid(which="major", axis="both")
            _ax.grid(which="minor", axis="x", linestyle=":", alpha=alpha_grid)
            _ax.minorticks_on()
            _ax.set_xlim(0., self.sim_dura + 2.)
            if i != len(ax) - 1:
                _ax.set_xticklabels([])

        # save plot
        print("save /tmp/%s_%s.pdf ..." % (fname_snip, title_snip))
        plt.savefig("/tmp/%s_%s.pdf" % (fname_snip, title_snip))

    def test_stdp_synapse(self):
        self.dendritic_delay = float('nan')
        self.init_params()
        # for self.dendritic_delay in [1., self.resolution]:
        for self.dendritic_delay in [1.]:
            self.init_params()
            # for self.nest_neuron_model in ["iaf_psc_exp", "iaf_cond_exp"]:
            for self.nest_neuron_model in ["iaf_psc_exp"]:
                fname_snip = "nest_neuron_mdl_" + self.nest_neuron_model
                self.do_nest_simulation_and_compare_to_reproduced_weight(fname_snip=fname_snip)
