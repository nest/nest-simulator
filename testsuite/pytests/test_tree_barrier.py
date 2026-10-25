# -*- coding: utf-8 -*-
#
# test_tree_barrier.py
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
Test that the spinning tree barrier of the update loop (kernel parameters tree_barrier and tree_barrier_fan_in)
gives exactly the same results as the OpenMP barriers.
"""

import nest
import numpy as np
import pytest

pytestmark = pytest.mark.skipif_missing_threads


def build_and_run(tree, threads, fan_in=8):
    nest.ResetKernel()
    nest.local_num_threads = threads
    nest.rng_seed = 321
    nest.tree_barrier = tree
    nest.tree_barrier_fan_in = fan_in

    n_ex, n_in = 80, 20
    neurons = nest.Create("iaf_psc_exp", n_ex + n_in, params={"I_e": 300.0})
    neurons.V_m = nest.random.uniform(-70.0, -55.0)
    noise = nest.Create("poisson_generator", params={"rate": 8000.0})
    rec = nest.Create("spike_recorder")
    nest.Connect(noise, neurons, syn_spec={"weight": 15.0, "delay": 1.0})
    nest.Connect(
        neurons[:n_ex],
        neurons,
        {"rule": "fixed_indegree", "indegree": 20},
        {"weight": nest.random.normal(20.0, 4.0), "delay": nest.random.uniform(0.5, 3.0)},
    )
    nest.Connect(
        neurons[n_ex:],
        neurons,
        {"rule": "fixed_indegree", "indegree": 10},
        {"weight": -80.0, "delay": nest.random.uniform(0.5, 3.0)},
    )
    nest.Connect(neurons, rec)
    nest.Simulate(100.0)
    nest.Simulate(100.0)
    ev = rec.events
    return ev["senders"], ev["times"], neurons.V_m


@pytest.mark.parametrize("threads, fan_in", [(1, 8), (2, 2), (3, 2), (5, 4), (8, 8), (9, 3), (16, 4)])
def test_identical_results(threads, fan_in):
    ref = build_and_run(False, threads)
    tree = build_and_run(True, threads, fan_in)
    assert len(ref[0]) > 100
    for a, b in zip(ref, tree):
        assert np.array_equal(a, b)


def test_parameters():
    nest.ResetKernel()
    assert nest.tree_barrier is False
    assert nest.tree_barrier_fan_in == 8
    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.tree_barrier_fan_in = 1
