# -*- coding: utf-8 -*-
#
# test_psp_amplitude_consistency.py
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
Name: testsuite::test_psp_amplitude_consistency - test the consistency of PSP amplitudes across models.

Synopsis: (test_psp_amplitude_consistency) run ->

Description:

This test computes the peak amplitude of the postsynaptic potential
for different neuron models and checks whether their PSP peak
amplitudes match approximately.

The maximal PSP amplitude is computed for different synaptic time
constants to ensure that the scaling of the postsynaptic response is
consinstent between models.

Test ported from SLI unittest.

Author:  December 2012, Gewaltig
SeeAlso: testsuite::test_iaf_psp, testsuite::test_iaf_ps_dc_accuracy
"""

import nest
import numpy as np
import pytest


def compute_psp(model, params, tau_syn):
    """
    Compute PSP peak amplitude for a given model and tau_syn.
    """
    nest.ResetKernel()
    nest.set(dict_miss_is_error=False)

    sg = nest.Create("spike_generator")
    sg.set(
        {
            "origin": 50.0,  # in ms
            "spike_times": [20.0],  # in ms
            "start": 0.0,  # in ms
            "stop": 50.0,  # in ms
        }
    )

    vm = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": 0.1})

    # Create neuron first (like SLI code does), then set parameters
    # This allows dict_miss_is_error=False to work properly
    neuron = nest.Create(model)

    neuron_params = params.copy()
    # Add tau_syn_ex to params (matching SLI: params /tau_syn_ex tau_syn put)
    neuron_params["tau_syn_ex"] = tau_syn
    # Set all parameters - dict_miss_is_error=False will ignore invalid ones
    neuron.set(neuron_params)

    nest.Connect(sg, neuron, syn_spec={"weight": 1.5, "delay": 1.0})
    nest.Connect(vm, neuron)

    nest.Simulate(150.0)

    V_m_values = vm.get("events", "V_m")
    V_max = np.max(V_m_values) if len(V_m_values) > 0 else 0.0

    return V_max


def test_psp_amplitude_consistency():
    """
    Test that PSP amplitudes are consistent across different neuron models.
    """
    nest.set_verbosity("M_ERROR")

    tau_syns = [0.5, 1.0, 2.0, 5.0, 15.0]  # synaptic time constants to test

    # Parameters for conductance based models
    P_cond = {
        "E_L": 0.0,  # resting potential in mV
        "g_L": 30.0,
        "V_m": 0.0,  # initial membrane potential in mV
        "V_th": 30.0,  # spike threshold in mV
        "I_e": 0.0,  # DC current in pA
        "E_ex": 40.0,
        "E_Na": 40.0,
        "E_in": -40.0,
        "E_K": -40.0,
        "C_m": 250.0,  # membrane capacity in pF
        "V_peak": 50.0,
    }

    # Parameters for current based models
    P_psc = {
        "tau_m": 10.0,
        "E_L": 0.0,  # resting potential in mV
        "V_m": 0.0,  # initial membrane potential in mV
        "V_th": 15.0,  # spike threshold in mV
        "I_e": 0.0,  # DC current in pA
        "C_m": 250.0,  # membrane capacity in pF
    }

    # Testing conductance based alpha response models
    node_models = nest.GetKernelStatus("node_models")
    if "aeif_cond_alpha" in node_models:
        # Test aeif_cond_alpha vs iaf_cond_alpha
        psp_aeif = [compute_psp("aeif_cond_alpha", P_cond, tau_syn) for tau_syn in tau_syns]
        psp_iaf = [compute_psp("iaf_cond_alpha", P_cond, tau_syn) for tau_syn in tau_syns]

        differences = np.array(psp_aeif) - np.array(psp_iaf)
        mean_squared_diff = np.mean(differences**2)
        assert mean_squared_diff < 1e-4, f"PSP mismatch between aeif_cond_alpha and iaf_cond_alpha: {mean_squared_diff}"

        # Test aeif_cond_exp vs iaf_cond_exp
        psp_aeif_exp = [compute_psp("aeif_cond_exp", P_cond, tau_syn) for tau_syn in tau_syns]
        psp_iaf_exp = [compute_psp("iaf_cond_exp", P_cond, tau_syn) for tau_syn in tau_syns]

        differences_exp = np.array(psp_aeif_exp) - np.array(psp_iaf_exp)
        mean_squared_diff_exp = np.mean(differences_exp**2)
        assert (
            mean_squared_diff_exp < 1e-4
        ), f"PSP mismatch between aeif_cond_exp and iaf_cond_exp: {mean_squared_diff_exp}"

    if "iaf_cond_exp_sfa_rr" in node_models:
        # Test iaf_cond_exp_sfa_rr vs iaf_cond_exp
        psp_sfa_rr = [compute_psp("iaf_cond_exp_sfa_rr", P_cond, tau_syn) for tau_syn in tau_syns]
        psp_iaf_exp = [compute_psp("iaf_cond_exp", P_cond, tau_syn) for tau_syn in tau_syns]

        differences_sfa = np.array(psp_sfa_rr) - np.array(psp_iaf_exp)
        mean_squared_diff_sfa = np.mean(differences_sfa**2)
        assert (
            mean_squared_diff_sfa < 1e-4
        ), f"PSP mismatch between iaf_cond_exp_sfa_rr and iaf_cond_exp: {mean_squared_diff_sfa}"
