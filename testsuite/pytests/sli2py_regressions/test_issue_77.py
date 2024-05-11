# -*- coding: utf-8 -*-
#
# test_issue_77.py
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
Regression test for Issue #77 (GitHub).
"""

import nest
import pytest

# The following models will not be tested:
skip_models = [
    "erfc_neuron",  # binary neuron
    "ginzburg_neuron",  # binary neuron
    "ignore_and_fire",  # input independent neuron
    "mcculloch_pitts_neuron",  # binary neuron
    "gif_pop_psc_exp",  # population model, not suitable for STDP
    "gauss_rate_ipn",  # rate neuron
    "lin_rate_ipn",  # rate neuron
    "lin_rate_opn",  # rate neuron
    "siegert_neuron",  # rate neuron
    "sigmoid_rate_gg_1998_ipn",  # rate neuron
    "sigmoid_rate_ipn",  # rate neuron
    "tanh_rate_ipn",  # rate neuron
    "tanh_rate_opn",  # rate neuron
    "threshold_lin_rate_ipn",  # rate neuron
    "threshold_lin_rate_opn",  # rate neuron
    "rate_transformer_gauss",  # rate transformer
    "rate_transformer_lin",  # rate transformer
    "rate_transformer_sigmoid",  # rate transformer
    "rate_transformer_sigmoid_gg_1998",  # rate transformer
    "rate_transformer_tanh",  # rate transformer
    "rate_transformer_threshold_lin",  # rate transformer
    "spike_train_injector",  # generator neuron, does not support spike input
    "music_cont_in_proxy",  # MUSIC device
    "music_cont_out_proxy",  # MUSIC device
    "music_event_in_proxy",  # MUSIC device
    "music_event_out_proxy",  # MUSIC device
    "music_message_in_proxy",  # MUSIC device
    "music_rate_in_proxy",  # MUSIC device
    "music_rate_out_proxy",  # MUSIC device
    "astrocyte_lr_1994",  # does not send spikes
    "eprop_readout_bsshslm_2020",  # does not send spikes
    "eprop_iaf_bsshslm_2020",  # does not support stdp synapses
    "eprop_iaf_adapt_bsshslm_2020",  # does not support stdp synapses
    "eprop_readout",  # does not send spikes
    "eprop_iaf",  # does not support stdp synapses
    "eprop_iaf_adapt",  # does not support stdp synapses
    "eprop_iaf_psc_delta",  # does not support stdp synapses
]

# The following models require connections to rport 1 or other specific parameters:
extra_params = {
    "aeif_psc_alpha": {"initial_weight": 80.0},
    "aeif_psc_delta": {"initial_weight": 80.0},
    "aeif_psc_exp": {"initial_weight": 80.0},
    "aeif_cond_alpha_multisynapse": {
        "params": {"E_rev": [-20.0], "tau_syn": [2.0]},
        "receptor_type": 1,
    },
    "aeif_cond_beta_multisynapse": {
        "params": {"E_rev": [-20.0], "tau_rise": [1.0], "tau_decay": [2.0]},
        "receptor_type": 1,
    },
    "iaf_cond_alpha_mc": {"receptor_type": 1},
    "iaf_psc_alpha_multisynapse": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "iaf_psc_exp_multisynapse": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "gif_cond_exp_multisynapse": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "gif_psc_exp_multisynapse": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "glif_cond": {"params": {"tau_syn": [0.2], "E_rev": [0.0]}, "receptor_type": 1},
    "glif_psc": {"params": {"tau_syn": [1.0]}, "receptor_type": 1},
    "glif_psc_double_alpha": {
        "params": {"tau_syn_fast": [1.0], "tau_syn_slow": [2.0], "amp_slow": [0.5]},
        "receptor_type": 1,
    },
    "ht_neuron": {"receptor_type": 1},
    "pp_cond_exp_mc_urbanczik": {"receptor_type": 1},
}

models = [
    model
    for model in nest.node_models
    if (nest.GetDefaults(model, "element_type") == "neuron") and model not in skip_models
]


@pytest.mark.parametrize("model", models)
def test_register_outgoing_spikes(model):
    """
    Ensure that all neuron models register outgoing spikes with archiving node.

    The test sends a very high-rate Poisson spike train into the neuron that
    should make any type of model neuron fire and checks both `t_spike` entry
    of the neuron (>0 if neuron has spiked) and checks that the connection
    weight differs from the initial value 1.0.
    """
    nest.ResetKernel()

    nrn = nest.Create(model)

    if model in extra_params:
        if "params" in extra_params[model]:
            nrn.set(extra_params[model].get("params"))

    # if the model is compartmental, we need to add at least a root compartment
    if "compartments" in nest.GetDefaults(model):
        nrn.compartments = {"parent_idx": -1}
        nrn.receptors = {"comp_idx": 0, "receptor_type": "AMPA"}

    pg = nest.Create("poisson_generator", params={"rate": 1e5})
    parrot = nest.Create("parrot_neuron_ps")
    srec = nest.Create("spike_recorder")

    # need to connect via parrot since generators cannot connect with
    # plastic synapses.
    nest.Connect(pg, parrot)

    receptor_type = 0
    initial_weight = 10.0
    if model in extra_params:
        if "receptor_type" in extra_params[model]:
            receptor_type = extra_params[model].get("receptor_type")
        if "initial_weight" in extra_params[model]:
            initial_weight = extra_params[model].get("initial_weight")

    syn_spec = {
        "synapse_model": "stdp_synapse",
        "weight": initial_weight,
        "receptor_type": receptor_type,
    }

    nest.Connect(parrot, nrn, "one_to_one", syn_spec)
    nest.Connect(nrn, srec)

    nest.Simulate(100.0)

    num_spikes = srec.n_events
    t_last_spike = nrn.t_spike
    weight_after_sim = nest.GetConnections(parrot).get("weight")

    assert num_spikes > 0
    assert t_last_spike > 0
    assert weight_after_sim != initial_weight
