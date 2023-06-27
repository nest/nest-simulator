# -*- coding: utf-8 -*-
#
# test_multimeter.py
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

import numpy.testing as nptest
import pytest

import nest

skip_models = [
    "gauss_rate_ipn",
    "lin_rate_ipn",
    "sigmoid_rate_ipn",
    "sigmoid_rate_gg_1998_ipn",
    "tanh_rate_ipn",
    "threshold_lin_rate_ipn",
    "lin_rate_opn",
    "tanh_rate_opn",
    "threshold_lin_rate_opn",
    "rate_transformer_gauss",
    "rate_transformer_lin",
    "rate_transformer_sigmoid",
    "rate_transformer_sigmoid_gg_1998",
    "rate_transformer_tanh",
    "rate_transformer_threshold_lin",
    "ac_generator",
    "dc_generator",
    "noise_generator",
    "step_current_generator",
    "step_rate_generator",
    "sinusoidal_poisson_generator",
    "erfc_neuron",
    "ginzburg_neuron",
    "mcculloch_pitts_neuron",
    "sinusoidal_gamma_generator",
    "siegert_neuron",
    "iaf_cond_alpha_mc",
    "gif_cond_exp_multisynapse",
    "aeif_cond_alpha_multisynapse",
    "aeif_cond_beta_multisynapse",
    "pp_cond_exp_mc_urbanczik",
]

extra_params = {
    "iaf_psc_alpha_multisynapse": {"receptor_type": 1},
    "iaf_psc_exp_multisynapse": {"receptor_type": 1},
    "gif_psc_exp_multisynapse": {"receptor_type": 1},
    "glif_psc": {"receptor_type": 1},
    "glif_cond": {"receptor_type": 1},
    "ht_neuron": {"receptor_type": 1},
}

# Obtain all models with non-empty recordables list
all_models_with_rec = [model for model in nest.node_models if nest.GetDefaults(model).get("recordables")]

# Obtain all models with non-empty recordables list and not in skip list
subset_models_with_rec = [
    model for model in nest.node_models if (nest.GetDefaults(model).get("recordables") and model not in skip_models)
]


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


def test_connect_multimeter_twice():
    """
    Ensure one multimeter can only be connected once to one neuron.

    First, we check that a multimeter can be connected to a neuron once. Then,
    we check that that we cannot connect the multimeter more than once.
    """

    nrn = nest.Create("iaf_psc_alpha")
    mm = nest.Create("multimeter")
    nest.Connect(mm, nrn)

    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(mm, nrn)


@pytest.mark.parametrize("model", all_models_with_rec)
def test_receptors_with_multiple_multimeters(model):
    """
    Test receptors when connecting to multiple multimeters.

    This test is to ensure that connections from two multimeters get
    receptors 1 and 2 for all models with recordables.
    """

    nrn = nest.Create(model)
    mm1 = nest.Create("multimeter", {"record_from": nrn.recordables})
    mm2 = nest.Create("multimeter", {"record_from": nrn.recordables})
    nest.Connect(mm1, nrn)
    nest.Connect(mm2, nrn)

    mm1_receptor = nest.GetConnections(mm1).get("receptor")
    mm2_receptor = nest.GetConnections(mm2).get("receptor")

    assert mm1_receptor == 1
    assert mm2_receptor == 2


@pytest.mark.parametrize("model", all_models_with_rec)
def test_recordables_are_recorded(model):
    """
    Test that recordables are recorded.

    For each model with recordables, set up minimal simulation recording
    from all recordables and test that data is provided. The test checks
    that the correct of amount of data is collected for each recordable.
    It also checks that the recording interval can be set.

    .. note::
       This test does not check if the data is meaningful.
    """

    nest.resolution = 2**-3  # Set to power of two to avoid rounding issues

    recording_interval = 2
    simtime = 10
    num_data_expected = simtime / recording_interval - 1

    nrn = nest.Create(model)
    recordables = nrn.recordables
    mm = nest.Create("multimeter", {"interval": recording_interval, "record_from": recordables})
    nest.Connect(mm, nrn)
    nest.Simulate(simtime)

    result = mm.events

    for r in recordables + ("times", "senders"):
        assert r in result
        assert len(result[r]) == num_data_expected


@pytest.mark.parametrize("model", subset_models_with_rec)
def test_identical_recording_from_multiple_multimeters(model):
    """
    Test identical recordings from multimeters with same configurations.

    In this test two identical multimeters are connected to the same neuron.
    They should record identical data.
    """

    nrn = nest.Create(model)
    recordables = nrn.recordables
    mm1 = nest.Create("multimeter", {"record_from": recordables})
    mm2 = nest.Create("multimeter", {"record_from": recordables})
    pge = nest.Create("poisson_generator", {"rate": 1e4})
    pgi = nest.Create("poisson_generator", {"rate": 1e4})

    receptor_type = 0
    if model in extra_params.keys():
        receptor_type = extra_params[model]["receptor_type"]

    esyn_spec = {"weight": 1.0, "delay": 1.0, "receptor_type": receptor_type}
    isyn_spec = {"weight": -1.0, "delay": 1.0, "receptor_type": receptor_type}

    nest.Connect(mm1, nrn)
    nest.Connect(mm2, nrn)
    nest.Connect(pge, nrn, syn_spec=esyn_spec)
    nest.Connect(pgi, nrn, syn_spec=isyn_spec)

    nest.Simulate(100.0)

    for recordable in recordables:
        nptest.assert_array_equal(mm1.events[recordable], mm2.events[recordable])
