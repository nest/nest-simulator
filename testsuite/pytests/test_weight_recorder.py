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

import numpy as np

import nest

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")
HAVE_OPENMP = nest.ll_api.sli_func("is_threaded")


@unittest.skipIf(not HAVE_OPENMP, "NEST was compiled without multi-threading")
@nest.ll_api.check_stack
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
        stdp_syn = nest.CopyModel("stdp_synapse", weight_recorder=wr, weight=1.0)

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(nest.AllToAll(pre, post, syn_spec=stdp_syn))
        nest.Connect(nest.AllToAll(sg, pre))

        connections = nest.GetConnections(pre, post)

        weights = np.array([])
        for i in range(100):
            nest.Simulate(1)
            weights = np.append(weights, connections.get("weight"))

        wr_weights = nest.GetStatus(wr, "events")[0]["weights"]

        self.addTypeEqualityFunc(type(wr_weights), self.is_subset)
        self.assertEqual(wr_weights, weights)

    def testMultipleThreads(self):
        """Weight Recorder Multi Threaded"""

        nest.ResetKernel()
        nest.local_num_threads = 2

        wr = nest.Create("weight_recorder")
        stdp_syn = nest.CopyModel("stdp_synapse", weight_recorder=wr, weight=1.0)

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(nest.AllToAll(pre, post, syn_spec=stdp_syn))
        nest.Connect(nest.AllToAll(sg, pre))

        connections = nest.GetConnections(pre, post)

        weights = np.array([])
        for i in range(100):
            nest.Simulate(1)
            weights = np.append(weights, connections.get("weight"))

        wr_weights = nest.GetStatus(wr, "events")[0]["weights"]

        self.addTypeEqualityFunc(type(wr_weights), self.is_subset)
        self.assertEqual(wr_weights, weights)

    def testDefinedSenders(self):
        """Weight Recorder Defined Subset Of Senders"""

        nest.ResetKernel()
        nest.local_num_threads = 1

        wr = nest.Create("weight_recorder")
        stdp_syn = nest.CopyModel("stdp_synapse", weight_recorder=wr, weight=1.0)

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(nest.AllToAll(pre, post, syn_spec=stdp_syn))
        nest.Connect(nest.AllToAll(sg, pre))

        nest.BuildNetwork()

        nest.SetStatus(wr, {"senders": pre[:3]})
        connections = nest.GetConnections(pre[:3], post)

        senders = np.array([])
        for i in range(100):
            nest.Simulate(1)
            senders = np.append(senders, connections.get("source"))

        wr_senders = nest.GetStatus(wr, "events")[0]["senders"]

        self.addTypeEqualityFunc(type(wr_senders), self.is_subset)
        self.assertEqual(wr_senders, senders)

    def testDefinedTargets(self):
        """Weight Recorder Defined Subset Of Targets"""

        nest.ResetKernel()
        nest.local_num_threads = 1

        wr = nest.Create("weight_recorder")
        stdp_syn = nest.CopyModel("stdp_synapse", weight_recorder=wr, weight=1.0)

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(nest.AllToAll(pre, post, syn_spec=stdp_syn))
        nest.Connect(nest.AllToAll(sg, pre))

        nest.BuildNetwork()

        nest.SetStatus(wr, {"targets": post[:3]})
        connections = nest.GetConnections(pre, post[:3])

        targets = np.array([])
        for i in range(100):
            nest.Simulate(1)
            targets = np.append(targets, connections.get("target"))

        wr_targets = nest.GetStatus(wr, "events")[0]["targets"]

        self.addTypeEqualityFunc(type(wr_targets), self.is_subset)
        self.assertEqual(wr_targets, targets)

    def testDefinedTargetsAndSenders(self):
        """Weight Recorder Defined Subset Of Targets and Senders"""

        nest.ResetKernel()
        nest.local_num_threads = 1

        wr = nest.Create("weight_recorder")
        stdp_syn = nest.CopyModel("stdp_synapse", weight_recorder=wr, weight=1.0)

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(nest.AllToAll(pre, post, syn_spec=stdp_syn))
        nest.Connect(nest.AllToAll(sg, pre))

        nest.BuildNetwork()

        nest.SetStatus(wr, {"senders": pre[1:3], "targets": post[:3]})

        # simulate before GetConnections
        # as order of connections changes at beginning of simulation (sorting)
        nest.Simulate(1)

        connections = nest.GetConnections(pre[1:3], post[:3])
        targets = np.array([])
        for i in range(1):
            nest.Simulate(1)
            targets = np.append(targets, connections.get("target"))

        wr_targets = nest.GetStatus(wr, "events")[0]["targets"]

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

        # Senders and targets lists empty
        self.assertFalse(nest.GetStatus(wr, "senders")[0])
        self.assertFalse(nest.GetStatus(wr, "targets")[0])

        nest.SetStatus(wr, {"senders": pre[1:3], "targets": post[3:]})

        gss = nest.GetStatus(wr, "senders")[0]
        gst = nest.GetStatus(wr, "targets")[0]

        self.assertEqual(gss.tolist(), [3, 4])
        self.assertEqual(gst.tolist(), [10, 11])

        nest.SetStatus(wr, {"senders": [2, 6], "targets": [8, 9]})
        gss = nest.GetStatus(wr, "senders")[0]
        gst = nest.GetStatus(wr, "targets")[0]
        self.assertEqual(gss.tolist(), [2, 6])
        self.assertEqual(gst.tolist(), [8, 9])

    def testMultapses(self):
        """Weight Recorder Multapses"""

        nest.ResetKernel()
        nest.local_num_threads = 2

        wr = nest.Create("weight_recorder")
        stdp_syn = nest.CopyModel("stdp_synapse", weight_recorder=wr, weight=1.0)

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})
        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create("parrot_neuron", 5)

        nest.Connect(nest.OneToOne(pre, post, syn_spec=stdp_syn))
        nest.Connect(nest.OneToOne(pre, post, syn_spec=stdp_syn))
        nest.Connect(nest.AllToAll(sg, pre))

        # simulate before GetConnections
        # as order of connections changes at beginning of simulation (sorting)
        nest.Simulate(1)

        conn = nest.GetConnections(pre, post)
        conn_dict = conn.get(["source", "target", "port"])

        connections = list(zip(conn_dict["source"], conn_dict["target"], conn_dict["port"]))

        nest.Simulate(100)

        wr_events = nest.GetStatus(wr, "events")[0]
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

        stdp_syn_0 = nest.CopyModel("stdp_synapse", weight_recorder=wr, weight=1.0, receptor_type=1)
        stdp_syn_1 = nest.CopyModel("stdp_synapse", weight_recorder=wr, weight=1.0, receptor_type=2)

        sg = nest.Create("spike_generator", params={"spike_times": [10.0, 15.0, 55.0, 70.0]})

        pre = nest.Create("parrot_neuron", 5)
        post = nest.Create(
            "aeif_cond_alpha_multisynapse", 5, {"V_th": -69.9, "tau_syn": [20.0, 30.0], "E_rev": [0.0, 0.0]}
        )

        nest.Connect(nest.OneToOne(pre, post, syn_spec=stdp_syn_0))
        nest.Connect(nest.OneToOne(pre, post, syn_spec=stdp_syn_1))
        nest.Connect(nest.AllToAll(sg, pre))

        connections = nest.GetConnections(pre, post)
        receptors = connections.get("receptor")
        sources = connections.get("source")
        targets = connections.get("target")
        connections = [(sources[i], targets[i], receptors[i]) for i in range(len(connections))]

        nest.Simulate(100)

        wr_events = nest.GetStatus(wr, "events")[0]
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
