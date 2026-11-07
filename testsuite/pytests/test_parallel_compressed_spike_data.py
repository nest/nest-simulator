# -*- coding: utf-8 -*-
#
# test_parallel_compressed_spike_data.py
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
Test the compressed spike data, which all threads build together when the connection infrastructure is set up.

The simulation with compressed spikes must give the same spikes as the simulation without. The tests cover several
synapse types in one network, more threads than neurons, and gap junctions; a further test adds connections after a
simulation, so that the compressed spike data are built twice.
In builds with assertions, the kernel also compares the compressed spike data with the result of the serial
algorithm. The file is linked into the MPI test directories for 2 and 4 processes.
"""

import nest
import numpy as np
import pytest

pytestmark = pytest.mark.skipif_missing_threads


def build_and_run(compressed, threads, synapse_models=("static_synapse",), n_ex=80, n_in=20, connect_after=False):
    """
    Simulate a random network and return the sorted spikes and the local spike counter.
    """

    nest.ResetKernel()
    nest.local_num_threads = threads
    nest.rng_seed = 123
    nest.use_compressed_spikes = compressed

    neurons = nest.Create("iaf_psc_exp", n_ex + n_in, params={"I_e": 300.0})
    neurons.V_m = nest.random.uniform(-70.0, -55.0)
    noise = nest.Create("poisson_generator", params={"rate": 8000.0})
    rec = nest.Create("spike_recorder")
    nest.Connect(noise, neurons, syn_spec={"weight": 15.0, "delay": 1.0})
    for synapse_model in synapse_models:
        nest.Connect(
            neurons[:n_ex],
            neurons,
            {"rule": "fixed_indegree", "indegree": min(20, n_ex)},
            {
                "synapse_model": synapse_model,
                "weight": nest.random.normal(10.0, 2.0),
                "delay": nest.random.uniform(0.5, 3.0),
            },
        )
        nest.Connect(
            neurons[n_ex:],
            neurons,
            {"rule": "fixed_indegree", "indegree": min(10, n_in)},
            {"synapse_model": synapse_model, "weight": -40.0, "delay": nest.random.uniform(0.5, 3.0)},
        )
    nest.Connect(neurons, rec)
    nest.Simulate(100.0)
    if connect_after:
        nest.Connect(neurons[n_ex:], neurons[:n_ex], {"rule": "pairwise_bernoulli", "p": 0.2}, {"weight": -20.0})
    nest.Simulate(100.0)
    ev = rec.events
    # the order of spikes within a time step depends on the order of delivery
    order = np.lexsort((ev["senders"], ev["times"]))
    return ev["senders"][order], ev["times"][order], nest.local_spike_counter


def assert_identical(ref, res):
    assert len(ref[0]) > 0
    for a, b in zip(ref, res):
        assert np.array_equal(a, b)


@pytest.mark.parametrize("threads", [1, 2, 3, 4, 7])
@pytest.mark.parametrize(
    "synapse_models", [("static_synapse",), ("static_synapse_hpc",), ("static_synapse", "stdp_synapse")]
)
def test_compressed_equals_uncompressed(threads, synapse_models):
    ref = build_and_run(False, threads, synapse_models)
    res = build_and_run(True, threads, synapse_models)
    assert_identical(ref, res)


def test_more_threads_than_neurons():
    ref = build_and_run(False, 16, n_ex=6, n_in=3)
    res = build_and_run(True, 16, n_ex=6, n_in=3)
    assert_identical(ref, res)


@pytest.mark.parametrize("threads", [2, 5])
def test_connect_after_simulate(threads):
    """
    Rebuild the compressed spike data after connections are added.

    Compressed and uncompressed spikes are not compared here: after connections are added, they give different spike
    trains also without the parallel construction. Builds with assertions compare the rebuilt structure with the result
    of the serial algorithm.
    """
    senders, times, local_spike_counter = build_and_run(True, threads, connect_after=True)
    assert len(senders) > 0 and local_spike_counter > 0


def run_gap_junctions(compressed, threads):
    nest.ResetKernel()
    nest.local_num_threads = threads
    nest.use_compressed_spikes = compressed
    nest.resolution = 0.05
    nest.wfr_comm_interval = 1.0
    nest.rng_seed = 123

    neurons = nest.Create("hh_psc_alpha_gap", 8)
    neurons[::2].I_e = 300.0
    neurons.V_m = nest.random.uniform(-70.0, -60.0)
    nest.Connect(
        neurons[:4],
        neurons[4:],
        {"rule": "one_to_one", "make_symmetric": True},
        {"synapse_model": "gap_junction", "weight": 0.5},
    )
    nest.Connect(neurons[:4], neurons[4:], {"rule": "all_to_all"}, {"weight": 20.0, "delay": 1.0})
    mm = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": 0.5})
    nest.Connect(mm, neurons)
    nest.Simulate(50.0)
    ev = mm.events
    order = np.lexsort((ev["senders"], ev["times"]))
    return ev["senders"][order], ev["times"][order], ev["V_m"][order]


@pytest.mark.parametrize("threads", [1, 3])
def test_gap_junctions(threads):
    ref = run_gap_junctions(False, threads)
    res = run_gap_junctions(True, threads)
    for a, b in zip(ref, res):
        assert np.array_equal(a, b)
