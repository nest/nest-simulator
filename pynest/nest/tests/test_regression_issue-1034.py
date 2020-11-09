# -*- coding: utf-8 -*-
#
# test_regression_issue-1034.py
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

# Please see `doc/model_details/test_post_trace.ipynb` for a version of this
# test that includes more documentation and plotting.


import nest
import numpy as np
import scipy as sp
import scipy.stats
import unittest


class PostTraceTester(object):
    '''Test that postsynaptic trace values returned from NEST are consistent
    with reference values generated in Python.

    For more information, please see the Jupyter notebook in
    `doc/model_details/test_post_trace.ipynb`.
    '''

    def __init__(self, pre_spike_times, post_spike_times, delay, resolution,
                 tau_minus, trace_match_atol, trace_match_rtol):
        self.pre_spike_times_ = pre_spike_times
        self.post_spike_times_ = post_spike_times
        self.delay_ = delay
        self.dendritic_delay_ = delay
        self.resolution_ = resolution
        self.tau_minus_ = tau_minus
        self.trace_match_atol_ = trace_match_atol
        self.trace_match_rtol_ = trace_match_rtol

        self.max_t_sp_ = max(np.amax(self.pre_spike_times_),
                             np.amax(self.post_spike_times_))
        self.sim_time_ = self.max_t_sp_ + 5 * self.delay_

    def run_post_trace_test_nest_(self,
                                  show_all_nest_trace_samples=False):

        nest.set_verbosity("M_WARNING")

        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': self.resolution_})

        wr = nest.Create('weight_recorder')
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec",
                       {"weight_recorder": wr, "weight": 1.})

        # create spike_generators with these times
        pre_sg_ps = nest.Create("spike_generator",
                                params={"spike_times": self.pre_spike_times_,
                                        'precise_times': True})
        post_sg_ps = nest.Create("spike_generator",
                                 params={"spike_times": self.post_spike_times_,
                                         'precise_times': True})

        # create parrot neurons and connect spike_generators
        pre_parrot_ps = nest.Create("parrot_neuron_ps")
        post_parrot_ps = nest.Create("parrot_neuron_ps",
                                     params={"tau_minus": self.tau_minus_})

        nest.Connect(pre_sg_ps, pre_parrot_ps,
                     syn_spec={"delay": self.delay_})
        nest.Connect(post_sg_ps, post_parrot_ps,
                     syn_spec={"delay": self.delay_})

        # create spike recorder --- debugging only
        spikes = nest.Create("spike_recorder")
        nest.Connect(pre_parrot_ps + post_parrot_ps, spikes)

        # connect both parrot neurons with a stdp synapse onto port 1
        # thereby spikes transmitted through the stdp connection are
        # not repeated postsynaptically.
        nest.Connect(
            pre_parrot_ps, post_parrot_ps,
            syn_spec={'synapse_model': 'stdp_synapse_rec',
                      'receptor_type': 1,
                      'delay': self.delay_})

        # get STDP synapse
        syn_ps = nest.GetConnections(source=pre_parrot_ps,
                                     synapse_model="stdp_synapse_rec")

        print("[py] Total simulation time: " + str(self.sim_time_) + " ms")
        n_steps = int(np.ceil(self.sim_time_ / self.delay_))
        trace_nest = []
        trace_nest_t = []
        t = nest.GetKernelStatus("time")
        trace_nest_t.append(t)
        post_tr = nest.GetStatus(post_parrot_ps)[0]['post_trace']
        trace_nest.append(post_tr)
        for step in range(n_steps):
            print("\n[py] simulating for " + str(self.delay_) + " ms")
            nest.Simulate(self.delay_)
            t = nest.GetKernelStatus("time")
            nearby_pre_spike = np.any(
                np.abs(t - np.array(self.pre_spike_times_) - self.delay_) < self.resolution_ / 2.)
            if show_all_nest_trace_samples or nearby_pre_spike:
                trace_nest_t.append(t)
                post_tr = nest.GetStatus(post_parrot_ps)[0]['post_trace']
                trace_nest.append(post_tr)
                print("[py] Received NEST trace: " +
                      str(post_tr) + " at time t = " + str(t))

        return trace_nest_t, trace_nest

    def run_post_trace_test_python_reference_(self, debug=False):
        """
        compute Python known-good reference of postsynaptic trace
        """

        n_timepoints = int(np.ceil(1000 * self.sim_time_))
        trace_python_ref = np.zeros(n_timepoints)

        for post_spike_time in self.post_spike_times_:
            t_sp = post_spike_time + self.delay_ + self.dendritic_delay_
            for i in range(n_timepoints):
                t = (i / float(n_timepoints - 1)) * self.sim_time_
                if t > t_sp:
                    trace_python_ref[i] += np.exp(-(t - t_sp) / self.tau_minus_)

        for pre_spike_time in self.pre_spike_times_:
            t_sp = pre_spike_time + self.delay_
            i = int(np.round(t_sp / self.sim_time_ * float(len(trace_python_ref) - 1)))
            if debug:
                print("* At t_sp = " + str(t_sp) + ", post_trace should be " + str(trace_python_ref[i]))

        return trace_python_ref

    def nest_trace_matches_ref_trace_(self, trace_nest_t, trace_nest,
                                      trace_python_ref, debug=True):
        """
        Trace values are returned from NEST at regular intervals, but only
        updated at presynaptic spike times.

        To match the NEST samples with the continuous reference trace, step
        backwards in time from the sampled value, to find the last time at
        which the trace value was updated, namely the time of occurrence of
        the last presynaptic spike.
        """

        for t, trace_nest_val in zip(trace_nest_t[1:], trace_nest[1:]):
            if debug:
                print("* Finding ref for NEST timepoint t = " + str(t) + ", trace = " + str(trace_nest_val))

            traces_match = False
            for i_search, t_search in enumerate(
                    reversed(np.array(self.pre_spike_times_) + self.delay_)):
                if t_search <= t:
                    _trace_at_t_search = trace_python_ref[int(np.round(
                        t_search / self.sim_time_ * float(len(trace_python_ref) - 1)))]
                    traces_match = np.allclose(
                        _trace_at_t_search,
                        trace_nest_val,
                        atol=self.trace_match_atol_,
                        rtol=self.trace_match_rtol_)
                    post_spike_occurred_at_t_search = np.any(
                        (t_search - (np.array(self.post_spike_times_) + self.delay_ + self.dendritic_delay_))**2 <
                        self.resolution_ / 2.)

                    if debug:
                        print("\t* Testing " + str(t_search) + "...")
                        print("\t   traces_match = " + str(traces_match))
                        print("\t   post_spike_occurred_at_t_search = " + str(post_spike_occurred_at_t_search))

                    if (not traces_match) and post_spike_occurred_at_t_search:
                        traces_match = np.allclose(
                            _trace_at_t_search + 1,
                            trace_nest_val,
                            atol=self.trace_match_atol_,
                            rtol=self.trace_match_rtol_)
                        if debug:
                            print("\t   traces_match = " + str(traces_match) + " (nest trace = " +
                                  str(trace_nest_val) + ", ref trace = " + str(_trace_at_t_search + 1) + ")")
                        if traces_match:
                            _trace_at_t_search += 1.

                    if (not traces_match) and post_spike_occurred_at_t_search:
                        traces_match = np.allclose(
                            _trace_at_t_search - 1,
                            trace_nest_val,
                            atol=self.trace_match_atol_,
                            rtol=self.trace_match_rtol_)
                        if debug:
                            print("\t   traces_match = " + str(traces_match) + " (nest trace = " +
                                  str(trace_nest_val) + ", ref trace = " + str(_trace_at_t_search - 1) + ")")
                        if traces_match:
                            _trace_at_t_search -= 1.

                    break

            if ((not traces_match) and i_search == len(self.pre_spike_times_) - 1):
                if debug:
                    print("\tthe time before the first pre spike")
                # the time before the first pre spike
                traces_match = trace_nest_val == 0.

            if not traces_match:
                return False

        return True

    def nest_trace_matches_python_trace(self):
        trace_nest_t, trace_nest = self.run_post_trace_test_nest_()
        trace_python_ref = self.run_post_trace_test_python_reference_()
        return self.nest_trace_matches_ref_trace_(
            trace_nest_t,
            trace_nest,
            trace_python_ref)


class PostTraceTestCase(unittest.TestCase):

    def test_post_trace(self):
        """
        construct a network of the form:
        - pre_spike_gen connects via static_synapse to pre_parrot
        - pre_parrot connects via stdp_synapse to post_parrot
        - post_spike_gen connects via static_synapse to post_parrot

        The spike times of the spike generators are defined in
        `pre_spike_times` and `post_spike_times`. From the perspective of the
        STDP synapse, spikes arrive with the following delays (with respect to
        the values in these lists):

        - for the presynaptic neuron: one synaptic delay in the static synapse
        - for the postsynaptic neuron: one synaptic delay in the static synapse
        - for the synapse itself: one dendritic delay between the post_parrot
          node and the synapse itself (see the C++ variable `dendritic_delay`).
        """

        resolution = .1     # [ms]
        delays = np.array([1., 5.])  # [ms]

        # spike test pattern 1: minimal reproducing example of the original bug
        pre_spike_times1 = np.array([2., 3., 10.])
        post_spike_times1 = np.array([1., 2., 3.])

        # spike test pattern 2: generate some random integer spike times
        t_sp_min = 1.
        t_sp_max = 50
        n_spikes = 10
        pre_spike_times2 = np.sort(
            np.unique(
                np.ceil(
                    sp.stats.uniform.rvs(
                        t_sp_min, t_sp_max - t_sp_min, n_spikes))))
        n_spikes = 50
        post_spike_times2 = np.sort(
            np.unique(
                np.ceil(
                    sp.stats.uniform.rvs(
                        t_sp_min, t_sp_max - t_sp_min, n_spikes))))
        tau_minus = 2.  # [ms]

        # for each parameter set, run the test
        # spike test pattern 3 is a pre/post-reversed version of test pattern 2
        pre_spike_times = [pre_spike_times1,
                           pre_spike_times2,
                           post_spike_times2]
        post_spike_times = [post_spike_times1,
                            post_spike_times2,
                            pre_spike_times2]

        for pre_spike_time, post_spike_time in zip(pre_spike_times,
                                                   post_spike_times):
            print("Pre spike times: [" + ", ".join([str(t) for t in pre_spike_time]) + "]")
            print("Post spike times: [" + ", ".join([str(t) for t in post_spike_time]) + "]")

            for delay in delays:
                test = PostTraceTester(
                    pre_spike_times=pre_spike_time,
                    post_spike_times=post_spike_time,
                    delay=delay,
                    resolution=resolution,
                    tau_minus=tau_minus,
                    trace_match_atol=1E-3,
                    trace_match_rtol=1E-3)
                self.assertTrue(test.nest_trace_matches_python_trace())


def suite():
    t = unittest.TestLoader().loadTestsFromTestCase(PostTraceTestCase)
    return unittest.TestSuite([t])


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
