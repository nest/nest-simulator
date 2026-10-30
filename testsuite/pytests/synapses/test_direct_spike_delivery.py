# -*- coding: utf-8 -*-
#
# test_direct_spike_delivery.py
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
Test that direct spike delivery (kernel parameter ``direct_spike_delivery``) gives exactly the same
results as the regular delivery path.

Direct delivery applies to static_synapse_hpc connections onto neuron models that support it
(currently iaf_psc_exp); all other targets must be served by the regular path.
"""

import nest
import numpy as np
import pytest

pytestmark = pytest.mark.skipif_missing_threads


def build_and_run(
    direct, target_model="iaf_psc_exp", synapse_model="static_synapse_hpc", extra_nodes=False, weight_recorder=False
):
    nest.ResetKernel()
    nest.local_num_threads = 2
    nest.rng_seed = 123
    nest.direct_spike_delivery = direct

    n_ex, n_in = 80, 20
    neurons = nest.Create(target_model, n_ex + n_in, params={"I_e": 300.0})
    neurons.V_m = nest.random.uniform(-70.0, -55.0)
    noise = nest.Create("poisson_generator", params={"rate": 8000.0})
    rec = nest.Create("spike_recorder")
    mm = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": 0.1})

    syn_spec = {"synapse_model": synapse_model}
    if weight_recorder:
        wr = nest.Create("weight_recorder")
        nest.CopyModel(synapse_model, "static_wr", {"weight_recorder": wr})
        syn_spec = {"synapse_model": "static_wr"}
    nest.Connect(noise, neurons, syn_spec={"weight": 15.0, "delay": 1.0})
    nest.Connect(
        neurons[:n_ex],
        neurons,
        {"rule": "fixed_indegree", "indegree": 20},
        dict(syn_spec, weight=nest.random.normal(20.0, 4.0), delay=nest.random.uniform(0.5, 3.0)),
    )
    nest.Connect(
        neurons[n_ex:],
        neurons,
        {"rule": "fixed_indegree", "indegree": 10},
        dict(syn_spec, weight=-80.0, delay=nest.random.uniform(0.5, 3.0)),
    )
    nest.Connect(neurons, rec)
    nest.Connect(mm, neurons[:10])
    nest.Simulate(200.0)
    if extra_nodes:
        # new nodes and connections between two calls to Simulate:
        # input buffers are prepared again, tables must be rebuilt
        new = nest.Create(target_model, 20, params={"I_e": 300.0})
        nest.Connect(
            neurons[:n_ex],
            new,
            {"rule": "fixed_indegree", "indegree": 10},
            dict(syn_spec, weight=nest.random.normal(20.0, 4.0), delay=nest.random.uniform(0.5, 3.0)),
        )
        nest.Connect(
            new, neurons[:n_ex], {"rule": "fixed_indegree", "indegree": 5}, dict(syn_spec, weight=10.0, delay=1.5)
        )
        nest.Connect(new, rec)
        nest.Simulate(200.0)
    ev = rec.get("events")
    vm = mm.get("events")
    return ev["senders"], ev["times"], vm["V_m"]


def assert_identical(a, b):
    for x, y in zip(a, b):
        np.testing.assert_array_equal(x, y)


def test_default_is_off_and_resets():
    nest.ResetKernel()
    assert nest.direct_spike_delivery is False
    nest.direct_spike_delivery = True
    assert nest.direct_spike_delivery is True
    nest.ResetKernel()
    assert nest.direct_spike_delivery is False


def test_identical_to_regular_delivery():
    ref = build_and_run(False)
    assert len(ref[0]) > 0, "network must spike"
    assert_identical(ref, build_and_run(True))


def test_identical_with_nodes_added_between_simulations():
    ref = build_and_run(False, extra_nodes=True)
    assert_identical(ref, build_and_run(True, extra_nodes=True))


def test_unsupported_target_model_uses_regular_path():
    ref = build_and_run(False, target_model="iaf_psc_alpha")
    assert_identical(ref, build_and_run(True, target_model="iaf_psc_alpha"))


def test_weight_recorder_still_receives_events():
    ref = build_and_run(False, weight_recorder=True)
    assert_identical(ref, build_and_run(True, weight_recorder=True))


def test_pointer_synapse_unaffected():
    ref = build_and_run(False, synapse_model="static_synapse")
    assert_identical(ref, build_and_run(True, synapse_model="static_synapse"))
