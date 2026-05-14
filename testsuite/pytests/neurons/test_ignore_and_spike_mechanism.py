# -*- coding: utf-8 -*-
#
# test_ignore_and_spike_mechanism.py
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
Test the ignore-and-spike mechanism, which causes a neuron to spike at
defined intervals with a specified initial offset, ignoring spikes
from its internal dynamics.
"""

import nest
import numpy as np
import pytest

SIM_DURATION = 10.0
RESOLUTION = 1.0


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()
    nest.set(resolution=RESOLUTION)
    nest.verbosity = nest.VerbosityLevel.WARNING


@pytest.mark.parametrize(
    "params",
    [
        dict(ignore_and_spike=True, ignore_and_spike_offset=0.0, ignore_and_spike_interval=10.0),
        dict(ignore_and_spike=True, ignore_and_spike_offset=2.0, ignore_and_spike_interval=0.0),
    ],
)
def test_invalid_ignore_and_spike_parameters(params):
    with pytest.raises(Exception):
        nest.Create("eprop_iaf", params)


def expected_times(start_time, stop_time, interval):
    return np.arange(start_time, stop_time + 0.5 * RESOLUTION, interval)


def append_expected_events(events_expected, sender, times):
    events_expected["senders"].extend([sender] * len(times))
    events_expected["times"].extend(times)


def simulate_and_update_expected(updated_nodes, events_expected, last_spike_times):
    start_time = nest.biological_time
    stop_time = start_time + SIM_DURATION
    updated_ids = [node.global_id for node in updated_nodes]

    for node in nest.GetNodes(dict(model="eprop_iaf")):
        if node.global_id in updated_ids or node.global_id not in last_spike_times:
            first_spike = start_time + node.ignore_and_spike_offset
        else:
            first_spike = last_spike_times[node.global_id] + node.ignore_and_spike_interval

        times = expected_times(first_spike, stop_time, node.ignore_and_spike_interval)
        append_expected_events(events_expected, node.global_id, times)

        if len(times) > 0:
            last_spike_times[node.global_id] = times[-1]

    nest.Simulate(SIM_DURATION)


def sorted_events(events):
    order = np.lexsort((events["senders"], events["times"]))
    return dict(senders=np.asarray(events["senders"])[order], times=np.asarray(events["times"])[order])


def test_ignore_and_spike_generates_expected_spikes():
    spike_recorder = nest.Create("spike_recorder")
    events_expected = dict(senders=[], times=[])
    last_spike_times = dict()

    nrn = nest.Create(
        "eprop_iaf", dict(ignore_and_spike=True, ignore_and_spike_offset=1.0, ignore_and_spike_interval=2.0)
    )
    nest.Connect(nrn, spike_recorder)

    nrn.set(ignore_and_spike_offset=2.0, ignore_and_spike_interval=3.0)
    simulate_and_update_expected([nrn], events_expected, last_spike_times)

    simulate_and_update_expected([], events_expected, last_spike_times)

    nrn.set(ignore_and_spike_offset=2.0, ignore_and_spike_interval=4.0)
    simulate_and_update_expected([nrn], events_expected, last_spike_times)

    nrn1 = nest.Create(
        "eprop_iaf", dict(ignore_and_spike=True, ignore_and_spike_offset=2.0, ignore_and_spike_interval=3.0)
    )
    nest.Connect(nrn1, spike_recorder)
    simulate_and_update_expected([nrn1], events_expected, last_spike_times)

    nrn1.set(ignore_and_spike_offset=4.0, ignore_and_spike_interval=20.0)
    simulate_and_update_expected([nrn1], events_expected, last_spike_times)

    events_simulated = sorted_events(spike_recorder.get("events"))
    events_expected = sorted_events(events_expected)

    assert events_simulated["times"] == pytest.approx(events_expected["times"], rel=1e-9)
