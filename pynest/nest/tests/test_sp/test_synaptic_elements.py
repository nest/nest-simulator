# -*- coding: utf-8 -*-
#
# test_synaptic_elements.py
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

__author__ = 'naveau'


class TestSynapticElements(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()

    def test_set_status(self):
        synaptic_element_dict = {
            u'SE': {u'z': 15.0, u'growth_curve': u'linear'}}

        neuron = nest.Create('iaf_psc_alpha', 1)
        nest.SetStatus(neuron, {'synaptic_elements': synaptic_element_dict})
        neuron_synaptic_elements = nest.GetStatus(
            neuron, 'synaptic_elements')[0]
        self.assertIn('SE', neuron_synaptic_elements)
        self.assertDictContainsSubset(
            synaptic_element_dict[u'SE'], neuron_synaptic_elements[u'SE'])

    def test_set_status_overwrite(self):
        synaptic_element_dict1 = {
            u'SE1': {u'z': 15.0, u'growth_curve': u'linear'}}
        synaptic_element_dict2 = {
            u'SE2': {u'z': 10.0, u'growth_curve': u'gaussian'}}

        neuron = nest.Create('iaf_psc_alpha', 1)
        nest.SetStatus(neuron, {'synaptic_elements': synaptic_element_dict1})
        nest.SetStatus(neuron, {'synaptic_elements': synaptic_element_dict2})

        neuron_synaptic_elements = nest.GetStatus(
            neuron, 'synaptic_elements')[0]
        self.assertNotIn('SE1', neuron_synaptic_elements)
        self.assertIn('SE2', neuron_synaptic_elements)
        self.assertDictContainsSubset(
            synaptic_element_dict2[u'SE2'], neuron_synaptic_elements[u'SE2'])

    def test_set_defaults(self):
        synaptic_element_dict = {
            u'SE': {u'z': 15.0, u'growth_curve': u'linear'}}

        nest.SetDefaults(
            'iaf_psc_alpha', {'synaptic_elements': synaptic_element_dict})
        neuron = nest.Create('iaf_psc_alpha', 1)
        neuron_synaptic_elements = nest.GetStatus(
            neuron, 'synaptic_elements')[0]
        self.assertIn('SE', neuron_synaptic_elements)
        self.assertDictContainsSubset(
            synaptic_element_dict[u'SE'], neuron_synaptic_elements[u'SE'])

    def test_set_defaults_overwrite(self):
        synaptic_element_dict1 = {
            u'SE1': {u'z': 15.0, u'growth_curve': u'linear'}}
        synaptic_element_dict2 = {
            u'SE2': {u'z': 10.0, u'growth_curve': u'gaussian'}}

        nest.SetDefaults(
            'iaf_psc_alpha', {'synaptic_elements': synaptic_element_dict1})
        nest.SetDefaults(
            'iaf_psc_alpha', {'synaptic_elements': synaptic_element_dict2})
        neuron = nest.Create('iaf_psc_alpha', 1)

        neuron_synaptic_elements = nest.GetStatus(
            neuron, 'synaptic_elements')[0]
        self.assertNotIn('SE1', neuron_synaptic_elements)
        self.assertIn('SE2', neuron_synaptic_elements)
        self.assertDictContainsSubset(
            synaptic_element_dict2[u'SE2'], neuron_synaptic_elements[u'SE2'])


def suite():
    test_suite = unittest.makeSuite(TestSynapticElements, 'test')
    return test_suite

if __name__ == '__main__':
    unittest.main()
