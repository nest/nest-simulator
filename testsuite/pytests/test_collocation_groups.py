# -*- coding: utf-8 -*-
#
# test_collocation_groups.py
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
Test that the collocation of the spike registers by several threads (kernel parameter ``collocation_groups``) gives
exactly the same results as the collocation by the master thread alone, for a single MPI process (no MPI buffers are
used then) and for several MPI processes.
"""

import nest
import numpy as np
import pytest

pytestmark = pytest.mark.skipif_missing_threads


def build_and_run(groups, threads=4, two_simulations=True, synapse_model="static_synapse", record_all=True):
    nest.ResetKernel()
    nest.local_num_threads = threads
    nest.rng_seed = 321
    nest.collocation_groups = groups

    n_ex, n_in = 80, 20
    neurons = nest.Create("iaf_psc_exp", n_ex + n_in, params={"I_e": 300.0})
    neurons.V_m = nest.random.uniform(-70.0, -55.0)
    noise = nest.Create("poisson_generator", params={"rate": 8000.0})
    rec = nest.Create("spike_recorder")
    syn = {"synapse_model": synapse_model}
    nest.Connect(noise, neurons, syn_spec={"weight": 15.0, "delay": 1.0})
    nest.Connect(
        neurons[:n_ex],
        neurons,
        {"rule": "fixed_indegree", "indegree": 20},
        dict(syn, weight=nest.random.normal(20.0, 4.0), delay=nest.random.uniform(0.5, 3.0)),
    )
    nest.Connect(
        neurons[n_ex:],
        neurons,
        {"rule": "fixed_indegree", "indegree": 10},
        dict(syn, weight=-80.0, delay=nest.random.uniform(0.5, 3.0)),
    )
    nest.Connect(neurons, rec)
    nest.Simulate(100.0)
    if two_simulations:
        # spikes of the last cycle of the first call must be delivered at the start of the second call
        nest.Simulate(100.0)
    ev = rec.events
    return ev["senders"], ev["times"], neurons.get("V_m"), nest.local_spike_counter


@pytest.mark.parametrize("threads, groups", [(1, 2), (4, 2), (4, 3), (4, 4), (7, 2), (7, 3), (7, 16)])
@pytest.mark.parametrize("synapse_model", ["static_synapse", "static_synapse_hpc"])
def test_identical_results(threads, groups, synapse_model):
    ref = build_and_run(1, threads, synapse_model=synapse_model)
    grp = build_and_run(groups, threads, synapse_model=synapse_model)
    assert len(ref[0]) > 100
    assert np.array_equal(ref[0], grp[0])
    assert np.array_equal(ref[1], grp[1])
    assert np.array_equal(ref[2], grp[2])
    assert ref[3] == grp[3]


def test_identical_results_single_simulation():
    ref = build_and_run(1, two_simulations=False)
    grp = build_and_run(2, two_simulations=False)
    assert np.array_equal(ref[0], grp[0]) and np.array_equal(ref[1], grp[1])


def test_parameter():
    nest.ResetKernel()
    assert nest.collocation_groups == 1
    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.collocation_groups = 0
    nest.collocation_groups = 2
    n = nest.Create("iaf_psc_exp", 2)
    nest.Connect(n, n)
    nest.Simulate(1.0)
    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.collocation_groups = 3
