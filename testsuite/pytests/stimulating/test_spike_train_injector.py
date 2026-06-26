# -*- coding: utf-8 -*-
#
# test_spike_train_injector.py
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
Test that ``spike_train_injector`` behaves as expected.

This set of tests verifies that the spike_train_injector correctly handles
the spike times for different options and emits spikes correctly in
simulation.
"""

import math

import nest
import pytest


@pytest.fixture
def reset_kernel():
    nest.ResetKernel()


@pytest.mark.parametrize(
    ("in_spike_times", "expected_spike_times", "precise_times", "allow_offgrid_times"),
    [
        ([1.0, 1.9999, 3.0001], [1.0, 2.0, 3.0], False, False),
        ([1.0, 1.05, 3.0001], [1.0, 1.1, 3.0], False, True),
        ([1.0, 1.05, 3.0001], [1.0, 1.05, 3.0001], True, False),
    ],
)
def test_set_spike_times(reset_kernel, in_spike_times, expected_spike_times, precise_times, allow_offgrid_times):
    """
    Test correct handling of spike times.

    This test verifies correct handling of spike times not coinciding with
    a step for different options. We set NEST to work with default resolution
    (step size) of 0.1 ms and default tic length of 0.001 ms.
    """

    nest.set(resolution=0.1, tics_per_ms=1000)

    inj_nrn = nest.Create(
        "spike_train_injector",
        params={
            "spike_times": in_spike_times,
            "precise_times": precise_times,
            "allow_offgrid_times": allow_offgrid_times,
        },
    )

    out_spike_times = inj_nrn.spike_times
    assert out_spike_times == pytest.approx(expected_spike_times)


@pytest.mark.parametrize("parrot_model", ["parrot_neuron", "parrot_neuron_ps"])
def test_spike_train_injector_in_simulation(reset_kernel, parrot_model):
    """
    Test spike train injector in simulation.

    This test verifies the behavior of the spike train injector neuron in
    simulations by using parrot neuron's spike repetition properties.
    """

    spike_time = 1.01
    delay = 0.2
    simtime = 2.0

    source = nest.Create("spike_train_injector", 1, {"spike_times": [spike_time], "precise_times": True})
    parrot = nest.Create(parrot_model)
    srec = nest.Create("spike_recorder")

    nest.Connect(source, parrot, syn_spec={"delay": delay})
    nest.Connect(parrot, srec)

    nest.Simulate(simtime)

    spike_data = srec.events
    post_time = spike_data["times"]

    expected_post_time = spike_time + delay
    if parrot_model == "parrot_neuron":
        # round-up
        expected_post_time = math.ceil(expected_post_time / nest.resolution) * nest.resolution

    assert post_time == pytest.approx(expected_post_time)
