# -*- coding: utf-8 -*-
#
# test_sp_autapses.py
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

import nest
import unittest

class TestStructuralPlasticityAutapses(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()
        nest.set_verbosity('M_INFO')

    def test_autapses(self):
	nest.CopyModel('static_synapse', 'synapse_ex')
	nest.SetDefaults('synapse_ex', {'weight': 1, 'delay': 1.0})
	nest.SetKernelStatus({
	    'structural_plasticity_synapses': {
		'synapse_ex': {
		    'synapse_model': 'synapse_ex',
		    'post_synaptic_element': 'Den_ex',
		    'pre_synaptic_element': 'Axon_ex',
		    'allow_autapses': False,
		},
	    }
	})
	

def suite():
    test_suite = unittest.makeSuite(TestStructuralPlasticityAutapses, 'test')
    return test_suite


if __name__ == '__main__':
    unittest.main()
