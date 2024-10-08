# -*- coding: utf-8 -*-
#
# test_connect_conngen.py
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
Conngen tests
"""

import unittest

import nest

try:
    import csa

    HAVE_CSA = True
except ImportError:
    HAVE_CSA = False

nest.ll_api.sli_run("statusdict/have_libneurosim ::")
HAVE_LIBNEUROSIM = nest.ll_api.sli_pop()


@nest.ll_api.check_stack
@unittest.skipIf(not HAVE_CSA, "Python CSA package is not available")
@unittest.skipIf(not HAVE_LIBNEUROSIM, "NEST was built without support for libneurosim")
class ConngenTestCase(unittest.TestCase):
    """Conngen tests"""

    def test_Conngen_OneToOne_params(self):
        """One-to-one connectivity using conngen Connect with parameters"""

        nest.ResetKernel()

        n_neurons = 4
        weight = 10000.0
        delay = 2.0

        sources = nest.Create("iaf_psc_alpha", n_neurons)
        targets = nest.Create("iaf_psc_alpha", n_neurons)

        # Create a connection set with values for weight and delay
        cg = csa.cset(csa.oneToOne, weight, delay)

        # Connect sources and targets using the connection set cs and
        # a parameter map mapping weight to position 0 in the value
        # set and delay to position 1
        params_map = {"weight": 0, "delay": 1}
        projection = nest.Conngen(sources, targets, cg=cg, params_map=params_map)
        nest.Connect(projection)
        nest.BuildNetwork()

        for i in range(n_neurons):
            # We expect all connections from sources to have the
            # correct targets, weights and delays
            conns = nest.GetConnections(sources[i])
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns.target, targets[i].get("global_id"))
            self.assertEqual(conns.weight, weight)
            self.assertEqual(conns.delay, delay)

            # We expect the targets to have no connections at all
            conns = nest.GetConnections(targets[i])
            self.assertEqual(len(conns), 0)

    def test_Conngen_OneToOne_synmodel(self):
        """One-to-one connectivity using conngen Connect and synapse_model"""

        nest.ResetKernel()

        n_neurons = 4
        synmodel = "stdp_synapse"
        tau_plus = 10.0

        sources = nest.Create("iaf_psc_alpha", n_neurons)
        targets = nest.Create("iaf_psc_alpha", n_neurons)

        # Create a plain connection set
        cg = csa.cset(csa.oneToOne)

        # Connect with a non-standard synapse model
        projection = nest.Conngen(sources, targets, cg=cg, syn_spec=nest.synapsemodels.stdp(tau_plus=tau_plus))
        nest.Connect(projection)
        nest.BuildNetwork()

        for i in range(n_neurons):
            # We expect all connections to have the correct targets
            # and the non-standard synapse model set
            conns = nest.GetConnections(sources[i])
            self.assertEqual(len(conns), 1)
            self.assertEqual(conns.target, targets[i].get("global_id"))
            self.assertEqual(conns.synapse_model, synmodel)
            self.assertEqual(conns.tau_plus, tau_plus)

            # We expect the targets to have no connections at all
            conns = nest.GetConnections(targets[i])
            self.assertEqual(len(conns), 0)

    def test_Conngen_error_unknown_synapse(self):
        """
        Error handling for unknown synapse model in conngen Connect
        """

        nest.ResetKernel()

        # Create a plain connection set
        cg = csa.cset(csa.oneToOne)

        n_neurons = 4

        pop = nest.Create("iaf_psc_alpha", n_neurons)
        projection = nest.Conngen(
            pop, pop, cg=cg, syn_spec=nest.synapsemodels.SynapseModel(synapse_model="fantasy_synapse")
        )
        nest.Connect(projection)

        self.assertRaisesRegex(nest.kernel.NESTError, "UnknownSynapseType", nest.BuildNetwork)

    def test_Conngen_error_collocated_synapses(self):
        """
        Error handling for collocated synapses in conngen Connect
        """

        nest.ResetKernel()

        # Create a plain connection set
        cg = csa.cset(csa.oneToOne)
        syn_spec = nest.CollocatedSynapses({"weight": -2.0}, {"weight": 2.0})
        pop = nest.Create("iaf_psc_alpha", 3)
        projection = nest.Conngen(pop, pop, cg=cg, syn_spec=syn_spec)

        nest.Connect(projection)
        self.assertRaisesRegex(nest.kernel.NESTError, "BadProperty", nest.BuildNetwork)

    def test_Conngen_error_weight_and_delay_in_synspec_and_conngen(self):
        """
        Error handling for conflicting weight/delay in conngen Connect
        """

        nest.ResetKernel()

        cg = csa.cset(csa.oneToOne, 10000.0, 2.0)
        params_map = {"weight": 0, "delay": 1}

        n_neurons = 4
        pop = nest.Create("iaf_psc_alpha", n_neurons)

        projection = nest.Conngen(pop, pop, cg=cg, params_map=params_map)

        synspec_w = nest.synapsemodels.static(weight=10.0)
        synspec_d = nest.synapsemodels.static(delay=10.0)
        synspec_wd = nest.synapsemodels.static(weigh=10.0, delay=10.0)

        projection.syn_spec = synspec_w
        nest.Connect(projection)
        self.assertRaisesRegex(nest.kernel.NESTError, "BadProperty", nest.BuildNetwork)
        nest.reset_projection_collection()

        projection.syn_spec = synspec_d
        nest.Connect(projection)
        self.assertRaisesRegex(nest.kernel.NESTError, "BadProperty", nest.BuildNetwork)
        nest.reset_projection_collection()

        projection.syn_spec = synspec_wd
        nest.Connect(projection)
        self.assertRaisesRegex(nest.kernel.NESTError, "BadProperty", nest.BuildNetwork)


def suite():
    suite = unittest.makeSuite(ConngenTestCase, "test")
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
