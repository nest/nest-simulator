# -*- coding: utf-8 -*-
#
# test_multimeter_stepping.py
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
Test multimeter recording in stepwise simulation.
"""

import nest
import pandas as pd
import pandas.testing as pdtest
import pytest

skip_models = [
    "eprop_readout_bsshslm_2020",  # extra timestep added to some recordables in update function
    "erfc_neuron",  # binary neuron
    "ginzburg_neuron",  # binary neuron
    "mcculloch_pitts_neuron",  # binary neuron
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
    "ac_generator",  # generator device, does not support spike input
    "dc_generator",  # generator device, does not support spike input
    "noise_generator",  # generator device, does not support spike input
    "step_current_generator",  # generator device, does not support spike input
    "step_rate_generator",  # generator device, does not support spike input
    "sinusoidal_poisson_generator",  # generator device, does not support spike input
    "sinusoidal_gamma_generator",  # generator device, does not support spike input
]

# The following models require connections to rport 1:
extra_params = {
    "iaf_psc_alpha_multisynapse": {"receptor_type": 1},
    "iaf_psc_exp_multisynapse": {"receptor_type": 1},
    "gif_psc_exp_multisynapse": {"receptor_type": 1},
    "gif_cond_exp_multisynapse": {"receptor_type": 1},
    "glif_psc": {"receptor_type": 1},
    "glif_psc_double_alpha": {"receptor_type": 1},
    "iaf_cond_alpha_mc": {"receptor_type": 1},
    "glif_cond": {"receptor_type": 1},
    "ht_neuron": {"receptor_type": 1},
    "aeif_cond_alpha_multisynapse": {"receptor_type": 1},
    "aeif_cond_beta_multisynapse": {"receptor_type": 1},
    "pp_cond_exp_mc_urbanczik": {"receptor_type": 1},
    "iaf_bw_2001": {"receptor_type": 1},
    "iaf_bw_2001_exact": {"receptor_type": 1},
}

# Obtain all models with non-empty recordables list
models = (
    model for model in nest.node_models if (nest.GetDefaults(model).get("recordables") and model not in skip_models)
)


def build_net(model):
    """
    Build network to be tested.

    A multimeter is set to record all recordables of the provided neuron model.
    The neuron receives Poisson input.
    """

    nest.ResetKernel()
    nrn = nest.Create(model)
    pg = nest.Create("poisson_generator", params={"rate": 1e4})
    mm = nest.Create("multimeter", {"interval": nest.resolution, "record_from": nrn.recordables})

    receptor_type = 0
    if model in extra_params.keys():
        receptor_type = extra_params[model]["receptor_type"]

    nest.Connect(mm, nrn)
    nest.Connect(pg, nrn, syn_spec={"receptor_type": receptor_type})

    return mm


@pytest.mark.parametrize("model", models)
def test_multimeter_stepping(model):
    """
    Test multimeter recording in stepwise simulation.

    The test first simulates the network for `50 x nest.min_delay`. Then, we
    reset and build the network again and perform 50 subsequent simulations
    with `nest.min_delay` simulation time. Both cases should produce identical
    results.
    """

    mm = build_net(model)
    nest.Simulate(50 * nest.min_delay)
    df = pd.DataFrame.from_dict(mm.events)

    mm_stepwise = build_net(model)
    for _ in range(50):
        nest.Simulate(nest.min_delay)

    df_stepwise = pd.DataFrame.from_dict(mm_stepwise.events)

    pdtest.assert_frame_equal(df, df_stepwise)
