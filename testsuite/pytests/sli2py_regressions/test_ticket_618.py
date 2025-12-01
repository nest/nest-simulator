# -*- coding: utf-8 -*-
#
# test_ticket_618.py
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

import math

import nest

"""
Regression test for Ticket #618.

Test ported from SLI regression test.
Ensure neuron models either reject degenerate tau configurations or handle them gracefully.

Author: Hans Ekkehard Plesser, 2012-12-11
"""


EXCLUDED_MODELS = {
    "eprop_iaf_bsshslm_2020",
    "eprop_iaf_adapt_bsshslm_2020",
    "eprop_readout_bsshslm_2020",
    "eprop_iaf",
    "eprop_iaf_adapt",
    "eprop_iaf_psc_delta",
    "eprop_iaf_psc_delta_adapt",
    "eprop_readout",
    "iaf_bw_2001",
}


def test_ticket_618_tau_parameters_raise_or_behave():
    """
    Ensure exact-integration models either raise an error or produce finite membrane potentials when
    tau_mem equals tau_syn.
    """

    failing_models = []

    for model in nest.node_models:
        if model in EXCLUDED_MODELS:
            continue

        nest.ResetKernel()

        defaults = nest.GetDefaults(model)
        if "V_m" not in defaults:
            continue

        tau_keys = [key for key in defaults if key.startswith("tau_")]
        if not tau_keys:
            continue

        tau_params = {key: 10.0 for key in tau_keys}

        try:
            neuron = nest.Create(model, params=tau_params)
        except Exception:
            continue

        nest.Simulate(10.0)

        v_m = neuron.get("V_m")

        if math.isnan(v_m):
            failing_models.append(model)

    assert not failing_models, f"Models with NaN membrane potential: {sorted(failing_models)}"
