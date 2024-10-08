# -*- coding: utf-8 -*-
#
# test_conn_builder.py
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

import unittest

import nest

__author__ = "naveau"


class TestSPBuilder(unittest.TestCase):
    def setUp(self):
        nest.ResetKernel()

    def test_synapse_initialisation_one_to_one(self):
        neurons = nest.Create(
            "iaf_psc_alpha",
            2,
            {"synaptic_elements": {"SE1": {"z": 0.0, "growth_rate": 0.0}, "SE2": {"z": 0.0, "growth_rate": 0.0}}},
        )
        nest.Connect(
            nest.OneToOne(
                neurons,
                neurons,
                syn_spec=nest.synapsemodels.static(pre_synaptic_element="SE1", post_synaptic_element="SE2"),
            )
        )
        nest.BuildNetwork()
        status_list = nest.GetStatus(neurons, "synaptic_elements")
        for status in status_list:
            self.assertEqual(1, status["SE1"]["z_connected"])
            self.assertEqual(1, status["SE2"]["z_connected"])

    def test_synapse_initialisation_all_to_all(self):
        neurons = nest.Create(
            "iaf_psc_alpha",
            2,
            {"synaptic_elements": {"SE1": {"z": 0.0, "growth_rate": 0.0}, "SE2": {"z": 0.0, "growth_rate": 0.0}}},
        )
        nest.Connect(
            nest.AllToAll(
                neurons,
                neurons,
                syn_spec=nest.synapsemodels.static(pre_synaptic_element="SE1", post_synaptic_element="SE2"),
            )
        )
        nest.BuildNetwork()
        status_list = nest.GetStatus(neurons, "synaptic_elements")
        for status in status_list:
            self.assertEqual(2, status["SE1"]["z_connected"])
            self.assertEqual(2, status["SE2"]["z_connected"])

    def test_not_implemented_rules(self):
        syn_model = "static_synapse"
        syn_dict = {"synapse_model": syn_model, "pre_synaptic_element": "SE1", "post_synaptic_element": "SE2"}
        neurons = nest.Create(
            "iaf_psc_alpha",
            2,
            {"synaptic_elements": {"SE1": {"z": 0.0, "growth_rate": 0.0}, "SE2": {"z": 0.0, "growth_rate": 0.0}}},
        )
        for projection in [
            nest.FixedIndegree(
                neurons,
                neurons,
                indegree=1,
                syn_spec=nest.synapsemodels.static(pre_synaptic_element="SE1", post_synaptic_element="SE2"),
            ),
            nest.FixedOutdegree(
                neurons,
                neurons,
                outdegree=1,
                syn_spec=nest.synapsemodels.static(pre_synaptic_element="SE1", post_synaptic_element="SE2"),
            ),
            nest.FixedTotalNumber(
                neurons,
                neurons,
                N=1,
                syn_spec=nest.synapsemodels.static(pre_synaptic_element="SE1", post_synaptic_element="SE2"),
            ),
            nest.PairwiseBernoulli(
                neurons,
                neurons,
                p=0.5,
                syn_spec=nest.synapsemodels.static(pre_synaptic_element="SE1", post_synaptic_element="SE2"),
            ),
        ]:
            try:
                nest.Connect(projection)
                nest.BuildNetwork()
            except nest.kernel.NESTError as e:
                msg = "This connection rule is not implemented for structural plasticity"
                self.assertRegex(str(e), msg)


def suite():
    test_suite = unittest.makeSuite(TestSPBuilder, "test")
    return test_suite


if __name__ == "__main__":
    unittest.main()
