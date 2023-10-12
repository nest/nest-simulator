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

import pytest
import nest
import numpy as np


def test_ConnectNeuronsWithEpropSynapse():
    """Ensures that the restriction to supported neuron models works."""

    nest.set_verbosity("M_WARNING")

    supported_source_models = [
        "eprop_iaf_psc_delta",
        "eprop_iaf_psc_delta_adapt",
    ]
    supported_target_models = supported_source_models + ["eprop_readout"]

    # Connect supported models with e-prop synapse

    for nms in supported_source_models:
        for nmt in supported_target_models:
            nest.ResetKernel()

            ns = nest.Create(nms, 1)
            nt = nest.Create(nmt, 1)

            nest.Connect(ns, nt, {"rule": "all_to_all"}, {"synapse_model": "eprop_synapse"})

    # Ensure that connecting not supported models fails

    for nm in [n for n in nest.node_models if n not in supported_target_models]:
        nest.ResetKernel()

        n = nest.Create(nm, 2)

        with pytest.raises(nest.kernel.NESTError):
            nest.Connect(n, n, {"rule": "all_to_all"}, {"synapse_model": "eprop_synapse"})


def test_EpropRegression():
    """
    Test correct computation of weights for a regresion task  by comparing the simulated losses with
    losses obtained in a simulation with the original, verified NEST implementation and with the original
    TensorFlow implementation.
    """

    # Initialize random generator

    rng_seed = 1
    np.random.seed(rng_seed)

    # Define timing of task

    n_batch = 1
    n_iter = 5

    steps = {
        "recall": 1,
        "recall_onset": 0,
        "sequence": 100,
        "shift": 4,
    }

    steps["task"] = n_iter * n_batch * steps["sequence"]
    steps["sim"] = steps["task"] + steps["shift"]

    duration = {"step": 1.0}

    duration.update({key: value * duration["step"] for key, value in steps.items()})

    # Set up simulation

    params_setup = {
        "eprop_reset_neurons_on_update": True,
        "eprop_update_interval": duration["sequence"],
        "print_time": False,
        "resolution": duration["step"],
        "total_num_virtual_procs": 1,
    }

    nest.ResetKernel()
    nest.set(**params_setup)

    # Create neurons

    n_in = 10
    n_rec = 10
    n_out = 1

    params_nrn_rec = {
        "C_m": 1.0,
        "c_reg": 300.0,
        "gamma": 0.3,
        "E_L": 0.0,
        "f_target": 10.0,
        "I_e": 0.0,
        "propagator_idx": 0,
        "surrogate_gradient": "pseudo_derivative",
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
        "start_learning": duration["recall_onset"],
        "tau_m": 30.0,
        "V_m": 0.0,
    }

    gen_spk_in = nest.Create("spike_generator", n_in)
    nrns_in = nest.Create("parrot_neuron", n_in)
    nrns_rec = nest.Create("eprop_iaf_psc_delta", n_rec, params_nrn_rec)
    nrns_out = nest.Create("eprop_readout", n_out, params_nrn_out)
    gen_rate_target = nest.Create("step_rate_generator", n_out)

    # Create recorders

    n_record = 1
    n_record_w = 1

    params_mm_rec = {"record_from": ["V_m", "surrogate_gradient", "learning_signal"], "interval": duration["sequence"]}
    params_mm_out = {
        "record_from": ["V_m", "readout_signal", "target_signal", "error_signal"],
        "interval": duration["sequence"],
    }

    params_wr = {
        "senders": nrns_in[:n_record_w] + nrns_rec[:n_record_w],
        "targets": nrns_rec[:n_record_w] + nrns_out,
    }

    nrns_rec_record = nrns_rec[:n_record]

    mm_rec = nest.Create("multimeter", params_mm_rec)
    mm_out = nest.Create("multimeter", params_mm_out)
    sr = nest.Create("spike_recorder")
    wr = nest.Create("weight_recorder", params_wr)

    params_mm_out = {"record_from": ["V_m", "readout_signal", "target_signal", "error_signal"]}
    mm_out = nest.Create("multimeter", params_mm_out)

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
        "batch_size": n_batch,
        "recall_duration": duration["recall"],
        "optimizer": "gradient_descent",
        "weight_recorder": wr,
    }

    params_syn_in = {
        "synapse_model": "eprop_synapse_iaf_psc_delta",
        "delay": duration["step"],
        "eta": 1e-4,
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_in_rec,
        "Wmax": 100.0,
        "Wmin": -100.0,
    }

    params_syn_rec = {
        "synapse_model": "eprop_synapse_iaf_psc_delta",
        "delay": duration["step"],
        "eta": 1e-4,
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_rec_rec,
        "Wmax": 100.0,
        "Wmin": -100.0,
    }

    params_syn_out = {
        "synapse_model": "eprop_synapse_readout",
        "delay": duration["step"],
        "eta": 1e-4,
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_rec_out,
        "Wmax": 100.0,
        "Wmin": -100.0,
    }

    params_syn_feedback = {
        "synapse_model": "eprop_learning_signal_connection",
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

    nest.SetDefaults("eprop_synapse_iaf_psc_delta", params_common_syn_eprop)
    nest.SetDefaults("eprop_synapse_readout", params_common_syn_eprop)

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

    input_spike_rate = 0.05
    spike_probability = duration["step"] * input_spike_rate
    dtype_in_spks = np.float32

    input_spike_bools = np.random.rand(n_batch, steps["sequence"], n_in) < spike_probability
    input_spike_bools = np.hstack(input_spike_bools.swapaxes(1, 2))
    input_spike_bools[:, 0] = 0

    sequence_starts = np.arange(0.0, duration["task"], duration["sequence"])
    params_gen_spk_in = []
    for input_spike_bool in input_spike_bools:
        input_spike_times = np.arange(duration["sequence"] * n_batch)[input_spike_bool]
        input_spike_times_all = [input_spike_times + start for start in sequence_starts]
        params_gen_spk_in.append({"spike_times": np.hstack(input_spike_times_all).astype(dtype_in_spks).tolist()})

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

    target_signal = generate_superimposed_sines(steps["sequence"], [1000.0, 500.0, 333.0, 200.0])

    params_gen_rate_target = {
        "amplitude_times": (np.arange(0, duration["task"], duration["step"]) + duration["step"]).tolist(),
        "amplitude_values": np.tile(target_signal, n_iter * n_batch).tolist(),
    }

    nest.SetStatus(gen_rate_target, params_gen_rate_target)

    # Simulate

    nest.Simulate(duration["sim"])

    # Read out recorders

    events_mm_out = mm_out.get("events")

    # Evaluate training error

    readout_signal = events_mm_out["readout_signal"]
    target_signal = events_mm_out["target_signal"]

    readout_signal = readout_signal[steps["shift"] - 1 :]
    target_signal = target_signal[steps["shift"] - 2 : -1]

    error = (readout_signal - target_signal) ** 2
    loss = 0.5 * np.add.reduceat(error, np.arange(0, steps["task"], steps["sequence"]))

    # Verify results

    loss_NEST_verification = np.array(
        [
            0.301624405492225800,
            0.300969013603013313,
            0.295005356673212760,
            0.299270730552683328,
            0.320109501410700492,
        ]
    )

    loss_TF_verification = np.array(
        [
            0.301624119281768799,
            0.300968766212463379,
            0.295005142688751221,
            0.299270510673522949,
            0.320109188556671143,
        ]
    )

    assert np.all(loss == loss_NEST_verification)
    assert np.all(loss - loss_TF_verification < 1e-6)


def test_EpropClassification():
    """
    Test correct computation of weights for a classification task by comparing the simulated losses with
    losses obtained in a simulation with the original, verified NEST implementation and with the original
    TensorFlow implementation.
    """

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
        "cue": 10,
        "spacing": 5,
        "bg_noise": 105,
        "recall": 15,
        "shift": 4,
    }

    steps["cues"] = n_cues * (steps["cue"] + steps["spacing"])
    steps["recall_onset"] = steps["cues"] + steps["bg_noise"]
    steps["sequence"] = steps["recall_onset"] + steps["recall"]
    steps["task"] = n_iter * n_batch * steps["sequence"]
    steps["sim"] = steps["task"] + steps["shift"]

    duration = {"step": 1.0}

    duration.update({key: value * duration["step"] for key, value in steps.items()})

    # Set up simulation

    params_setup = {
        "eprop_reset_neurons_on_update": True,
        "eprop_update_interval": duration["sequence"],
        "print_time": False,
        "resolution": duration["step"],
        "total_num_virtual_procs": 1,
    }

    nest.ResetKernel()
    nest.set(**params_setup)

    # Create neurons

    n_in = 4
    n_ad = 5
    n_reg = 5
    n_rec = n_ad + n_reg
    n_out = 2

    params_nrn_reg = {
        "C_m": 1.0,
        "c_reg": 2.0,
        "E_L": 0.0,
        "f_target": 10.0,
        "gamma": 0.3,
        "I_e": 0.0,
        "propagator_idx": 1,
        "surrogate_gradient": "pseudo_derivative",
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
        "propagator_idx": 1,
        "surrogate_gradient": "pseudo_derivative",
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
        "loss": "cross_entropy_loss",
        "start_learning": duration["recall_onset"],
        "tau_m": 20.0,
        "V_m": 0.0,
    }

    gen_spk_in = nest.Create("spike_generator", n_in)
    nrns_in = nest.Create("parrot_neuron", n_in)
    nrns_reg = nest.Create("eprop_iaf_psc_delta", n_reg, params_nrn_reg)
    nrns_ad = nest.Create("eprop_iaf_psc_delta_adapt", n_ad, params_nrn_ad)
    nrns_out = nest.Create("eprop_readout", n_out, params_nrn_out)
    gen_rate_target = nest.Create("step_rate_generator", n_out)

    nrns_rec = nrns_reg + nrns_ad

    # Create recorders

    n_record = 1
    n_record_w = 1

    params_mm_rec = {"record_from": ["V_m", "surrogate_gradient", "learning_signal"], "interval": duration["sequence"]}
    params_mm_out = {
        "record_from": ["V_m", "readout_signal", "target_signal", "error_signal"],
        "interval": duration["sequence"],
    }

    params_wr = {
        "senders": nrns_in[:n_record_w] + nrns_rec[:n_record_w],
        "targets": nrns_rec[:n_record_w] + nrns_out,
    }

    nrns_rec_record = nrns_rec[:n_record]

    mm_rec = nest.Create("multimeter", params_mm_rec)
    mm_out = nest.Create("multimeter", params_mm_out)
    sr = nest.Create("spike_recorder")
    wr = nest.Create("weight_recorder", params_wr)

    params_mm_out = {"record_from": ["V_m", "readout_signal", "target_signal", "error_signal"]}
    mm_out = nest.Create("multimeter", params_mm_out)

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
        "adam_beta1": 0.9,
        "adam_beta2": 0.999,
        "adam_epsilon": 1e-8,
        "optimizer": "adam",
        "batch_size": n_batch,
        "recall_duration": duration["recall"],
        "weight_recorder": wr,
    }

    params_syn_in_reg = {
        "synapse_model": "eprop_synapse_iaf_psc_delta",
        "adam_m": 0.0,
        "adam_v": 0.0,
        "delay": duration["step"],
        "eta": 5e-3,
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_in_rec[:n_reg, :],
        "Wmax": 100.0,
        "Wmin": -100.0,
    }

    params_syn_in_adapt = {
        "synapse_model": "eprop_synapse_iaf_psc_delta_adapt",
        "adam_m": 0.0,
        "adam_v": 0.0,
        "delay": duration["step"],
        "eta": 5e-3,
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_in_rec[n_reg:, :],
        "Wmax": 100.0,
        "Wmin": -100.0,
    }

    params_syn_rec_reg = {
        "synapse_model": "eprop_synapse_iaf_psc_delta",
        "adam_m": 0.0,
        "adam_v": 0.0,
        "delay": duration["step"],
        "eta": 5e-3,
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_rec_rec[:n_reg, :],
        "Wmax": 100.0,
        "Wmin": -100.0,
    }

    params_syn_rec_adapt = {
        "synapse_model": "eprop_synapse_iaf_psc_delta_adapt",
        "adam_m": 0.0,
        "adam_v": 0.0,
        "delay": duration["step"],
        "eta": 5e-3,
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_rec_rec[n_reg:, :],
        "Wmax": 100.0,
        "Wmin": -100.0,
    }
    params_syn_out = {
        "synapse_model": "eprop_synapse_readout",
        "adam_m": 0.0,
        "adam_v": 0.0,
        "delay": duration["step"],
        "eta": 5e-3,
        "tau_m_readout": params_nrn_out["tau_m"],
        "weight": weights_rec_out,
        "Wmax": 100.0,
        "Wmin": -100.0,
    }

    params_syn_feedback = {
        "synapse_model": "eprop_learning_signal_connection",
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

    nest.SetDefaults("eprop_synapse_iaf_psc_delta", params_common_syn_eprop)
    nest.SetDefaults("eprop_synapse_iaf_psc_delta_adapt", params_common_syn_eprop)
    nest.SetDefaults("eprop_synapse_readout", params_common_syn_eprop)

    nest.Connect(gen_spk_in, nrns_in, params_conn_one_to_one, params_syn_static)
    nest.Connect(nrns_in, nrns_reg, params_conn_all_to_all, params_syn_in_reg)
    nest.Connect(nrns_in, nrns_ad, params_conn_all_to_all, params_syn_in_adapt)
    nest.Connect(nrns_rec, nrns_reg, params_conn_all_to_all, params_syn_rec_reg)
    nest.Connect(nrns_rec, nrns_ad, params_conn_all_to_all, params_syn_rec_adapt)
    nest.Connect(nrns_rec, nrns_out, params_conn_all_to_all, params_syn_out)
    nest.Connect(nrns_out, nrns_rec, params_conn_all_to_all, params_syn_feedback)
    nest.Connect(gen_rate_target, nrns_out, params_conn_one_to_one, params_syn_rate_target)
    nest.Connect(nrns_out, nrns_out, params_conn_all_to_all, params_syn_out_out)

    nest.Connect(nrns_in + nrns_rec, sr, params_conn_all_to_all, params_syn_static)

    nest.Connect(mm_rec, nrns_rec_record, params_conn_all_to_all, params_syn_static)
    nest.Connect(mm_out, nrns_out, params_conn_all_to_all, params_syn_static)

    # Create input and output

    def generate_evidence_accumulation_input_output(
        n_batch, n_in, prob_group, input_spike_rate, n_cues, n_input_symbols, steps
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

        input_spike_prob = np.zeros((n_batch, steps["sequence"], n_in))

        for b_idx in range(n_batch):
            for c_idx in range(n_cues):
                cue = batched_cues[b_idx, c_idx]

                step_start = c_idx * (steps["cue"] + steps["spacing"]) + steps["spacing"]
                step_stop = step_start + steps["cue"]

                pop_nrn_start = cue * n_pop_nrn
                pop_nrn_stop = pop_nrn_start + n_pop_nrn

                input_spike_prob[b_idx, step_start:step_stop, pop_nrn_start:pop_nrn_stop] = input_spike_rate

        input_spike_prob[:, -steps["recall"] :, 2 * n_pop_nrn : 3 * n_pop_nrn] = input_spike_rate
        input_spike_prob[:, :, 3 * n_pop_nrn :] = input_spike_rate / 4.0
        input_spike_bools = input_spike_prob > np.random.rand(input_spike_prob.size).reshape(input_spike_prob.shape)
        input_spike_bools[:, [0, -1], :] = 0

        target_cues = np.zeros(n_batch, dtype=int)
        target_cues[:] = np.sum(batched_cues, axis=1) > int(n_cues / 2)

        return input_spike_bools, target_cues

    input_spike_rate = 0.04
    dtype_in_spks = np.float32

    input_spike_bools_list = []
    target_cues_list = []

    for iteration in range(n_iter):
        input_spike_bools, target_cues = generate_evidence_accumulation_input_output(
            n_batch, n_in, prob_group, input_spike_rate, n_cues, n_input_symbols, steps
        )
        input_spike_bools_list.append(input_spike_bools)
        target_cues_list.extend(target_cues.tolist())

    input_spike_bools_arr = np.array(input_spike_bools_list).reshape(steps["task"], n_in)
    timeline_task = np.arange(0, duration["task"], duration["step"])

    params_gen_spk_in = [
        {"spike_times": timeline_task[input_spike_bools_arr[:, nrn_in_idx]].astype(dtype_in_spks).tolist()}
        for nrn_in_idx in range(n_in)
    ]

    target_rate_changes = np.zeros((n_out, n_batch * n_iter))
    target_rate_changes[np.array(target_cues_list), np.arange(n_batch * n_iter)] = 1

    params_gen_rate_target = [
        {
            "amplitude_times": (np.arange(0.0, duration["task"], duration["sequence"]) + duration["step"]).tolist(),
            "amplitude_values": target_rate_changes[nrn_out_idx].tolist(),
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

    readout_signal = np.array([readout_signal[senders == i] for i in np.unique(senders)])
    target_signal = np.array([target_signal[senders == i] for i in np.unique(senders)])

    readout_signal = readout_signal[:, steps["shift"] - 1 :]
    target_signal = target_signal[:, steps["shift"] - 2 : -1]

    readout_signal = readout_signal.reshape(n_out, n_iter, n_batch, steps["sequence"])[:, :, :, -steps["recall"] :]
    target_signal = target_signal.reshape(n_out, n_iter, n_batch, steps["sequence"])[:, :, :, -steps["recall"] :]

    loss = -np.mean(np.sum(target_signal * np.log(readout_signal), axis=0), axis=(1, 2))

    # Verify results

    loss_NEST_verification = np.array(
        [
            0.693414902818640488,
            0.693127416632571935,
            0.693126013248211215,
            0.693134668073821847,
            0.691480338113467763,
        ]
    )

    loss_TF_verification = np.array(
        [
            0.693414807319641113,
            0.693127453327178955,
            0.693125963211059570,
            0.693134665489196777,
            0.691480994224548340,
        ]
    )

    assert np.all(loss == loss_NEST_verification)
    assert np.all(loss - loss_TF_verification < 1e-7)
