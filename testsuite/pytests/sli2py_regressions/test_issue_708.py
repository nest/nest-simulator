# -*- coding: utf-8 -*-
#
# test_issue_708.py
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
Regression test for Issue #708 (GitHub).

This test ensures that CopyModel works with connection types that use secondary events.
"""

import nest
import pytest


@pytest.mark.skipif_without_gsl
def test_copymodel_with_secondary_events():
    """
    Test that CopyModel works with connection types that use secondary events.
    """

    neuron_in = nest.Create("hh_psc_alpha_gap", params={"I_e": 200.0})
    neurons_out = nest.Create("hh_psc_alpha_gap", n=2)

    nest.CopyModel("gap_junction", "syn0", {"weight": 5.0})
    nest.CopyModel("gap_junction", "syn1", {"weight": 10.0})

    nest.Connect(
        neuron_in,
        neurons_out[0],
        conn_spec={"rule": "one_to_one", "make_symmetric": True},
        syn_spec={"synapse_model": "syn0"},
    )

    nest.Connect(
        neuron_in,
        neurons_out[1],
        conn_spec={"rule": "one_to_one", "make_symmetric": True},
        syn_spec={"synapse_model": "syn1"},
    )

    V_m_ini = neurons_out.V_m

    nest.Simulate(10.0)

    # Check that both neurons have become depolarized due to input from neuron_in
    assert all(neurons_out.V_m > V_m_ini)

    # Check stronger effect on second neuron due to larger weight
    assert neurons_out[1].V_m > neurons_out[0].V_m
