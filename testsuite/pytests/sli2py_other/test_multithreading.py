# -*- coding: utf-8 -*-
#
# test_multithreading.py
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
This is a simple testscript to test if multithreading is working
correctly. The following things are tested:
  * Does setting the number of threads to x result in x threads?
  * Does ResetKernel reset the number of threads to 1?
  * Does default node distribution (modulo) work as expected?
  * Are spikes transmitted between threads as expected?
"""
import pytest
import nest
import numpy as np
import numpy.testing as nptest

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture(autouse=True)
def prepare():
    nest.ResetKernel()


def test_check_nodes_distribution():
    nest.local_num_threads = 4

    neurons = nest.Create("iaf_psc_alpha", nest.local_num_threads)
    vps = np.array(neurons.tolist()) % nest.local_num_threads
    assert (neurons.vp == vps).all()


def test_transmitted_spikes_btw_threads():
    nest.local_num_threads = 4

    sg = nest.Create("spike_generator", {"spike_times": [1.0]})
    pA = nest.Create("parrot_neuron", nest.local_num_threads)
    pB = nest.Create("parrot_neuron", nest.local_num_threads)
    sr = nest.Create("spike_recorder")

    nest.Connect(sg, pA, "all_to_all", syn_spec={"delay": 1.})
    nest.Connect(pA, pB, "all_to_all", syn_spec={"delay": 1.})
    nest.Connect(pB, sr)

    t_sim = 1.0 + 3 * 1.0
    nest.Simulate(t_sim)

    sr_times = sr.get("events")["times"]

    excepted = [3] * (nest.local_num_threads ** 2)

    nptest.assert_array_equal(sr_times, excepted)
