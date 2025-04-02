# -*- coding: utf-8 -*-
#
# test_issue_327.py
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
Regression test for Issue #327 (GitHub).

This test ensures that NEST handles round-off correctly for large, precise spike times.
"""

import nest
import pytest


def test_spike_generator_large_precise_times():
    """
    Test that spike_generator handles large precise times correctly.
    """
    nest.ResetKernel()
    nest.SetKernelStatus({"resolution": 0.1})

    sg = nest.Create(
        "spike_generator",
        params={
            "precise_times": True,
            "spike_times": [353538.4 - 0.1],  # Adjust for use of parrot_neuron by subtracting min_delay
        },
    )

    pn = nest.Create("parrot_neuron_ps")
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})

    nest.Connect(sg, pn)
    nest.Connect(pn, sr)

    nest.Simulate(360000)

    events = nest.GetStatus(sr, "events")[0]
    times = events["times"]
    offsets = events["offsets"]

    # Check correct step, account for delay 1 ms
    assert times[0] == 3535393

    # Check for correct offset, precision limited by spike time * eps
    assert abs(offsets[0]) < 1e-9
