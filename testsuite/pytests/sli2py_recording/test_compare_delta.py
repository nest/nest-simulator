# -*- coding: utf-8 -*-
#
# test_compare_delta.py
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
Test that spike timings of plain and canon iaf_psc populations match empirical data given preconfigured settings.
"""
import nest
import numpy as np
import pytest


def test_simulation_completes():
    """
    Create population variations and connect spike generator and recorder to both. Simulate and check event
    dictionary for timings matching empirical data, in either order to account for different neurons being recorded
    first.
    """
    neuron_params = {
        "E_L": -49.0,
        "V_m": -60.0,
        "V_th": -50.0,
        "V_reset": -60.0,
        "C_m": 200.0,
        "tau_m": 20.0,
        "t_ref": 5.0,
    }

    nest.ResetKernel()
    nest.resolution = 0.01

    population_plain = nest.Create("iaf_psc_delta", params=neuron_params)
    population_canon = nest.Create("iaf_psc_delta_ps", params=neuron_params)

    spike_generator = nest.Create(
        "spike_generator", {"spike_times": [1.0, 2.0, 3.0, 4.0, 5.0, 10.5, 12.0], "precise_times": False}
    )
    spike_recorder = nest.Create("spike_recorder")
    nest.SetDefaults("static_synapse", {"delay": 0.1, "weight": 2.5})

    for population in [population_plain, population_canon]:
        nest.Connect(spike_generator, population)
        nest.Connect(population, spike_recorder)

    nest.Simulate(200.0)

    spike_recs = spike_recorder.get("events", ["senders", "times"])

    assert np.all(np.isin(np.array([1, 2]), spike_recs["senders"].T[:2]))
    assert np.all(spike_recs["times"].T[:2] == pytest.approx(4.1))
