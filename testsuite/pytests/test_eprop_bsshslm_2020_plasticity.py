# -*- coding: utf-8 -*-
#
# test_eprop_bsshslm_2020_plasticity.py
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

import nest
import numpy as np
import pytest

nest.set_verbosity("M_WARNING")

supported_source_models = ["eprop_iaf_bsshslm_2020", "eprop_iaf_adapt_bsshslm_2020"]
supported_target_models = supported_source_models + ["eprop_readout_bsshslm_2020"]


@pytest.fixture(autouse=True)
def fix_resolution():
    nest.ResetKernel()


@pytest.mark.parametrize("source_model", supported_source_models)
@pytest.mark.parametrize("target_model", supported_target_models)
def test_connect_with_eprop_synapse(source_model, target_model):
    """Ensures that the restriction to supported neuron models works."""

    # Connect supported models with e-prop synapse
    src = nest.Create(source_model)
    tgt = nest.Create(target_model)
    nest.Connect(src, tgt, "all_to_all", {"synapse_model": "eprop_synapse_bsshslm_2020", "delay": nest.resolution})


@pytest.mark.parametrize("target_model", set(nest.node_models) - set(supported_target_models))
def test_unsupported_model_raises(target_model):
    """Confirm that connecting a non-eprop neuron as target via an eprop_synapse_bsshslm_2020 raises an error."""

    src_nrn = nest.Create(supported_source_models[0])
    tgt_nrn = nest.Create(target_model)

    with pytest.raises(nest.kernel.NESTError):
        nest.Connect(src_nrn, tgt_nrn, "all_to_all", {"synapse_model": "eprop_synapse_bsshslm_2020"})


def test_eprop_regression():
    """
    Test correct computation of losses for a regression task
    (for details on the task, see nest-simulator/pynest/examples/eprop_plasticity/eprop_supervised_regression_sine-waves.py)
    by comparing the simulated losses with

        1. NEST reference losses to catch scenarios in which the e-prop model does not work as intended (e.g.,
           potential future changes to the NEST code base or a faulty installation). These reference losses
           were obtained from a simulation with the verified NEST e-prop implementation run with
           Linux 4.15.0-213-generic, Python v3.11.6, Numpy v1.26.0, and NEST@3304c6b5c.

        2. TensorFlow reference losses to check the faithfulness to the original model. These reference losses were
           obtained from a simulation with the original TensorFlow implementation
           (https://github.com/INM-6/eligibility_propagation/blob/eprop_in_nest/Figure_3_and_S7_e_prop_tutorials/tutorial_pattern_generation.py,
            a modified fork of the original model at https://github.com/IGITUGraz/eligibility_propagation) run with
            Linux 4.15.0-213-generic, Python v3.6.10, Numpy v1.18.0, TensorFlow v1.15.0, and
            INM6/eligibility_propagation@7df7d2627.
    """  # pylint: disable=line-too-long # noqa: E501

    # Initialize random generator
    rng_seed = 1
    np.random.seed(rng_seed)

    # Define timing of task

    n_batch = 1
    n_iter = 5

    steps = {
        "sequence": 1000,
    }

    steps["learning_window"] = steps["sequence"]
    steps["task"] = n_iter * n_batch * steps["sequence"]

    steps.update(
        {
            "offset_gen": 1,
            "delay_in_rec": 1,
            "delay_rec_out": 1,
            "delay_out_norm": 1,
            "extension_sim": 1,
        }
    )

    steps["total_offset"] = (
        steps["offset_gen"] + steps["delay_in_rec"] + steps["delay_rec_out"] + steps["delay_out_norm"]
    )

    steps["sim"] = steps["task"] + steps["total_offset"] + steps["extension_sim"]

    duration = {"step": 1.0}

    duration.update({key: value * duration["step"] for key, value in steps.items()})

    # Set up simulation

    params_setup = {
        "eprop_learning_window": duration["learning_window"],
        "eprop_reset_neurons_on_update": True,
        "eprop_update_interval": duration["sequence"],
        "print_time": False,
        "resolution": duration["step"],
        "total_num_virtual_procs": 1,
    }

    nest.ResetKernel()
    nest.set(**params_setup)

    # Create neurons

    n_in = 100
    n_rec = 100
    n_out = 1

    params_nrn_rec = {
        "C_m": 1.0,
        "c_reg": 300.0,
        "gamma": 0.3,
        "E_L": 0.0,
        "f_target": 10.0,
        "I_e": 0.0,
        "regular_spike_arrival": False,
        "surrogate_gradient_function": "piecewise_linear",
        "t_ref": 0.0,
        "tau_m": 30.0,
        "V_m": 0.0,
        "V_th": 0.03,
    }

    params_nrn_out = {
        "C_m": 1.0,
        "E_L": 0.0,
        "I_e": 0.0,
        "loss": "mean_squared_error",
        "regular_spike_arrival": False,
        "tau_m": 30.0,
        "V_m": 0.0,
    }

    gen_spk_in = nest.Create("spike_generator", n_in)
    nrns_in = nest.Create("parrot_neuron", n_in)
    nrns_rec = nest.Create("eprop_iaf_bsshslm_2020", n_rec, params_nrn_rec)
    nrns_out = nest.Create("eprop_readout_bsshslm_2020", n_out, params_nrn_out)
    gen_rate_target = nest.Create("step_rate_generator", n_out)

    # Create recorders

    n_record = 1
    n_record_w = 1

    params_mm_rec = {
        "record_from": ["V_m", "surrogate_gradient", "learning_signal"],
        "start": duration["offset_gen"] + duration["delay_in_rec"],
        "interval": duration["sequence"],
    }

    params_mm_out = {
        "record_from": ["V_m", "readout_signal", "readout_signal_unnorm", "target_signal", "error_signal"],
        "start": duration["total_offset"],
        "interval": duration["step"],
    }

    params_wr = {
        "senders": nrns_in[:n_record_w] + nrns_rec[:n_record_w],
        "targets": nrns_rec[:n_record_w] + nrns_out,
    }

    mm_rec = nest.Create("multimeter", params_mm_rec)
    mm_out = nest.Create("multimeter", params_mm_out)
    sr = nest.Create("spike_recorder")
    wr = nest.Create("weight_recorder", params_wr)

    nrns_rec_record = nrns_rec[:n_record]

    # Create connections

    params_conn_all_to_all = {"rule": "all_to_all", "allow_autapses": False}
    params_conn_one_to_one = {"rule": "one_to_one"}

    dtype_weights = np.float32
    weights_in_rec = np.array(np.random.randn(n_in, n_rec).T / np.sqrt(n_in), dtype=dtype_weights)
    weights_rec_rec = np.array(np.random.randn(n_rec, n_rec).T / np.sqrt(n_rec), dtype=dtype_weights)
    np.fill_diagonal(weights_rec_rec, 0.0)
    weights_rec_out = np.array(np.random.randn(n_rec, n_out).T / np.sqrt(n_rec), dtype=dtype_weights)
    weights_out_rec = np.array(np.random.randn(n_rec, n_out) / np.sqrt(n_rec), dtype=dtype_weights)

    params_common_syn_eprop = {
        "optimizer": {
            "type": "gradient_descent",
            "batch_size": n_batch,
            "eta": 1e-4,
            "Wmin": -100.0,
            "Wmax": 100.0,
        },
        "weight_recorder": wr,
        "average_gradient": False,
    }

    params_syn_in = {
        "synapse_model": "eprop_synapse_bsshslm_2020",
        "delay": duration["step"],
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_in_rec,
    }

    params_syn_rec = {
        "synapse_model": "eprop_synapse_bsshslm_2020",
        "delay": duration["step"],
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_rec_rec,
    }

    params_syn_out = {
        "synapse_model": "eprop_synapse_bsshslm_2020",
        "delay": duration["step"],
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_rec_out,
    }

    params_syn_feedback = {
        "synapse_model": "eprop_learning_signal_connection_bsshslm_2020",
        "delay": duration["step"],
        "weight": weights_out_rec,
    }

    params_syn_rate_target = {
        "synapse_model": "rate_connection_delayed",
        "delay": duration["step"],
        "receptor_type": 2,
    }

    params_syn_static = {
        "synapse_model": "static_synapse",
        "delay": duration["step"],
    }

    nest.SetDefaults("eprop_synapse_bsshslm_2020", params_common_syn_eprop)

    nest.Connect(gen_spk_in, nrns_in, params_conn_one_to_one, params_syn_static)
    nest.Connect(nrns_in, nrns_rec, params_conn_all_to_all, params_syn_in)
    nest.Connect(nrns_rec, nrns_rec, params_conn_all_to_all, params_syn_rec)
    nest.Connect(nrns_rec, nrns_out, params_conn_all_to_all, params_syn_out)
    nest.Connect(nrns_out, nrns_rec, params_conn_all_to_all, params_syn_feedback)
    nest.Connect(gen_rate_target, nrns_out, params_conn_one_to_one, params_syn_rate_target)

    nest.Connect(nrns_in + nrns_rec, sr, params_conn_all_to_all, params_syn_static)

    nest.Connect(mm_rec, nrns_rec_record, params_conn_all_to_all, params_syn_static)
    nest.Connect(mm_out, nrns_out, params_conn_all_to_all, params_syn_static)

    # Create input

    input_spike_prob = 0.05
    dtype_in_spks = np.float32

    input_spike_bools = np.random.rand(n_batch, steps["sequence"], n_in) < input_spike_prob
    input_spike_bools = np.hstack(input_spike_bools.swapaxes(1, 2))
    input_spike_bools[:, 0] = 0

    sequence_starts = np.arange(0.0, duration["task"], duration["sequence"]) + duration["offset_gen"]
    params_gen_spk_in = []
    for input_spike_bool in input_spike_bools:
        input_spike_times = np.arange(0.0, duration["sequence"] * n_batch, duration["step"])[input_spike_bool]
        input_spike_times_all = [input_spike_times + start for start in sequence_starts]
        params_gen_spk_in.append({"spike_times": np.hstack(input_spike_times_all).astype(dtype_in_spks)})

    nest.SetStatus(gen_spk_in, params_gen_spk_in)

    # Create output

    def generate_superimposed_sines(steps_sequence, periods):
        n_sines = len(periods)

        amplitudes = np.random.uniform(low=0.5, high=2.0, size=n_sines)
        phases = np.random.uniform(low=0.0, high=2.0 * np.pi, size=n_sines)

        sines = [
            A * np.sin(np.linspace(phi, phi + 2.0 * np.pi * (steps_sequence // T), steps_sequence))
            for A, phi, T in zip(amplitudes, phases, periods)
        ]

        superposition = sum(sines)
        superposition -= superposition[0]
        superposition /= max(np.abs(superposition).max(), 1e-6)
        return superposition

    target_signal = generate_superimposed_sines(steps["sequence"], [1000, 500, 333, 200])

    params_gen_rate_target = {
        "amplitude_times": np.arange(0.0, duration["task"], duration["step"]) + duration["total_offset"],
        "amplitude_values": np.tile(target_signal, n_iter * n_batch),
    }

    nest.SetStatus(gen_rate_target, params_gen_rate_target)

    # Simulate

    nest.Simulate(duration["sim"])

    # Read out recorders

    events_mm_out = mm_out.get("events")

    # Evaluate training error

    readout_signal = events_mm_out["readout_signal"]
    target_signal = events_mm_out["target_signal"]

    error = (readout_signal - target_signal) ** 2
    loss = 0.5 * np.add.reduceat(error, np.arange(0, steps["task"], steps["sequence"]))

    # Verify results

    loss_NEST_reference = np.array(
        [
            101.964356999041,
            103.466731126205,
            103.340607074771,
            103.680244037686,
            104.412775748752,
        ]
    )

    loss_TF_reference = np.array(
        [
            101.964363098144,
            103.466735839843,
            103.340606689453,
            103.680244445800,
            104.412780761718,
        ]
    )

    assert np.allclose(loss, loss_NEST_reference, rtol=1e-8)
    assert np.allclose(loss, loss_TF_reference, rtol=1e-7)


def test_eprop_classification():
    """
    Test correct computation of losses for a classification task
    (for details on the task, see nest-simulator/pynest/examples/eprop_plasticity/eprop_supervised_classification_evidence-accumulation.py)
    by comparing the simulated losses with

        1. NEST reference losses to catch scenarios in which the e-prop model does not work as intended (e.g.,
           potential future changes to the NEST code base or a faulty installation). These reference losses
           were obtained from a simulation with the verified NEST e-prop implementation run with
           Linux 4.15.0-213-generic, Python v3.11.6, Numpy v1.26.0, and NEST@3304c6b5c.

        2. TensorFlow reference losses to check the faithfulness to the original model. These reference losses were
           obtained from a simulation with the original TensorFlow implementation
           (https://github.com/INM-6/eligibility_propagation/blob/eprop_in_nest/Figure_3_and_S7_e_prop_tutorials/tutorial_evidence_accumulation_with_alif.py,
           a modified fork of the original model at https://github.com/IGITUGraz/eligibility_propagation) run with
           Linux 4.15.0-213-generic, Python v3.6.10, Numpy v1.18.0, TensorFlow v1.15.0, and
           INM6/eligibility_propagation@7df7d2627.
    """  # pylint: disable=line-too-long # noqa: E501

    # Initialize random generator

    rng_seed = 1
    np.random.seed(rng_seed)

    # Define timing of task

    n_batch = 1
    n_iter = 5

    n_input_symbols = 4
    n_cues = 7
    prob_group = 0.3

    steps = {
        "cue": 100,
        "spacing": 50,
        "bg_noise": 1050,
        "recall": 150,
    }

    steps["cues"] = n_cues * (steps["cue"] + steps["spacing"])
    steps["sequence"] = steps["cues"] + steps["bg_noise"] + steps["recall"]
    steps["learning_window"] = steps["recall"]
    steps["task"] = n_iter * n_batch * steps["sequence"]

    steps.update(
        {
            "offset_gen": 1,
            "delay_in_rec": 1,
            "delay_rec_out": 1,
            "delay_out_norm": 1,
            "extension_sim": 1,
        }
    )

    steps["total_offset"] = (
        steps["offset_gen"] + steps["delay_in_rec"] + steps["delay_rec_out"] + steps["delay_out_norm"]
    )

    steps["sim"] = steps["task"] + steps["total_offset"] + steps["extension_sim"]

    duration = {"step": 1.0}

    duration.update({key: value * duration["step"] for key, value in steps.items()})

    # Set up simulation

    params_setup = {
        "eprop_learning_window": duration["learning_window"],
        "eprop_reset_neurons_on_update": True,
        "eprop_update_interval": duration["sequence"],
        "print_time": False,
        "resolution": duration["step"],
        "total_num_virtual_procs": 1,
    }

    nest.ResetKernel()
    nest.set(**params_setup)

    # Create neurons

    n_in = 40
    n_ad = 50
    n_reg = 50
    n_rec = n_ad + n_reg
    n_out = 2

    params_nrn_reg = {
        "C_m": 1.0,
        "c_reg": 2.0,
        "E_L": 0.0,
        "f_target": 10.0,
        "gamma": 0.3,
        "I_e": 0.0,
        "regular_spike_arrival": True,
        "surrogate_gradient_function": "piecewise_linear",
        "t_ref": 5.0,
        "tau_m": 20.0,
        "V_m": 0.0,
        "V_th": 0.6,
    }

    params_nrn_ad = {
        "adapt_tau": 2000.0,
        "adaptation": 0.0,
        "C_m": 1.0,
        "c_reg": 2.0,
        "E_L": 0.0,
        "f_target": 10.0,
        "gamma": 0.3,
        "I_e": 0.0,
        "regular_spike_arrival": True,
        "surrogate_gradient_function": "piecewise_linear",
        "t_ref": 5.0,
        "tau_m": 20.0,
        "V_m": 0.0,
        "V_th": 0.6,
    }

    params_nrn_ad["adapt_beta"] = (
        1.7 * (1.0 - np.exp(-1.0 / params_nrn_ad["adapt_tau"])) / (1.0 - np.exp(-1.0 / params_nrn_ad["tau_m"]))
    )

    params_nrn_out = {
        "C_m": 1.0,
        "E_L": 0.0,
        "I_e": 0.0,
        "loss": "cross_entropy",
        "regular_spike_arrival": False,
        "tau_m": 20.0,
        "V_m": 0.0,
    }

    gen_spk_in = nest.Create("spike_generator", n_in)
    nrns_in = nest.Create("parrot_neuron", n_in)
    nrns_reg = nest.Create("eprop_iaf_bsshslm_2020", n_reg, params_nrn_reg)
    nrns_ad = nest.Create("eprop_iaf_adapt_bsshslm_2020", n_ad, params_nrn_ad)
    nrns_out = nest.Create("eprop_readout_bsshslm_2020", n_out, params_nrn_out)
    gen_rate_target = nest.Create("step_rate_generator", n_out)

    nrns_rec = nrns_reg + nrns_ad

    # Create recorders

    n_record = 1
    n_record_w = 1

    params_mm_rec = {
        "record_from": ["V_m", "surrogate_gradient", "learning_signal"],
        "start": duration["offset_gen"] + duration["delay_in_rec"],
        "interval": duration["sequence"],
    }

    params_mm_out = {
        "record_from": ["V_m", "readout_signal", "readout_signal_unnorm", "target_signal", "error_signal"],
        "start": duration["total_offset"],
        "interval": duration["step"],
    }

    params_wr = {
        "senders": nrns_in[:n_record_w] + nrns_rec[:n_record_w],
        "targets": nrns_rec[:n_record_w] + nrns_out,
    }

    mm_rec = nest.Create("multimeter", params_mm_rec)
    mm_out = nest.Create("multimeter", params_mm_out)
    sr = nest.Create("spike_recorder")
    wr = nest.Create("weight_recorder", params_wr)

    nrns_rec_record = nrns_rec[:n_record]

    # Create connections

    params_conn_all_to_all = {"rule": "all_to_all", "allow_autapses": False}
    params_conn_one_to_one = {"rule": "one_to_one"}

    def calculate_glorot_dist(fan_in, fan_out):
        glorot_scale = 1.0 / max(1.0, (fan_in + fan_out) / 2.0)
        glorot_limit = np.sqrt(3.0 * glorot_scale)
        glorot_distribution = np.random.uniform(low=-glorot_limit, high=glorot_limit, size=(fan_in, fan_out))
        return glorot_distribution

    dtype_weights = np.float32
    weights_in_rec = np.array(np.random.randn(n_in, n_rec).T / np.sqrt(n_in), dtype=dtype_weights)
    weights_rec_rec = np.array(np.random.randn(n_rec, n_rec).T / np.sqrt(n_rec), dtype=dtype_weights)
    np.fill_diagonal(weights_rec_rec, 0.0)
    weights_rec_out = np.array(calculate_glorot_dist(n_rec, n_out).T, dtype=dtype_weights)
    weights_out_rec = np.array(np.random.randn(n_rec, n_out), dtype=dtype_weights)

    params_common_syn_eprop = {
        "optimizer": {
            "type": "adam",
            "batch_size": n_batch,
            "beta_1": 0.9,
            "beta_2": 0.999,
            "epsilon": 1e-8,
            "eta": 5e-3,
            "Wmin": -100.0,
            "Wmax": 100.0,
        },
        "weight_recorder": wr,
        "average_gradient": True,
    }

    params_syn_in = {
        "synapse_model": "eprop_synapse_bsshslm_2020",
        "delay": duration["step"],
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_in_rec,
    }

    params_syn_rec = {
        "synapse_model": "eprop_synapse_bsshslm_2020",
        "delay": duration["step"],
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_rec_rec,
    }

    params_syn_out = {
        "synapse_model": "eprop_synapse_bsshslm_2020",
        "delay": duration["step"],
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_rec_out,
    }

    params_syn_feedback = {
        "synapse_model": "eprop_learning_signal_connection_bsshslm_2020",
        "delay": duration["step"],
        "weight": weights_out_rec,
    }

    params_syn_out_out = {
        "synapse_model": "rate_connection_delayed",
        "delay": duration["step"],
        "receptor_type": 1,
        "weight": 1.0,
    }

    params_syn_rate_target = {
        "synapse_model": "rate_connection_delayed",
        "delay": duration["step"],
        "receptor_type": 2,
    }

    params_syn_static = {
        "synapse_model": "static_synapse",
        "delay": duration["step"],
    }

    nest.SetDefaults("eprop_synapse_bsshslm_2020", params_common_syn_eprop)

    nest.Connect(gen_spk_in, nrns_in, params_conn_one_to_one, params_syn_static)
    nest.Connect(nrns_in, nrns_rec, params_conn_all_to_all, params_syn_in)
    nest.Connect(nrns_rec, nrns_rec, params_conn_all_to_all, params_syn_rec)
    nest.Connect(nrns_rec, nrns_out, params_conn_all_to_all, params_syn_out)
    nest.Connect(nrns_out, nrns_rec, params_conn_all_to_all, params_syn_feedback)
    nest.Connect(gen_rate_target, nrns_out, params_conn_one_to_one, params_syn_rate_target)
    nest.Connect(nrns_out, nrns_out, params_conn_all_to_all, params_syn_out_out)

    nest.Connect(nrns_in + nrns_rec, sr, params_conn_all_to_all, params_syn_static)

    nest.Connect(mm_rec, nrns_rec_record, params_conn_all_to_all, params_syn_static)
    nest.Connect(mm_out, nrns_out, params_conn_all_to_all, params_syn_static)

    # Create input and output

    def generate_evidence_accumulation_input_output(
        n_batch, n_in, prob_group, input_spike_prob, n_cues, n_input_symbols, steps
    ):
        n_pop_nrn = n_in // n_input_symbols

        prob_choices = np.array([prob_group, 1 - prob_group], dtype=np.float32)
        idx = np.random.choice([0, 1], n_batch)
        probs = np.zeros((n_batch, 2), dtype=np.float32)
        probs[:, 0] = prob_choices[idx]
        probs[:, 1] = prob_choices[1 - idx]

        batched_cues = np.zeros((n_batch, n_cues), dtype=int)
        for b_idx in range(n_batch):
            batched_cues[b_idx, :] = np.random.choice([0, 1], n_cues, p=probs[b_idx])

        input_spike_probs = np.zeros((n_batch, steps["sequence"], n_in))

        for b_idx in range(n_batch):
            for c_idx in range(n_cues):
                cue = batched_cues[b_idx, c_idx]

                step_start = c_idx * (steps["cue"] + steps["spacing"]) + steps["spacing"]
                step_stop = step_start + steps["cue"]

                pop_nrn_start = cue * n_pop_nrn
                pop_nrn_stop = pop_nrn_start + n_pop_nrn

                input_spike_probs[b_idx, step_start:step_stop, pop_nrn_start:pop_nrn_stop] = input_spike_prob

        input_spike_probs[:, -steps["recall"] :, 2 * n_pop_nrn : 3 * n_pop_nrn] = input_spike_prob
        input_spike_probs[:, :, 3 * n_pop_nrn :] = input_spike_prob / 4.0
        input_spike_bools = input_spike_probs > np.random.rand(input_spike_probs.size).reshape(input_spike_probs.shape)
        input_spike_bools[:, 0, :] = 0

        target_cues = np.zeros(n_batch, dtype=int)
        target_cues[:] = np.sum(batched_cues, axis=1) > int(n_cues / 2)

        return input_spike_bools, target_cues

    input_spike_prob = 0.04
    dtype_in_spks = np.float32

    input_spike_bools_list = []
    target_cues_list = []

    for iteration in range(n_iter):
        input_spike_bools, target_cues = generate_evidence_accumulation_input_output(
            n_batch, n_in, prob_group, input_spike_prob, n_cues, n_input_symbols, steps
        )
        input_spike_bools_list.append(input_spike_bools)
        target_cues_list.extend(target_cues.tolist())

    input_spike_bools_arr = np.array(input_spike_bools_list).reshape(steps["task"], n_in)
    timeline_task = np.arange(0.0, duration["task"], duration["step"]) + duration["offset_gen"]

    params_gen_spk_in = [
        {"spike_times": timeline_task[input_spike_bools_arr[:, nrn_in_idx]].astype(dtype_in_spks)}
        for nrn_in_idx in range(n_in)
    ]

    target_rate_changes = np.zeros((n_out, n_batch * n_iter))
    target_rate_changes[np.array(target_cues_list), np.arange(n_batch * n_iter)] = 1

    params_gen_rate_target = [
        {
            "amplitude_times": np.arange(0.0, duration["task"], duration["sequence"]) + duration["total_offset"],
            "amplitude_values": target_rate_changes[nrn_out_idx],
        }
        for nrn_out_idx in range(n_out)
    ]

    nest.SetStatus(gen_spk_in, params_gen_spk_in)
    nest.SetStatus(gen_rate_target, params_gen_rate_target)

    # Simulate

    nest.Simulate(duration["sim"])

    # Read out recorders

    events_mm_out = mm_out.get("events")

    # Evaluate training error

    readout_signal = events_mm_out["readout_signal"]
    target_signal = events_mm_out["target_signal"]
    senders = events_mm_out["senders"]

    readout_signal = np.array([readout_signal[senders == i] for i in set(senders)])
    target_signal = np.array([target_signal[senders == i] for i in set(senders)])

    readout_signal = readout_signal.reshape((n_out, n_iter, n_batch, steps["sequence"]))
    readout_signal = readout_signal[:, :, :, -steps["learning_window"] :]

    target_signal = target_signal.reshape((n_out, n_iter, n_batch, steps["sequence"]))
    target_signal = target_signal[:, :, :, -steps["learning_window"] :]

    loss = -np.mean(np.sum(target_signal * np.log(readout_signal), axis=0), axis=(1, 2))

    # Verify results

    loss_NEST_reference = np.array(
        [
            0.741152550006,
            0.740388187700,
            0.665785233177,
            0.663644193322,
            0.729428962844,
        ]
    )

    loss_TF_reference = np.array(
        [
            0.741152524948,
            0.740388214588,
            0.665785133838,
            0.663644134998,
            0.729429066181,
        ]
    )

    assert np.allclose(loss, loss_NEST_reference, rtol=1e-8)
    assert np.allclose(loss, loss_TF_reference, rtol=1e-6)
