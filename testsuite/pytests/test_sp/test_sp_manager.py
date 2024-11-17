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

import unittest

import nest
import numpy as np

__author__ = "naveau"


def extract_dict_a_from_b(a, b):
    return dict((k, b[k]) for k in a.keys() if k in b.keys())


class TestStructuralPlasticityManager(unittest.TestCase):
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


    def test_register_synapses(self):
        for syn_model in nest.synapse_models:
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                nest.SetDefaults(syn_model, {"delay": 0.5})
                syn_dict = {"synapse_model": syn_model, "pre_synaptic_element": "SE1", "post_synaptic_element": "SE2"}
                # For co-dependent properties, we use `set()` instead of kernel attributes
                nest.set(min_delay=0.1, max_delay=1.0)
                nest.structural_plasticity_synapses = {"syn1": syn_dict}
                kernel_status = nest.structural_plasticity_synapses
                assert "syn1" in kernel_status
                for kv in extract_dict_a_from_b(kernel_status["syn1"], syn_dict).items():
                    assert kv in kernel_status["syn1"].items()

    def test_min_max_delay_using_default_delay(self):
        nest.ResetKernel()
        delay = 1.0
        syn_model = "static_synapse"
        nest.structural_plasticity_synapses = {
            "syn1": {
                "synapse_model": syn_model,
                "pre_synaptic_element": "SE1",
                "post_synaptic_element": "SE2",
            }
        }
        assert nest.min_delay <= delay
        assert nest.max_delay >= delay

    def test_getting_kernel_status(self):
        neuron_model = "iaf_psc_alpha"
        nest.CopyModel("static_synapse", "synapse_ex")
        nest.SetDefaults("synapse_ex", {"weight": 1.0, "delay": 1.0})
        nest.structural_plasticity_synapses = {
            "synapse_ex": {
                "synapse_model": "synapse_ex",
                "post_synaptic_element": "Den_ex",
                "pre_synaptic_element": "Axon_ex",
            }
        }

        growth_curve = {
            "growth_curve": "gaussian",
            "growth_rate": 0.0001,  # (elements/ms)
            "continuous": False,
            "eta": 0.0,  # Ca2+
            "eps": 0.05,
        }

        """
        Now we assign the growth curves to the corresponding synaptic
        elements
        """
        synaptic_elements = {
            "Den_ex": growth_curve,
            "Den_in": growth_curve,
            "Axon_ex": growth_curve,
        }

        nest.Create(neuron_model, 2, {"synaptic_elements": synaptic_elements})

        sp_synapses = nest.structural_plasticity_synapses["synapse_ex"]
        assert "pre_synaptic_element" in sp_synapses
        assert "post_synaptic_element" in sp_synapses
        assert sp_synapses["pre_synaptic_element"] == "Axon_ex"
        assert sp_synapses["post_synaptic_element"] == "Den_ex"

        assert nest.structural_plasticity_update_interval == 10000.0
    
    def test_synapse_creation(self):
        for syn_model in nest.synapse_models:
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                syn_dict = {"synapse_model": syn_model, "pre_synaptic_element": "SE1", "post_synaptic_element": "SE2"}
                nest.structural_plasticity_synapses = {"syn1": syn_dict}
                neurons = nest.Create(
                    "iaf_psc_alpha",
                    2,
                    {
                        "synaptic_elements": {
                            "SE1": {"z": 10.0, "growth_rate": 0.0},
                            "SE2": {"z": 10.0, "growth_rate": 0.0},
                        }
                    },
                )
                nest.EnableStructuralPlasticity()
                nest.Simulate(10.0)
                status = nest.GetStatus(neurons, "synaptic_elements")
                for st_neuron in status:
                    assert st_neuron["SE1"]["z_connected"] == 10
                    assert st_neuron["SE2"]["z_connected"] == 10

                assert len(nest.GetConnections(neurons, neurons, syn_model)) == 20
                break

    
    def test_synapse_creation_distance_dependent(self):
        for syn_model in nest.synapse_models:
            if syn_model not in self.exclude_synapse_model:
                nest.ResetKernel()
                positions = np.random.uniform([0, 0], [10, 10], size=(2, 2))

                syn_dict = {"synapse_model": syn_model, "pre_synaptic_element": "SE1", "post_synaptic_element": "SE2"}
                nest.structural_plasticity_synapses = {"syn1": syn_dict}
                neurons = nest.Create(
                    "iaf_psc_alpha",
                    2,
                    {
                        "synaptic_elements": {
                            "SE1": {"z": 10.0, "growth_rate": 0.0},
                            "SE2": {"z": 10.0, "growth_rate": 0.0},
                        }
                    }, 
                    positions=nest.spatial.free(pos=positions.tolist())
                )        
                nest.structural_plasticity_gaussian_kernel_sigma = 1.0
                nest.structural_plasticity_cache_probabilities = True
                nest.EnableStructuralPlasticity()
                nest.Simulate(10.0)
                status = nest.GetStatus(neurons, "synaptic_elements")
                for st_neuron in status:
                    assert st_neuron["SE1"]["z_connected"] == 10
                    assert st_neuron["SE2"]["z_connected"] == 10

                assert len(nest.GetConnections(neurons, neurons, syn_model)) == 20
                break


    def test_structural_plasticity_with_positions(self):
        #Test structural plasticity when neurons have positions in multiple dimensions.
        nest.ResetKernel()
        nest.structural_plasticity_update_interval = 1.0
        nest.structural_plasticity_gaussian_kernel_sigma = 1.0

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

        neurons = nest.Create("iaf_psc_alpha", num_neurons, neuron_params, positions=nest.spatial.free(pos=positions.tolist()))

        nest.EnableStructuralPlasticity()
        nest.Simulate(10.0)

        connections = nest.GetConnections(neurons, neurons, synapse_model="sp_synapse")
        self.assertGreater(len(connections), 0, "No connections were created")

        # Check that neurons closer together are more likely to be connected
        distances = []
        for conn in connections:
            pre_pos = positions[conn.source-1]
            post_pos = positions[conn.target-1]
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

        # Enable distance-dependent structural plasticity
        nest.structural_plasticity_gaussian_kernel_sigma = 1.0

        # Expecting a NEST CppException due to missing positions
        with self.assertRaises(nest.NESTErrors.CppException) as context:
            nest.EnableStructuralPlasticity()
            nest.Simulate(10.0)

        # Check the error message
        self.assertIn(
            "Undefined positions detected for neuron ID 1. Please provide valid positions or disable distance dependency.",
            str(context.exception)
        )
    def test_distance_dependent_without_positions(self):
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

        # Enable distance dependency
        nest.structural_plasticity_gaussian_kernel_sigma = 1.0

        # Expecting a NEST CppException due to missing positions
        with self.assertRaises(nest.NESTErrors.CppException) as context:
            nest.EnableStructuralPlasticity()
            nest.Simulate(10.0)

        # Check the error message
        self.assertIn(
            "No neurons found. Please provide positions, or disable distance dependency",
            str(context.exception)
        )


def suite():
    test_suite = unittest.makeSuite(TestStructuralPlasticityManager, "test")
    return test_suite


if __name__ == "__main__":
    unittest.main()
