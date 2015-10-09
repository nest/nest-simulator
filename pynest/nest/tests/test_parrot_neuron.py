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

import nest
import unittest


@nest.check_stack
class ParrotNeuronTestCase(unittest.TestCase):
    """Check parrot_neuron spike repetition properties"""

    def setUp(self):
        nest.set_verbosity(100)
        nest.ResetKernel()

        # set up source spike generator, as well as parrot neurons
        self.spike_time = 1.
        self.source = nest.Create("spike_generator", 1, {"spike_times": [self.spike_time]})
        self.parrot = nest.Create('parrot_neuron')
        self.spikes = nest.Create("spike_detector")

        # record source and parrot spikes
        nest.Connect(self.source, self.spikes)
        nest.Connect(self.parrot, self.spikes)

        self.gid_source = nest.GetStatus(self.source)[0]["global_id"]
        self.gid_parrot = nest.GetStatus(self.parrot)[0]["global_id"]

    def test_ParrotNeuronRepeatSpike(self):
        """Check parrot_neuron repeats spikes on port 0"""

        # connect with arbitrary delay
        delay = .7
        nest.Connect(self.source, self.parrot, syn_spec={"delay": delay})

        nest.Simulate(3.)

        # get spike from parrot neuron
        events = nest.GetStatus(self.spikes)[0]["events"]
        post_time = events['times'][events['senders'] == self.gid_parrot]

        # assert spike was repeated at correct time
        assert post_time, "Parrot neuron failed to repeat spike."
        assert self.spike_time + delay == post_time, "Parrot neuron repeated spike at wrong delay"

    def test_ParrotNeuronIgnoreSpike(self):
        """Check parrot_neuron ignores spikes on port 1"""

        # connect with arbitrary delay to port 1
        nest.Connect(self.source, self.parrot, syn_spec={"receptor_type": 1})
        nest.Simulate(3.)

        # get spike from parrot neuron, assert it was ignored
        events = nest.GetStatus(self.spikes)[0]["events"]
        post_time = events['times'][events['senders'] == self.gid_parrot]
        assert len(post_time) == 0, "Parrot neuron failed to ignore spike arriving on port 1"


@nest.check_stack
class ParrotNeuronSTDPTestCase(unittest.TestCase):
    """
    Check STDP protocol between two parrot_neurons connected by a stdp_synapse.
    Exact pre- and post-synaptic spike times are set by spike_generators
    connected to each parrot neuron. Additional spikes sent through the
    stdp_synapse are explicitly ignored in the postsynaptic parrot_neuron
    by setting the stdp_synapse to connect to port 1.
    """

    def test_ParrotNeuronSTDPProtocol(self):
        """Check STDP between parrot_neurons changes weights, ignores transmitted spikes"""

        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()

        # set pre and postsynaptic spike times
        dt = 5.
        delay = 1.  # delay for connections

        pre_times = [100., 300., 500., 700., 900.]
        pre_times = [k-delay for k in pre_times]  # set the correct real spike times
        post_times = [k+dt for k in pre_times]

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {"spike_times": pre_times})
        post_spikes = nest.Create("spike_generator", 1, {"spike_times": post_times})

        # create parrot neurons and connect spike_generators
        pre_parrot = nest.Create("parrot_neuron", 1)
        post_parrot = nest.Create("parrot_neuron", 1)
        nest.Connect(pre_spikes, pre_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, post_parrot, syn_spec={"delay": delay})

        # create spike detector
        spikes = nest.Create("spike_detector")
        nest.Connect(post_parrot, spikes)

        # connect both parrot neurons with a stdp synapse onto port 1
        # thereby spikes transmitted through the stdp connection are
        # not repeated postsynaptically.
        syn_spec = {
           "model": "stdp_synapse",
           "receptor_type": 1,  # set receptor 1 postsynaptically, to not generate extra spikes
        }
        conn_spec = {
           "rule": "one_to_one",
        }
        nest.Connect(pre_parrot, post_parrot, syn_spec=syn_spec, conn_spec=conn_spec)

        # get STDP synapse and initial weight
        syn = nest.GetConnections(source=pre_parrot, synapse_model="stdp_synapse")
        initial_weight = nest.GetStatus(syn)[0]['weight']

        nest.Simulate(1000)

        # check that the STDP protocol has elicited a weight change.
        final_weight = nest.GetStatus(syn)[0]['weight']
        assert final_weight != initial_weight, "Parrot neuron STDP protocol failed to elicit weight changes"

        # check that postsynaptic spikes are only and exactly those received
        # by the spike_generator.
        post_spiketimes = nest.GetStatus(spikes)[0]['events']['times']
        assert (
            len(post_times) == len(post_spiketimes) and all(post_spiketimes - post_times == delay)
        ), "Parrot neuron failed to ignore transmitted spikes"


def suite():

    # makeSuite is sort of obsolete http://bugs.python.org/issue2721
    # using loadTestsFromTestCase instead.
    suite1 = unittest.TestLoader().loadTestsFromTestCase(ParrotNeuronTestCase)
    suite2 = unittest.TestLoader().loadTestsFromTestCase(ParrotNeuronSTDPTestCase)
    return unittest.TestSuite([suite1, suite2])


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
