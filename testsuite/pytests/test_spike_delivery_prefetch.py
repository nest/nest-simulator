# -*- coding: utf-8 -*-
#
# test_spike_delivery_prefetch.py
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
Test that software prefetching in the spike delivery (kernel parameter ``spike_delivery_prefetch``) gives exactly the
same results as the delivery without prefetching.
"""

import nest
import numpy as np
import pytest


def build_and_run(prefetch, threads, synapse_model):
    nest.ResetKernel()
    nest.local_num_threads = threads
    nest.rng_seed = 123
    nest.spike_delivery_prefetch = prefetch

    n_ex, n_in = 160, 40
    neurons = nest.Create("iaf_psc_alpha", n_ex + n_in, params={"I_e": 300.0})
    neurons.V_m = nest.random.uniform(-70.0, -55.0)
    noise = nest.Create("poisson_generator", params={"rate": 8000.0})
    rec = nest.Create("spike_recorder")
    mm = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": 0.1})
    syn = {"synapse_model": synapse_model}
    nest.Connect(noise, neurons, syn_spec={"weight": 15.0, "delay": 1.0})
    nest.Connect(
        neurons[:n_ex],
        neurons,
        {"rule": "fixed_indegree", "indegree": 40},
        dict(syn, weight=nest.random.normal(20.0, 4.0), delay=nest.random.uniform(0.5, 3.0)),
    )
    nest.Connect(
        neurons[n_ex:],
        neurons,
        {"rule": "fixed_indegree", "indegree": 10},
        dict(syn, weight=-80.0, delay=nest.random.uniform(0.5, 3.0)),
    )
    nest.Connect(neurons, rec)
    nest.Connect(mm, neurons[:10])
    nest.Simulate(100.0)
    nest.Simulate(100.0)
    ev = rec.events
    return ev["senders"], ev["times"], mm.events["V_m"], nest.local_spike_counter


def test_parameter():
    nest.ResetKernel()
    assert nest.spike_delivery_prefetch is False
    nest.spike_delivery_prefetch = True
    assert nest.spike_delivery_prefetch is True
    nest.ResetKernel()
    assert nest.spike_delivery_prefetch is False


@pytest.mark.parametrize(
    "threads",
    [
        1,
        pytest.param(2, marks=pytest.mark.skipif_missing_threads),
        pytest.param(4, marks=pytest.mark.skipif_missing_threads),
    ],
)
@pytest.mark.parametrize("synapse_model", ["static_synapse", "static_synapse_hpc", "stdp_synapse"])
def test_identical_results(threads, synapse_model):
    ref = build_and_run(False, threads, synapse_model)
    assert len(ref[0]) > 100
    res = build_and_run(True, threads, synapse_model)
    for a, b in zip(ref, res):
        assert np.array_equal(a, b)
