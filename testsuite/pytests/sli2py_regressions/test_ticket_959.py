# -*- coding: utf-8 -*-
#
# test_ticket_959.py
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
Ensures that pp_psc_delta produces the same results when run 100 times 10ms as when run 1 time 1000ms.
"""

import nest
import numpy as np


def record_spikes(sim_time, repeats):
    nest.ResetKernel()
    nest.set_verbosity("M_ERROR")

    neuron_params = {"tau_sfa": 34.0, "q_sfa": 0.0}

    population = nest.Create("pp_psc_delta", params=neuron_params)
    spike_recorder = nest.Create("spike_recorder")

    nest.Connect(population, spike_recorder)

    for w in range(0, repeats):
        nest.Simulate(sim_time)
    events = spike_recorder.get("events")
    return np.vstack((events["senders"], events["times"]))


def test_generation_matches():
    rec1 = record_spikes(1000, 1)
    rec2 = record_spikes(10, 100)

    assert np.all(rec1 == rec2)
