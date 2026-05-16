# -*- coding: utf-8 -*-
#
# test_flush_event_filtering.py
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
Test that connection types that do not support flush events filter them out.

Connection types that do not support flush events (e.g., static_synapse) do not
transmit flush events, preventing them from reaching recorders and corrupting
spike train analyses.
"""

import nest
import pytest


def test_flush_events_not_transmitted():
    """
    Verify that static_synapse does not transmit flush events to spike recorder.
    """
    nest.ResetKernel()

    spike_time = 10.0
    sg = nest.Create("spike_generator", params=dict(spike_times=[spike_time]))
    pt = nest.Create("parrot_neuron", params=dict(flush_event_send_interval=5.0))
    sr = nest.Create("spike_recorder")

    nest.Connect(sg, pt)
    nest.Connect(pt, sr)

    nest.Simulate(30.0)

    spike_times = sr.get("events")["times"]

    assert (
        len(spike_times) == 1
    ), f"Expected 1 spike, got {len(spike_times)}. Flush events may have been transmitted incorrectly."

    expected_time = spike_time + nest.GetDefaults("static_synapse")["delay"]
    assert spike_times[0] == pytest.approx(expected_time, abs=0.01)
