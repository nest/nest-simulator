#! /usr/bin/env python

#
#  test_event_out_proxy_multithreading_issue-696_receiver.py
#
#  This file is part of NEST.
#
#  Copyright (C) 2004 The NEST Initiative
#
#  NEST is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
#
#  NEST is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
#

import music
import itertools
import numpy as np

setup = music.Setup()

proxy = setup.publishEventInput('in')
assert proxy.isConnected()

spike_buffers = [[] for _ in range(proxy.width())]


def collect_spike(time, _, index):
    print("Received spike from neuron {}: {}".format(index, time))
    spike_buffers[index].append(time)

proxy.map(collect_spike, music.Index.GLOBAL, size=proxy.width(), base=0)

for time in itertools.takewhile(lambda t: t < 1.0, setup.runtime(0.01)):
    pass

for spike_buffer in spike_buffers:
    expected_times = [(t + 0.1) * 1e-3 for t in [0.1, 0.2, 0.3]]
    msg = "{} vs {}".format(spike_buffer, expected_times)
    assert np.isclose(spike_buffer, expected_times).all(), msg
