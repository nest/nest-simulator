# -*- coding: utf-8 -*-
#
# test_labeled_synapses.py
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
Test setting and getting labels on synapses.
"""

import unittest
import nest

HAVE_GSL = nest.sli_func("statusdict/have_gsl ::")

@nest.check_stack
@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
class LabeledSynapsesTestCase(unittest.TestCase):
    """Test labeled synapses"""

    def default_network(self):
        nest.ResetKernel()
        # set volume transmitter for stdp_dopamine_synapse_lbl
        vol = nest.Create('volume_transmitter', 3)
        nest.SetDefaults('stdp_dopamine_synapse',{'vt':vol[0]})
        nest.SetDefaults('stdp_dopamine_synapse_lbl',{'vt':vol[1]})
        nest.SetDefaults('stdp_dopamine_synapse_hpc',{'vt':vol[2]})

        # create neurons that accept all synapse connections (especially gap junctions)...
        # hh_psc_alpha_gap is only available with GSL, hence the skipIf above
        return nest.Create("hh_psc_alpha_gap", 5)

    def test_SetLabelToSynapseOnConnect(self):
        """Set a label to a labeled synapse on connect."""

        labeled_synapse_models = [s for s in nest.Models(mtype='synapses') if s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network()

            # set a label during connection
            nest.Connect(a, a, {"rule": "one_to_one"}, {"model": syn, "synapse_label": 123})
            c = nest.GetConnections(a, a)
            self.assertTrue(all([status['synapse_label']==123 for status in nest.GetStatus(c)]))

    def test_SetLabelToSynapseSetStatus(self):
        """Set a label to a labeled synapse on SetStatus."""

        labeled_synapse_models = [s for s in nest.Models(mtype='synapses') if s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network()

            # set no label during connection
            nest.Connect(a, a, {"rule": "one_to_one"}, {"model": syn})
            c = nest.GetConnections(a, a)
            # still unlabeled
            self.assertTrue(all([status['synapse_label']==-1 for status in nest.GetStatus(c)]))

            # set a label
            nest.SetStatus(c, {'synapse_label': 123})
            self.assertTrue(all([status['synapse_label']==123 for status in nest.GetStatus(c)]))

    def test_SetLabelToSynapseSetDefaults(self):
        """Set a label to a labeled synapse on SetDefaults."""

        labeled_synapse_models = [s for s in nest.Models(mtype='synapses') if s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network()

            # set a label during SetDefaults
            nest.SetDefaults(syn, {'synapse_label': 123})
            nest.Connect(a, a, {"rule": "one_to_one"}, {"model": syn})
            c = nest.GetConnections(a, a)
            self.assertTrue(all([status['synapse_label']==123 for status in nest.GetStatus(c)]))

    def test_GetLabeledSynapses(self):
        """Get labeled synapses with GetConnections."""

        labeled_synapse_models = [s for s in nest.Models(mtype='synapses') if s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network()

            # some more connections
            nest.Connect(a, a, {"rule": "one_to_one"}, {"model": "static_synapse"})
            # set a label during connection
            nest.Connect(a, a, {"rule": "one_to_one"}, {"model": syn, "synapse_label": 123})
            c = nest.GetConnections(a, a, synapse_label=123)
            self.assertTrue(all([status['synapse_label']==123 for status in nest.GetStatus(c)]))

    def test_SetLabelToNotLabeledSynapse(self):
        """Try set a label to an 'un-label-able' synapse."""
        labeled_synapse_models = [s for s in nest.Models(mtype='synapses') if not s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network()

            # try set a label during SetDefaults
            with self.assertRaises(nest.NESTError):
                nest.SetDefaults(syn, {'synapse_label': 123})

            # try set on connect
            with self.assertRaises(nest.NESTError):
                nest.Connect(a, a, {"rule": "one_to_one"}, {"model": syn, "synapse_label":123})

            # plain connection
            nest.Connect(a, a, {"rule": "one_to_one"}, {"model": syn})
            # try set on SetStatus
            c = nest.GetConnections(a, a)
            with self.assertRaises(nest.NESTError):
                nest.SetStatus(c, {'synapse_label': 123})

def suite():
    suite = unittest.makeSuite(LabeledSynapsesTestCase,'test')
    return suite

def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
