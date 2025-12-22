# -*- coding: utf-8 -*-
#
# test_cont_delay_synapse.py
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
import numpy as np
import pytest


@pytest.fixture
def prepare_kernel():
    nest.ResetKernel()


def run_simulation(resolution, delay, explicit=False):
    nest.resolution = resolution

    n = nest.Create("iaf_psc_delta_ps")
    sg = nest.Create("spike_generator", params={"precise_times": True, "spike_times": [2.0, 5.5]})
    sr = nest.Create("spike_recorder")

    # Set the delay explicitly to each connection
    if explicit:
        nest.Connect(sg, n, syn_spec={"synapse_model": "cont_delay_synapse", "weight": 100.0, "delay": delay})
        for conn in nest.GetConnections(source=sg):
            conn.set({"delay": delay})
    else:
        nest.SetDefaults("cont_delay_synapse", {"weight": 100.0, "delay": delay})
        nest.Connect(sg, n, syn_spec={"synapse_model": "cont_delay_synapse"})

    nest.Connect(n, sr)

    nest.Simulate(10.0)
    actual_spike_times = sr.events["times"]
    return actual_spike_times


@pytest.mark.parametrize(
    "expected_spike_times, resolution, delay, explicit",
    [[[3.0, 6.5], 1.0, 1.0, False], [[3.7, 7.2], 1.0, 1.7, False], [[3.7, 7.2], 1.0, 1.7, True]],
)
def test_delay_compatible_with_resolution(prepare_kernel, expected_spike_times, resolution, delay, explicit):
    actual_spike_times = run_simulation(resolution, delay, explicit)
    np.testing.assert_array_equal(actual_spike_times, expected_spike_times)


def test_delay_shorter_than_resolution(prepare_kernel):
    with pytest.raises(nest.NESTErrors.BadDelay):
        actual_spike_times = run_simulation(1.0, 0.7)
