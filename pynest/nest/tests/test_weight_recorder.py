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


@nest.check_stack
class WeightRecorderTestCase(unittest.TestCase):
    """Tests for the Weight Recorder"""

    def testSingleThread(self):
        """Weight Recorder Single Threaded"""

        nest.ResetKernel()
        nest.SetKernelStatus({"local_num_threads": 2})

        wr = nest.Create('weight_recorder')
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec",
                       {"weight_recorder": wr[0], "weight": 1.})

        sg = nest.Create("spike_generator",
                         params={"spike_times": [10., 15., 55., 70.]})
        pre = nest.Create("parrot_neuron", 2)
        post = nest.Create("parrot_neuron", 2)

        nest.Connect(pre, post, syn_spec="stdp_synapse_rec")
        nest.Connect(sg, pre)

        connections = nest.GetConnections(pre, post)

        weights = [1.]
        for i in range(100):
            nest.Simulate(1)
            weights.append(nest.GetStatus(connections, "weight")[0])

        weights = np.round(weights, 6)
        wr_weights = np.round(nest.GetStatus(wr, "events")[0]["weights"], 6)

        self.assertTrue(all([w in weights for w in wr_weights]))

    def testMultipleThreads(self):
        """Weight Recorder Multi Threaded"""

        nest.ResetKernel()
        nest.SetKernelStatus({"local_num_threads": 2})

        wr = nest.Create('weight_recorder')
        nest.CopyModel("stdp_synapse", "stdp_synapse_rec",
                       {"weight_recorder": wr[0], "weight": 1.})

        sg = nest.Create("spike_generator",
                         params={"spike_times": [10., 15., 55., 70.]})
        pre = nest.Create("parrot_neuron", 2)
        post = nest.Create("parrot_neuron", 2)

        nest.Connect(pre, post, syn_spec="stdp_synapse_rec")
        nest.Connect(sg, pre)

        connections = nest.GetConnections(pre, post)

        weights = [1.]
        for i in range(100):
            nest.Simulate(1)
            weights.append(nest.GetStatus(connections, "weight")[0])

        weights = np.round(weights, 6)
        wr_weights = np.round(nest.GetStatus(wr, "events")[0]["weights"], 6)

        self.assertTrue(all([w in weights for w in wr_weights]))


def suite():

    suite = unittest.TestLoader().loadTestsFromTestCase(WeightRecorderTestCase)
    return suite


if __name__ == "__main__":

    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
