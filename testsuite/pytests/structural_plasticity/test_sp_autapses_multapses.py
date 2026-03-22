# -*- coding: utf-8 -*-
#
# test_sp_autapses_multapses.py
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
from pprint import pprint

import nest


class TestStructuralPlasticityAutapses(unittest.TestCase):
    def test_autapses(self):
        """Test if allow_autoapses can be set on structural plasticity synapses."""

        nest.ResetKernel()
        nest.CopyModel("static_synapse", "synapse_ex")
        nest.SetDefaults("synapse_ex", {"weight": 1.0, "delay": 1.0})
        nest.structural_plasticity_synapses = {
            "synapse_ex": {
                "synapse_model": "synapse_ex",
                "post_synaptic_element": "Den_ex",
                "pre_synaptic_element": "Axon_ex",
                "allow_autapses": False,
            },
        }

        assert nest.structural_plasticity_synapses["synapse_ex"]["allow_multapses"]  # default
        assert not nest.structural_plasticity_synapses["synapse_ex"]["allow_autapses"]

    def test_multapses(self):
        """Test if allow_multapses can be set on structural plasticity synapses."""

        nest.ResetKernel()
        nest.CopyModel("static_synapse", "synapse_ex")
        nest.SetDefaults("synapse_ex", {"weight": 1.0, "delay": 1.0})
        nest.structural_plasticity_synapses = {
            "synapse_ex": {
                "synapse_model": "synapse_ex",
                "post_synaptic_element": "Den_ex",
                "pre_synaptic_element": "Axon_ex",
                "allow_multapses": False,
            },
        }

        assert nest.structural_plasticity_synapses["synapse_ex"]["allow_autapses"]  # the default
        assert not nest.structural_plasticity_synapses["synapse_ex"]["allow_multapses"]


def suite():
    test_suite = unittest.makeSuite(TestStructuralPlasticityAutapses, "test")
    return test_suite


if __name__ == "__main__":
    unittest.main()
