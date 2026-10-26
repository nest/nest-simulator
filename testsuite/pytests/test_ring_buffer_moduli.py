# -*- coding: utf-8 -*-
#
# test_ring_buffer_moduli.py
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
Test the mapping of delays to ring-buffer bins (modulo tables of the EventDeliveryManager).

A spike sent at time t with delay d must arrive at the target at t + d, for every delay between the minimal and the
maximal delay, for the step-based ring buffers (parrot_neuron) and the slice-based ring buffers (parrot_neuron_ps), for
different ratios of max_delay to min_delay and when the simulation is split into pieces that are not multiples of
min_delay.
"""

import nest
import numpy as np
import pytest

RESOLUTION = 0.1


def run(parrot_model, min_delay, max_delay, chunks, spike_times):
    nest.ResetKernel()
    nest.set(resolution=RESOLUTION, local_num_threads=2)
    delays = np.round(np.arange(min_delay, max_delay + RESOLUTION / 2, RESOLUTION), 1)
    nest.set(min_delay=min_delay, max_delay=max_delay)
    precise = parrot_model.endswith("_ps")
    sg = nest.Create("spike_generator", params={"spike_times": spike_times, "precise_times": precise})
    pre = nest.Create(parrot_model)
    nest.Connect(sg, pre, syn_spec={"delay": min_delay})
    post = nest.Create(parrot_model, len(delays))
    rec = nest.Create("spike_recorder")
    for k, d in enumerate(delays):
        nest.Connect(pre, post[k], syn_spec={"delay": float(d)})
    nest.Connect(post, rec)
    for c in chunks:
        nest.Simulate(c)
    ev = rec.get("events")
    sent = np.asarray(spike_times) + min_delay  # the first parrot repeats the spikes after min_delay
    expected = sorted((int(post[k].global_id), round(t + d, 6)) for k, d in enumerate(delays) for t in sent)
    got = sorted(zip(ev["senders"].tolist(), np.round(ev["times"], 6).tolist()))
    return expected, got


@pytest.mark.parametrize("parrot_model", ["parrot_neuron", "parrot_neuron_ps"])
@pytest.mark.parametrize("min_delay, max_delay", [(0.1, 0.1), (0.1, 2.0), (0.3, 1.0), (0.5, 1.7), (1.0, 3.0)])
@pytest.mark.parametrize("chunks", [[20.0], [3.7, 0.1, 5.3, 10.9], [0.1] * 40 + [16.0]])
def test_spikes_arrive_after_their_delay(parrot_model, min_delay, max_delay, chunks):
    spike_times = [1.0, 2.3, 4.0, 7.7, 11.1]
    expected, got = run(parrot_model, min_delay, max_delay, chunks, spike_times)
    assert got == expected
