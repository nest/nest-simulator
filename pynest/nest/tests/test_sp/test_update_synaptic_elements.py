# -*- coding: utf-8 -*-
#
# test_update_synaptic_elements.py
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


class TestUpdateSynapticElements(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_update_synaptic_elements(self):
        growth_curve_axonal = {
            'growth_curve': "gaussian",
            'growth_rate': 0.0001,  # Beta (elements/ms)
            'eta': 0.4,
            'eps': 0.7,
        }
        growth_curve_dendritic_E = {
            'growth_curve': "gaussian",
            'growth_rate': 0.0001,  # Beta (elements/ms)
            'eta': 0.1,
            'eps': 0.7,
        }
        structural_p_elements = {
            'Den_ex': growth_curve_dendritic_E,
            'Axon': growth_curve_axonal
        }

        new_growth_curve_axonal = {
            'eta': 0.0,
            'eps': 0.0,
        }
        elements_to_update = {
            'Axon': new_growth_curve_axonal
        }

        neuron = nest.Create('iaf_psc_alpha', 1)
        nest.SetStatus(neuron, {'synaptic_elements': structural_p_elements})
        neuron_synaptic_elements = nest.GetStatus(
            neuron, 'synaptic_elements')[0]
        self.assertIn('Den_ex', neuron_synaptic_elements)
        self.assertIn('Axon', neuron_synaptic_elements)

        self.assertDictContainsSubset(
            structural_p_elements[u'Axon'],
            neuron_synaptic_elements[u'Axon'])
        self.assertDictContainsSubset(
            structural_p_elements[u'Den_ex'],
            neuron_synaptic_elements[u'Den_ex'])

        # Update Axon elements
        nest.SetStatus(neuron, 'synaptic_elements_param', elements_to_update)
        neuron_synaptic_elements = nest.GetStatus(
            neuron, 'synaptic_elements')[0]
        self.assertIn('Den_ex', neuron_synaptic_elements)
        self.assertIn('Axon', neuron_synaptic_elements)

        # Should have been updated
        self.assertDictContainsSubset(
            elements_to_update[u'Axon'], neuron_synaptic_elements[u'Axon'])
        # Should be unchanged
        self.assertDictContainsSubset(
            structural_p_elements[u'Den_ex'],
            neuron_synaptic_elements[u'Den_ex'])


def suite():
    test_suite = unittest.makeSuite(TestUpdateSynapticElements, 'test')
    return test_suite

if __name__ == '__main__':
    unittest.main()
