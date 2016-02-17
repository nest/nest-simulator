# -*- coding: utf-8 -*-
#
# test_stdp_symmetric_synapse.py
#
# This file is part of NEST.
#
# Copyright (C) 2016 The NEST Initiative
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

# This script tests the stdp_symmetric_synapse in NEST.

import nest
import unittest
from math import exp


@nest.check_stack
class STDPSymmetricConnectionTestCase(unittest.TestCase):

    """Check stdp_symmetric_connection model properties."""

    def setUp(self):
        """Set up the test."""
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()

        # settings
        self.dendritic_delay = 1.0
        self.decay_duration = 5.0
        self.synapse_model = "stdp_symmetric_synapse"
        self.syn_spec = {
            "model": self.synapse_model,
            "delay": self.dendritic_delay,
            "weight": 5.0,
            "eta": 0.001,
            "alpha": 0.1,
            "tau": 20.,
            "Kplus": 0.0,
            "Wmax": 100.0,
        }

        # setup basic circuit
        self.pre_neuron = nest.Create("parrot_neuron")
        self.post_neuron = nest.Create("parrot_neuron")
        nest.Connect(self.pre_neuron, self.post_neuron, syn_spec=self.syn_spec)

    def generateSpikes(self, neuron, times):
        """Trigger spike to given neuron at specified times."""
        delay = 1.
        gen = nest.Create("spike_generator", 1,
                          {"spike_times": [t-delay for t in times]})
        nest.Connect(gen, neuron, syn_spec={"delay": delay})

    def status(self, which):
        """Get synapse parameter status."""
        stats = nest.GetConnections(self.pre_neuron,
                                    synapse_model=self.synapse_model)
        return nest.GetStatus(stats, [which])[0][0]

    def decay(self, time, Kplus):
        """Decay variables."""
        Kplus *= exp(- time / self.syn_spec["tau"])
        return Kplus

    def facilitate(self, w, Kplus):
        """Facilitate weight."""
        return (w/self.syn_spec['Wmax'] + (self.syn_spec['eta'] * Kplus))

    def depress(self, w):
        """Depress weight."""
        return (w/self.syn_spec['Wmax'] - (self.syn_spec['alpha'] *
                                           self.syn_spec['eta']))

    def assertAlmostEqualDetailed(self, expected, given, message):
        """Improve assetAlmostEqual with detailed message."""
        messageWithValues = ("%s (expected: `%s` was: `%s`" % (message,
                                                               str(expected),
                                                               str(given)))
        self.assertAlmostEqual(given, expected, msg=messageWithValues)

    def test_badPropertiesSetupsThrowExceptions(self):
        """Check that exceptions are thrown when setting bad parameters."""
        def setupProperty(property):
            bad_syn_spec = self.syn_spec.copy()
            bad_syn_spec.update(property)
            nest.Connect(self.pre_neuron, self.post_neuron,
                         syn_spec=bad_syn_spec)

        def badPropertyWith(content, parameters):
            self.assertRaisesRegexp(nest.NESTError, "BadProperty(.+)" +
                                    content, setupProperty, parameters)

        badPropertyWith("Kplus", {"Kplus": -1.0})

    def test_varsZeroAtStart(self):
        """Check that pre and post-synaptic variables are zero at start."""
        self.assertAlmostEqualDetailed(0.0, self.status("Kplus"),
                                       "Kplus should be zero")

    def test_preVarsIncreaseWithPreSpike(self):
        """Check that pre-synaptic variable, Kplus increase after each pre-synaptic spike."""
        self.generateSpikes(self.pre_neuron, [2.0])

        Kplus = self.status("Kplus")

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(Kplus + 1.0, self.status("Kplus"),
                                       "Kplus should have increased by 1")

    def test_preVarsDecayAfterPreSpike(self):
        """Check that pre-synaptic variables Kplus decay after each pre-synaptic spike."""
        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.pre_neuron,
                            [2.0 + self.decay_duration])  # trigger computation

        Kplus = self.decay(self.decay_duration, 1.0)
        Kplus += 1.0

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(Kplus, self.status("Kplus"),
                                       "Kplus should have decay")

    def test_preVarsDecayAfterPostSpike(self):
        """Check that pre-synaptic variables Kplus decay after each post-synaptic spike."""
        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.post_neuron, [3.0, 4.0])
        self.generateSpikes(self.pre_neuron,
                            [2.0 + self.decay_duration])  # trigger computation

        Kplus = self.decay(self.decay_duration, 1.0)
        Kplus += 1.0

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(Kplus, self.status("Kplus"),
                                       "Kplus should have decay")

    def test_weightChangeWhenPrePostSpikes(self):
        """Check that weight changes whenever a pre-post spike pair happen."""
        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.post_neuron, [4.0])
        self.generateSpikes(self.pre_neuron, [6.0])  # trigger computation

        weight = self.syn_spec['weight']
        Kplus = self.decay(self.decay_duration, 1.0)

        Kplus = self.decay(2.0, Kplus)
        weight = self.facilitate(weight, Kplus)
        weight = self.depress(weight)
        Kplus += 1.0

        Kplus = self.decay(2.0 + self.dendritic_delay, Kplus)
        weight = self.facilitate(weight, Kplus)

        nest.Simulate(20.0)
        self.assertAlmostEqualDetailed(weight, self.status("weight"),
                                       "weight should have decreased")

    def test_maxWeightStaturatesWeight(self):
        """Check that setting maximum weight property keep weight limited."""
        limited_weight = self.status("weight") + 1e-10
        limited_syn_spec = self.syn_spec
        limited_syn_spec['Wmax'] = limited_weight
        nest.Connect(self.pre_neuron, self.post_neuron,
                     syn_spec=limited_syn_spec)

        self.generateSpikes(self.pre_neuron, [2.0])
        self.generateSpikes(self.post_neuron, [3.0])
        self.generateSpikes(self.pre_neuron, [4.0])  # trigger computation

        nest.Simulate(5.0)
        self.assertAlmostEqualDetailed(limited_weight,
                                       self.status("weight"),
                                       "weight should have been limited")


def suite():
    return unittest.makeSuite(STDPSymmetricConnectionTestCase, "test")


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())

if __name__ == "__main__":
    run()
