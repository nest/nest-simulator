# -*- coding: utf-8 -*-
#
# test_regression_issue-2069.py
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


class SynSpecCopyTestCase(unittest.TestCase):
    def test_syn_spec_copied(self):
        """Check if simple syn_spec is copied"""

        nodes = nest.Create("iaf_psc_alpha", 4)
        # When connecting, the weight list will be converted to a numpy array. We need to make sure our
        # syn_spec, especially the weight list, is not modified.
        syn_spec = {"synapse_model": "stdp_synapse", "weight": [1.0, 2.0, 3.0, 4.0]}

        nest.Connect(nodes, nodes, "one_to_one", syn_spec=syn_spec)

        self.assertDictEqual(syn_spec, {"synapse_model": "stdp_synapse", "weight": [1.0, 2.0, 3.0, 4.0]})

    def test_syn_spec_copied_with_parameter(self):
        """Check if simple syn_spec is copied when weight is nest.Parameter"""

        nodes = nest.Create("iaf_psc_alpha", 4)
        weight_param = nest.random.uniform(0.0, 5)
        syn_spec = {"synapse_model": "stdp_synapse", "weight": weight_param}

        nest.Connect(nodes, nodes, syn_spec=syn_spec)

        self.assertDictEqual(syn_spec, {"synapse_model": "stdp_synapse", "weight": weight_param})


def suite():
    t = unittest.TestLoader().loadTestsFromTestCase(SynSpecCopyTestCase)
    return unittest.TestSuite([t])


if __name__ == "__main__":
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())
