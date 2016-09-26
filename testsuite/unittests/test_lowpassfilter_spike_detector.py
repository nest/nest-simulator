# -*- coding: utf-8 -*-
#
# test_lowpassfilter_spike_detector.py
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

# Begin Documentation
# Name: testsuite::test_lowpassfilter_spikedetector - script to test
#       fast low-pass filtering device.
# author: Sepehr Mahmoudian
# date: August 2016

import math
import sys
import unittest

import nest
import numpy as np


def lowpass_filter_python(senders, spike_times, reporting_times, tau=30.0):
    uniq_senders = (np.array)(np.unique(senders))
    total_spikes = np.size(spike_times)
    traces = np.zeros(total_spikes,
                      dtype=[('traces', float),
                             ('times', float),
                             ('senders', int)])

    # Here the trace is calculated at each spike time for each node

    # index in the traces array, each idx corresponds to one spike
    # time for a node.
    trace_idx = 0
    for gid in uniq_senders:

        # getting the spikes that are for this node
        spktimes_ = spike_times[np.where(senders == gid)]
        time = 0.
        trace = 0.

        # If the node did not spike, then the trace should always be 0
        if spktimes_.size == 0:
            traces[idx] = (trace, time, gid)
            trace_idx += 1

        for i, st in enumerate(spktimes_):

            # if the spike time was not the same as the previous
            # spike time for this node, it is necessary to increment
            # the idx.
            if (np.isclose(time, st) is False and
               np.isclose(time, 0.0) is False):
                trace_idx += 1

            # Calculating decay
            trace = math.exp((time - st) / tau) * trace
            # Adding impulse
            trace += 1 / tau
            # Setting the time to the last spike time
            time = st

            # Adding the trace to the record array
            traces[trace_idx] = (trace, time, gid)

            # if this is the last spike for this node,
            # it is necessary to increment the idx
            if (i == len(spktimes_) - 1):
                trace_idx += 1

    # Here the trace is calculated for each report time by calculating
    # it forward from each spike time to reporting time

    ktrace_values = np.zeros(uniq_senders.size * reporting_times.size)
    ktrace_times = np.zeros(uniq_senders.size * reporting_times.size)
    ktrace_gids = np.zeros(uniq_senders.size * reporting_times.size,
                           dtype=np.int64)

    trace_idx = 0
    for gid in uniq_senders:

        node_traces = traces[np.where(traces['senders'] == gid)]
        spike_times = node_traces['times']

        # idx stores the index of the time greater than the sample time.
        # idx - 1 gives back the index of time smaller than the current time.
        for st in reporting_times:
            idx = np.searchsorted(spike_times, st)

            if idx == 0:  # this means no suitable index
                sample_trace = 0.0

            else:
                sample_trace = math.exp((spike_times[idx - 1] - st) / tau)
                sample_trace = sample_trace * node_traces[idx - 1]['traces']

            if spike_times.size > idx:
                if (np.isclose(spike_times[idx], st)):  # if they are the same
                    sample_trace = node_traces[idx]['traces']

            ktrace_values[trace_idx] = sample_trace
            ktrace_times[trace_idx] = st
            ktrace_gids[trace_idx] = gid

            trace_idx += 1

    return {'ktrace_senders': ktrace_gids,
            'ktrace_times': ktrace_times,
            'ktrace_values': ktrace_values}


@nest.check_stack
class LowpassFilterSpikeDetector(unittest.TestCase):
    def test_Defaults(self):
        """ 1) Checking default values. """

        lpfsd = nest.Create('lowpassfilter_spike_detector')
        st = nest.GetStatus(lpfsd)[0]
        self.assertAlmostEquals(st['tau_filter'], 30.)
        self.assertEquals(st['record_spikes'], False)
        self.assertEquals(st['filter_report_interval'], 1)

    def test_SettingStatus(self):
        """ 2) Checking to see if setting Status works. """

        nest.ResetKernel()
        set_start_times = [0., 4.]
        set_stop_times = [2., 6.]
        lpfsd = nest.Create('lowpassfilter_spike_detector', 1,
                            params={'filter_start_times': set_start_times,
                                    'filter_stop_times': set_stop_times,
                                    'filter_report_interval': 2.,
                                    'tau_filter': 20.,
                                    'record_spikes': False})
        st = nest.GetStatus(lpfsd)[0]

        self.assertAlmostEquals(st['tau_filter'], 20.)
        self.assertEquals(st['filter_report_interval'], 2)
        self.assertEquals(st['record_spikes'], False)

        # Checking start and stop times.
        get_start_times = st['filter_start_times']
        get_stop_times = st['filter_stop_times']
        num_inter = len(set(set_start_times).intersection(get_start_times))
        self.assertEquals(num_inter, len(set_start_times))
        num_inter = len(set(set_stop_times).intersection(get_stop_times))
        self.assertEquals(num_inter, len(set_stop_times))

    def test_SettingInvalidBlocks(self):
        """ 3) Setting invalid filter blocks. """

        nest.ResetKernel()
        set_start_times = [0.0, 5.5, 9.0]
        set_stop_times = [1.0, 8.5, 10.0]
        lpfsd = nest.Create('lowpassfilter_spike_detector', 1,
                            params={'filter_start_times': set_start_times,
                                    'filter_stop_times': set_stop_times,
                                    'filter_report_interval': 2.})

        # The rest are invalid because interval is less than 2. and they
        # should be removed by the device.
        valid_start_times = [5.5]
        valid_stop_times = [8.5]

        st = nest.GetStatus(lpfsd)[0]
        get_start_times = st['filter_start_times']
        get_stop_times = st['filter_stop_times']

        num_inter = len(set(valid_start_times).intersection(get_start_times))
        self.assertEquals(num_inter, len(valid_start_times))
        num_inter = len(set(valid_stop_times).intersection(get_stop_times))
        self.assertEquals(num_inter, len(valid_stop_times))

    def test_LastTrace(self):
        """ 4) Checking to see if the device calculates correctly. """
        start_times = [53.0]
        stop_times = [54.0]
        spike_times = np.array([5., 5.9, 6.0, 7., 9., 9.5, 10.0])

        nest.ResetKernel()
        nest.SetKernelStatus({'local_num_threads': 1, 'resolution': 0.1})
        lpfsd = nest.Create('lowpassfilter_spike_detector', 1,
                            params={'filter_start_times': start_times,
                                    'filter_stop_times': stop_times,
                                    'filter_report_interval': 1.})

        spikeGen = nest.Create('spike_generator', 1,
                               params={'spike_times': spike_times})
        nest.Connect(spikeGen, lpfsd, syn_spec={'delay': 1.5})
        nest.Simulate(55.6)

        traces = nest.GetStatus(lpfsd, 'filter_events')[0]['filter_values']
        # One can calculate the result numerically.
        # This line checks against result calculated on pen and paper
        device_trace = traces[len(traces) - 1]
        calcul_trace = 0.04959474
        self.assertAlmostEquals(device_trace, calcul_trace)

        reporting_times = np.array([54.])
        senders = np.ones_like(spike_times).tolist()
        # This checks against result calculated using a python script
        python_trace = lowpass_filter_python(senders,
                                             spike_times,
                                             reporting_times)['ktrace_values']
        self.assertAlmostEquals(device_trace, python_trace)

    def test_SegmentedSimulation(self):
        """ 5) Checking to see if the result is correct if simulation is
               stopped and is continued many times."""

        # checking it with multiple intervals
        for report_interval in range(1, 6, 1):
            # checking each interval with multiple min_delay values
            for min_delay in range(1, 52, 1):
                start_times = [0.0]
                stop_times = [54.0]
                spike_times = np.array([5., 5.9, 6.0, 7., 9., 9.5, 10.0])
                nest.ResetKernel()
                nest.SetKernelStatus({'local_num_threads': 1,
                                      'resolution': 0.1})
                lpfsd = nest.Create('lowpassfilter_spike_detector', 1,
                                    params={'filter_start_times': start_times,
                                            'filter_stop_times': stop_times,
                                            'filter_report_interval':
                                            report_interval * 1.0})

                spikeGen = nest.Create('spike_generator', 1,
                                       params={'spike_times': spike_times})
                nest.Connect(spikeGen, lpfsd,
                             syn_spec={'delay': min_delay * 0.1})
                nest.Simulate(0.6)
                nest.Simulate(0.3)
                nest.Simulate(1.2)
                nest.Simulate(1.6)
                nest.Simulate(1.5)
                nest.Simulate(3.6)
                nest.Simulate(0.8)
                nest.Simulate(0.6)
                nest.Simulate(1.3)
                nest.Simulate(7.6)
                nest.Simulate(4.5)
                nest.Simulate(0.2)
                nest.Simulate(1.3)
                nest.Simulate(5.6)
                nest.Simulate(3.9)
                nest.Simulate(1.5)
                nest.Simulate(0.2)
                nest.Simulate(1.3)
                nest.Simulate(3.6)
                nest.Simulate(0.8)
                nest.Simulate(0.6)
                nest.Simulate(0.2)
                nest.Simulate(1.3)
                nest.Simulate(9.6)
                nest.Simulate(3.9)

                device_result = nest.GetStatus(lpfsd, 'filter_events')[0]
                device_traces = device_result['filter_values']
                reporting_times = np.arange(report_interval,
                                            54.0 + report_interval,
                                            report_interval)
                senders = np.ones_like(spike_times).tolist()
                # This checks against result calculated using a python script
                python_resul = lowpass_filter_python(senders,
                                                     spike_times,
                                                     reporting_times)
                python_traces = python_resul['ktrace_values']
                for i in range(len(device_traces)):
                    self.assertAlmostEquals(device_traces[i], python_traces[i])

    def test_WithLocalAndGlobalNodes(self):
        """ 6) Checking to see if the result is correct if simulation includes
               both local and global nodes, and multiple threads. """

        start_times = [53.0]
        stop_times = [54.0]
        report_interval = 1.0

        for threads in range(1, 3, 1):
            spike_times = np.array([5., 5.9, 6.0, 7., 9., 9.5, 10.0])
            nest.ResetKernel()
            nest.SetKernelStatus({'local_num_threads': threads,
                                  'resolution': 0.1})
            lpfsd = nest.Create('lowpassfilter_spike_detector', 1,
                                params={'filter_start_times': start_times,
                                        'filter_stop_times': stop_times,
                                        'filter_report_interval':
                                        report_interval,
                                        'record_spikes': True})

            spikeGen = nest.Create('spike_generator',
                                   1, params={'spike_times': spike_times})
            nest.Connect(spikeGen, lpfsd, syn_spec={'delay': 1.5})

            neuron = nest.Create('iaf_psc_exp', params={'I_e': 1000.0})
            nest.Connect(neuron, lpfsd, syn_spec={'delay': 1.5})

            nest.Simulate(55.6)

            filter_events = nest.GetStatus(lpfsd, 'filter_events')[0]
            device_traces = filter_events['filter_values']
            device_senders = filter_events['senders']

            # Checking the trace value for the spike generator
            spikeGen_id = nest.GetStatus(spikeGen)[0]['global_id']
            device_spikegen_trace = device_traces[
                                    np.where(device_senders == spikeGen_id)]
            reporting_times = np.array([54.])
            senders = np.ones_like(spike_times).tolist()
            # This checks against result calculated using a python script
            python_resul = lowpass_filter_python(senders,
                                                 spike_times,
                                                 reporting_times)
            python_trace = python_resul['ktrace_values']
            self.assertAlmostEquals(device_spikegen_trace, python_trace)

            # Checking the trace value for the neuron
            neuron_id = nest.GetStatus(neuron)[0]['global_id']
            device_neuron_trace = device_traces[
                                         np.where(device_senders == neuron_id)]
            reporting_times = np.array([54.])
            events = nest.GetStatus(lpfsd, 'events')[0]
            event_times = events['times']
            spike_times = event_times[np.where(events['senders'] == neuron_id)]
            senders = np.ones_like(spike_times).tolist()
            # This checks against result calculated using a python script
            python_resul = lowpass_filter_python(senders,
                                                 spike_times,
                                                 reporting_times)
            python_trace = python_resul['ktrace_values']
            self.assertAlmostEquals(device_neuron_trace, python_trace)


def suite():
    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite = (unittest.TestLoader().
             loadTestsFromTestCase(LowpassFilterSpikeDetector))
    return unittest.TestSuite([suite])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run() #test
