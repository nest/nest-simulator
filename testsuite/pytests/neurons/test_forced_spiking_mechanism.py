# -*- coding: utf-8 -*-
#
# test_forced_spiking_mechanism.py
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
Test forced spiking mechanism.
"""

import nest
import numpy as np
import pytest


def test_forced_spiking_mechanism():
    """
    Test the forced spiking mechanism, which causes a neuron to spike at
    defined intervals with a specified initial offset, ignoring spikes
    from its internal dynamics.
    """

    duration_sim = 50.0  # ms
    interval = 10.0  # ms
    offset = 5.0  # ms

    spikes_target = np.arange(offset, duration_sim + offset, interval)

    nest.verbosity = nest.VerbosityLevel.WARNING
    nest.ResetKernel()
    nest.resolution = 1.0  # ms

    nrn_params = dict(
        forced_spiking=True,
        forced_spiking_interval=interval,
        forced_spiking_offset=offset,
    )

    sg = nest.Create("spike_generator", dict(spike_times=np.arange(1.0, duration_sim)))

    nrn = nest.Create("eprop_iaf", 1, nrn_params)

    sr = nest.Create("spike_recorder")

    nest.Connect(sg, nrn, syn_spec=dict(weight=1000.0))
    nest.Connect(nrn, sr)

    nest.Simulate(duration_sim)

    spikes_simulation = sr.get("events", "times")

    assert spikes_simulation == pytest.approx(spikes_target)
