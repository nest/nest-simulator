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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See thed
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with NEST.  If not, see <http://www.gnu.org/licenses/>.

# This script tests the parrot_neuron in NEST.

import nest
import unittest
import math
import numpy as np
import scipy as sp
import scipy.stats
import matplotlib.pyplot as plt
import matplotlib.ticker as plticker


#@nest.check_stack
class PostTraceTestCase(unittest.TestCase):

    trace_match_atol_ = 1E-2
    trace_match_rtol_ = 1E-2

    def run_post_trace_test_nest_(self, pre_spike_times, post_spike_times, resolution, delay, sim_time, tau_minus, show_all_nest_trace_samples=False):

        print("Pre spike times: [" + ", ".join([str(t) for t in pre_spike_times]) + "]")
        print("Post spike times: [" + ", ".join([str(t) for t in post_spike_times]) + "]")

        nest.set_verbosity("M_WARNING")

        post_weights = {'parrot': [], 'parrot_ps': []}

        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': resolution})

        wr = nest.Create('weight_recorder')
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec",
                    {"weight_recorder": wr[0], "weight": 1.})

        # create spike_generators with these times
        """pre_sg = nest.Create("spike_generator",
                            params={"spike_times": pre_spike_times,
                                    'allow_offgrid_spikes': True})
        post_sg = nest.Create("spike_generator",
                            params={"spike_times": post_spike_times,
                                    'allow_offgrid_spikes': True})"""
        pre_sg_ps = nest.Create("spike_generator",
                                params={"spike_times": pre_spike_times})
                                        #'precise_times': True})
        post_sg_ps = nest.Create("spike_generator",
                                params={"spike_times": post_spike_times})
#                                            'precise_times': True})

        # create parrot neurons and connect spike_generators
        #pre_parrot = nest.Create("parrot_neuron")
        #post_parrot = nest.Create("parrot_neuron", params={"tau_minus" : tau_minus})
        pre_parrot_ps = nest.Create("parrot_neuron_ps")
        post_parrot_ps = nest.Create("parrot_neuron_ps", params={"tau_minus" : tau_minus})

        #nest.Connect(pre_sg, pre_parrot,
        #            syn_spec={"delay": delay})
        #nest.Connect(post_sg, post_parrot,
        #            syn_spec={"delay": delay})
        nest.Connect(pre_sg_ps, pre_parrot_ps,
                    syn_spec={"delay": delay})
        nest.Connect(post_sg_ps, post_parrot_ps,
                    syn_spec={"delay": delay})

        # create spike detector --- debugging only
        spikes = nest.Create("spike_detector")#,
           #                 params={'precise_times': True})
        nest.Connect(
            #pre_parrot + post_parrot +
            pre_parrot_ps + post_parrot_ps,
            spikes
        )

        # connect both parrot neurons with a stdp synapse onto port 1
        # thereby spikes transmitted through the stdp connection are
        # not repeated postsynaptically.
        #nest.Connect(
        #    pre_parrot, post_parrot,
        #    syn_spec={'model': 'stdp_synapse_rec', 'receptor_type': 1, 'delay' : delay})
        nest.Connect(
            pre_parrot_ps, post_parrot_ps,
            syn_spec={'model': 'stdp_synapse_rec', 'receptor_type': 1, 'delay' : delay})

        # get STDP synapse
        #syn = nest.GetConnections(source=pre_parrot,
        #                        synapse_model="stdp_synapse_rec")
        syn_ps = nest.GetConnections(source=pre_parrot_ps,
                                    synapse_model="stdp_synapse_rec")

        print("[py] Total simulation time: " + str(sim_time) + " ms")
        n_steps = int(np.ceil(sim_time / delay))
        trace_nest = []
        trace_nest_t = []
        t = nest.GetStatus([0], "time")[0]
        trace_nest_t.append(t)
        post_trace_value = nest.GetStatus(post_parrot_ps)[0]['post_trace']
        trace_nest.append(post_trace_value)
        for step in range(n_steps):
            print("\n[py] simulating for " + str(delay) + " ms")
            nest.Simulate(delay)
            t = nest.GetStatus([0], "time")[0]
            if show_all_nest_trace_samples or np.any(np.abs(t - np.array(pre_spike_times) - delay) < resolution/2.):
                trace_nest_t.append(t)
                post_trace_value = nest.GetStatus(post_parrot_ps)[0]['post_trace']
                trace_nest.append(post_trace_value)
                print("[py] Received NEST trace: " + str(post_trace_value) + " at time t = " + str(t))

        return trace_nest_t, trace_nest



    def run_post_trace_test_python_reference_(self, pre_spike_times, post_spike_times, resolution, delay, dendritic_delay, sim_time, tau_minus):
        """
        compute Python known-good reference of postsynaptic trace
        """

        n_timepoints = 100 * int(np.ceil(max(np.amax(pre_spike_times), np.amax(post_spike_times))))
        trace_python_ref = np.zeros(n_timepoints)
        n_spikes = len(post_spike_times)
        for sp_idx in range(n_spikes):
            t_sp = post_spike_times[sp_idx] + delay + dendritic_delay
            for i in range(n_timepoints):
                t = (i / float(n_timepoints - 1)) * sim_time
                if t > t_sp + 1E-3:
                    trace_python_ref[i] += np.exp(-(t - t_sp) / tau_minus)

        n_spikes = len(pre_spike_times)
        for sp_idx in range(n_spikes):
            t_sp = pre_spike_times[sp_idx] + delay
            i = int(np.round(t_sp / sim_time * float(len(trace_python_ref) - 1)))
            print("* At t_sp = " + str(t_sp) + ", post_trace should be " + str(trace_python_ref[i]))

        return trace_python_ref


    def plot_run(self, trace_nest_t, trace_nest, trace_python_ref, pre_spike_times, post_spike_times, resolution, delay, dendritic_delay, sim_time, fname):

        fig, ax = plt.subplots(nrows=3)
        ax1, ax2, ax3 = ax


        #
        #   pre spikes
        #

        ax1.set_ylim([0., 1.])
        ax1.set_ylabel("Pre spikes")
        n_spikes = len(pre_spike_times)
        for i in range(n_spikes):
            ax1.plot(2 * [pre_spike_times[i] + delay], ax1.get_ylim(), linewidth=2, color="blue", alpha=.4)


        #
        #   post spikes
        #

        ax2.set_ylim([0., 1.])
        ax2.set_ylabel("Post spikes")
        n_spikes = len(post_spike_times)
        for i in range(n_spikes):
            ax2.plot(2 * [post_spike_times[i] + delay + dendritic_delay], [0, 1], linewidth=2, color="red", alpha=.4)


        #
        #   traces
        #

        ax3.legend()
        ax3.set_ylabel("Synaptic trace")
        ax3.set_ylim([0., np.amax(trace_python_ref)])
        ax3.plot(np.linspace(0., sim_time, len(trace_python_ref)), trace_python_ref, label="Expected", color="cyan", alpha=.6)
        ax3.scatter(trace_nest_t, trace_nest, marker=".", alpha=.5, color="orange", label="NEST")
        # print("trace_nest_t = " + str(trace_nest_t) + ", trace_nest = " + str(trace_nest))

        #
        #   Trace values are returned from NEST at regular intervals, but only updated at presynaptic spike times.
        #
        #   Step backwards in time from the sampled value, to find the last time at which the trace value was updated, namely the time of occurrence of the last presynaptic spike.
        #

        n_timepoints = len(trace_nest_t)
        for i in range(n_timepoints):
            t = trace_nest_t[i]
            print("* Finding ref for NEST timepoint t = " + str(t) + ", trace = " + str(trace_nest[i]))
            for i_search, t_search in enumerate(reversed(np.array(pre_spike_times) + delay)):
                if t_search <= t:
                    print("\t* Testing " + str(t_search) + "...")
                    _trace_at_t_search = trace_python_ref[int(np.round(t_search / sim_time * float(len(trace_python_ref) - 1)))]
                    #if (t_search - trace_nest_t[i])**2 > resolution/2. \
                    #idx = np.argmin((t_search - (np.array(post_spike_times) + delay + dendritic_delay))**2)
                    #t_found = (t_search - (np.array(post_spike_times) + delay + dendritic_delay)
                    traces_match = np.allclose(_trace_at_t_search, trace_nest[i], atol=self.trace_match_atol_, rtol=self.trace_match_rtol_)
                    print("\t   traces_match = " + str(traces_match))
                    if not traces_match:
                        post_spike_occurred_at_t_search = np.any((t_search - (np.array(post_spike_times) + delay + dendritic_delay))**2 < resolution/2.)
                        print("\t   post_spike_occurred_at_t_search = " + str(post_spike_occurred_at_t_search))
                        if post_spike_occurred_at_t_search:
                            traces_match = np.allclose(_trace_at_t_search + 1, trace_nest[i], atol=self.trace_match_atol_, rtol=self.trace_match_rtol_)
                            print("\t   traces_match = " + str(traces_match) + " (nest trace = " + str(trace_nest[i]) + ", ref trace = " + str(_trace_at_t_search+1) + ")")
                            if traces_match:
                                _trace_at_t_search += 1.

                        if not traces_match and post_spike_occurred_at_t_search:
                            traces_match = np.allclose(_trace_at_t_search - 1, trace_nest[i], atol=self.trace_match_atol_, rtol=self.trace_match_rtol_)
                            print("\t   traces_match = " + str(traces_match) + " (nest trace = " + str(trace_nest[i]) + ", ref trace = " + str(_trace_at_t_search-1) + ")")
                            if traces_match:
                                _trace_at_t_search -= 1.

                    ax3.scatter(t_search, _trace_at_t_search, 100, marker=".", color="#A7FF00FF", facecolor="none")#"#FFFFFF7F")
                    ax3.plot([trace_nest_t[i], t_search], [trace_nest[i], _trace_at_t_search], linewidth=.5, color="#0000007F")
                    break

        
        for _ax in ax:
            _ax.xaxis.set_major_locator(plticker.MultipleLocator(base=10*delay))
            _ax.xaxis.set_minor_locator(plticker.MultipleLocator(base=delay))
            _ax.grid(which="major", axis="both")
            _ax.grid(which="minor", axis="x", linestyle=":", alpha=.4)
            #_ax.minorticks_on()
            _ax.set_xlim(0., sim_time)

        ax3.set_xlabel("Time [ms]")
        fig.suptitle("Postsynaptic trace testbench. Spike times are\nshown from the perspective of the STDP synapse.")

        print("* Saving to " + fname)
        fig.savefig(fname, dpi=300.)


    def nest_trace_matches_ref_trace(self, trace_nest_t, trace_nest, trace_python_ref, pre_spike_times, post_spike_times, resolution, delay, dendritic_delay, sim_time):
        """
        Trace values are returned from NEST at regular intervals, but only updated at presynaptic spike times.
        
        To match the NEST samples with the continuous reference trace, step backwards in time from the sampled value, to find the last time at which the trace value was updated, namely the time of occurrence of the last presynaptic spike.
        """

        trace_nest_adjusted = trace_nest.copy()

        n_timepoints = len(trace_nest_t)
        for i in range(n_timepoints)[1:]:
            t = trace_nest_t[i]
            print("* Finding ref for NEST timepoint t = " + str(t) + ", trace = " + str(trace_nest[i]))

            traces_match = False
            for i_search, t_search in enumerate(reversed(np.array(pre_spike_times) + delay)):
                if t_search <= t:
                    print("\t* Testing " + str(t_search) + "...")
                    _trace_at_t_search = trace_python_ref[int(np.round(t_search / sim_time * float(len(trace_python_ref) - 1)))]
                    traces_match = np.allclose(_trace_at_t_search, trace_nest[i], atol=self.trace_match_atol_, rtol=self.trace_match_rtol_)
                    print("\t   traces_match = " + str(traces_match))
                    post_spike_occurred_at_t_search = np.any((t_search - (np.array(post_spike_times) + delay + dendritic_delay))**2 < resolution/2.)
                    print("\t   post_spike_occurred_at_t_search = " + str(post_spike_occurred_at_t_search))

                    if (not traces_match) and post_spike_occurred_at_t_search:
                        traces_match = np.allclose(_trace_at_t_search + 1, trace_nest[i], atol=self.trace_match_atol_, rtol=self.trace_match_rtol_)
                        print("\t   traces_match = " + str(traces_match) + " (nest trace = " + str(trace_nest[i]) + ", ref trace = " + str(_trace_at_t_search+1) + ")")
                        if traces_match:
                            _trace_at_t_search += 1.

                    if (not traces_match) and post_spike_occurred_at_t_search:
                        traces_match = np.allclose(_trace_at_t_search - 1, trace_nest[i], atol=self.trace_match_atol_, rtol=self.trace_match_rtol_)
                        print("\t   traces_match = " + str(traces_match) + " (nest trace = " + str(trace_nest[i]) + ", ref trace = " + str(_trace_at_t_search-1) + ")")
                        if traces_match:
                            _trace_at_t_search -= 1.

                    break

            if (not traces_match) and i_search == len(pre_spike_times) - 1:
                print("\tthe time before the first pre spike")
                # the time before the first pre spike
                traces_match = trace_nest[i] == 0.

            if not traces_match:
                return False

        return True

    def test_post_trace(self):
        """
        construct a network of the form:

                               static_synapse                   stdp_synapse                    static_synapse
            [ pre_spike_gen ] ----(delay)----o [ pre_parrot ] ----(delay)----o [ post_parrot ] o----(delay)---- [ post_spike_gen ]


        The spike times of the spike generators are defined in `pre_spike_times` and `post_spike_times`. From the perspective of the STDP synapse, spikes arrive with the following delays (with respect to the values in these lists):

        - for the presynaptic neuron: one synaptic delay in the leftmost static synapse
        - for the postsynaptic neuron: one synaptic delay in the rightmost static synapse
        - for the synapse itself: one dendritic delay between the post_parrot node and the synapse itself (see the C++ variable `dendritic_delay`).
        """

        show_all_nest_trace_samples = True

        resolution = .1     # [ms]
        delays = np.array([1., 5.])  # [ms]

        # pre_spike_times1 = [2., 5., 7., 8., 10., 11., 15., 17., 20., 21., 22., 23., 26., 28.]      # [ms]
        # post_spike_times1 = [3., 7., 8., 10., 12., 13., 14., 16., 17., 18., 19., 20., 21., 22.]      # [ms]
        
        # pre_spike_times = [10., 11., 12., 13., 14., 15., 25., 35., 45., 50., 51., 52., 70.]      # [ms]
        # post_spike_times = [10., 11., 12., 13., 30., 40., 50., 51., 52., 53., 54.]      # [ms]

        # pre_spike_times = [220., 300.]      # [ms]
        # post_spike_times = [150., 250., 350.]      # [ms]

        # pre_spike_times = [301., 302.]      # [ms]
        # post_spike_times = [100., 300.]      # [ms]

        # pre_spike_times = [   4.,   6.  , 6.]    # [ms]
        # post_spike_times = [  2.,   6. ]      # [ms]

        #pre_spike_times1 = np.sort(np.unique(1 + np.round(100 * np.abs(np.random.randn(100)))))      # [ms]
        #post_spike_times1 = np.sort(np.unique(1 + np.round(100 * np.sort(np.abs(np.random.randn(100))))))     # [ms]

        t_sp_min = 1.
        t_sp_max = 50
        n_spikes = 10 #int(t_sp_max)
        pre_spike_times2 = np.sort(np.unique(np.ceil(sp.stats.uniform.rvs(t_sp_min, t_sp_max - t_sp_min, n_spikes))))
        n_spikes = 50
        post_spike_times2 = np.sort(np.unique(np.ceil(sp.stats.uniform.rvs(t_sp_min, t_sp_max - t_sp_min, n_spikes))))
        tau_minus = 2.	# [ms]

        #pre_spike_times1 = [2.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 11.0, 12.0, 13.0, 16.0, 17.0, 18.0, 20.0, 21.0, 22.0, 24.0, 26.0, 28.0, 29.0, 30.0, 31.0, 32.0, 33.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 46.0, 48.0, 50.0]
        #post_spike_times1 = [2.0, 4.0, 5.0, 6.0, 8.0, 13.0, 14.0, 17.0, 18.0, 19.0, 21.0, 22.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 46.0, 48.0, 49.0, 50.0]

        #pre_spike_times1 = np.array([2.0, 7.0, 13.0, 18, 23, 28, 33, 37])
        #post_spike_times1 = np.array([2.0, 7.0, 13.0, 18, 23, 28, 33, 37])

        # pre_spike_times = [np.concatenate((pre_spike_times1, np.array([70.])))]
        # post_spike_times = [post_spike_times1]

        # pre_spike_times = [np.array(a) for a in pre_spike_times]
        # post_spike_times = [np.array(a) for a in post_spike_times]

        # pre_spike_times.append(post_spike_times[0].copy())
        # post_spike_times.append(pre_spike_times[0].copy())

        # minimal reproducing example of the original bug
        pre_spike_times1 = np.array([2., 3., 10.])
        post_spike_times1 = np.array([1., 2., 3.])

        # pre_spike_times2 = np.array([3.0, 4.0, 9.0, 10.0, 11.0, 13.0, 14.0, 17.0, 18.0, 19.0, 20.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 34.0, 35.0, 37.0, 39.0, 40.0, 43.0, 44.0, 46.0, 47.0, 48.0, 49.0, 50.0])
        # post_spike_times2 = np.array([5.0, 10.0, 15.0, 22.0, 23.0, 28.0, 39.0, 41.0, 47.0])#[1., 2., 3.])


        # minimal reproducing example of the original bug
        # pre_spike_times2 = np.array([16.0, 17.0, 18.0])
        # post_spike_times2 = np.array([12.0, 14.0, 17.0, 18.0])

        # for each parameter set, run the test
        pre_spike_times = [pre_spike_times1, pre_spike_times2, post_spike_times2]
        post_spike_times = [post_spike_times1, post_spike_times2, pre_spike_times2]

        for delay in delays:
            dendritic_delay = delay
            for spike_times_idx in range(len(pre_spike_times)):
                fname = "/tmp/traces_[delay=" + str(delay) + "]_[experiment=" + str(spike_times_idx) + "].png"
                sim_time = np.amax(np.concatenate((pre_spike_times[spike_times_idx], post_spike_times[spike_times_idx]))) + 5 * delay
                trace_nest_t, trace_nest = self.run_post_trace_test_nest_(pre_spike_times[spike_times_idx], post_spike_times[spike_times_idx], resolution, delay, sim_time, tau_minus, show_all_nest_trace_samples)
                trace_python_ref = self.run_post_trace_test_python_reference_(pre_spike_times[spike_times_idx], post_spike_times[spike_times_idx], resolution, delay, dendritic_delay, sim_time, tau_minus)
                self.plot_run(trace_nest_t, trace_nest, trace_python_ref, pre_spike_times[spike_times_idx], post_spike_times[spike_times_idx], resolution, delay, dendritic_delay, sim_time, fname)
                if not self.nest_trace_matches_ref_trace(trace_nest_t, trace_nest, trace_python_ref, pre_spike_times[spike_times_idx], post_spike_times[spike_times_idx], resolution, delay, dendritic_delay, sim_time):
                    import pdb;pdb.set_trace()
                self.assertTrue(self.nest_trace_matches_ref_trace(trace_nest_t, trace_nest, trace_python_ref, pre_spike_times[spike_times_idx], post_spike_times[spike_times_idx], resolution, delay, dendritic_delay, sim_time))



def suite():
    suite1 = unittest.TestLoader().loadTestsFromTestCase(PostTraceTestCase)
    return unittest.TestSuite([suite1])


def run():
    runner = unittest.TextTestRunner(verbosity=99)
    runner.run(suite())


if __name__ == "__main__":
    #unittest.findTestCases(__main__).debug()
    #run()
    PostTraceTestCase().test_post_trace()
