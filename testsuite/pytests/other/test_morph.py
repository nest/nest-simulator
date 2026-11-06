# -*- coding: utf-8 -*-
#
# test_morph.py
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
Tests of structure-aware communication (morph).

Morph (Lober, Diesmann, Kunkel 2026, Neuromorph Comput Eng 6:024024) exchanges the spikes of connections created
with the ``conn_spec`` property ``long_range`` among all processes once per interval of ``threshold_delay`` and
all other spikes every ``min_delay`` within a local group of processes (``comm_local_ranks``).

The tests run with one process and, through symbolic links in ``mpi_direct/2`` and ``mpi_direct/4``, with two
and four processes. Tests that need more processes skip themselves. Spike trains are compared process by process:
the conventional exchange and morph place the neurons identically, so every process records the same spikes.
"""

import nest
import numpy as np
import pytest

THRESHOLD = 1.0  # ms


def num_procs():
    return nest.NumProcesses()


def rank_of(node_id):
    """Return the rank of the process that owns the node (MPIManager::get_process_id_of_node_id())."""
    return node_id % nest.total_num_virtual_procs % nest.NumProcesses()


def recorded_spikes(sr):
    """Return the local spikes of a spike recorder as sorted array of (sender, time in steps, offset)."""
    ev = sr.get("events")
    data = np.column_stack([ev["senders"], ev["times"], ev["offsets"]]) if len(ev["senders"]) else np.zeros((0, 3))
    return data[np.lexsort((data[:, 2], data[:, 0], data[:, 1]))]


def reset(threads=2, seed=12345, **morph_params):
    """Reset the kernel and set threads, seed and the parameters of morph."""
    nest.ResetKernel()
    nest.set(local_num_threads=threads if nest.build_info["have_threads"] else 1, rng_seed=seed)
    if morph_params:
        nest.SetKernelStatus(morph_params)


# ----------------------------------------------------------------------------------------------------------------
# Interface without morph


def test_interface_without_morph_raises(have_morph):
    """Ensure that a build without morph rejects the interface of morph and names the build option."""

    if have_morph:
        pytest.skip("NEST was built with morph.")

    nest.ResetKernel()
    for params in ({"threshold_delay": 1.0}, {"comm_local_ranks": [0]}, {"buffer_size_spike_data_long_delay": 4}):
        with pytest.raises(nest.NESTError, match="-Dwith-morph=ON"):
            nest.SetKernelStatus(params)
    for attr in ("threshold_delay", "comm_local_ranks", "buffer_size_spike_data_long_delay"):
        assert attr not in nest.GetKernelStatus()
        with pytest.raises(NotImplementedError, match="-Dwith-morph=ON"):
            getattr(nest, attr)

    n = nest.Create("iaf_psc_alpha", 2)
    with pytest.raises(nest.NESTError, match="-Dwith-morph=ON"):
        nest.Connect(n, n, {"rule": "all_to_all", "long_range": True})
    nest.Connect(n, n, {"rule": "all_to_all", "long_range": False})
    # num_connections counts the connections of the local targets
    assert nest.num_connections == 2 * sum(rank_of(i) == nest.Rank() for i in n.tolist())


# ----------------------------------------------------------------------------------------------------------------
# Kernel parameters


@pytest.mark.skipif_missing_morph
def test_defaults():
    """Ensure that morph is off by default and that the local group consists of the own process."""

    nest.ResetKernel()
    assert np.isinf(nest.threshold_delay)
    assert list(nest.comm_local_ranks) == [nest.Rank()]


@pytest.mark.skipif_missing_morph
@pytest.mark.parametrize("value", [0.0, -1.0])
def test_threshold_delay_must_be_positive(value):
    """Ensure that threshold_delay must be positive."""

    nest.ResetKernel()
    with pytest.raises(nest.NESTError, match="threshold_delay > 0"):
        nest.SetKernelStatus({"threshold_delay": value})


@pytest.mark.skipif_missing_morph
def test_threshold_delay_set_and_switch_off():
    """Ensure that threshold_delay is stored and that infinity switches morph off."""

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    assert nest.threshold_delay == THRESHOLD
    nest.threshold_delay = float("inf")
    assert np.isinf(nest.threshold_delay)


@pytest.mark.skipif_missing_morph
def test_threshold_delay_multiple_of_min_delay():
    """Ensure that Simulate requires threshold_delay to be a multiple of min_delay."""

    nest.ResetKernel()
    nest.SetKernelStatus({"min_delay": 0.2, "max_delay": 2.0, "threshold_delay": 0.5})
    nest.Create("iaf_psc_alpha", 2)
    with pytest.raises(nest.NESTError, match="multiple of min_delay"):
        nest.Simulate(1.0)


@pytest.mark.skipif_missing_morph
def test_threshold_delay_not_after_connections():
    """Ensure that threshold_delay cannot change once connections exist."""

    nest.ResetKernel()
    n = nest.Create("iaf_psc_alpha", 4 * nest.total_num_virtual_procs)
    nest.Connect(n, n, "one_to_one")
    with pytest.raises(nest.NESTError, match="before connections"):
        nest.threshold_delay = THRESHOLD


@pytest.mark.skipif_missing_morph
def test_threshold_delay_not_after_simulate():
    """Ensure that threshold_delay cannot change after the simulation started; the same value is accepted."""

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    nest.Create("iaf_psc_alpha", 2)
    nest.Simulate(2.0)
    nest.threshold_delay = THRESHOLD
    with pytest.raises(nest.NESTError, match="after the simulation"):
        nest.threshold_delay = 2.0


@pytest.mark.skipif_missing_morph
def test_comm_local_ranks_validation():
    """Ensure that comm_local_ranks must be a valid group containing the own rank."""

    nest.ResetKernel()
    with pytest.raises(nest.NESTError, match="comm_local_ranks"):
        nest.SetKernelStatus({"comm_local_ranks": []})
    with pytest.raises(nest.NESTError, match="between 0 and"):
        nest.SetKernelStatus({"comm_local_ranks": [num_procs()]})
    with pytest.raises(nest.NESTError, match="twice"):
        nest.SetKernelStatus({"comm_local_ranks": [nest.Rank(), nest.Rank()]})
    if num_procs() > 1:
        with pytest.raises(nest.NESTError, match="own rank"):
            nest.SetKernelStatus({"comm_local_ranks": [(nest.Rank() + 1) % num_procs()]})


@pytest.mark.skipif_missing_morph
def test_reset_kernel_restores_defaults():
    """Ensure that ResetKernel switches morph off and restores the local group of the own process."""

    nest.ResetKernel()
    nest.SetKernelStatus({"threshold_delay": THRESHOLD, "comm_local_ranks": list(range(num_procs()))})
    assert list(nest.comm_local_ranks) == list(range(num_procs()))
    nest.ResetKernel()
    assert np.isinf(nest.threshold_delay)
    assert list(nest.comm_local_ranks) == [nest.Rank()]


@pytest.mark.skipif_missing_morph
def test_comm_local_ranks_not_after_connections():
    """Ensure that comm_local_ranks cannot change once connections exist."""

    if num_procs() == 1:
        pytest.skip("needs more than one process")
    nest.ResetKernel()
    n = nest.Create("iaf_psc_alpha", 4 * nest.total_num_virtual_procs)
    nest.Connect(n, n, "one_to_one")
    with pytest.raises(nest.NESTError, match="before connections"):
        nest.SetKernelStatus({"comm_local_ranks": list(range(num_procs()))})


# ----------------------------------------------------------------------------------------------------------------
# Connections


@pytest.mark.skipif_missing_morph
def test_long_range_requires_threshold_delay():
    """Ensure that long_range connections require threshold_delay."""

    nest.ResetKernel()
    n = nest.Create("iaf_psc_alpha", 4)
    with pytest.raises(nest.NESTError, match="threshold_delay"):
        nest.Connect(n, n, {"rule": "all_to_all", "long_range": True}, {"delay": 2.0})


@pytest.mark.skipif_missing_morph
def test_long_range_delay_at_least_threshold():
    """Ensure that long_range connections need a delay of at least threshold_delay; equality is allowed."""

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    n = nest.Create("iaf_psc_alpha", 4)
    with pytest.raises(nest.NESTError, match="at least threshold_delay"):
        nest.Connect(n, n, {"rule": "all_to_all", "long_range": True}, {"delay": THRESHOLD / 2})
    nest.Connect(n, n, {"rule": "all_to_all", "long_range": True}, {"delay": THRESHOLD})


@pytest.mark.skipif_missing_morph
def test_long_range_not_for_devices():
    """Ensure that connections from or to devices cannot be long_range."""

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    n = nest.Create("iaf_psc_alpha", 2 * nest.total_num_virtual_procs)
    pg = nest.Create("poisson_generator")
    sr = nest.Create("spike_recorder")
    with pytest.raises(nest.NESTError, match="devices"):
        nest.Connect(pg, n, {"rule": "all_to_all", "long_range": True}, {"delay": 2.0})
    with pytest.raises(nest.NESTError, match="devices"):
        nest.Connect(n, sr, {"rule": "all_to_all", "long_range": True}, {"delay": 2.0})


@pytest.mark.skipif_missing_morph
def test_long_range_not_for_secondary_connections():
    """Ensure that connections that do not transmit spikes (gap junctions) cannot be long_range."""

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    g = nest.Create("hh_psc_alpha_gap", 2 * nest.total_num_virtual_procs)
    with pytest.raises(nest.NESTError, match="transmit spikes"):
        nest.Connect(
            g[0::2],
            g[1::2],
            {"rule": "one_to_one", "make_symmetric": True, "long_range": True},
            {"synapse_model": "gap_junction", "weight": 0.5},
        )


@pytest.mark.skipif_missing_morph
def test_long_range_not_with_arrays():
    """Ensure that Connect with arrays of node IDs rejects long_range."""

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    nest.Create("iaf_psc_alpha", 2)
    with pytest.raises(ValueError, match="long_range"):
        nest.Connect(
            np.array([1, 2]),
            np.array([2, 1]),
            {"rule": "one_to_one", "long_range": True},
            {"synapse_model": "static_synapse", "delay": 2.0},
        )


@pytest.mark.skipif_missing_morph
def test_short_connection_across_local_groups_raises():
    """Ensure that a short-path connection between local groups raises an error."""

    if num_procs() == 1:
        pytest.skip("needs more than one process")
    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    n = nest.Create("iaf_psc_alpha", 4 * num_procs())
    with pytest.raises(nest.NESTError, match="crosses the boundary"):
        nest.Connect(n, n, "all_to_all", {"delay": 0.5})


@pytest.mark.skipif_missing_morph
def test_connection_status_on_both_paths():
    """
    Ensure that get and set address the right connection if the same pair is connected on both paths.

    The neurons lie on one process, so that the short connections stay within the local group.
    """

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    n = nest.Create("iaf_psc_alpha", 4 * nest.total_num_virtual_procs)
    ids = n.tolist()
    same_rank = [i for i in ids if rank_of(i) == rank_of(ids[0])]
    pre, post = nest.NodeCollection(same_rank[:2]), nest.NodeCollection(same_rank[2:4])
    nest.Connect(pre, post, "one_to_one", {"weight": 1.0, "delay": 0.5})
    nest.Connect(pre, post, {"rule": "one_to_one", "long_range": True}, {"weight": 2.0, "delay": 2.0})

    conns = nest.GetConnections(pre, post)
    if len(conns) == 0:
        return  # the pair lies on another process
    status = conns.get(["delay", "weight", "long_range"])
    assert sorted(status["delay"]) == [0.5, 0.5, 2.0, 2.0]
    for d, w, is_long in zip(status["delay"], status["weight"], status["long_range"]):
        assert (d, w) == ((2.0, 2.0) if is_long else (0.5, 1.0))

    for c, is_long in zip(conns, status["long_range"]):
        if is_long:
            c.set({"weight": 5.0})
    status = conns.get(["weight", "long_range"])
    for w, is_long in zip(status["weight"], status["long_range"]):
        assert w == (5.0 if is_long else 1.0)


@pytest.mark.skipif_missing_morph
def test_disconnect_long_connections():
    """Ensure that Disconnect with a SynapseCollection removes the long connections only; simulation continues."""

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    n = nest.Create("iaf_psc_alpha", 4 * nest.total_num_virtual_procs)
    ids = n.tolist()
    same_rank = [i for i in ids if rank_of(i) == rank_of(ids[0])]
    pre, post = nest.NodeCollection(same_rank[:2]), nest.NodeCollection(same_rank[2:4])
    nest.Connect(pre, post, "one_to_one", {"delay": 0.5})
    nest.Connect(pre, post, {"rule": "one_to_one", "long_range": True}, {"delay": 2.0})
    nest.Simulate(10.0)

    conns = nest.GetConnections(pre, post)
    n_before = nest.GetKernelStatus("num_connections")
    if len(conns) > 0:
        for c, is_long in zip(conns, conns.get("long_range")):
            if is_long:
                nest.Disconnect(c)
        assert sorted(nest.GetConnections(pre, post).get("delay")) == [0.5, 0.5]
        assert nest.GetKernelStatus("num_connections") == n_before - 2
    nest.Simulate(10.0)


@pytest.mark.skipif_missing_morph
def test_disconnect_without_long_range_removes_long_connections():
    """Ensure that Disconnect without long_range removes connections that exist only on the long path."""

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    n = nest.Create("iaf_psc_alpha", 4 * nest.total_num_virtual_procs)
    ids = n.tolist()
    same_rank = [i for i in ids if rank_of(i) == rank_of(ids[0])]
    pre, post = nest.NodeCollection(same_rank[:2]), nest.NodeCollection(same_rank[2:4])
    nest.Connect(pre, post, {"rule": "one_to_one", "long_range": True}, {"delay": THRESHOLD})
    nest.Disconnect(pre, post, conn_spec="one_to_one")
    assert len(nest.GetConnections(pre, post)) == 0
    nest.Simulate(10.0)


@pytest.mark.skipif_missing_morph
def test_sender_of_long_range_spikes():
    """
    Ensure that the sender of a spike delivered on the long path is the source of the long connection.

    The weight recorder reads the sender from the source table; for the long path, the lcid indexes the source
    table of the long path.
    """

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    wr = nest.Create("weight_recorder")
    nest.CopyModel("static_synapse", "static_synapse_wr", {"weight_recorder": wr})
    n = nest.Create("iaf_psc_alpha", 4 * nest.total_num_virtual_procs, {"I_e": 500.0})
    pre, post = n[: len(n) // 2], n[len(n) // 2 :]
    # short connections in the opposite direction occupy the same lcids in the short source table
    nest.Connect(post, pre, "one_to_one", {"synapse_model": "static_synapse_wr", "weight": 0.0, "delay": 0.5})
    nest.Connect(pre, post, {"rule": "one_to_one", "long_range": True}, {"synapse_model": "static_synapse_wr"})
    nest.Simulate(50.0)
    events = wr.get("events")
    long_senders = set(events["senders"][np.array(events["targets"]) > pre.tolist()[-1]])
    assert len(long_senders) > 0
    assert long_senders <= set(pre.tolist())


@pytest.mark.skipif_missing_morph
def test_connections_from_and_to_devices():
    """Ensure that connections from and to devices can be queried and changed while morph is active."""

    nest.ResetKernel()
    nest.threshold_delay = THRESHOLD
    n = nest.Create("iaf_psc_alpha", 2 * nest.total_num_virtual_procs)
    pg = nest.Create("poisson_generator", params={"rate": 1000.0})
    sr = nest.Create("spike_recorder")
    nest.Connect(pg, n)
    nest.Connect(n, sr)
    from_pg = nest.GetConnections(pg)
    if len(from_pg) > 0:
        assert all(np.atleast_1d(from_pg.get("weight")) == 1.0)
        assert not any(np.atleast_1d(from_pg.get("long_range")))
        from_pg.set({"weight": 3.0})
        assert all(np.atleast_1d(from_pg.get("weight")) == 3.0)
    to_sr = nest.GetConnections(target=sr)
    if len(to_sr) > 0:
        assert all(np.atleast_1d(to_sr.get("delay")) > 0)
    nest.Simulate(10.0)


# ----------------------------------------------------------------------------------------------------------------
# Spike trains: morph versus conventional exchange


def simulate_areas(morph, inter_delay_low=THRESHOLD + 0.1, group_size=1, inter=True, T=300.0):
    """
    Simulate one area per process and return the local spikes.

    Area i is the strided slice all[i::n_proc] of the neurons, so that it lies on process i (round-robin
    placement). Inter-area connections are long_range with morph. With ``group_size`` > 1 the local groups
    consist of ``group_size`` consecutive processes.
    """

    n_proc, rank = num_procs(), nest.Rank()
    params = {}
    if morph:
        params["threshold_delay"] = THRESHOLD
        if group_size > 1:
            first = rank // group_size * group_size
            params["comm_local_ranks"] = list(range(first, min(first + group_size, n_proc)))
    reset(**params)
    size = 120
    nodes = nest.Create("iaf_psc_exp", size * n_proc)
    nodes.V_m = nest.random.uniform(-70.0, -56.0)
    ids = nodes.tolist()
    areas = [nest.NodeCollection(ids[i::n_proc]) for i in range(n_proc)]
    pg = nest.Create("poisson_generator", params={"rate": 4000.0})
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    long_spec = {"long_range": True} if morph else {}
    for a in areas:
        n_e = int(0.8 * size)
        delay = nest.random.uniform(0.1, 1.5)
        nest.Connect(a[:n_e], a, {"rule": "fixed_indegree", "indegree": 24}, {"weight": 30.0, "delay": delay})
        nest.Connect(a[n_e:], a, {"rule": "fixed_indegree", "indegree": 6}, {"weight": -120.0, "delay": delay})
        nest.Connect(pg, a, "all_to_all", {"weight": 40.0, "delay": 0.1})
        nest.Connect(a, sr)
    if inter:
        for i, a in enumerate(areas):
            for j, b in enumerate(areas):
                if i != j:
                    nest.Connect(
                        b[: int(0.8 * size)],
                        a,
                        dict({"rule": "fixed_indegree", "indegree": 10}, **long_spec),
                        {"weight": 30.0, "delay": nest.random.uniform(inter_delay_low, inter_delay_low + 2.0)},
                    )
    nest.Simulate(T)
    return recorded_spikes(sr)


@pytest.mark.skipif_missing_morph
@pytest.mark.parametrize("inter_delay_low", [THRESHOLD + 0.1, THRESHOLD])
def test_morph_spikes_equal_conventional(inter_delay_low):
    """
    Ensure that morph gives the spikes of the conventional exchange, also with delays equal to the threshold.

    With ``inter_delay_low`` equal to the threshold, about 5 % of the inter-area delays equal the threshold after
    rounding to the resolution.
    """

    conv = simulate_areas(False, inter_delay_low)
    morph = simulate_areas(True, inter_delay_low)
    assert len(conv) > 0
    np.testing.assert_array_equal(conv, morph)


@pytest.mark.skipif_missing_morph
def test_morph_then_conventional_in_one_process():
    """
    Ensure that a conventional simulation after a simulation with morph in the same process works.

    The number of processes of the short-path exchange changes from the local group to all processes; the MPI
    buffer of the short path must be partitioned anew (regression: out-of-range access in the send buffer).
    """

    morph = simulate_areas(True)
    conv = simulate_areas(False)
    np.testing.assert_array_equal(conv, morph)


@pytest.mark.skipif_missing_morph
def test_morph_spikes_equal_conventional_local_groups_of_two():
    """Ensure that morph with local groups of two processes gives the spikes of the conventional exchange."""

    if num_procs() < 4:
        pytest.skip("needs at least four processes")
    conv = simulate_areas(False)
    morph = simulate_areas(True, group_size=2)
    np.testing.assert_array_equal(conv, morph)


@pytest.mark.skipif_missing_morph
def test_comparison_detects_long_range_spikes():
    """Ensure that the spikes differ without inter-area connections, so that the comparison sees long-range spikes."""

    if num_procs() == 1:
        pytest.skip("needs more than one process")
    with_inter = simulate_areas(True)
    without_inter = simulate_areas(True, inter=False)
    assert not np.array_equal(with_inter, without_inter)


def simulate_spanning_groups(morph, model, T=300.0):
    """
    Simulate areas that span both processes of a local group of two processes and return the local spikes.

    Short-path connections connect the two processes of a group; with morph, connections between areas are
    long_range. Requires an even number of processes.
    """

    n_proc, rank = num_procs(), nest.Rank()
    params = {}
    if morph:
        first = rank // 2 * 2
        params = {"threshold_delay": THRESHOLD, "comm_local_ranks": [first, first + 1]}
    reset(**params)
    nodes = nest.Create(model, 100 * n_proc)
    nodes.V_m = nest.random.uniform(-70.0, -56.0)
    ids = nodes.tolist()
    areas = [nest.NodeCollection([i for i in ids if rank_of(i) // 2 == a]) for a in range(n_proc // 2)]
    pg = nest.Create("poisson_generator", params={"rate": 4000.0})
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    long_spec = {"long_range": True} if morph else {}
    for a in areas:
        n_e = int(0.8 * len(a))
        delay = nest.random.uniform(0.1, 1.5)
        nest.Connect(a[:n_e], a, {"rule": "fixed_indegree", "indegree": 24}, {"weight": 30.0, "delay": delay})
        nest.Connect(a[n_e:], a, {"rule": "fixed_indegree", "indegree": 6}, {"weight": -120.0, "delay": delay})
        nest.Connect(pg, a, "all_to_all", {"weight": 40.0, "delay": 0.1})
        nest.Connect(a, sr)
    for i, a in enumerate(areas):
        for j, b in enumerate(areas):
            if i != j:
                nest.Connect(
                    b[: int(0.8 * len(b))],
                    a,
                    dict({"rule": "fixed_indegree", "indegree": 10}, **long_spec),
                    {"weight": 30.0, "delay": nest.random.uniform(THRESHOLD, 3.0)},
                )
    nest.Simulate(T)
    return recorded_spikes(sr)


@pytest.mark.skipif_missing_morph
@pytest.mark.parametrize("model", ["iaf_psc_exp", "iaf_psc_exp_ps"])
def test_morph_spikes_equal_conventional_groups_spanning_processes(model):
    """
    Ensure that morph gives the conventional spikes if areas span the processes of their local group.

    The model ``iaf_psc_exp_ps`` switches on off-grid spiking.
    """

    if num_procs() < 2 or num_procs() % 2 != 0:
        pytest.skip("needs an even number of processes")
    conv = simulate_spanning_groups(False, model)
    morph = simulate_spanning_groups(True, model)
    assert len(conv) > 0
    np.testing.assert_array_equal(conv, morph)


def simulate_gap_junctions(morph, T=200.0):
    """Simulate neurons coupled by gap junctions between processes and return the local spikes."""

    reset(**({"threshold_delay": THRESHOLD} if morph else {}))
    n = nest.Create("hh_psc_alpha_gap", 20 * num_procs())
    n.I_e = nest.random.uniform(100.0, 300.0)
    # pairs of neighboring node IDs lie on different processes (round-robin placement)
    nest.Connect(
        n[0::2],
        n[1::2],
        {"rule": "one_to_one", "make_symmetric": True},
        {"synapse_model": "gap_junction", "weight": 5.0},
    )
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    nest.Connect(n, sr)
    nest.Simulate(T)
    return recorded_spikes(sr)


@pytest.mark.skipif_missing_morph
def test_morph_gap_junctions_equal_conventional():
    """Ensure that gap junctions between processes give the same spikes with morph active."""

    np.testing.assert_array_equal(simulate_gap_junctions(False), simulate_gap_junctions(True))


# ----------------------------------------------------------------------------------------------------------------
# Structural plasticity


def simulate_structural_plasticity(morph, T=500.0):
    """Simulate a network with structural plasticity and return the local spikes and the number of connections."""

    params = {"threshold_delay": THRESHOLD, "comm_local_ranks": list(range(num_procs()))} if morph else {}
    reset(threads=1, **params)  # structural plasticity requires a single thread
    nest.structural_plasticity_update_interval = 100.0
    nest.CopyModel("static_synapse", "synapse_sp", {"weight": 20.0, "delay": 1.0})
    nest.structural_plasticity_synapses = {
        "synapse_sp": {
            "synapse_model": "synapse_sp",
            "post_synaptic_element": "Den_ex",
            "pre_synaptic_element": "Axon_ex",
        }
    }
    nest.EnableStructuralPlasticity()
    growth = {"growth_curve": "linear", "growth_rate": 0.02, "eps": 0.5, "continuous": False}
    n = nest.Create(
        "iaf_psc_alpha", 40 * num_procs(), {"synaptic_elements": {"Den_ex": growth, "Axon_ex": growth}, "I_e": 400.0}
    )
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    nest.Connect(n, sr)
    nest.Simulate(T)
    return recorded_spikes(sr), nest.GetKernelStatus("num_connections")


@pytest.mark.skipif_missing_morph
def test_structural_plasticity_with_morph():
    """
    Ensure that structural plasticity gives the same spikes and connections with morph active.

    New connections are created on the short path. With a local group of all processes they never cross a
    group boundary.
    """

    if num_procs() > 2:
        # With four processes, a simulation with structural plasticity crashes in deliver_events_() if another
        # simulation ran before in the same process, also without morph (NEST main 579a024).
        pytest.skip("two simulations with structural plasticity in one process crash with more than two processes")
    conv, n_conv = simulate_structural_plasticity(False)
    morph, n_morph = simulate_structural_plasticity(True)
    assert len(conv) > 0
    assert n_conv > 40  # structural plasticity created connections (40 local connections to the recorder)
    assert n_conv == n_morph
    np.testing.assert_array_equal(conv, morph)


@pytest.mark.skipif_missing_morph
def test_structural_plasticity_requires_local_group_of_all_processes():
    """Ensure that structural plasticity with morph and local groups smaller than all processes raises an error."""

    if num_procs() == 1:
        pytest.skip("needs more than one process")
    reset(threads=1, threshold_delay=THRESHOLD)
    nest.CopyModel("static_synapse", "synapse_sp", {"weight": 20.0, "delay": 1.0})
    nest.structural_plasticity_synapses = {
        "synapse_sp": {
            "synapse_model": "synapse_sp",
            "post_synaptic_element": "Den_ex",
            "pre_synaptic_element": "Axon_ex",
        }
    }
    nest.EnableStructuralPlasticity()
    nest.Create("iaf_psc_alpha", 4 * num_procs())
    with pytest.raises(nest.NESTError, match="structural plasticity"):
        nest.Simulate(10.0)
