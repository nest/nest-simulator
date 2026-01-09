# -*- coding: utf-8 -*-
#
# test_sp_manager_spatial.py
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
import numpy as np


class TestStructuralPlasticityManagerSpatial(unittest.TestCase):
    def setUp(self):
        nest.ResetKernel()
        nest.set_verbosity("M_INFO")
        self.exclude_synapse_model = [
            "stdp_dopamine_synapse",
            "stdp_dopamine_synapse_lbl",
            "stdp_dopamine_synapse_hpc",
            "stdp_dopamine_synapse_hpc_lbl",
            "gap_junction",
            "gap_junction_lbl",
            "diffusion_connection",
            "diffusion_connection_lbl",
            "rate_connection_instantaneous",
            "rate_connection_instantaneous_lbl",
            "rate_connection_delayed",
            "rate_connection_delayed_lbl",
        ]

    HAVE_GSL = nest.ll_api.sli_func("statusdict/have_gsl ::")

    def test_synapse_creation_distance_dependent(self):
        for syn_model in nest.synapse_models:
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                # assign random 2d positions
                positions = np.random.uniform([0, 0], [10, 10], size=(2, 2))

                syn_dict = {
                    "synapse_model": syn_model,
                    "pre_synaptic_element": "SE1",
                    "post_synaptic_element": "SE2",
                }
                nest.structural_plasticity_synapses = {"syn1": syn_dict}
                # create 2 neurons that with 10 presynaptic and 10 postsymaptic elements each.
                neurons = nest.Create(
                    "iaf_psc_alpha",
                    2,
                    {
                        "synaptic_elements": {
                            "SE1": {"z": 10.0, "growth_rate": 0.0},
                            "SE2": {"z": 10.0, "growth_rate": 0.0},
                        }
                    },
                    positions=nest.spatial.free(pos=positions.tolist()),
                )
                # set update interval to 10 seconds
                nest.structural_plasticity_update_interval = 10000
                nest.EnableStructuralPlasticity(use_gaussian_kernel=True, gaussian_kernel_sigma=1.0)
                nest.Simulate(10.0)
                status = nest.GetStatus(neurons, "synaptic_elements")
                for st_neuron in status:
                    assert st_neuron["SE1"]["z_connected"] == 10
                    assert st_neuron["SE2"]["z_connected"] == 10

                assert len(nest.GetConnections(neurons, neurons, syn_model)) == 20
                break

    def test_structural_plasticity_with_positions(self):
        # Test structural plasticity when neurons have positions in multiple dimensions.
        nest.ResetKernel()
        nest.structural_plasticity_update_interval = 1.0

        syn_model = "static_synapse"
        nest.CopyModel(syn_model, "sp_synapse")
        nest.SetDefaults("sp_synapse", {"weight": 1.0, "delay": 1.0})

        nest.structural_plasticity_synapses = {
            "sp_syn": {
                "synapse_model": "sp_synapse",
                "post_synaptic_element": "Den_ex",
                "pre_synaptic_element": "Axon_ex",
            }
        }

        num_neurons = 30
        positions = np.random.uniform(0, 10, (num_neurons, 3))  # 3D positions
        neuron_params = {
            "synaptic_elements": {
                "Axon_ex": {"z": 1.0, "growth_rate": 1.0},
                "Den_ex": {"z": 1.0, "growth_rate": 1.0},
            },
        }

        neurons = nest.Create(
            "iaf_psc_alpha",
            num_neurons,
            neuron_params,
            positions=nest.spatial.free(pos=positions.tolist()),
        )

        nest.EnableStructuralPlasticity(use_gaussian_kernel=True, gaussian_kernel_sigma=1.0)
        nest.Simulate(10.0)

        connections = nest.GetConnections(neurons, neurons, synapse_model="sp_synapse")
        self.assertGreater(len(connections), 0, "No connections were created")

        # Check that neurons closer together are more likely to be connected
        distances = []
        for conn in connections:
            pre_pos = positions[conn.source - 1]
            post_pos = positions[conn.target - 1]
            distance = np.linalg.norm(np.array(pre_pos) - np.array(post_pos))
            distances.append(distance)

        avg_distance = np.mean(distances)
        self.assertLess(avg_distance, 10.0, "Average connection distance is too large")

    def test_distance_dependent_without_positions(self):
        # Test if an error is raised when distance dependency is enabled without providing positions.
        nest.ResetKernel()

        syn_model = "static_synapse"
        nest.CopyModel(syn_model, "sp_synapse")
        nest.SetDefaults("sp_synapse", {"weight": 1.0, "delay": 1.0})

        nest.structural_plasticity_synapses = {
            "sp_syn": {
                "synapse_model": "sp_synapse",
                "post_synaptic_element": "Den_ex",
                "pre_synaptic_element": "Axon_ex",
            }
        }

        num_neurons = 10
        neuron_params = {
            "synaptic_elements": {
                "Axon_ex": {"z": 1.0, "growth_rate": 1.0},
                "Den_ex": {"z": 1.0, "growth_rate": 1.0},
            },
        }

        # Create neurons without positions
        neurons = nest.Create("iaf_psc_alpha", num_neurons, params=neuron_params)

        # Expecting a NEST CppException due to missing positions
        with self.assertRaises(nest.NESTErrors.CppException) as context:
            nest.EnableStructuralPlasticity(use_gaussian_kernel=True, gaussian_kernel_sigma=1.0)
            nest.Simulate(10.0)

        # Check the error message
        self.assertIn(
            "No neurons with valid positions found. Please provide valid positions, or disable distance dependency.",
            str(context.exception),
        )
