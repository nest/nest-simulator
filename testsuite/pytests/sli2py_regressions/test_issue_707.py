# -*- coding: utf-8 -*-
#
# test_issue_707.py
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
Regression test for Issue #707 (GitHub).

This test ensures that the weight of gap junctions can be recorded with the weight_recorder.
"""

import nest
import pytest


@pytest.mark.skipif_without_gsl
def test_gap_junction_weight_recording():
    """
    Test that the weight of gap junctions can be recorded with the weight_recorder.
    """
    nest.ResetKernel()

    neuron_in = nest.Create("hh_psc_alpha_gap")
    neuron_out = nest.Create("hh_psc_alpha_gap")
    wr = nest.Create("weight_recorder")

    nest.SetDefaults("gap_junction", {"weight_recorder": wr})

    nest.Connect(
        neuron_in,
        neuron_out,
        conn_spec={"rule": "one_to_one", "make_symmetric": True},
        syn_spec={"synapse_model": "gap_junction", "weight": 2.0},
    )

    nest.Simulate(10.0)

    assert wr.get("events", "weights")[0] == 2.0
