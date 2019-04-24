# -*- coding: utf-8 -*-
#
# test_disconnect_multiple.py
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


class TestDisconnect(unittest.TestCase):

    def setUp(self):
        nest.ResetKernel()
        nest.set_verbosity('M_ERROR')
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
            'rate_connection_delayed_lbl',
            'clopath_synapse',
            'clopath_synapse_lbl'
        ]

    def test_multiple_synapse_deletion_all_to_all(self):
        for syn_model in nest.Models('synapses'):
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                nest.CopyModel('static_synapse', 'my_static_synapse')
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
                neurons = nest.Create('iaf_psc_alpha', 10, {
                    'synaptic_elements': {
                        'SE1': {'z': 0.0, 'growth_rate': 0.0},
                        'SE2': {'z': 0.0, 'growth_rate': 0.0}
                    }
                })

                nest.Connect(neurons, neurons, "all_to_all", syn_dict)

                # Test if the connected synaptic elements before the simulation
                # are correct
                status = nest.GetStatus(neurons, 'synaptic_elements')
                for st_neuron in status:
                    self.assertEqual(10, st_neuron['SE1']['z_connected'])
                    self.assertEqual(10, st_neuron['SE2']['z_connected'])

                src_neurons = neurons[:5]
                tgt_neurons = neurons[5:]

                conns = nest.GetConnections(src_neurons, tgt_neurons,
                                            syn_model)
                assert conns

                conndictionary = {'rule': 'all_to_all'}
                syndictionary = {'synapse_model': syn_model}
                nest.Disconnect(
                    src_neurons,
                    tgt_neurons,
                    conndictionary,
                    syndictionary
                )
                status = nest.GetStatus(neurons, 'synaptic_elements')
                for st_neuron in status[0:5]:
                    self.assertEqual(5, st_neuron['SE1']['z_connected'])
                    self.assertEqual(10, st_neuron['SE2']['z_connected'])
                for st_neuron in status[5:10]:
                    self.assertEqual(10, st_neuron['SE1']['z_connected'])
                    self.assertEqual(5, st_neuron['SE2']['z_connected'])

    def test_multiple_synapse_deletion_one_to_one(self):
        for syn_model in nest.Models('synapses'):
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                nest.CopyModel('static_synapse', 'my_static_synapse')
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
                neurons = nest.Create('iaf_psc_alpha', 10, {
                    'synaptic_elements': {
                        'SE1': {'z': 0.0, 'growth_rate': 0.0},
                        'SE2': {'z': 0.0, 'growth_rate': 0.0}
                    }
                })

                nest.Connect(neurons, neurons, "all_to_all", syn_dict)

                # Test if the connected synaptic elements before the simulation
                # are correct
                status = nest.GetStatus(neurons, 'synaptic_elements')
                for st_neuron in status:
                    self.assertEqual(10, st_neuron['SE1']['z_connected'])
                    self.assertEqual(10, st_neuron['SE2']['z_connected'])

                src_neurons = neurons[:5]
                tgt_neurons = neurons[5:]

                conns = nest.GetConnections(src_neurons, tgt_neurons,
                                            syn_model)
                assert conns

                conndictionary = {'rule': 'one_to_one'}
                syndictionary = {'synapse_model': syn_model}
                nest.Disconnect(
                    src_neurons,
                    tgt_neurons,
                    conndictionary,
                    syndictionary
                )
                status = nest.GetStatus(neurons, 'synaptic_elements')
                for st_neuron in status[0:5]:
                    self.assertEqual(9, st_neuron['SE1']['z_connected'])
                    self.assertEqual(10, st_neuron['SE2']['z_connected'])
                for st_neuron in status[5:10]:
                    self.assertEqual(10, st_neuron['SE1']['z_connected'])
                    self.assertEqual(9, st_neuron['SE2']['z_connected'])

    def test_multiple_synapse_deletion_one_to_one_no_sp(self):
        for syn_model in nest.Models('synapses'):
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                nest.CopyModel('static_synapse', 'my_static_synapse')
                neurons = nest.Create('iaf_psc_alpha', 10)
                syn_dict = {'synapse_model': syn_model}
                nest.Connect(neurons, neurons, "all_to_all", syn_dict)

                src_neurons = neurons[:5]
                tgt_neurons = neurons[5:]

                conns = nest.GetConnections(src_neurons, tgt_neurons,
                                            syn_model)
                assert len(conns) == 25

                conndictionary = {'rule': 'one_to_one'}
                syndictionary = {'synapse_model': syn_model}
                nest.Disconnect(
                    src_neurons,
                    tgt_neurons,
                    conndictionary,
                    syndictionary
                )

                conns = nest.GetConnections(src_neurons, tgt_neurons,
                                            syn_model)
                assert len(conns) == 20

    def test_single_synapse_deletion_sp(self):
        for syn_model in nest.Models('synapses'):
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                nest.CopyModel('static_synapse', 'my_static_synapse')
                syn_dict = {
                    'synapse_model': syn_model,
                    'pre_synaptic_element': 'SE1',
                    'post_synaptic_element': 'SE2'
                }
                # nest.SetKernelStatus(
                #   {'structural_plasticity_synapses': {'syn1': syn_dict}}
                # )
                neurons = nest.Create('iaf_psc_alpha', 2, {
                    'synaptic_elements': {
                        'SE1': {'z': 0.0, 'growth_rate': 0.0},
                        'SE2': {'z': 0.0, 'growth_rate': 0.0}
                    }
                })
                nest.Connect(neurons, neurons, "all_to_all", syn_dict)
                nest.Connect(neurons, neurons, "all_to_all",
                             {'synapse_model': 'my_static_synapse'})

                # Test if the connected synaptic elements before the simulation
                # are correct
                status = nest.GetStatus(neurons, 'synaptic_elements')
                for st_neuron in status:
                    self.assertEqual(2, st_neuron['SE1']['z_connected'])
                    self.assertEqual(2, st_neuron['SE2']['z_connected'])

                srcId = 0
                targId = 1

                conns = nest.GetConnections(
                    neurons[srcId], neurons[targId], syn_model)
                assert conns
                nest.Disconnect(
                    neurons[srcId], neurons[targId], syn_spec=syn_dict)
                status = nest.GetStatus(neurons, 'synaptic_elements')
                self.assertEqual(1, status[srcId]['SE1']['z_connected'])
                self.assertEqual(2, status[srcId]['SE2']['z_connected'])
                self.assertEqual(2, status[targId]['SE1']['z_connected'])
                self.assertEqual(1, status[targId]['SE2']['z_connected'])

                conns = nest.GetConnections(
                    neurons[srcId], neurons[targId], syn_model)
                assert not conns

    def test_disconnect_defaults(self):

        nodes = nest.Create('iaf_psc_alpha', 5)
        nest.Connect(nodes, nodes)
        self.assertEqual(nest.GetKernelStatus('num_connections'), 25)

        nest.Disconnect(nodes, nodes)

        self.assertEqual(nest.GetKernelStatus('num_connections'), 20)

    def test_disconnect_all_to_all(self):

        nodes = nest.Create('iaf_psc_alpha', 5)
        nest.Connect(nodes, nodes)

        self.assertEqual(nest.GetKernelStatus('num_connections'), 25)

        nest.Disconnect(nodes, nodes, 'all_to_all')

        self.assertEqual(nest.GetKernelStatus('num_connections'), 0)

    def test_disconnect_static_synapse(self):

        nodes = nest.Create('iaf_psc_alpha', 5)
        nest.Connect(nodes, nodes)

        self.assertEqual(nest.GetKernelStatus('num_connections'), 25)

        nest.Disconnect(nodes, nodes, syn_spec='static_synapse')

        self.assertEqual(nest.GetKernelStatus('num_connections'), 20)


def suite():
    test_suite = unittest.makeSuite(TestDisconnect, 'test')
    return test_suite


if __name__ == '__main__':
    unittest.main(verbosity=2)
