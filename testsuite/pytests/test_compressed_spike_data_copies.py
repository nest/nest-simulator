# -*- coding: utf-8 -*-
#
# test_compressed_spike_data_copies.py
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
Test that several copies of the compressed spike data (kernel parameter ``compressed_spike_data_copies``) give exactly
the same results as a single copy.
"""

import nest
import numpy as np
import pytest

pytestmark = pytest.mark.skipif_missing_threads


def build_and_run(copies, threads=4, compressed=True, change_between=None, synapse_model="static_synapse"):
    nest.ResetKernel()
    nest.local_num_threads = threads
    nest.rng_seed = 321
    nest.use_compressed_spikes = compressed
    nest.compressed_spike_data_copies = copies

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
    if change_between is not None:
        nest.compressed_spike_data_copies = change_between
    nest.Simulate(100.0)
    ev = rec.events
    return ev["senders"], ev["times"], nest.local_spike_counter


@pytest.mark.parametrize("threads, copies", [(1, 2), (4, 2), (4, 3), (4, 4), (7, 2), (7, 3), (7, 16)])
@pytest.mark.parametrize("synapse_model", ["static_synapse", "static_synapse_hpc"])
def test_identical_results(threads, copies, synapse_model):
    ref = build_and_run(1, threads, synapse_model=synapse_model)
    cpy = build_and_run(copies, threads, synapse_model=synapse_model)
    assert len(ref[0]) > 100
    for a, b in zip(ref, cpy):
        assert np.array_equal(a, b)


@pytest.mark.parametrize("before, after", [(1, 2), (3, 1), (2, 4)])
def test_change_between_simulations(before, after):
    ref = build_and_run(1)
    cpy = build_and_run(before, change_between=after)
    for a, b in zip(ref, cpy):
        assert np.array_equal(a, b)


def test_without_compressed_spikes():
    ref = build_and_run(1, compressed=False)
    cpy = build_and_run(3, compressed=False)
    for a, b in zip(ref, cpy):
        assert np.array_equal(a, b)


def test_parameter():
    nest.ResetKernel()
    assert nest.compressed_spike_data_copies == 1
    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.compressed_spike_data_copies = 0
    nest.compressed_spike_data_copies = 2
    assert nest.compressed_spike_data_copies == 2
    nest.ResetKernel()
    assert nest.compressed_spike_data_copies == 1
