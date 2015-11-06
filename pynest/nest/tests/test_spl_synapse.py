# -*- coding: utf-8 -*-
#
# test_quantal_stp_synapse.py
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

# This script compares the two variants of the Tsodyks/Markram synapse in NEST.

import nest
import numpy
import unittest


@nest.check_stack
class SplSynapseTestCase(unittest.TestCase):
    """Compare quantal_stp_synapse with its deterministic equivalent."""

    def test_creation(self):

        syn_spec = {
           "model": "stdp_spl_synapse",
           "receptor_type": 1,
        }
        conn_spec = {
           "rule": "one_to_one",
        }
        nest.Connect(self.pre_parrot, self.post_parrot, syn_spec=syn_spec, conn_spec=conn_spec)

        # get STDP synapse and weight before protocol
        syn = nest.GetConnections(source=self.pre_parrot, synapse_model="stdp_spl_synapse")
        nest.SetStatus(syn, {'n_pot_conns': 3})
        syn_status = nest.GetStatus(syn)[0]

        nest.Simulate(210.)

        print syn_status

    def setUp(self):

        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 1.})

        # set pre and postsynaptic spike times
        dt = 10.
        delay = 1.  # delay for connections
        dspike = 50.  # ISI

        # set the correct real spike times for generators (correcting for delays)
        pre_times = [100. - delay, 200. - delay]
        post_times = [120. - delay, 140. - delay, 160. - delay]

        # create spike_generators with these times
        pre_spikes = nest.Create("spike_generator", 1, {"spike_times": pre_times})
        post_spikes = nest.Create("spike_generator", 1, {"spike_times": post_times})

        # create parrot neurons and connect spike_generators
        self.pre_parrot = nest.Create("parrot_neuron", 1)
        self.post_parrot = nest.Create("parrot_neuron", 1)
        nest.SetStatus(self.post_parrot, {'tau_minus': 20., 'tau_minus_triplet': 2000.})

        nest.Connect(pre_spikes, self.pre_parrot, syn_spec={"delay": delay})
        nest.Connect(post_spikes, self.post_parrot, syn_spec={"delay": delay})

        # create spike detector
        self.spikes = nest.Create("spike_detector")
        nest.Connect(self.pre_parrot, self.spikes)
        nest.Connect(self.post_parrot, self.spikes)

def suite():

    suite = unittest.makeSuite(SplSynapseTestCase,'test')
    return suite

def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()


