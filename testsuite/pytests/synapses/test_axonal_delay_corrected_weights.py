# -*- coding: utf-8 -*-
#
# test_axonal_delay_corrected_weights.py
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
Verify retrospective weight correction for ``stdp_pl_synapse_hom_ax_delay``.

``test_stdp_pl_synapse_hom`` compares weights recorded by a weight recorder,
which reflect pre-correction transmitted weights. Here we check the corrected
results against a causal reference evaluated with the full postsynaptic spike
train:

- Final synapse weights once all pending corrections have completed.
- Delivered postsynaptic input (via the target neuron's ``V_m`` trace), which
  catches wrong ``WeightCorrectionEvent`` deltas when multiple presynaptic
  spikes on the same connection overlap within one critical window.

Short presynaptic inter-spike intervals (below ``axonal_delay - dendritic_delay``)
exercise multi-spike overlap and entry hand-off. All neuron models supporting
predominantly axonal delays are tested, including the boundary
``axonal_delay == dendritic_delay`` where the buffer is active but no
corrections should fire.
"""

from math import exp

import nest
import numpy as np
import pytest

RESOLUTION = 0.1
SIMTIME = 120.0
DRIVE_DELAY = 1.0
DRIVE_WEIGHT = 3000.0
N_PRE = 8
N_POST = 5
INITIAL_WEIGHT = 0.5
TAU_MINUS = 33.7
SYNAPSE_PARAMS = {"lambda": 0.1, "alpha": 0.1, "mu": 0.4, "tau_plus": 20.0}
EPS = 1e-6

# Neuron models supporting predominantly axonal delays; see
# Node::supports_axonal_delay_corrections().
SUPPORTED_NEURONS = ["iaf_psc_alpha", "iaf_psc_exp", "iaf_psc_delta", "parrot_neuron"]


def _facilitate(weight, kplus):
    return weight + SYNAPSE_PARAMS["lambda"] * pow(weight, SYNAPSE_PARAMS["mu"]) * kplus


def _depress(weight, kminus):
    new_weight = weight - SYNAPSE_PARAMS["alpha"] * SYNAPSE_PARAMS["lambda"] * weight * kminus
    return new_weight if new_weight > 0.0 else 0.0


def _causal_spike_weights(pre_spikes, post_spikes, axonal_delay, dendritic_delay):
    """Compute per-spike weights as ``send()`` would with full post history.

    Mirrors ``stdp_pl_synapse_hom_ax_delay::send()``: facilitation integrates
    over ``(t_last_pre + d_axon - d_dend, t_pre + d_axon - d_dend]`` (matching
    ``get_history()``), and depression reads the postsynaptic trace strictly
    before ``t_pre + d_axon - d_dend`` (matching ``get_K_value()``).
    ``t_lastspike_`` starts at ``-d_axon`` so the synapse starts at t = 0.
    """
    weight = INITIAL_WEIGHT
    kplus = 0.0
    t_last_pre = -axonal_delay
    spike_weights = []

    for t_pre in pre_spikes:
        t_arrival = t_pre + axonal_delay - dendritic_delay
        for t_post in post_spikes:
            if t_last_pre + axonal_delay - dendritic_delay + EPS < t_post <= t_arrival + EPS:
                minus_dt = t_last_pre + axonal_delay - (t_post + dendritic_delay)
                weight = _facilitate(weight, kplus * exp(minus_dt / SYNAPSE_PARAMS["tau_plus"]))
        kminus = sum(exp((t_post - t_arrival) / TAU_MINUS) for t_post in post_spikes if t_arrival - t_post > EPS)
        weight = _depress(weight, kminus)
        spike_weights.append(weight)
        kplus = kplus * exp((t_last_pre - t_pre) / SYNAPSE_PARAMS["tau_plus"]) + 1.0
        t_last_pre = t_pre

    return spike_weights


def _causal_weight(pre_spikes, post_spikes, axonal_delay, dendritic_delay):
    """Return the final weight produced by ``_causal_spike_weights``."""
    spike_weights = _causal_spike_weights(pre_spikes, post_spikes, axonal_delay, dendritic_delay)
    return spike_weights[-1] if spike_weights else INITIAL_WEIGHT


def _simulate(axonal_delay, dendritic_delay, pre_interval, num_threads, neuron_model="iaf_psc_alpha"):
    """Drive prescribed spike trains through an all-to-all ax-delay projection."""
    nest.ResetKernel()
    nest.set(resolution=RESOLUTION, local_num_threads=num_threads)

    stop = SIMTIME - 2.0 * (SIMTIME / 6.0)
    pre_drive = [np.round(np.arange(1.0 + 0.1 * i, stop, pre_interval + 0.1 * (i % 5)), 1) for i in range(N_PRE)]
    post_drive = [np.round(np.arange(2.0 + 0.1 * j, stop, 1.3 + 0.1 * (j % 7)), 1) for j in range(N_POST)]

    # parrot_neuron repeats its input spikes at known times; postsynaptic spike
    # times are read back from the recorder.
    pre = nest.Create("parrot_neuron", N_PRE)
    # parrot_neuron has no refractory period and repeats port 0 only, so connect
    # plastic synapses to port 1 to avoid perturbing a parrot target's firing.
    is_parrot = neuron_model == "parrot_neuron"
    post_params = {"tau_minus": TAU_MINUS} if is_parrot else {"tau_minus": TAU_MINUS, "t_ref": 0.0}
    post = nest.Create(neuron_model, N_POST, params=post_params)
    pre_gen = nest.Create("spike_generator", N_PRE, params=[{"spike_times": t} for t in pre_drive])
    post_gen = nest.Create("spike_generator", N_POST, params=[{"spike_times": t} for t in post_drive])

    nest.Connect(pre_gen, pre, "one_to_one", syn_spec={"synapse_model": "static_synapse", "delay": DRIVE_DELAY})
    nest.Connect(
        post_gen,
        post,
        "one_to_one",
        syn_spec={"synapse_model": "static_synapse", "weight": DRIVE_WEIGHT, "delay": DRIVE_DELAY},
    )

    recorder = nest.Create("spike_recorder")
    nest.Connect(pre + post, recorder, syn_spec={"synapse_model": "static_synapse"})

    nest.SetDefaults("stdp_pl_synapse_hom_ax_delay", SYNAPSE_PARAMS)
    syn_spec = {
        "synapse_model": "stdp_pl_synapse_hom_ax_delay",
        "weight": INITIAL_WEIGHT,
        "axonal_delay": axonal_delay,
        "dendritic_delay": dendritic_delay,
    }
    if is_parrot:
        syn_spec["receptor_type"] = 1
    nest.Connect(pre, post, "all_to_all", syn_spec=syn_spec)

    nest.Simulate(SIMTIME)

    senders = recorder.events["senders"]
    times = recorder.events["times"]
    recorded = {node: np.sort(times[senders == node]) for node in pre.tolist() + post.tolist()}

    # Under MPI, spike_recorder only sees local senders, whereas connections
    # live on the target rank. Derive presynaptic times from pre_drive and
    # verify them against the recorder wherever the parrot neuron is local.
    trains = {}
    for node, drive in zip(pre.tolist(), pre_drive):
        trains[node] = np.round(drive + DRIVE_DELAY, 10)
        if len(recorded[node]) > 0:
            assert recorded[node] == pytest.approx(trains[node]), f"parrot {node} did not repeat its input"
    # Postsynaptic targets are local to this rank, so their recorded trains are complete.
    for node in post.tolist():
        trains[node] = recorded[node]

    return nest.GetConnections(synapse_model="stdp_pl_synapse_hom_ax_delay"), trains


def _record_vm(neuron_model, post_drive, simtime, connect_pre):
    """Simulate a driven target neuron and return its V_m trace and spike times."""
    nest.ResetKernel()
    nest.set(resolution=RESOLUTION, local_num_threads=1)

    post = nest.Create(neuron_model, params={"tau_minus": TAU_MINUS, "t_ref": 0.0})
    post_gen = nest.Create("spike_generator", params={"spike_times": post_drive})
    nest.Connect(post_gen, post, syn_spec={"weight": DRIVE_WEIGHT, "delay": DRIVE_DELAY})

    connect_pre(post)

    sr = nest.Create("spike_recorder")
    nest.Connect(post, sr)
    mm = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": RESOLUTION})
    nest.Connect(mm, post)

    nest.Simulate(simtime)
    return mm.events["V_m"], np.sort(sr.events["times"])


@pytest.mark.parametrize(
    "axonal_delay, dendritic_delay, pre_interval",
    [
        (2.0, 0.5, 0.7),  # multiple spikes in flight on one connection
        (2.0, 0.0, 0.7),  # purely axonal delay, same regime
        (3.0, 0.1, 0.7),  # axonal delay spanning several min-delay intervals
        (2.0, 0.5, 3.0),  # interval > axonal - dendritic: no hand-off needed
        (1.0, 0.5, 0.7),  # critical window shorter than one min-delay interval
        (1.5, 1.5, 0.7),  # axonal == dendritic: no corrections required
    ],
)
# NEST raises on local_num_threads > 1 when built without OpenMP support.
@pytest.mark.parametrize("num_threads", [1, pytest.param(4, marks=pytest.mark.skipif_missing_threads)])
@pytest.mark.parametrize("neuron_model", SUPPORTED_NEURONS)
def test_corrected_weights_match_causal_reference(
    axonal_delay, dendritic_delay, pre_interval, num_threads, neuron_model
):
    """Verify that final synapse weights match the causal reference."""
    connections, trains = _simulate(axonal_delay, dendritic_delay, pre_interval, num_threads, neuron_model)

    sources = connections.source
    targets = connections.target
    weights = connections.weight
    if not isinstance(sources, (list, tuple)):
        sources, targets, weights = [sources], [targets], [weights]

    assert len(sources) > 0, "no local ax-delay connections to check"

    reference = {}
    for source, target, weight in zip(sources, targets, weights):
        if (source, target) not in reference:
            reference[(source, target)] = _causal_weight(trains[source], trains[target], axonal_delay, dendritic_delay)
        expected = reference[(source, target)]
        assert weight == pytest.approx(
            expected, rel=1e-9
        ), f"synapse {source}->{target} ended at {weight!r}, expected {expected!r}"


@pytest.mark.parametrize("neuron_model", SUPPORTED_NEURONS)
def test_corrections_actually_fire(neuron_model):
    """Verify that corrections fire when axonal > dendritic and never when equal."""
    _simulate(axonal_delay=2.0, dendritic_delay=0.5, pre_interval=0.7, num_threads=1, neuron_model=neuron_model)
    assert nest.num_corrections > 0

    _simulate(axonal_delay=1.5, dendritic_delay=1.5, pre_interval=0.7, num_threads=1, neuron_model=neuron_model)
    assert nest.num_corrections == 0


@pytest.mark.parametrize(
    "axonal_delay, dendritic_delay, pre_interval",
    [
        (2.0, 0.5, 0.5),  # multiple spikes in flight on one connection
        (3.0, 0.5, 0.5),  # axonal delay spanning several min-delay intervals
        (2.0, 0.5, 3.0),  # interval > axonal - dendritic: no hand-off needed
        (1.0, 0.5, 0.5),  # critical window shorter than one min-delay interval
        (1.5, 1.5, 0.5),  # axonal == dendritic: no corrections required
    ],
)
@pytest.mark.parametrize("neuron_model", ["iaf_psc_alpha", "iaf_psc_exp", "iaf_psc_delta"])
def test_delivered_weights_match_causal_reference(axonal_delay, dendritic_delay, pre_interval, neuron_model):
    """Verify that delivered spike weights match the causal reference.

    Compares the target neuron's ``V_m`` trace against a reference neuron
    driven with the exact causal per-spike weights. Requires
    ``dendritic_delay > 0`` so pending spikes remain in the target's ring
    buffer when corrections arrive.
    """
    simtime = 40.0
    # Binary-exact intervals keep t_spike on exact grid steps in Time::ms_stamp().
    pre_drive = np.arange(1.0, 30.0, pre_interval)
    post_drive = np.arange(2.0, 30.0, 1.5)
    pre_spikes = pre_drive + DRIVE_DELAY

    def connect_plastic(post):
        pre = nest.Create("parrot_neuron")
        pre_gen = nest.Create("spike_generator", params={"spike_times": pre_drive})
        nest.Connect(pre_gen, pre, syn_spec={"delay": DRIVE_DELAY})
        nest.SetDefaults("stdp_pl_synapse_hom_ax_delay", SYNAPSE_PARAMS)
        nest.Connect(
            pre,
            post,
            syn_spec={
                "synapse_model": "stdp_pl_synapse_hom_ax_delay",
                "weight": INITIAL_WEIGHT,
                "axonal_delay": axonal_delay,
                "dendritic_delay": dendritic_delay,
            },
        )

    vm_actual, post_spikes = _record_vm(neuron_model, post_drive, simtime, connect_plastic)
    causal_weights = _causal_spike_weights(pre_spikes, post_spikes, axonal_delay, dendritic_delay)

    def connect_reference(post):
        pre_gen = nest.Create("spike_generator", params={"spike_times": pre_spikes, "spike_weights": causal_weights})
        nest.Connect(pre_gen, post, syn_spec={"delay": axonal_delay + dendritic_delay})

    vm_ref, _ = _record_vm(neuron_model, post_drive, simtime, connect_reference)
    assert vm_actual == pytest.approx(vm_ref, rel=1e-9, abs=1e-9)

