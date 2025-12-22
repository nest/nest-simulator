# -*- coding: utf-8 -*-
#
# test_weight_recorder.py
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
Test of events
"""

import unittest

import nest
import numpy as np

HAVE_GSL = nest.build_info["have_gsl"]
HAVE_THREADS = nest.build_info["have_threads"]


@unittest.skipIf(not HAVE_THREADS, "NEST was compiled without multi-threading")
class WeightRecorderTestCase(unittest.TestCase):
    """Tests for the Weight Recorder"""

    def is_subset(self, a, b, places=6, msg=None):
        a = np.round(a, places)
        b = np.round(b, places)

        if set(a) <= set(b):
            return

        msg = self._formatMessage(
            msg,
            """List A is not subset of list B
                                  and/or the items are not equal within
                                  a certain range of precision.
                                  List A is {0} and list B is {1}""".format(
                a, b
            ),
        )
        raise self.failureException(msg)

    def testSingleThread(self):
        """Weight Recorder Single Threaded"""

        nest.ResetKernel()
        nest.local_num_threads = 1

        wr = nest.Create("weight_recorder")
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec", {"weight_recorder": wr, "weight": 1.0})

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(pre, post, syn_spec="stdp_synapse_rec")
        nest.Connect(sg, pre)

        connections = nest.GetConnections(pre, post)

        weights = np.array([])
        for i in range(100):
            nest.Simulate(1)
            weights = np.append(weights, connections.weight)

        wr_weights = wr.events["weights"]

        self.addTypeEqualityFunc(type(wr_weights), self.is_subset)
        self.assertEqual(wr_weights, weights)

    @unittest.skipIf(not HAVE_THREADS, "NEST was compiled without multi-threading")
    def testMultipleThreads(self):
        """Weight Recorder Multi Threaded"""

        nest.ResetKernel()
        nest.local_num_threads = 2

        wr = nest.Create("weight_recorder")
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec", {"weight_recorder": wr, "weight": 1.0})

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(pre, post, syn_spec="stdp_synapse_rec")
        nest.Connect(sg, pre)

        connections = nest.GetConnections(pre, post)

        weights = np.array([])
        for i in range(100):
            nest.Simulate(1)
            weights = np.append(weights, connections.weight)

        wr_weights = wr.events["weights"]

        self.addTypeEqualityFunc(type(wr_weights), self.is_subset)
        self.assertEqual(wr_weights, weights)

    def testDefinedSenders(self):
        """Weight Recorder Defined Subset Of Senders"""

        nest.ResetKernel()
        nest.local_num_threads = 1

        wr = nest.Create("weight_recorder")
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec", {"weight_recorder": wr, "weight": 1.0})

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(pre, post, syn_spec="stdp_synapse_rec")
        nest.Connect(sg, pre)

        wr.senders = pre[:3]
        connections = nest.GetConnections(pre[:3], post)

        senders = np.array([])
        for i in range(100):
            nest.Simulate(1)
            senders = np.append(senders, connections.source)

        wr_senders = wr.events["senders"]

        self.addTypeEqualityFunc(type(wr_senders), self.is_subset)
        self.assertEqual(wr_senders, senders)

    def testDefinedTargets(self):
        """Weight Recorder Defined Subset Of Targets"""

        nest.ResetKernel()
        nest.local_num_threads = 1

        wr = nest.Create("weight_recorder")
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec", {"weight_recorder": wr, "weight": 1.0})

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(pre, post, syn_spec="stdp_synapse_rec")
        nest.Connect(sg, pre)

        wr.targets = post[:3]
        connections = nest.GetConnections(pre, post[:3])

        targets = np.array([])
        for i in range(100):
            nest.Simulate(1)
            targets = np.append(targets, connections.target)

        wr_targets = wr.events["targets"]

        self.addTypeEqualityFunc(type(wr_targets), self.is_subset)
        self.assertEqual(wr_targets, targets)

    def testDefinedTargetsAndSenders(self):
        """Weight Recorder Defined Subset Of Targets and Senders"""

        nest.ResetKernel()
        nest.local_num_threads = 1

        wr = nest.Create("weight_recorder")
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec", {"weight_recorder": wr, "weight": 1.0})

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(pre, post, syn_spec="stdp_synapse_rec")
        nest.Connect(sg, pre)

        wr.set({"senders": pre[1:3], "targets": post[:3]})

        # simulate before GetConnections
        # as order of connections changes at beginning of simulation (sorting)
        nest.Simulate(1)

        connections = nest.GetConnections(pre[1:3], post[:3])
        targets = np.array([])
        for i in range(1):
            nest.Simulate(1)
            targets = np.append(targets, connections.target)

        wr_targets = wr.events["targets"]

        self.addTypeEqualityFunc(type(wr_targets), self.is_subset)
        self.assertEqual(wr_targets, targets)

    def test_senders_and_targets(self):
        """
        Senders and targets for weight recorder works as NodeCollection and list.

        NOTE: This test was moved from test_NodeCollection.py and may overlap
        with test already present in this test suite. If that is the case,
        consider to just drop this test.
        """

        nest.ResetKernel()

        wr = nest.Create("weight_recorder")
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        # Senders and targets lists empty initially
        assert wr.senders.tolist() == []
        assert wr.targets.tolist() == []

        wr.senders = pre[1:3]
        wr.targets = post[3:]

        gss = wr.senders
        gst = wr.targets

        assert gss.tolist() == [3, 4]
        assert gst.tolist() == [10, 11]

        wr.senders = nest.NodeCollection([2, 6])
        wr.targets = nest.NodeCollection([8, 9])

        gss = wr.senders
        gst = wr.targets

        assert gss.tolist() == [2, 6]
        assert gst.tolist() == [8, 9]

    def testMultapses(self):
        """Weight Recorder Multapses"""

        nest.ResetKernel()
        nest.local_num_threads = 2

        wr = nest.Create("weight_recorder")
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec", {"weight_recorder": wr, "weight": 1.0})

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(pre, post, "one_to_one", syn_spec="stdp_synapse_rec")
        nest.Connect(pre, post, "one_to_one", syn_spec="stdp_synapse_rec")
        nest.Connect(sg, pre)

        # simulate before GetConnections
        # as order of connections changes at beginning of simulation (sorting)
        nest.Simulate(1)

        conn = nest.GetConnections(pre, post)
        conn_dict = conn.get(["source", "target", "port"])

        connections = list(zip(conn_dict["source"], conn_dict["target"], conn_dict["port"]))

        nest.Simulate(100)

        wr_events = wr.events
        senders = wr_events["senders"]
        targets = wr_events["targets"]
        ports = wr_events["ports"]
        ids = list(zip(senders, targets, ports))

        # create an array of object dtype to use np.unique to get
        # unique ids
        unique_ids = np.empty(len(ids), dtype=object)
        for i, v in enumerate(ids):
            unique_ids[i] = v
        unique_ids = np.unique(unique_ids)

        self.assertEqual(sorted(unique_ids), sorted(connections))

    @unittest.skipIf(not HAVE_GSL, "GSL is not available")
    def testRPorts(self):
        """Weight Recorder rports"""

        nest.ResetKernel()
        nest.local_num_threads = 1

        wr = nest.Create("weight_recorder")

        nest.CopyModel("stdp_synapse", "stdp_synapse_rec_0", {"weight_recorder": wr, "weight": 1.0, "receptor_type": 1})

        nest.CopyModel("stdp_synapse", "stdp_synapse_rec_1", {"weight_recorder": wr, "weight": 1.0, "receptor_type": 2})

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})

        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create(
            "aeif_cond_alpha_multisynapse", 5, {"V_th": -69.9, "tau_syn": [20.0, 30.0], "E_rev": [0.0, 0.0]}
        )

        nest.Connect(pre, post, "one_to_one", syn_spec="stdp_synapse_rec_0")
        nest.Connect(pre, post, "one_to_one", syn_spec="stdp_synapse_rec_1")
        nest.Connect(sg, pre)

        connections = nest.GetConnections(pre, post)
        receptors = connections.receptor
        sources = connections.source
        targets = connections.target
        connections = [(sources[i], targets[i], receptors[i]) for i in range(len(connections))]

        nest.Simulate(100)

        wr_events = wr.events
        senders = wr_events["senders"]
        targets = wr_events["targets"]
        rports = wr_events["receptors"]
        ids = list(zip(senders, targets, rports))

        # create an array of object dtype to use np.unique to get
        # unique ids
        unique_ids = np.empty(len(ids), dtype=object)
        for i, v in enumerate(ids):
            unique_ids[i] = v
        unique_ids = np.unique(unique_ids)

        # should be 10 connections
        self.assertEqual(sorted(unique_ids), sorted(connections))


def suite():
    suite = unittest.TestLoader().loadTestsFromTestCase(WeightRecorderTestCase)
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
