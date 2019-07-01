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

HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")


@nest.ll_api.check_stack
@unittest.skipIf(not HAVE_GSL, 'GSL is not available')
class LabeledSynapsesTestCase(unittest.TestCase):
    """Test labeled synapses"""

    def default_network(self, syn_model):
        nest.ResetKernel()
        # set volume transmitter for stdp_dopamine_synapse_lbl
        vol = nest.Create('volume_transmitter', 3)
        nest.SetDefaults('stdp_dopamine_synapse',
                         {'vt': vol.get('global_id')[0]})
        nest.SetDefaults('stdp_dopamine_synapse_lbl',
                         {'vt': vol.get('global_id')[1]})
        nest.SetDefaults('stdp_dopamine_synapse_hpc',
                         {'vt': vol.get('global_id')[2]})

        self.rate_model_connections = [
            'rate_connection_instantaneous',
            'rate_connection_instantaneous_lbl',
            'rate_connection_delayed',
            'rate_connection_delayed_lbl'
        ]

        self.siegert_connections = [
            'diffusion_connection',
            'diffusion_connection_lbl'
        ]

        self.clopath_connections = [
            'clopath_synapse',
            'clopath_synapse_lbl'
        ]

        # create neurons that accept all synapse connections (especially gap
        # junctions)... hh_psc_alpha_gap is only available with GSL, hence the
        # skipIf above
        neurons = nest.Create("hh_psc_alpha_gap", 5)

        # in case of rate model connections use the lin_rate_ipn model instead
        if syn_model in self.rate_model_connections:
            neurons = nest.Create("lin_rate_ipn", 5)

        # in case of siegert connections use the siegert_neuron model instead
        if syn_model in self.siegert_connections:
            neurons = nest.Create("siegert_neuron", 5)

        # in case of the clopath stdp synapse use the a supported model instead
        if syn_model in self.clopath_connections:
            neurons = nest.Create("hh_psc_alpha_clopath", 5)

        return neurons

    def test_SetLabelToSynapseOnConnect(self):
        """Set a label to a labeled synapse on connect."""

        labeled_synapse_models = [s for s in nest.Models(
            mtype='synapses') if s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network(syn)

            # see if symmetric connections are required
            symm = nest.GetDefaults(syn, 'requires_symmetric')

            # set a label during connection
            nest.Connect(a, a, {"rule": "one_to_one", "make_symmetric": symm},
                         {"synapse_model": syn, "synapse_label": 123})
            c = nest.GetConnections(a, a)
            self.assertTrue(
                all([x == 123 for x in c.get('synapse_label')])
            )

    def test_SetLabelToSynapseSetStatus(self):
        """Set a label to a labeled synapse on SetStatus."""

        labeled_synapse_models = [s for s in nest.Models(
            mtype='synapses') if s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network(syn)

            # see if symmetric connections are required
            symm = nest.GetDefaults(syn, 'requires_symmetric')

            # set no label during connection
            nest.Connect(a, a, {"rule": "one_to_one", "make_symmetric": symm},
                         {"synapse_model": syn})
            c = nest.GetConnections(a, a)
            # still unlabeled
            self.assertTrue(
                all([x == -1 for x in c.get('synapse_label')])
            )

            # set a label
            c.set({'synapse_label': 123})
            self.assertTrue(
                all([x == 123 for x in c.get('synapse_label')])
            )

    def test_SetLabelToSynapseSetDefaults(self):
        """Set a label to a labeled synapse on SetDefaults."""

        labeled_synapse_models = [s for s in nest.Models(
            mtype='synapses') if s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network(syn)

            # see if symmetric connections are required
            symm = nest.GetDefaults(syn, 'requires_symmetric')

            # set a label during SetDefaults
            nest.SetDefaults(syn, {'synapse_label': 123})
            nest.Connect(a, a, {"rule": "one_to_one", "make_symmetric": symm},
                         {"synapse_model": syn})
            c = nest.GetConnections(a, a)
            self.assertTrue(
                all([x == 123 for x in c.get('synapse_label')])
            )

    def test_GetLabeledSynapses(self):
        """Get labeled synapses with GetConnections."""

        labeled_synapse_models = [s for s in nest.Models(
            mtype='synapses') if s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network(syn)

            # see if symmetric connections are required
            symm = nest.GetDefaults(syn, 'requires_symmetric')

            # some more connections
            synapse_type = "static_synapse"
            if syn in self.rate_model_connections:
                synapse_type = "rate_connection_instantaneous"
            if syn in self.siegert_connections:
                synapse_type = "diffusion_connection"
            nest.Connect(a, a, {"rule": "one_to_one"},
                         {"synapse_model": synapse_type})
            # set a label during connection
            nest.Connect(a, a, {"rule": "one_to_one", "make_symmetric": symm},
                         {"synapse_model": syn, "synapse_label": 123})
            c = nest.GetConnections(a, a, synapse_label=123)
            self.assertTrue(
                all([x == 123 for x in c.get('synapse_label')])
            )

    def test_SetLabelToNotLabeledSynapse(self):
        """Try set a label to an 'un-label-able' synapse."""
        labeled_synapse_models = [s for s in nest.Models(
            mtype='synapses') if not s.endswith("_lbl")]
        for syn in labeled_synapse_models:
            a = self.default_network(syn)

            # see if symmetric connections are required
            symm = nest.GetDefaults(syn, 'requires_symmetric')

            # try set a label during SetDefaults
            with self.assertRaises(nest.kernel.NESTError):
                nest.SetDefaults(syn, {'synapse_label': 123})

            # try set on connect
            with self.assertRaises(nest.kernel.NESTError):
                nest.Connect(a, a, {"rule": "one_to_one",
                                    "make_symmetric": symm},
                             {"synapse_model": syn, "synapse_label": 123})

            # plain connection
            nest.Connect(a, a, {"rule": "one_to_one", "make_symmetric": symm},
                         {"synapse_model": syn})
            # try set on SetStatus
            c = nest.GetConnections(a, a)

            with self.assertRaises(nest.kernel.NESTError):
                c.set({'synapse_label': 123})


def suite():
    suite = unittest.makeSuite(LabeledSynapsesTestCase, 'test')
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
