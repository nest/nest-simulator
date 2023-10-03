# -*- coding: utf-8 -*-
#
# test_eprop_plasticity.py
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

"""
Test functionality of e-prop plasticity.
"""


import unittest
import nest
import numpy as np


@nest.ll_api.check_stack
class EpropPlasticityTestCase(unittest.TestCase):
    """Test Eprop Plasticity"""

    def test_ConnectNeuronsWithEpropSynapse(self):
        """Ensures that the restriction to supported neuron models works."""

        nest.set_verbosity("M_WARNING")

        supported_source_models = [
            "eprop_iaf_psc_delta",
            "eprop_iaf_psc_delta_adapt",
        ]
        supported_target_models = supported_source_models + ["eprop_readout"]

        # connect supported models with e-prop synapse
        for nms in supported_source_models:
            for nmt in supported_target_models:
                nest.ResetKernel()

                ns = nest.Create(nms, 1)
                nt = nest.Create(nmt, 1)

                nest.Connect(ns, nt, {"rule": "all_to_all"}, {"synapse_model": "eprop_synapse"})

        # ensure that connecting not supported models fails
        for nm in [n for n in nest.node_models if n not in supported_target_models]:
            nest.ResetKernel()

            n = nest.Create(nm, 2)

            # try to connect with e-prop synapse
            with self.assertRaises(nest.kernel.NESTError):
                nest.Connect(n, n, {"rule": "all_to_all"}, {"synapse_model": "eprop_synapse"})

    def test_EpropRegression(self):
        """
        Test correct computation of weights for a regresion task  by comparing the simulated weights with
        weights obtained in a simulation with the original, verified NEST implementation.
        """

        update_interval = 20.0
        resolution = 1.0
        start = 15.0

        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.set(
            **{
                "eprop_update_interval": update_interval,
                "eprop_reset_neurons_on_update": True,
                "resolution": resolution,
            }
        )

        params_common_syn_eprop = {
            "adam_beta1": 0.9,
            "adam_beta2": 0.999,
            "adam_epsilon": 1e-8,
            "optimizer": "adam",
            "batch_size": 2,
            "recall_duration": 1.0,
        }

        nest.SetDefaults("eprop_synapse", params_common_syn_eprop)

        input_spike_times = [
            3,
            9,
            12,
            12,
            24,
            25,
            27,
            27,
            30,
            31,
            31,
            32,
            33,
            35,
            36,
            37,
            42,
            44,
            51,
            61,
            63,
            67,
            74,
            86,
            87,
            90,
            91,
            94,
            95,
            97,
            98,
            102,
            105,
            106,
            107,
            112,
            113,
            117,
            118,
            118,
            122,
            137,
            142,
            145,
            148,
            159,
            161,
            163,
            163,
            165,
            166,
            172,
            173,
            177,
            178,
            185,
            187,
            192,
            196,
            198,
        ]

        amplitude_times = [
            4,
            7,
            9,
            22,
            31,
            34,
            47,
            48,
            49,
            50,
            51,
            52,
            64,
            65,
            66,
            67,
            70,
            72,
            73,
            76,
            77,
            80,
            86,
            90,
            92,
            116,
            117,
            124,
            125,
            129,
            131,
            147,
            160,
            162,
            180,
            185,
            189,
            193,
            195,
            197,
        ]

        amplitude_values = [
            -0.20482851,
            0.40339019,
            0.04219736,
            -0.03976273,
            -0.25253361,
            -0.41144706,
            0.47646563,
            -0.34712983,
            -0.39120884,
            0.37268387,
            0.50902544,
            0.59836020,
            -0.27753054,
            -0.10917075,
            -0.61107463,
            -0.47397566,
            0.49120301,
            0.41084973,
            0.17864480,
            0.80558483,
            0.16935632,
            0.86313939,
            0.71965767,
            -0.44847632,
            0.23480911,
            -0.46243277,
            0.52705365,
            -0.30392348,
            -0.48669351,
            -0.45724139,
            0.40109051,
            -0.35719718,
            -0.85236732,
            -0.17672786,
            -0.47658833,
            0.46707856,
            -0.91437254,
            0.86384759,
            0.83470746,
            0.11669997,
        ]

        neuron_params_adaptive = {
            "C_m": 20.0,
            "E_L": 0.0,
            "I_e": 0.0,
            "V_m": 0.0,
            "V_th": 0.6,
            "adapt_beta": 0.017424185109823985,
            "adapt_tau": 2000.0,
            "adaptation": 0.0,
            "gamma": 0.3,
            "t_ref": 5.0 * resolution,
            "tau_m": 20.0,
            "propagator_idx": 0,
        }

        neuron_params_regular = {
            "C_m": 20.0,
            "c_reg": 150.0,
            "E_L": 0.0,
            "f_target": 10.0,
            "gamma": 0.3,
            "I_e": 0.0,
            "t_ref": 5.0 * resolution,
            "tau_m": 20.0,
            "V_m": 0.0,
            "V_th": 0.6,
            "propagator_idx": 0,
        }

        neuron_params_readout = {
            "C_m": 20.0,
            "E_L": 0.0,
            "I_e": 0.0,
            "loss": "mean_squared_error",
            "start_learning": start,
            "tau_m": 20.0,
            "V_m": 0.0,
        }

        params_syn_in = {
            "delay": resolution,
            "eta": 1e-3,
            "synapse_model": "eprop_synapse_rec",
            "tau_m_readout": 20.0,
            "weight": np.array([0.2149, -0.24120]).reshape(2, 1),
            "Wmax": 100.0,
            "Wmin": -100.0,
        }

        params_syn_rec = {
            "delay": resolution,
            "eta": 1e-3,
            "synapse_model": "eprop_synapse_rec",
            "tau_m_readout": 20.0,
            "weight": np.array([[0.0, 0.3193], [0.1466, 0.0]]),
            "Wmax": 100.0,
            "Wmin": -100.0,
        }

        params_syn_out = {
            "delay": resolution,
            "eta": 1e-3,
            "synapse_model": "eprop_synapse_rec",
            "tau_m_readout": 20.0,
            "weight": np.array([-0.956, 0.8101]).reshape(1, 2),
            "Wmax": 100.0,
            "Wmin": -100.0,
        }

        params_syn_feedback = {
            "delay": resolution,
            "synapse_model": "eprop_learning_signal_connection",
            "weight": np.array([0.0128, -0.7011]).reshape(2, 1),
        }

        params_syn_target_rate = {
            "delay": resolution,
            "receptor_type": 2,
            "synapse_model": "rate_connection_delayed",
            "weight": 1.0,
        }

        params_syn_static = {
            "delay": resolution,
            "synapse_model": "static_synapse",
            "weight": 1.0,
        }

        target_generator = nest.Create(
            "step_rate_generator", {"amplitude_times": amplitude_times, "amplitude_values": amplitude_values}
        )

        spike_generator = nest.Create("spike_generator", {"spike_times": input_spike_times})

        wr = nest.Create("weight_recorder", 1)
        nest.CopyModel("eprop_synapse", "eprop_synapse_rec", {"weight_recorder": wr})

        in_nrns = nest.Create("parrot_neuron", 1)
        neuron_reg = nest.Create("eprop_iaf_psc_delta", 1, neuron_params_regular)
        neuron_ad = nest.Create("eprop_iaf_psc_delta_adapt", 1, neuron_params_adaptive)
        rec_nrns = neuron_reg + neuron_ad
        out_nrns = nest.Create("eprop_readout", 1, neuron_params_readout)

        conn_ata = {"rule": "all_to_all", "allow_autapses": False}
        nest.Connect(spike_generator, in_nrns, {"rule": "one_to_one"}, params_syn_static)
        nest.Connect(in_nrns, rec_nrns, {"rule": "all_to_all"}, params_syn_in)
        nest.Connect(rec_nrns, rec_nrns, conn_ata, params_syn_rec)
        nest.Connect(rec_nrns, out_nrns, conn_ata, params_syn_out)
        nest.Connect(out_nrns, rec_nrns, conn_ata, params_syn_feedback)
        nest.Connect(target_generator, out_nrns, {"rule": "one_to_one"}, params_syn_target_rate)

        nest.Simulate(202.0)

        # Evaluation
        syn_weights = np.array(nest.GetStatus(wr)[0]["events"]["weights"])

        correct_weights = [
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2158999707148223,
            -0.2412,
            0.2158999707148223,
            -0.2412,
            0.2158999707148223,
            -0.2412,
            0.2158999707148223,
            -0.2412,
            0.2158999707148223,
            -0.2412,
            0.2158999707148223,
            -0.2412,
            0.2158999707148223,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.2167103359116738,
            -0.2412,
            0.21759003475854116,
            -0.2412,
            0.21759003475854116,
            -0.2412,
            0.21759003475854116,
            -0.2412,
            0.21759003475854116,
            -0.2412,
            0.21759003475854116,
            -0.2412,
            0.21759003475854116,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
            0.2183626908070995,
            -0.2412,
        ]

        self.assertTrue(np.allclose(syn_weights, correct_weights, rtol=1e-7))

    def test_EpropClassification(self):
        """
        Test correct computation of weights for a classification task by comparing the simulated weights with
        weights obtained in a simulation with the original, verified NEST implementation.
        """

        update_interval = 20.0
        resolution = 1.0
        start = 15.0

        nest.set_verbosity("M_WARNING")
        nest.ResetKernel()
        nest.set(
            **{
                "resolution": resolution,
            }
        )

        nest.eprop_update_interval = update_interval
        nest.eprop_reset_neurons_on_update = True

        params_common_syn_eprop = {
            "adam_beta1": 0.9,
            "adam_beta2": 0.999,
            "adam_epsilon": 1e-8,
            "optimizer": "adam",
            "batch_size": 2,
            "recall_duration": 1.0,
        }

        nest.SetDefaults("eprop_synapse", params_common_syn_eprop)

        input_spike_times = [
            3,
            9,
            12,
            12,
            24,
            25,
            27,
            27,
            30,
            31,
            31,
            32,
            33,
            35,
            36,
            37,
            42,
            44,
            51,
            61,
            63,
            67,
            74,
            86,
            87,
            90,
            91,
            94,
            95,
            97,
            98,
            102,
            105,
            106,
            107,
            112,
            113,
            117,
            118,
            118,
            122,
            137,
            142,
            145,
            148,
            159,
            161,
            163,
            163,
            165,
            166,
            172,
            173,
            177,
            178,
            185,
            187,
            192,
            196,
            198,
        ]

        amplitude_times = [
            4,
            7,
            9,
            22,
            31,
            34,
            47,
            48,
            49,
            50,
            51,
            52,
            64,
            65,
            66,
            67,
            70,
            72,
            73,
            76,
            77,
            80,
            86,
            90,
            92,
            116,
            117,
            124,
            125,
            129,
            131,
            147,
            160,
            162,
            180,
            185,
            189,
            193,
            195,
            197,
        ]

        amplitude_values = [
            -0.20482851,
            0.40339019,
            0.04219736,
            -0.03976273,
            -0.25253361,
            -0.41144706,
            0.47646563,
            -0.34712983,
            -0.39120884,
            0.37268387,
            0.50902544,
            0.59836020,
            -0.27753054,
            -0.10917075,
            -0.61107463,
            -0.47397566,
            0.49120301,
            0.41084973,
            0.17864480,
            0.80558483,
            0.16935632,
            0.86313939,
            0.71965767,
            -0.44847632,
            0.23480911,
            -0.46243277,
            0.52705365,
            -0.30392348,
            -0.48669351,
            -0.45724139,
            0.40109051,
            -0.35719718,
            -0.85236732,
            -0.17672786,
            -0.47658833,
            0.46707856,
            -0.91437254,
            0.86384759,
            0.83470746,
            0.11669997,
        ]

        neuron_params_adaptive = {
            "adapt_beta": 0.017424185109823985,
            "adapt_tau": 2000.0,
            "adaptation": 0.0,
            "C_m": 20.0,
            "c_reg": 150.0,
            "E_L": 0.0,
            "f_target": 10.0,
            "gamma": 0.3,
            "I_e": 0.0,
            "t_ref": 5.0 * resolution,
            "tau_m": 20.0,
            "V_m": 0.0,
            "V_th": 0.6,
            "propagator_idx": 1,
        }

        neuron_params_regular = {
            "C_m": 20.0,
            "c_reg": 150.0,
            "E_L": 0.0,
            "f_target": 10.0,
            "gamma": 0.3,
            "I_e": 0.0,
            "t_ref": 5.0 * resolution,
            "tau_m": 20.0,
            "V_m": 0.0,
            "V_th": 0.6,
            "propagator_idx": 1,
        }

        neuron_params_readout = {
            "C_m": 20.0,
            "E_L": 0.0,
            "I_e": 0.0,
            "loss": "softmax",
            "start_learning": start,
            "tau_m": 20.0,
            "V_m": 0.0,
        }

        params_syn_in = {
            "delay": resolution,
            "eta": 1e-3,
            "synapse_model": "eprop_synapse_rec",
            "tau_m_readout": 20.0,
            "weight": np.array([0.2149, -0.24120]).reshape(2, 1),
            "Wmax": 100.0,
            "Wmin": -100.0,
        }

        params_syn_rec = {
            "delay": resolution,
            "eta": 1e-3,
            "synapse_model": "eprop_synapse_rec",
            "tau_m_readout": 20.0,
            "weight": np.array([[0.0, 0.3193], [0.1466, 0.0]]),
            "Wmax": 100.0,
            "Wmin": -100.0,
        }

        params_syn_out = {
            "delay": resolution,
            "eta": 1e-3,
            "synapse_model": "eprop_synapse_rec",
            "tau_m_readout": 20.0,
            "weight": np.array([[-0.956, 0.8101], [0.7144, 0.6283]]),
            "Wmax": 100.0,
            "Wmin": -100.0,
        }

        params_syn_out_out = {
            "delay": resolution,
            "receptor_type": 1,
            "synapse_model": "rate_connection_delayed",
            "weight": 1.0,
        }

        params_syn_feedback = {
            "delay": resolution,
            "synapse_model": "eprop_learning_signal_connection",
            "weight": np.array([[0.0128, -0.7011], [-0.2649, 0.8783]]),
        }

        params_syn_target_rate = {
            "delay": resolution,
            "receptor_type": 2,
            "synapse_model": "rate_connection_delayed",
            "weight": 1.0,
        }

        params_syn_static = {"synapse_model": "static_synapse", "weight": 1.0, "delay": resolution}

        target_generator = nest.Create(
            "step_rate_generator", {"amplitude_times": amplitude_times, "amplitude_values": amplitude_values}
        )

        spike_generator = nest.Create("spike_generator", {"spike_times": input_spike_times})

        wr = nest.Create("weight_recorder", 1)
        nest.CopyModel("eprop_synapse", "eprop_synapse_rec", {"weight_recorder": wr})

        in_nrns = nest.Create("parrot_neuron", 1)
        neuron_reg = nest.Create("eprop_iaf_psc_delta", 1, neuron_params_regular)
        neuron_ad = nest.Create("eprop_iaf_psc_delta_adapt", 1, neuron_params_adaptive)
        rec_nrns = neuron_reg + neuron_ad
        out_nrns = nest.Create("eprop_readout", 2, neuron_params_readout)

        conn_ata = {"rule": "all_to_all", "allow_autapses": False}
        nest.Connect(spike_generator, in_nrns, {"rule": "one_to_one"}, params_syn_static)
        nest.Connect(in_nrns, rec_nrns, {"rule": "all_to_all"}, params_syn_in)
        nest.Connect(rec_nrns, rec_nrns, conn_ata, params_syn_rec)
        nest.Connect(rec_nrns, out_nrns, conn_ata, params_syn_out)
        nest.Connect(out_nrns, rec_nrns, conn_ata, params_syn_feedback)
        nest.Connect(out_nrns, out_nrns, conn_ata, params_syn_out_out)
        nest.Connect(target_generator, out_nrns, {"rule": "all_to_all"}, params_syn_target_rate)

        nest.Simulate(202.0)

        # Evaluation
        syn_weights = np.array(nest.GetStatus(wr)[0]["events"]["weights"])

        correct_weights = [
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.1466,
            -0.956,
            0.7144,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.1466,
            -0.956,
            0.7144,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.1466,
            -0.956,
            0.7144,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.2149,
            -0.2412,
            0.1466,
            -0.956,
            0.7144,
            0.21390000010339627,
            -0.2412,
            0.21390000010339627,
            -0.2412,
            0.21390000010339627,
            -0.2412,
            0.21390000010339627,
            -0.2412,
            0.21390000010339627,
            -0.2412,
            0.21390000010339627,
            -0.2412,
            0.21390000010339627,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.1466,
            -0.9576700561411153,
            0.7127299436821352,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.1466,
            -0.9576700561411153,
            0.7127299436821352,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.1466,
            -0.9576700561411153,
            0.7127299436821352,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.21348690635327325,
            -0.2412,
            0.1466,
            -0.9576700561411153,
            0.7127299436821352,
            0.2127739356753828,
            -0.2412,
            0.2127739356753828,
            -0.2412,
            0.2127739356753828,
            -0.2412,
            0.2127739356753828,
            -0.2412,
            0.2127739356753828,
            -0.2412,
            0.2127739356753828,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.1466,
            -0.9588076969014643,
            0.7115488877968893,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.1466,
            -0.9588076969014643,
            0.7115488877968893,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.21225362092242817,
            -0.2412,
            0.1466,
            -0.9588076969014643,
            0.7115488877968893,
            0.21225362092242817,
            -0.2412,
        ]

        self.assertTrue(np.allclose(syn_weights, correct_weights, rtol=1e-7))


def suite():
    suite = unittest.makeSuite(EpropPlasticityTestCase, "test")
    return suite


def run():
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite())


if __name__ == "__main__":
    run()
