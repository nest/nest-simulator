# -*- coding: utf-8 -*-
#
# test_default_delays_weights.py
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
Test that correct delays and weights are set if synaptic defaults are given or overridden.
"""

import nest
import pytest

default_delay = nest.GetDefaults("static_synapse", "delay")
default_weight = nest.GetDefaults("static_synapse", "weight")


@pytest.mark.parametrize(
    "wd_spec, w_expect, d_expect",
    [
        [{}, default_weight, default_delay],
        [{"delay": 15.5}, default_weight, 15.5],
        [{"weight": 23.4}, 23.4, default_delay],
        [{"weight": 17.3, "delay": 10.1}, 17.3, 10.1],
    ],
)
def test_default_delay_and_weight(wd_spec, w_expect, d_expect):
    nest.ResetKernel()
    neuron = nest.Create("iaf_psc_alpha")
    nest.Connect(neuron, neuron, syn_spec={"synapse_model": "static_synapse", **wd_spec})
    conn = nest.GetConnections()[0].get()

    assert conn["weight"] == pytest.approx(w_expect)
    assert conn["delay"] == pytest.approx(d_expect)
