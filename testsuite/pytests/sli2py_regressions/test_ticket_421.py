# -*- coding: utf-8 -*-
#
# test_ticket_421.py
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

import nest
import pytest

"""
Regression test for Ticket #421.

Test ported from SLI regression test.
Ensure neuron models with steady-state V_m keep their membrane potential after one time step.

Author: Hans Ekkehard Plesser, 2010-05-05
"""


EXCLUDE_MODELS = {
    "aeif_cond_exp",
    "aeif_cond_alpha",
    "a2eif_cond_exp",
    "a2eif_cond_exp_HW",
    "aeif_cond_alpha_multisynapse",
    "aeif_psc_delta_clopath",
    "aeif_cond_alpha_astro",
    "aeif_psc_exp",
    "aeif_psc_alpha",
    "aeif_psc_delta",
    "aeif_cond_beta_multisynapse",
    "hh_cond_exp_traub",
    "hh_cond_beta_gap_traub",
    "hh_psc_alpha",
    "hh_psc_alpha_clopath",
    "hh_psc_alpha_gap",
    "ht_neuron",
    "ht_neuron_fs",
    "iaf_cond_exp_sfa_rr",
    "izhikevich",
    "eprop_iaf_bsshslm_2020",
    "eprop_iaf_adapt_bsshslm_2020",
    "eprop_readout_bsshslm_2020",
    "eprop_iaf",
    "eprop_iaf_adapt",
    "eprop_iaf_psc_delta",
    "eprop_iaf_psc_delta_adapt",
    "eprop_readout",
}

RESOLUTION = 0.125  # 2**-3


def _models_with_vm():
    models = []
    for model in nest.node_models:
        if model in EXCLUDE_MODELS:
            continue
        defaults = nest.GetDefaults(model)
        if "V_m" not in defaults:
            continue
        vm0 = defaults["V_m"]
        models.append((model, vm0))
    return models


@pytest.mark.parametrize("model,vm0", _models_with_vm())
def test_ticket_421_membrane_potential_stays_constant(model, vm0):
    """
    Ensure membrane potential remains at the initial steady-state value after one update step.
    """

    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": RESOLUTION})

    neuron = nest.Create(model)

    nest.Simulate(RESOLUTION)

    vm_values = neuron.get("V_m")
    vm = vm_values[0] if isinstance(vm_values, (tuple, list)) else vm_values

    assert vm == pytest.approx(vm0, abs=1e-13)
