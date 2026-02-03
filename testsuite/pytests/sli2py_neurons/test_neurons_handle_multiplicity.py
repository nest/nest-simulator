# -*- coding: utf-8 -*-
#
# test_neurons_handle_multiplicity.py
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
This test inputs two spikes at the same time into each neuron.
The spikes arrive once as one event with multiplicity two, once
as two events with multiplicity one. The membrane potential after
the spikes have arrived must be identical in both cases.
"""

import nest
import numpy as np
import numpy.testing as nptest
import pytest

# The following models will not be tested:
skip_list = [
    "ginzburg_neuron",  # binary neuron
    "ignore_and_fire",  # input independent neuron
    "mcculloch_pitts_neuron",  # binary neuron
    "erfc_neuron",  # binary neuron
    "lin_rate_ipn",  # rate neuron
    "lin_rate_opn",  # rate neuron
    "tanh_rate_ipn",  # rate neuron
    "tanh_rate_opn",  # rate neuron
    "threshold_lin_rate_ipn",  # rate neuron
    "threshold_lin_rate_opn",  # rate neuron
    "siegert_neuron",  # rate neuron
    "gauss_rate_ipn",  # rate neuron
    "sigmoid_rate_gg_1998_ipn",  # rate neuron
    "sigmoid_rate_ipn",  # rate neuron
    "rate_transformer_lin",  # rate transformer
    "rate_transformer_tanh",  # rate transformer
    "rate_transformer_threshold_lin",  # rate transformer
    "rate_transformer_gauss",  # rate transformer
    "rate_transformer_sigmoid",  # rate transformer
    "rate_transformer_sigmoid_gg_1998",  # rate transformer
    "parrot_neuron",
    "parrot_neuron_ps",
    "spike_train_injector",  # spike emitting neuron, does not support spike input
    "cm_default",  # cannot readout V_m directly
    "iaf_cond_alpha_mc",  # cannot readout V_m directly
    "pp_cond_exp_mc_urbanczik",  # cannot readout V_m directly
    "music_event_in_proxy",  # music device
    "music_event_out_proxy",  # music device
    "music_cont_in_proxy",  # music device
    "music_cont_out_proxy",  # music device
    "music_message_in_proxy",  # music device
    "music_message_out_proxy",  # music device
    "music_rate_in_proxy",  # music device
    "music_rate_out_proxy",  # music device
    "astrocyte_lr_1994",  # does not have V_m
]

extra_params = {
    "iaf_psc_alpha_multisynapse": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "iaf_psc_exp_multisynapse": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "gif_psc_exp_multisynapse": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "gif_cond_exp_multisynapse": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "glif_cond": {"params": {"tau_syn": [1.0], "E_rev": [-85.0]}, "receptor_type": 1},
    "glif_psc": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "glif_psc_double_alpha": {
        "params": {"tau_syn_fast": [1.0], "tau_syn_slow": [2.0], "amp_slow": [0.5]},
        "receptor_type": 1,
    },
    "aeif_cond_alpha_multisynapse": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "aeif_cond_beta_multisynapse": {
        "params": {"E_rev": [0.0], "tau_rise": [1.0], "tau_decay": [1.0]},
        "receptor_type": 1,
    },
    "ht_neuron": {"receptor_type": 1},
    "iaf_bw_2001": {"receptor_type": 1},  # cannot test NMDA port since pre-synaptic
    "iaf_bw_2001_exact": {"receptor_type": 1},  # also must be of same neuron type in that case
}


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_spike_multiplicity_parrot_neuron():
    multiplicities = [1, 3, 2]
    spikes = [1.0, 2.0, 3.0]
    sg = nest.Create(
        "spike_generator",
        {"spike_times": spikes, "spike_multiplicities": multiplicities},
    )
    pn = nest.Create("parrot_neuron")
    sr = nest.Create("spike_recorder")

    nest.Connect(sg, pn)
    nest.Connect(pn, sr)

    nest.Simulate(10.0)

    spike_times = sr.get("events")["times"]
    expected_spike_times = []
    for t, m in zip(spikes, multiplicities):
        expected_spike_times.extend([t + nest.min_delay] * m)
    nptest.assert_array_equal(spike_times, expected_spike_times)


@pytest.mark.parametrize(
    "model",
    [
        model
        for model in nest.node_models
        if model not in skip_list and nest.GetDefaults(model)["element_type"] == "neuron"
    ],
)
def test_spike_multiplicity(model):
    n1 = nest.Create(model)
    n2 = nest.Create(model)

    receptor_type = 0
    if model in extra_params.keys():
        if "params" in extra_params[model].keys():
            params = extra_params[model]["params"]
            n1.set(params)
            n2.set(params)
        if "receptor_type" in extra_params[model].keys():
            receptor_type = extra_params[model]["receptor_type"]

    # Two spike generators send one spike with default multiplicity of 1
    # A third spike generator sends one spike with multiplicity of 2
    sg1 = nest.Create("spike_generator", {"spike_times": [5.0]})
    sg2 = nest.Create("spike_generator", {"spike_times": [5.0]})
    sg3 = nest.Create("spike_generator", {"spike_times": [5.0], "spike_multiplicities": [2]})

    syn_spec = {
        "synapse_model": "static_synapse",
        "receptor_type": receptor_type,
    }

    # Neuron n1 receives two spikes with multiplicity 1
    nest.Connect(sg1, n1, "all_to_all", syn_spec)
    nest.Connect(sg2, n1, "all_to_all", syn_spec)

    # Neuron n2 receives one spike with multiplicity 2
    nest.Connect(sg3, n2, "all_to_all", syn_spec)

    # Get v_m before simulation
    v1_0 = n1.get("V_m")
    v2_0 = n2.get("V_m")

    assert v1_0 == pytest.approx(v2_0)

    # Simulate
    nest.Simulate(8.0)

    v1 = n1.get("V_m")
    v2 = n2.get("V_m")

    assert v1 == pytest.approx(v2)
    assert v1_0 != v1
