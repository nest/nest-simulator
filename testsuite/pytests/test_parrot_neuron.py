# -*- coding: utf-8 -*-
#
# test_parrot_neuron.py
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
# See test_parrot_neuron_ps.py for an equivalent test of the precise parrot.

import math
import unittest

import nest


@nest.ll_api.check_stack
class ParrotNeuronTestCase(unittest.TestCase):
    """Check parrot_neuron spike repetition properties"""

    def setUp(self):
        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()

        # set up source spike generator, as well as parrot neurons
        self.spike_time = 1.0
        self.delay = 0.2
        self.source = nest.Create("spike_generator", 1, {"spike_times": [self.spike_time]})
        self.parrot = nest.Create("parrot_neuron")
        self.spikes = nest.Create("spike_recorder")

        # record source and parrot spikes
        nest.Connect(nest.AllToAll(self.source, self.spikes))
        nest.Connect(nest.AllToAll(self.parrot, self.spikes))

    def test_ParrotNeuronRepeatSpike(self):
        """Check parrot_neuron repeats spikes on port 0"""

        # connect with arbitrary delay
        nest.Connect(nest.AllToAll(self.source, self.parrot, syn_spec=nest.synapsemodels.static(delay=self.delay)))
        nest.Simulate(self.spike_time + 2 * self.delay)

        # get spike from parrot neuron
        events = nest.GetStatus(self.spikes)[0]["events"]
        post_time = events["times"][events["senders"] == self.parrot[0].get("global_id")]

        # assert spike was repeated at correct time
        assert post_time, "Parrot neuron failed to repeat spike."
        assert self.spike_time + self.delay == post_time, "Parrot neuron repeated spike at wrong delay"

    def test_ParrotNeuronIgnoreSpike(self):
        """Check parrot_neuron ignores spikes on port 1"""

        # connect with arbitrary delay to port 1
        nest.Connect(
            nest.AllToAll(
                self.source, self.parrot, syn_spec=nest.synapsemodels.static(receptor_type=1, delay=self.delay)
            )
        )
        nest.Simulate(self.spike_time + 2.0 * self.delay)

        # get spike from parrot neuron, assert it was ignored
        events = nest.GetStatus(self.spikes)[0]["events"]
        post_time = events["times"][events["senders"] == self.parrot.get("global_id")]
        assert len(post_time) == 0, "Parrot neuron failed to ignore spike arriving on port 1"

    def test_ParrotNeuronOutgoingMultiplicity(self):
        """
        Check parrot_neuron correctly repeats multiple spikes

        The parrot_neuron receives two spikes in a single time step.
        We check that both spikes are forwarded to the spike_recorder.
        """

        # connect twice
        nest.Connect(nest.AllToAll(self.source, self.parrot, syn_spec=nest.synapsemodels.static(delay=self.delay)))
        nest.Connect(nest.AllToAll(self.source, self.parrot, syn_spec=nest.synapsemodels.static(delay=self.delay)))
        nest.Simulate(self.spike_time + 2.0 * self.delay)

        # get spikes from parrot neuron, assert two were transmitted
        events = nest.GetStatus(self.spikes)[0]["events"]
        post_times = events["times"][events["senders"] == self.parrot.get("global_id")]
        assert (
            len(post_times) == 2 and post_times[0] == post_times[1]
        ), "Parrot neuron failed to correctly repeat multiple spikes."


@nest.ll_api.check_stack
class ParrotNeuronPoissonTestCase(unittest.TestCase):
    """Check parrot_neuron spike repetition properties"""

    def test_ParrotNeuronIncomingMultiplicity(self):
        """
        Check parrot_neuron heeds multiplicity information in incoming spikes.

        This test relies on the fact that poisson_generator transmits
        multiple spikes during a time step using multiplicity, and that
        these spikes are delivered directly, i.e., without multiplicity-
        unrolling in send_remote().

        We create a high-rate poisson_generator. If parrot_neuron
        ignored multiplicity, it would only transmit one spike per time
        step. We chain two parrot_neurons to check against any loss.
        """

        # set up source spike generator, as well as parrot neurons
        resolution = 0.1  # ms
        rate = 1000000.0  # spikes / s
        delay = 1.0  # ms
        t_base = 1000.0  # ms
        t_sim = t_base + 3 * delay  # after t_sim, spikes from t_base arrived
        spikes_expected = rate * t_base / 1000.0
        spikes_std = math.sqrt(spikes_expected)

        # if the test is to be meaningful we must expect signficantly more
        # spikes than time steps
        assert spikes_expected - 3 * spikes_std > 10.0 * t_sim / resolution, "Internal inconsistency: too few spikes."

        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.resolution = resolution
        nest.rng_seed = 123

        source = nest.Create("poisson_generator", params={"rate": rate})
        parrots = nest.Create("parrot_neuron", 2)
        spike_rec = nest.Create("spike_recorder")

        nest.Connect(nest.AllToAll(source, parrots[:1], syn_spec=nest.synapsemodels.static(delay=delay)))
        nest.Connect(nest.AllToAll(parrots[:1], parrots[1:], syn_spec=nest.synapsemodels.static(delay=delay)))
        nest.Connect(nest.AllToAll(parrots[1:], spike_rec))

        nest.Simulate(t_sim)

        n_spikes = nest.GetStatus(spike_rec)[0]["n_events"]
        assert n_spikes > spikes_expected - 3 * spikes_std, "parrot_neuron loses spikes."
        assert n_spikes < spikes_expected + 3 * spikes_std, "parrot_neuron adds spikes."


@nest.ll_api.check_stack
class ParrotNeuronSTDPTestCase(unittest.TestCase):
    """
    Check STDP protocol between two parrot_neurons connected by a stdp_synapse.
    Exact pre- and postsynaptic spike times are set by spike_generators
    connected to each parrot neuron. Additional spikes sent through the
    stdp_synapse are explicitly ignored in the postsynaptic parrot_neuron
    by setting the stdp_synapse to connect to port 1.
    """

    def run_protocol(self, dt):
        """Set up a network with pre-post spike pairings
        with t_post - t_pre = dt"""

        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()

        # set pre and postsynaptic spike times
        delay = 1.0  # delay for connections
        dspike = 100.0  # ISI

        # set the correct real spike times for generators (correcting for
        # delays)
        pre_times = [100.0, 100.0 + dspike]
        post_times = [k + dt for k in pre_times]

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {"spike_times": pre_times})
        post_spikes = nest.Create("spike_generator", 1, {"spike_times": post_times})

        # create parrot neurons and connect spike_generators
        pre_parrot = nest.Create("parrot_neuron", 1)
        post_parrot = nest.Create("parrot_neuron", 1)

        nest.Connect(nest.AllToAll(pre_spikes, pre_parrot, syn_spec=nest.synapsemodels.static(delay=delay)))
        nest.Connect(nest.AllToAll(post_spikes, post_parrot, syn_spec=nest.synapsemodels.static(delay=delay)))

        # create spike recorder
        spikes = nest.Create("spike_recorder")
        nest.Connect(nest.AllToAll(pre_parrot, spikes))
        nest.Connect(nest.AllToAll(post_parrot, spikes))

        # connect both parrot neurons with a stdp synapse onto port 1
        # thereby spikes transmitted through the stdp connection are
        # not repeated postsynaptically.
        syn_spec = nest.synapsemodels.stdp(
            # set receptor 1 postsynaptically, to not generate extra spikes
            receptor_type=1,
        )
        nest.Connect(nest.OneToOne(pre_parrot, post_parrot, syn_spec=syn_spec))

        # get STDP synapse and weight before protocol
        syn = nest.GetConnections(source=pre_parrot, synapse_model="stdp_synapse")
        w_pre = syn.get("weight")

        last_time = max(pre_times[-1], post_times[-1])
        nest.Simulate(last_time + 2 * delay)

        # get weight post protocol
        w_post = syn.get("weight")

        return w_pre, w_post

    def test_ParrotNeuronSTDPProtocolPotentiation(self):
        """Check pre-post spike pairings between parrot_neurons
        increments weights."""

        dt = 10.0
        w_pre, w_post = self.run_protocol(dt)
        assert (
            w_pre < w_post
        ), "Parrot neuron STDP potentiation \
            protocol failed to elicit positive weight changes."

    def test_ParrotNeuronSTDPProtocolDepression(self):
        """Check post-pre spike pairings between parrot_neurons
        decrement weights."""

        dt = -10.0
        w_pre, w_post = self.run_protocol(dt)
        assert (
            w_pre > w_post
        ), "Parrot neuron STDP potentiation \
        protocol failed to elicit negative weight changes."


def suite():
    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite1 = unittest.TestLoader().loadTestsFromTestCase(ParrotNeuronTestCase)
    suite2 = unittest.TestLoader().loadTestsFromTestCase(ParrotNeuronPoissonTestCase)
    suite3 = unittest.TestLoader().loadTestsFromTestCase(ParrotNeuronSTDPTestCase)
    return unittest.TestSuite([suite1, suite2, suite3])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
