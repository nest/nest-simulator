# -*- coding: utf-8 -*-
#
# test_stdp_triplet_synapse.py
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

# This script tests the stdp_triplet_synapse in NEST.

import nest
import unittest
from math import exp
import numpy as np


@nest.ll_api.check_stack
class STDPTripletConnectionTestCase(unittest.TestCase):
    """Check stdp_triplet_connection model properties."""

    def setUp(self):
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()

        # settings
        self.dendritic_delay = 1.0
        self.decay_duration = 5.0
        self.synapse_model = "stdp_triplet_synapse"
        self.syn_spec = {
            "synapse_model": self.synapse_model,
            "delay": self.dendritic_delay,
            # set receptor 1 post-synaptically, to not generate extra spikes
            "receptor_type": 1,
            "weight": 5.0,
            "tau_plus": 16.8,
            "tau_plus_triplet": 101.0,
            "Aplus": 0.1,
            "Aminus": 0.1,
            "Aplus_triplet": 0.1,
            "Aminus_triplet": 0.1,
            "Kplus": 0.0,
            "Kplus_triplet": 0.0,
            "Wmax": 100.0,
        }
        self.post_neuron_params = {
            "tau_minus": 33.7,
            "tau_minus_triplet": 125.0,
        }

        # setup basic circuit
        self.pre_neuron = nest.Create("parrot_neuron")
        self.post_neuron = nest.Create(
            "parrot_neuron", 1, params=self.post_neuron_params)
        nest.Connect(self.pre_neuron, self.post_neuron, syn_spec=self.syn_spec)

    def generateSpikes(self, neuron, times):
        """Trigger spike to given neuron at specified times."""
        delay = 1.
        gen = nest.Create("spike_generator", 1, {
                          "spike_times": [t - delay for t in times]})
        nest.Connect(gen, neuron, syn_spec={"delay": delay})

    def status(self, which):
        """Get synapse parameter status."""
        stats = nest.GetConnections(
            self.pre_neuron, synapse_model=self.synapse_model)
        return stats.get(which) if len(stats) == 1 else stats.get(which)[0]

    def decay(self, time, Kplus, Kplus_triplet, Kminus, Kminus_triplet):
        """Decay variables."""
        Kplus *= exp(-time / self.syn_spec["tau_plus"])
        Kplus_triplet *= exp(-time / self.syn_spec["tau_plus_triplet"])
        Kminus *= exp(-time / self.post_neuron_params["tau_minus"])
        Kminus_triplet *= exp(-time /
                              self.post_neuron_params["tau_minus_triplet"])
        return (Kplus, Kplus_triplet, Kminus, Kminus_triplet)

    def facilitate(self, w, Kplus, Kminus_triplet):
        """Facilitate weight."""
        Wmax = self.status("Wmax")
        return np.sign(Wmax) * (abs(w) + Kplus * (
            self.syn_spec["Aplus"] +
            self.syn_spec["Aplus_triplet"] * Kminus_triplet)
        )

    def depress(self, w, Kminus, Kplus_triplet):
        """Depress weight."""
        Wmax = self.status("Wmax")
        return np.sign(Wmax) * (abs(w) - Kminus * (
            self.syn_spec["Aminus"] +
            self.syn_spec["Aminus_triplet"] * Kplus_triplet)
        )

    def assertAlmostEqualDetailed(self, expected, given, message):
        """Improve assetAlmostEqual with detailed message."""
        messageWithValues = "%s (expected: `%s` was: `%s`" % (
            message, str(expected), str(given))
        self.assertAlmostEqual(given, expected, msg=messageWithValues)

    def test_badPropertiesSetupsThrowExceptions(self):
        """Check that exceptions are thrown when setting bad parameters."""

        def setupProperty(property):
            bad_syn_spec = self.syn_spec.copy()
            bad_syn_spec.update(property)
            nest.Connect(self.pre_neuron, self.post_neuron,
                         syn_spec=bad_syn_spec)

        def badPropertyWith(content, parameters):
            self.assertRaisesRegexp(
                nest.kernel.NESTError, "BadProperty(.+)" + content,
                setupProperty, parameters
            )

        badPropertyWith("Kplus", {"Kplus": -1.0})
        badPropertyWith("Kplus_triplet", {"Kplus_triplet": -1.0})

    def test_varsZeroAtStart(self):
        """Check that pre and post-synaptic variables are zero at start."""
        self.assertAlmostEqualDetailed(
            0.0, self.status("Kplus"), "Kplus should be zero")
        self.assertAlmostEqualDetailed(0.0, self.status(
            "Kplus_triplet"), "Kplus_triplet should be zero")

    def test_preVarsIncreaseWithPreSpike(self):
        """Check that pre-synaptic variables (Kplus, Kplus_triplet) increase
        after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [2.0])

        Kplus = self.status("Kplus")
        Kplus_triplet = self.status("Kplus_triplet")

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(
            Kplus + 1.0,
            self.status("Kplus"),
            "Kplus should have increased by 1")
        self.assertAlmostEqualDetailed(
            Kplus_triplet + 1.0,
            self.status("Kplus_triplet"),
            "Kplus_triplet should have increased by 1")

    def test_preVarsDecayAfterPreSpike(self):
        """Check that pre-synaptic variables (Kplus, Kplus_triplet) decay
        after each pre-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [2.0])
        # trigger computation
        self.generateSpikes(self.pre_neuron, [2.0 + self.decay_duration])

        (Kplus, Kplus_triplet, _, _) = self.decay(
            self.decay_duration, 1.0, 1.0, 0.0, 0.0)
        Kplus += 1.0
        Kplus_triplet += 1.0

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(
            Kplus, self.status("Kplus"), "Kplus should have decay")
        self.assertAlmostEqualDetailed(Kplus_triplet, self.status(
            "Kplus_triplet"), "Kplus_triplet should have decay")

    def test_preVarsDecayAfterPostSpike(self):
        """Check that pre-synaptic variables (Kplus, Kplus_triplet) decay
        after each post-synaptic spike."""

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.post_neuron, [3.0, 4.0])
        # trigger computation
        self.generateSpikes(self.pre_neuron, [2.0 + self.decay_duration])

        (Kplus, Kplus_triplet, _, _) = self.decay(
            self.decay_duration, 1.0, 1.0, 0.0, 0.0)
        Kplus += 1.0
        Kplus_triplet += 1.0

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(
            Kplus, self.status("Kplus"), "Kplus should have decay")
        self.assertAlmostEqualDetailed(Kplus_triplet, self.status(
            "Kplus_triplet"), "Kplus_triplet should have decay")

    def test_weightChangeWhenPrePostSpikes(self):
        """Check that weight changes whenever a pre-post spike pair happen."""

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.post_neuron, [4.0])
        self.generateSpikes(self.pre_neuron, [6.0])  # trigger computation

        Kplus = self.status("Kplus")
        Kplus_triplet = self.status("Kplus_triplet")
        Kminus = 0.0
        Kminus_triplet = 0.0
        weight = self.status("weight")
        Wmax = self.status("Wmax")

        (Kplus, Kplus_triplet, Kminus, Kminus_triplet) = self.decay(
            2.0, Kplus, Kplus_triplet, Kminus, Kminus_triplet)
        weight = self.depress(weight, Kminus, Kplus_triplet)
        Kplus += 1.0
        Kplus_triplet += 1.0

        (Kplus, Kplus_triplet, Kminus, Kminus_triplet) = self.decay(
            2.0 + self.dendritic_delay, Kplus, Kplus_triplet,
            Kminus, Kminus_triplet
        )
        weight = self.facilitate(weight, Kplus, Kminus_triplet)
        Kminus += 1.0
        Kminus_triplet += 1.0

        (Kplus, Kplus_triplet, Kminus, Kminus_triplet) = self.decay(
            2.0 - self.dendritic_delay, Kplus, Kplus_triplet,
            Kminus, Kminus_triplet
        )
        weight = self.depress(weight, Kminus, Kplus_triplet)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(weight, self.status(
            "weight"), "weight should have decreased")

    def test_weightChangeWhenPrePostPreSpikes(self):
        """Check that weight changes whenever a pre-post-pre spike triplet
        happen."""

        self.generateSpikes(self.pre_neuron, [2.0, 6.0])
        self.generateSpikes(self.post_neuron, [4.0])
        self.generateSpikes(self.pre_neuron, [8.0])  # trigger computation

        Kplus = self.status("Kplus")
        Kplus_triplet = self.status("Kplus_triplet")
        Kminus = 0.0
        Kminus_triplet = 0.0
        weight = self.status("weight")
        Wmax = self.status("Wmax")

        (Kplus, Kplus_triplet, Kminus, Kminus_triplet) = self.decay(
            2.0, Kplus, Kplus_triplet, Kminus, Kminus_triplet)
        weight = self.depress(weight, Kminus, Kplus_triplet)
        Kplus += 1.0
        Kplus_triplet += 1.0

        (Kplus, Kplus_triplet, Kminus, Kminus_triplet) = self.decay(
            2.0 + self.dendritic_delay, Kplus, Kplus_triplet,
            Kminus, Kminus_triplet
        )

        weight = self.facilitate(weight, Kplus, Kminus_triplet)
        Kminus += 1.0
        Kminus_triplet += 1.0

        (Kplus, Kplus_triplet, Kminus, Kminus_triplet) = self.decay(
            2.0 - self.dendritic_delay, Kplus, Kplus_triplet,
            Kminus, Kminus_triplet
        )
        weight = self.depress(weight, Kminus, Kplus_triplet)
        Kplus += 1.0
        Kplus_triplet += 1.0

        (Kplus, Kplus_triplet, Kminus, Kminus_triplet) = self.decay(
            2.0, Kplus, Kplus_triplet, Kminus, Kminus_triplet)
        weight = self.depress(weight, Kminus, Kplus_triplet)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(weight, self.status(
            "weight"), "weight should have decreased")

    def test_maxWeightStaturatesWeight(self):
        """Check that setting maximum weight property keep weight limited."""

        limited_weight = self.status("weight") + 1e-10
        limited_syn_spec = self.syn_spec.copy()
        limited_syn_spec.update({"Wmax": limited_weight})
        nest.Connect(self.pre_neuron, self.post_neuron,
                     syn_spec=limited_syn_spec)

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.pre_neuron, [3.0])  # trigger computation

        nest.Simulate(20.0)
        print(limited_weight)
        print(self.status('weight'))
        self.assertAlmostEqualDetailed(limited_weight, self.status(
            "weight"), "weight should have been limited")


@nest.ll_api.check_stack
class STDPTripletInhTestCase(STDPTripletConnectionTestCase):

    def setUp(self):
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()

        # settings
        self.dendritic_delay = 1.0
        self.decay_duration = 5.0
        self.synapse_model = "stdp_triplet_synapse"
        self.syn_spec = {
            "synapse_model": self.synapse_model,
            "delay": self.dendritic_delay,
            # set receptor 1 post-synaptically, to not generate extra spikes
            "receptor_type": 1,
            "weight": -5.0,
            "tau_plus": 16.8,
            "tau_plus_triplet": 101.0,
            "Aplus": 0.1,
            "Aminus": 0.1,
            "Aplus_triplet": 0.1,
            "Aminus_triplet": 0.1,
            "Kplus": 0.0,
            "Kplus_triplet": 0.0,
            "Wmax": -100.0,
        }
        self.post_neuron_params = {
            "tau_minus": 33.7,
            "tau_minus_triplet": 125.0,
        }

        # setup basic circuit
        self.pre_neuron = nest.Create("parrot_neuron")
        self.post_neuron = nest.Create("parrot_neuron", 1,
                                       params=self.post_neuron_params)
        nest.Connect(self.pre_neuron, self.post_neuron, syn_spec=self.syn_spec)


def suite_inh():
    return unittest.makeSuite(STDPTripletInhTestCase, "test")


def suite():
    return unittest.makeSuite(STDPTripletConnectionTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
    runner.run(suite_inh())


if __name__ == "__main__":
    run()
