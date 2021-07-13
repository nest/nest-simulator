# -*- coding: utf-8 -*-
#
# test_parrot_neuron_ps.py
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

# This script tests the parrot_neuron_ps in NEST.
# It is very similar to test_parrot_neuron.py, but uses precise spike times.

import nest
import unittest
import math


def _round_up(simtime):
    """
    Returns simulation time rounded up to next multiple of resolution.
    """

    res = nest.GetKernelStatus('resolution')
    return res * math.ceil(float(simtime) / float(res))


@nest.ll_api.check_stack
class ParrotNeuronPSTestCase(unittest.TestCase):
    """Check parrot_neuron spike repetition properties"""

    def setUp(self):
        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()

        # set up source spike generator, as well as parrot neurons
        self.spike_time = 1.01
        self.delay = .2
        self.source = nest.Create("spike_generator", 1,
                                  {"spike_times": [self.spike_time],
                                   'precise_times': True})
        self.parrot = nest.Create('parrot_neuron_ps')
        self.spikes = nest.Create("spike_recorder")

        # record source and parrot spikes
        nest.Connect(self.source, self.spikes)
        nest.Connect(self.parrot, self.spikes)

    def test_ParrotNeuronRepeatSpike(self):
        """Check parrot_neuron repeats spikes on port 0"""

        # connect with arbitrary delay
        nest.Connect(self.source, self.parrot, syn_spec={"delay": self.delay})
        nest.Simulate(_round_up(self.spike_time + 2 * self.delay))

        # get spike from parrot neuron
        events = nest.GetStatus(self.spikes)[0]["events"]
        post_time = events['times'][
            events['senders'] == self.parrot[0].get('global_id')]

        # assert spike was repeated at correct time
        assert post_time, "Parrot neuron failed to repeat spike."
        assert self.spike_time + self.delay == post_time, \
            "Parrot neuron repeated spike at wrong delay"

    def test_ParrotNeuronIgnoreSpike(self):
        """Check parrot_neuron ignores spikes on port 1"""

        # connect with arbitrary delay to port 1
        nest.Connect(self.source, self.parrot,
                     syn_spec={"receptor_type": 1, "delay": self.delay})
        nest.Simulate(_round_up(self.spike_time + 2. * self.delay))

        # get spike from parrot neuron, assert it was ignored
        events = nest.GetStatus(self.spikes)[0]["events"]
        post_time = events['times'][
            events['senders'] == self.parrot.get('global_id')]
        assert len(post_time) == 0, \
            "Parrot neuron failed to ignore spike arriving on port 1"

    def test_ParrotNeuronOutgoingMultiplicity(self):
        """
        Check parrot_neuron correctly repeats multiple spikes

        The parrot_neuron receives two spikes in a single time step.
        We check that both spikes are forwarded to the spike_recorder.
        """

        # connect twice
        nest.Connect(self.source, self.parrot, syn_spec={"delay": self.delay})
        nest.Connect(self.source, self.parrot, syn_spec={"delay": self.delay})
        nest.Simulate(_round_up(self.spike_time + 2. * self.delay))

        # get spikes from parrot neuron, assert two were transmitted
        events = nest.GetStatus(self.spikes)[0]["events"]
        post_times = events['times'][
            events['senders'] == self.parrot.get('global_id')]
        assert len(post_times) == 2 and post_times[0] == post_times[1], \
            "Parrot neuron failed to correctly repeat multiple spikes."


@nest.ll_api.check_stack
class ParrotNeuronPSPoissonTestCase(unittest.TestCase):
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

        Note: Even though we test parrot_neuron_ps, we drive it with the
        plain poisson_generator, since only that generator uses multiplicity.
        """

        # set up source spike generator, as well as parrot neurons
        h = 0.1  # ms
        rate = 1000000.  # spikes / s
        delay = 1.    # ms
        t_base = 1000.  # ms
        t_sim = t_base + 3 * delay   # after t_sim, spikes from t_base arrived
        spikes_expected = rate * t_base / 1000.
        spikes_std = math.sqrt(spikes_expected)

        # if the test is to be meaningful we must expect signficantly more
        # spikes than time steps
        assert spikes_expected - 3 * spikes_std > 10. * t_sim / h, \
            "Internal inconsistency: too few spikes."

        nest.set_verbosity('M_WARNING')
        nest.ResetKernel()
        nest.SetKernelStatus({'resolution': h,
                              'rng_seed': 123})

        source = nest.Create('poisson_generator', params={'rate': rate})
        parrots = nest.Create('parrot_neuron_ps', 2)
        spike_rec = nest.Create('spike_recorder')

        nest.Connect(source, parrots[:1], syn_spec={'delay': delay})
        nest.Connect(parrots[:1], parrots[1:], syn_spec={'delay': delay})
        nest.Connect(parrots[1:], spike_rec)

        nest.Simulate(_round_up(t_sim))

        n_spikes = nest.GetStatus(spike_rec)[0]['n_events']
        assert n_spikes > spikes_expected - 3 * spikes_std, \
            "parrot_neuron loses spikes."
        assert n_spikes < spikes_expected + 3 * spikes_std, \
            "parrot_neuron adds spikes."


@nest.ll_api.check_stack
class ParrotNeuronPSSTDPTestCase(unittest.TestCase):
    """
    Check STDP protocol between two parrot_neurons_ps connected by a
    stdp_synapse. Exact pre- and postsynaptic spike times are set by
    spike_generators connected to each parrot neuron. Additional spikes sent
    through the stdp_synapse are explicitly ignored in the postsynaptic
    parrot_neuron_ps by setting the stdp_synapse to connect to port 1.
    """

    def run_protocol(self, dt):
        """Set up a network with pre-post spike pairings with
        t_post - t_pre = dt"""

        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()

        # set pre and postsynaptic spike times
        delay = 1.  # delay for connections
        dspike = 100.  # ISI

        # set the correct real spike times for generators (correcting for
        # delays)
        pre_times = [100., 100. + dspike]
        post_times = [k + dt for k in pre_times]

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator",
                                 params={"spike_times": pre_times,
                                         'precise_times': True})
        post_spikes = nest.Create("spike_generator",
                                  params={"spike_times": post_times,
                                          'precise_times': True})

        # create parrot neurons and connect spike_generators
        pre_parrot = nest.Create("parrot_neuron_ps", 1)
        post_parrot = nest.Create("parrot_neuron_ps", 1)

        nest.Connect(pre_spikes, pre_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, post_parrot, syn_spec={"delay": delay})

        # create spike recorder
        spikes = nest.Create("spike_recorder")
        nest.Connect(pre_parrot, spikes)
        nest.Connect(post_parrot, spikes)

        # connect both parrot neurons with a stdp synapse onto port 1
        # thereby spikes transmitted through the stdp connection are
        # not repeated postsynaptically.
        syn_spec = {
            "synapse_model": "stdp_synapse",
            # set receptor 1 postsynaptically, to not generate extra spikes
            "receptor_type": 1,
        }
        conn_spec = {
            "rule": "one_to_one",
        }
        nest.Connect(pre_parrot, post_parrot,
                     syn_spec=syn_spec, conn_spec=conn_spec)

        # get STDP synapse and weight before protocol
        syn = nest.GetConnections(
            source=pre_parrot, synapse_model="stdp_synapse")
        w_pre = syn.get('weight')

        last_time = max(pre_times[-1], post_times[-1])
        nest.Simulate(_round_up(last_time + 2 * delay))

        # get weight post protocol
        w_post = syn.get('weight')

        return w_pre, w_post

    def test_ParrotNeuronSTDPProtocolPotentiation(self):
        """Check pre-post spike pairings between parrot_neurons increments
        weights."""

        dt = 10.
        w_pre, w_post = self.run_protocol(dt)
        assert w_pre < w_post, "Parrot neuron STDP potentiation protocol \
            failed to elicit positive weight changes."

    def test_ParrotNeuronSTDPProtocolDepression(self):
        """Check post-pre spike pairings between parrot_neurons decrement
        weights."""

        dt = -10.
        w_pre, w_post = self.run_protocol(dt)
        assert w_pre > w_post, "Parrot neuron STDP potentiation protocol \
            failed to elicit negative weight changes."


def suite():

    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite1 = unittest.TestLoader().loadTestsFromTestCase(
        ParrotNeuronPSTestCase)
    suite2 = unittest.TestLoader().loadTestsFromTestCase(
        ParrotNeuronPSPoissonTestCase)
    suite3 = unittest.TestLoader().loadTestsFromTestCase(
        ParrotNeuronPSSTDPTestCase)
    return unittest.TestSuite([suite1, suite2, suite3])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
