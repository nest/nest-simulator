# -*- coding: utf-8 -*-
#
# test_stdp_multiplicity.py
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

# This script tests the parrot_neuron in NEST.

import nest
import unittest
import math
import numpy as np
import matplotlib.pyplot as plt




@nest.check_stack
class PostTraceTestCase(unittest.TestCase):

    def test_post_trace(self):
        """
        """

        resolution = .5     # [ms]

        delay = 1.  # [ms]

        pre_spike_times1 = [2., 5., 7., 8., 10., 11., 15., 17., 20., 21., 22., 23., 26., 28.]      # [ms]
        post_spike_times1 = [3., 7., 8., 10., 12., 13., 14., 16., 17., 18., 19., 20., 21., 22.]      # [ms]
        
        pre_spike_times = [pre_spike_times1]
        post_spike_times = [post_spike_times1]

        # pre_spike_times = [10., 11., 12., 13., 14., 15., 25., 35., 45., 50., 51., 52., 70.]      # [ms]
        # post_spike_times = [10., 11., 12., 13., 30., 40., 50., 51., 52., 53., 54.]      # [ms]

        # pre_spike_times = [220., 300.]      # [ms]
        # post_spike_times = [150., 250., 350.]      # [ms]

        # pre_spike_times = [301., 302.]      # [ms]
        # post_spike_times = [100., 300.]      # [ms]

        # pre_spike_times = [   4.,   6.  , 6.]    # [ms]
        # post_spike_times = [  2.,   6. ]      # [ms]

        # pre_spike_times = 1 + np.round(100 * np.sort(np.abs(np.random.randn(100))))      # [ms]
        # post_spike_times =  np.sort(np.unique(1 + np.round(100 * np.sort(np.abs(np.random.randn(100))))))     # [ms]

        for spike_times_idx in range(len(pre_spike_times)):

            print("Pre spike times: " + str(pre_spike_times[spike_times_idx]))
            print("Post spike times: " + str(post_spike_times[spike_times_idx]))

            nest.set_verbosity("M_WARNING")

            post_weights = {'parrot': [], 'parrot_ps': []}

            nest.ResetKernel()
            nest.SetKernelStatus({'resolution': resolution})

            wr = nest.Create('weight_recorder')
            nest.CopyModel("stdp_synapse", "stdp_synapse_rec",
                        {"weight_recorder": wr[0], "weight": 1.})

            # create spike_generators with these times
            pre_sg = nest.Create("spike_generator",
                                params={"spike_times": pre_spike_times[spike_times_idx],
                                        'allow_offgrid_spikes': True})
            post_sg = nest.Create("spike_generator",
                                params={"spike_times": post_spike_times[spike_times_idx],
                                        'allow_offgrid_spikes': True})
            pre_sg_ps = nest.Create("spike_generator",
                                    params={"spike_times": pre_spike_times[spike_times_idx],
                                            'precise_times': True})
            post_sg_ps = nest.Create("spike_generator",
                                    params={"spike_times": post_spike_times[spike_times_idx],
                                            'precise_times': True})

            # create parrot neurons and connect spike_generators
            pre_parrot = nest.Create("parrot_neuron")
            post_parrot = nest.Create("parrot_neuron")
            pre_parrot_ps = nest.Create("parrot_neuron_ps")
            post_parrot_ps = nest.Create("parrot_neuron_ps")

            nest.Connect(pre_sg, pre_parrot,
                        syn_spec={"delay": delay})
            nest.Connect(post_sg, post_parrot,
                        syn_spec={"delay": delay})
            nest.Connect(pre_sg_ps, pre_parrot_ps,
                        syn_spec={"delay": delay})
            nest.Connect(post_sg_ps, post_parrot_ps,
                        syn_spec={"delay": delay})

            # create spike detector --- debugging only
            spikes = nest.Create("spike_detector",
                                params={'precise_times': True})
            nest.Connect(
                pre_parrot + post_parrot +
                pre_parrot_ps + post_parrot_ps,
                spikes
            )

            # connect both parrot neurons with a stdp synapse onto port 1
            # thereby spikes transmitted through the stdp connection are
            # not repeated postsynaptically.
            nest.Connect(
                pre_parrot, post_parrot,
                syn_spec={'model': 'stdp_synapse_rec', 'receptor_type': 1})
            nest.Connect(
                pre_parrot_ps, post_parrot_ps,
                syn_spec={'model': 'stdp_synapse_rec', 'receptor_type': 1})

            # get STDP synapse and weight before protocol
            syn = nest.GetConnections(source=pre_parrot,
                                    synapse_model="stdp_synapse_rec")
            w_pre = nest.GetStatus(syn)[0]['weight']
            syn_ps = nest.GetConnections(source=pre_parrot_ps,
                                        synapse_model="stdp_synapse_rec")
            w_pre_ps = nest.GetStatus(syn)[0]['weight']

            print()
            print("[py] w_pre = " + str(w_pre))
            print("[py] w_pre_ps = " + str(w_pre_ps))
            print()

            sim_time = np.amax(np.concatenate((pre_spike_times[spike_times_idx], post_spike_times[spike_times_idx]))) + 5 * delay
            n_steps = int(np.ceil(sim_time / resolution)) + 1
            trace_nest = []
            trace_nest_t = []
            t = nest.GetStatus([0], "time")[0]
            trace_nest_t.append(t)
            post_trace_value = nest.GetStatus(post_parrot)[0]['post_trace']
            trace_nest.append(post_trace_value)
            for step in range(n_steps):
                nest.Simulate(resolution)
                t = nest.GetStatus([0], "time")[0]
                # if True:
                if np.any(np.abs(t - np.array(pre_spike_times[spike_times_idx]) - delay) < resolution/2.):
                    trace_nest_t.append(t)
                    post_trace_value = nest.GetStatus(post_parrot)[0]['post_trace']
                    trace_nest.append(post_trace_value)
                    print("In Python: Getting trace = " + str(post_trace_value) + " at time t = " + str(t))

            # get weight post protocol
            w_post = nest.GetStatus(syn)[0]['weight']
            w_post_ps = nest.GetStatus(syn_ps)[0]['weight']


            tau_minus = nest.GetStatus(post_parrot)[0]['tau_minus']
            # post_trace_value = nest.GetStatus(post_parrot)[0]['post_trace']
            # print("kljpjijiiiiiiiiiiiiiiiiiiiiiiiiiiiiii " + str(post_trace_value))

            fig, ax = plt.subplots(nrows=3)
            ax1, ax3, ax2 = ax

            n_spikes = len(pre_spike_times[spike_times_idx])
            for i in range(n_spikes):
                ax1.plot(2 * [pre_spike_times[spike_times_idx][i] + delay], [0, 1], linewidth=2, color="blue", alpha=.4)

            n_spikes = len(post_spike_times[spike_times_idx])
            for i in range(n_spikes):
                    ax3.plot(2 * [post_spike_times[spike_times_idx][i] + delay], [0, 1], linewidth=2, color="red", alpha=.4)

            ref_post_trace = np.zeros(1000)
            n_spikes = len(post_spike_times[spike_times_idx])
            for sp_idx in range(n_spikes):
                t_sp = post_spike_times[spike_times_idx][sp_idx] + delay
                for i in range(len(ref_post_trace)):
                    t = (i / float(len(ref_post_trace - 1))) * sim_time
                    if t >= t_sp:
                        ref_post_trace[i] += np.exp(-(t - t_sp -delay) / tau_minus)

            ax2.plot(np.linspace(0., sim_time, len(ref_post_trace)), ref_post_trace, label="Expected", color="cyan", alpha=.6)


            # fn_nest_trace_values = "/tmp/trace_vals_0x7ff985894370.txt"
            # print("Please enter fn_nest_trace_values now:")
            # import pdb;pdb.set_trace()
            # s = open(fn_nest_trace_values, "r")
            # l = s.readlines()
            # nest_spike_times = []
            # nest_trace_values = []
            # for line in l:
            #     line_split = line.split()
            #     nest_spike_times.append(float(line_split[0]))
            #     nest_trace_values.append(float(line_split[1]))
            # ax2.scatter(nest_spike_times, nest_trace_values, label="NEST", color="orange")


            ax2.scatter(trace_nest_t, trace_nest, marker=".", alpha=.5, color="orange", label="NEST")


            ax2.set_xlabel("Time [ms]")
            ax1.set_ylabel("Pre spikes")
            ax3.set_ylabel("Post spikes")
            ax2.set_ylabel("Trace")
            ax2.legend()
            for _ax in ax:
                _ax.grid(which="major", axis="both")
                _ax.grid(which="minor", axis="x", linestyle=":", alpha=.4)
                _ax.minorticks_on()
                # _ax.grid(major=True, minor=True)
                _ax.set_xlim(0., sim_time)
            fig.savefig("/tmp/traces.png")

            print("[py] w_post = " + str(w_post))
            print("[py] w_post_ps = " + str(w_post_ps))
            print()

            wr_weights = nest.GetStatus(wr, "events")[0]["weights"]
            print("[py] wr_weights = " + str(wr_weights))


            assert w_post != w_pre, "Plain parrot weight did not change."
            assert w_post_ps != w_pre_ps, "Precise parrot \
                weight did not change."

            post_weights['parrot'].append(w_post)
            post_weights['parrot_ps'].append(w_post_ps)


def suite():
    suite1 = unittest.TestLoader().loadTestsFromTestCase(PostTraceTestCase)
    return unittest.TestSuite([suite1])


def run():
    runner = unittest.TextTestRunner(verbosity=99)
    runner.run(suite())


if __name__ == "__main__":
    #unittest.findTestCases(__main__).debug()
    run()
