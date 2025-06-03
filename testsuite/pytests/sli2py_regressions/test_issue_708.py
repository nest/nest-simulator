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
    nest.ResetKernel()

    neuron_in = nest.Create("hh_psc_alpha_gap")
    neuron_out1 = nest.Create("hh_psc_alpha_gap")
    neuron_out2 = nest.Create("hh_psc_alpha_gap")
    vm1 = nest.Create("voltmeter")
    vm2 = nest.Create("voltmeter")

    nest.CopyModel("gap_junction", "syn0")
    nest.CopyModel("gap_junction", "syn1")

    nest.SetStatus(neuron_in, {"I_e": 200.0})
    nest.SetStatus(vm1, {"interval": 1.0})
    nest.SetStatus(vm2, {"interval": 1.0})

    nest.Connect(
        neuron_in,
        neuron_out1,
        conn_spec={"rule": "one_to_one", "make_symmetric": True},
        syn_spec={"synapse_model": "syn0", "weight": 10.0},
    )

    nest.Connect(
        neuron_in,
        neuron_out2,
        conn_spec={"rule": "one_to_one", "make_symmetric": True},
        syn_spec={"synapse_model": "syn1", "weight": 10.0},
    )

    nest.Connect(vm1, neuron_out1)
    nest.Connect(vm2, neuron_out2)

    nest.Simulate(10.0)

    # Check that neuron_out1 received the input
    events_vm1 = nest.GetStatus(vm1, "events")[0]
    V_m_vm1 = events_vm1["V_m"]
    assert V_m_vm1[8] > -6.960401e01

    # Check that neuron_out2 received the input
    events_vm2 = nest.GetStatus(vm2, "events")[0]
    V_m_vm2 = events_vm2["V_m"]
    assert V_m_vm2[8] > -6.960401e01
