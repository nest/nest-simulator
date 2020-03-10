# -*- coding: utf-8 -*-
#
# test_sp_manager.py
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
from .utils import extract_dict_a_from_b

__author__ = 'naveau'


class TestStructuralPlasticityManager(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()
        nest.set_verbosity('M_INFO')
        self.exclude_synapse_model = [
            'stdp_dopamine_synapse',
            'stdp_dopamine_synapse_lbl',
            'stdp_dopamine_synapse_hpc',
            'stdp_dopamine_synapse_hpc_lbl',
            'gap_junction',
            'gap_junction_lbl',
            'diffusion_connection',
            'diffusion_connection_lbl',
            'rate_connection_instantaneous',
            'rate_connection_instantaneous_lbl',
            'rate_connection_delayed',
            'rate_connection_delayed_lbl'
        ]

    def test_register_synapses(self):
        for syn_model in nest.Models('synapses'):
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                nest.SetDefaults(syn_model, {'delay': 0.5})
                syn_dict = {
                    'synapse_model': syn_model,
                    'pre_synaptic_element': 'SE1',
                    'post_synaptic_element': 'SE2'
                }
                nest.SetKernelStatus({
                    'min_delay': 0.1,
                    'max_delay': 1.0,
                    'structural_plasticity_synapses': {'syn1': syn_dict}
                })
                kernel_status = nest.GetKernelStatus(
                    'structural_plasticity_synapses')
                self.assertIn('syn1', kernel_status)
                self.assertEqual(kernel_status['syn1'], extract_dict_a_from_b(
                    kernel_status['syn1'], syn_dict))

    def test_min_max_delay_using_default_delay(self):
        nest.ResetKernel()
        delay = 1.0
        syn_model = 'static_synapse'
        nest.SetStructuralPlasticityStatus(
            {
                'structural_plasticity_synapses': {
                    'syn1': {
                        'synapse_model': syn_model,
                        'pre_synaptic_element': 'SE1',
                        'post_synaptic_element': 'SE2',
                    }
                }
            }
        )
        self.assertLessEqual(nest.GetKernelStatus('min_delay'), delay)
        self.assertGreaterEqual(nest.GetKernelStatus('max_delay'), delay)

    def test_synapse_creation(self):
        for syn_model in nest.Models('synapses'):
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                syn_dict = {
                    'synapse_model': syn_model,
                    'pre_synaptic_element': 'SE1',
                    'post_synaptic_element': 'SE2'
                }
                nest.SetStructuralPlasticityStatus({
                    'structural_plasticity_synapses': {'syn1': syn_dict}
                })
                neurons = nest.Create('iaf_psc_alpha', 2, {
                    'synaptic_elements': {
                        'SE1': {'z': 10.0, 'growth_rate': 0.0},
                        'SE2': {'z': 10.0, 'growth_rate': 0.0}
                    }
                })
                nest.EnableStructuralPlasticity()
                nest.Simulate(10.0)
                status = nest.GetStatus(neurons, 'synaptic_elements')
                for st_neuron in status:
                    self.assertEqual(10, st_neuron['SE1']['z_connected'])
                    self.assertEqual(10, st_neuron['SE2']['z_connected'])

                self.assertEqual(
                    20, len(nest.GetConnections(neurons, neurons, syn_model)))
                break


def suite():
    test_suite = unittest.makeSuite(TestStructuralPlasticityManager, 'test')
    return test_suite


if __name__ == '__main__':
    unittest.main()
