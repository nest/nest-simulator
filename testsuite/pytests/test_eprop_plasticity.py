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

import nest
import numpy as np
import pytest

nest.set_verbosity("M_WARNING")

supported_source_models = ["eprop_iaf", "eprop_iaf_adapt", "eprop_iaf_psc_delta", "eprop_iaf_psc_delta_adapt"]
supported_target_models = supported_source_models + ["eprop_readout"]


@pytest.fixture(autouse=True)
def fix_resolution():
    nest.ResetKernel()


@pytest.mark.parametrize("source_model", supported_source_models)
@pytest.mark.parametrize("target_model", supported_target_models)
def test_connect_with_eprop_synapse(source_model, target_model):
    """
    Ensure that the restriction to supported neuron models works.
    """

    # Connect supported models with e-prop synapse
    src = nest.Create(source_model)
    tgt = nest.Create(target_model)
    nest.Connect(src, tgt, "all_to_all", {"synapse_model": "eprop_synapse", "delay": nest.resolution})


@pytest.mark.parametrize("target_model", set(nest.node_models) - set(supported_target_models))
def test_unsupported_model_raises(target_model):
    """
    Confirm that connecting a non-eprop neuron as target via an eprop_synapse raises an error.
    """

    src_nrn = nest.Create(supported_source_models[0])
    tgt_nrn = nest.Create(target_model)

    with pytest.raises(nest.kernel.NESTError):
        nest.Connect(src_nrn, tgt_nrn, "all_to_all", {"synapse_model": "eprop_synapse"})


@pytest.mark.parametrize(
    "neuron_model,optimizer,loss_nest_reference",
    [
        (
            "eprop_iaf",
            "adam",
            [
                0.14000491891386,
                0.28735818029752,
                0.22263713699860,
                0.23015072431384,
                0.17565248170694,
            ],
        ),
        (
            "eprop_iaf_adapt",
            "gradient_descent",
            [
                0.02242584487453,
                0.01916753791522,
                0.00571493935652,
                0.00037875376195,
                0.00037823157578,
            ],
        ),
        (
            "eprop_iaf_psc_delta",
            "gradient_descent",
            [
                0.27039631749159,
                0.51390204375229,
                0.53473859833866,
                0.53608474802202,
                0.49323580244201,
            ],
        ),
        (
            "eprop_iaf_psc_delta_adapt",
            "gradient_descent",
            [
                0.16416646182675,
                0.27952159282830,
                0.29937515524341,
                0.26160957500433,
                0.26384514215024,
            ],
        ),
    ],
)
def test_eprop_regression(neuron_model, optimizer, loss_nest_reference):
    """
    Test correct computation of losses for a regression task (for details on the task, see
    nest-simulator/pynest/examples/eprop_plasticity/eprop_supervised_regression_sine-waves.py) by
    comparing the simulated losses with NEST reference losses to catch scenarios in which the e-prop
    model does not work as intended (e.g., potential future changes to the NEST code base or a
    faulty installation). These reference losses were obtained from a simulation with the verified
    NEST e-prop implementation run with Linux 6.5.0-28-generic, Python v3.12.3, Numpy v1.26.4, and
    NEST@9b65de4bf.
    """

    # Initialize random generator
    rng_seed = 1
    np.random.seed(rng_seed)

    # Define timing of task

    group_size = 1
    n_iter = 5

    steps = {
        "sequence": 100,
    }

    steps["learning_window"] = steps["sequence"]
    steps["task"] = n_iter * group_size * steps["sequence"]

    steps.update(
        {
            "offset_gen": 1,
            "delay_in_rec": 1,
            "extension_sim": 3,
        }
    )

    steps["delays"] = steps["delay_in_rec"]

    steps["total_offset"] = steps["offset_gen"] + steps["delays"]

    steps["sim"] = steps["task"] + steps["total_offset"] + steps["extension_sim"]

    duration = {"step": 1.0}

    duration.update({key: value * duration["step"] for key, value in steps.items()})

    # Set up simulation

    params_setup = {
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

    params_nrn_out = {
        "C_m": 1.0,
        "E_L": 0.0,
        "eprop_isi_trace_cutoff": 100,
        "I_e": 0.0,
        "tau_m": 30.0,
        "V_m": 0.0,
    }

    params_nrn_rec = {
        "beta": 33.3,
        "C_m": 1.0,
        "c_reg": 300.0 / duration["sequence"],
        "E_L": 0.0,
        "eprop_isi_trace_cutoff": 100,
        "f_target": 10.0,
        "gamma": 10.0,
        "I_e": 0.0,
        "kappa": 0.97,
        "kappa_reg": 0.97,
        "surrogate_gradient_function": "piecewise_linear",
        "t_ref": 0.0,
        "tau_m": 30.0,
        "V_m": 0.0,
        "V_th": 0.03,
    }

    scale_factor = 1.0 - params_nrn_rec["kappa"]

    if neuron_model in ["eprop_iaf_psc_delta", "eprop_iaf_psc_delta_adapt"]:
        params_nrn_rec["V_reset"] = -0.5
        params_nrn_rec["c_reg"] = 2.0 / duration["sequence"] / scale_factor**2
        params_nrn_rec["V_th"] = 0.5
    elif neuron_model == "eprop_iaf_adapt":
        params_nrn_rec["adapt_beta"] = 0.0174
        params_nrn_rec["adapt_tau"] = 2000.0
        params_nrn_rec["adaptation"] = 0.0

    gen_spk_in = nest.Create("spike_generator", n_in)
    nrns_in = nest.Create("parrot_neuron", n_in)
    nrns_rec = nest.Create(neuron_model, n_rec, params_nrn_rec)
    nrns_out = nest.Create("eprop_readout", n_out, params_nrn_out)
    gen_rate_target = nest.Create("step_rate_generator", n_out)
    gen_learning_window = nest.Create("step_rate_generator")

    # Create recorders

    n_record = 1
    n_record_w = 1

    params_mm_rec = {
        "interval": duration["step"],
        "record_from": ["V_m", "surrogate_gradient", "learning_signal"],
        "start": duration["offset_gen"] + duration["delay_in_rec"],
        "stop": duration["offset_gen"] + duration["delay_in_rec"] + duration["task"],
    }

    params_mm_out = {
        "interval": duration["step"],
        "record_from": ["V_m", "readout_signal", "target_signal", "error_signal"],
        "start": duration["total_offset"],
        "stop": duration["total_offset"] + duration["task"],
    }

    params_wr = {
        "senders": nrns_in[:n_record_w] + nrns_rec[:n_record_w],
        "targets": nrns_rec[:n_record_w] + nrns_out,
        "start": duration["total_offset"],
        "stop": duration["total_offset"] + duration["task"],
    }

    params_sr = {
        "start": duration["offset_gen"],
        "stop": duration["total_offset"] + duration["task"],
    }

    mm_rec = nest.Create("multimeter", params_mm_rec)
    mm_out = nest.Create("multimeter", params_mm_out)
    sr = nest.Create("spike_recorder", params_sr)
    wr = nest.Create("weight_recorder", params_wr)

    nrns_rec_record = nrns_rec[:n_record]

    # Create connections

    params_conn_all_to_all = {"rule": "all_to_all", "allow_autapses": False}
    params_conn_one_to_one = {"rule": "one_to_one"}

    dtype_weights = np.float32
    weights_in_rec = np.array(np.random.randn(n_in, n_rec).T / np.sqrt(n_in), dtype=dtype_weights)
    weights_rec_rec = np.array(np.random.randn(n_rec, n_rec).T / np.sqrt(n_rec), dtype=dtype_weights)
    np.fill_diagonal(weights_rec_rec, 0.0)
    weights_rec_out = np.array(np.random.randn(n_rec, n_out).T / np.sqrt(n_rec), dtype=dtype_weights) * scale_factor
    weights_out_rec = np.array(np.random.randn(n_rec, n_out) / np.sqrt(n_rec), dtype=dtype_weights)

    if neuron_model in ["eprop_iaf", "eprop_iaf_adapt"]:
        weights_in_rec *= scale_factor
        weights_rec_rec *= scale_factor
        weights_out_rec *= scale_factor
    elif neuron_model in ["eprop_iaf_psc_delta", "eprop_iaf_psc_delta_adapt"]:
        weights_out_rec /= scale_factor

    params_common_syn_eprop = {
        "optimizer": {
            "type": optimizer,
            "batch_size": 1,
            "eta": 1e-4 * scale_factor**2,
            "optimize_each_step": True,
            "Wmin": -100.0,
            "Wmax": 100.0,
        },
        "weight_recorder": wr,
    }

    if optimizer == "adam":
        params_common_syn_eprop["optimizer"]["beta_1"] = 0.9
        params_common_syn_eprop["optimizer"]["beta_2"] = 0.999
        params_common_syn_eprop["optimizer"]["epsilon"] = 1e-7

    params_syn_base = {
        "synapse_model": "eprop_synapse",
        "delay": duration["step"],
    }

    params_syn_in = params_syn_base.copy()
    params_syn_in["weight"] = weights_in_rec

    params_syn_rec = params_syn_base.copy()
    params_syn_rec["weight"] = weights_rec_rec

    params_syn_out = params_syn_base.copy()
    params_syn_out["weight"] = weights_rec_out

    params_syn_feedback = {
        "synapse_model": "eprop_learning_signal_connection",
        "delay": duration["step"],
        "weight": weights_out_rec,
    }

    params_syn_learning_window = {
        "synapse_model": "rate_connection_delayed",
        "delay": duration["step"],
        "receptor_type": 1,
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

    nest.SetDefaults("eprop_synapse", params_common_syn_eprop)

    nest.Connect(gen_spk_in, nrns_in, params_conn_one_to_one, params_syn_static)
    nest.Connect(nrns_in, nrns_rec, params_conn_all_to_all, params_syn_in)
    nest.Connect(nrns_rec, nrns_rec, params_conn_all_to_all, params_syn_rec)
    nest.Connect(nrns_rec, nrns_out, params_conn_all_to_all, params_syn_out)
    nest.Connect(nrns_out, nrns_rec, params_conn_all_to_all, params_syn_feedback)
    nest.Connect(gen_rate_target, nrns_out, params_conn_one_to_one, params_syn_rate_target)
    nest.Connect(gen_learning_window, nrns_out, params_conn_all_to_all, params_syn_learning_window)

    nest.Connect(nrns_in + nrns_rec, sr, params_conn_all_to_all, params_syn_static)

    nest.Connect(mm_rec, nrns_rec_record, params_conn_all_to_all, params_syn_static)
    nest.Connect(mm_out, nrns_out, params_conn_all_to_all, params_syn_static)

    # Create input

    input_spike_prob = 0.05
    dtype_in_spks = np.float32

    input_spike_bools = (np.random.rand(steps["sequence"], n_in) < input_spike_prob).swapaxes(0, 1)

    sequence_starts = np.arange(0.0, duration["task"], duration["sequence"]) + duration["offset_gen"]
    params_gen_spk_in = []
    for input_spike_bool in input_spike_bools:
        input_spike_times = np.arange(0.0, duration["sequence"], duration["step"])[input_spike_bool]
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
        "amplitude_values": np.tile(target_signal, n_iter * group_size),
    }

    nest.SetStatus(gen_rate_target, params_gen_rate_target)

    # Create learning window

    params_gen_learning_window = {
        "amplitude_times": [duration["total_offset"]],
        "amplitude_values": [1.0],
    }

    nest.SetStatus(gen_learning_window, params_gen_learning_window)

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

    readout_signal = readout_signal.reshape((n_out, n_iter, group_size, steps["sequence"]))
    target_signal = target_signal.reshape((n_out, n_iter, group_size, steps["sequence"]))

    loss = 0.5 * np.mean(np.sum((readout_signal - target_signal) ** 2, axis=3), axis=(0, 2))

    # Verify results

    assert np.allclose(loss, loss_nest_reference, rtol=1e-8)


def test_unsupported_surrogate_gradient():
    """
    Confirm that selecting an unsupported surrogate gradient raises an error.
    """

    params_nrn_rec = {
        "surrogate_gradient_function": "unsupported_surrogate_gradient",
    }

    with pytest.raises(nest.kernel.NESTError):
        nrn = nest.Create("eprop_iaf", 1, params_nrn_rec)
        nest.Simulate(1.0)


@pytest.mark.parametrize(
    "surrogate_gradient_type,surrogate_gradient_reference",
    [
        (
            "piecewise_linear",
            [
                0.21030100902759,
                0.18933659619924,
                0.17036721102500,
                0.15320300152221,
                0.13767218251307,
            ],
        ),
        (
            "exponential",
            [
                0.28011776848615,
                0.26861558105999,
                0.25861553046549,
                0.24988832085299,
                0.24224569105944,
            ],
        ),
        (
            "fast_sigmoid_derivative",
            [
                0.20044113201621,
                0.19020804299944,
                0.18160934703950,
                0.17432167613232,
                0.16809903522604,
            ],
        ),
        (
            "arctan",
            [
                0.03689910976700,
                0.03308744332685,
                0.03008806500220,
                0.02768876792046,
                0.02574163163544,
            ],
        ),
    ],
)
def test_eprop_surrogate_gradients(surrogate_gradient_type, surrogate_gradient_reference):
    """
    Test correct computation of surrogate gradients by comparing the simulated surrogate gradients
    with NEST reference surrogate gradients. These reference surrogate gradients were obtained from
    a simulation with the verified NEST e-prop implementation run with Linux 5.8.7-1-default, Python
    v3.12.5, Numpy v2.0.1, and NEST@d04fe550d.
    """

    rng_seed = 1
    np.random.seed(rng_seed)

    duration = {
        "step": 1.0,
        "sim": 20.0,
    }

    params_setup = {
        "print_time": False,
        "resolution": duration["step"],
        "total_num_virtual_procs": 1,
    }

    nest.ResetKernel()
    nest.set(**params_setup)

    params_nrn_rec = {
        "beta": 1.7,
        "C_m": 1.0,
        "c_reg": 0.0,
        "E_L": 0.0,
        "gamma": 0.5,
        "I_e": 0.0,
        "surrogate_gradient_function": surrogate_gradient_type,
        "t_ref": 3.0,
        "V_m": 0.0,
        "V_th": 0.6,
    }

    gen_spk_in = nest.Create("spike_generator", 1)
    nrns_in = nest.Create("parrot_neuron", 1)
    nrns_rec = nest.Create("eprop_iaf", 1, params_nrn_rec)

    params_mm_rec = {
        "interval": duration["step"],
        "record_from": ["surrogate_gradient", "V_m"],
    }

    mm_rec = nest.Create("multimeter", params_mm_rec)

    params_conn_one_to_one = {"rule": "one_to_one"}
    params_syn = {
        "synapse_model": "eprop_synapse",
        "delay": duration["step"],
        "weight": 0.3,
    }
    params_syn_static = {
        "synapse_model": "static_synapse",
        "delay": duration["step"],
    }

    nest.SetStatus(gen_spk_in, {"spike_times": [1.0, 2.0, 3.0, 5.0, 9.0, 11.0]})

    nest.Connect(gen_spk_in, nrns_in, params_conn_one_to_one, params_syn_static)
    nest.Connect(nrns_in, nrns_rec, params_conn_one_to_one, params_syn)
    nest.Connect(mm_rec, nrns_rec, params_conn_one_to_one, params_syn_static)

    nest.Simulate(duration["sim"])
    events_mm_rec = mm_rec.get("events")
    surrogate_gradient = events_mm_rec["surrogate_gradient"][-5:]

    assert np.allclose(surrogate_gradient, surrogate_gradient_reference, rtol=1e-8)
