# -*- coding: utf-8 -*-
#
# test_issue_3108.py
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


import itertools

import nest
import pytest

"""
Test in this file were developed for regressions under three MPI processes.

They should be run with 1, 3 and 4 MPI processes to ensure all passes under various settings.

The spatial tests test that NodeCollection::rank_local_begin() works.
The connect tests test that NodeCollection::thread_local_begin() works.
"""

# Needs up to 16 cores when run on 4 MPI ranks.
# Experiences severe slowdown on Github runners under Linux with MPI and OpenMP
pytestmark = pytest.mark.requires_many_cores

if nest.ll_api.sli_func("is_threaded"):
    num_threads = [1, 2, 3, 4]
else:
    num_threads = [1]


@pytest.mark.parametrize("n_threads", num_threads)
@pytest.mark.parametrize(
    "transform",
    [
        lambda nc: nc[::3],
        lambda nc: nc[1:],
        lambda nc: nc[:5] + nc[8:],
        lambda nc: (nc[:5] + nc[8:])[-1:],
        lambda nc: (nc[:5] + nc[8:])[::2],
        lambda nc: (nc[:5] + nc[8:])[::3],
        lambda nc: (nc[:5] + nc[9:])[::2],
        lambda nc: (nc[:5] + nc[9:])[::3],
        lambda nc: (nc[:5] + nc[8:])[7:],
        lambda nc: (nc[:5] + nc[8:])[7::3],
    ],
)
def test_slice_node_collections(n_threads, transform):
    nest.ResetKernel()
    nest.local_num_threads = n_threads

    n_orig = nest.Create("parrot_neuron", 128)
    n_orig_gids = n_orig._to_array()["All"]

    n_sliced = transform(n_orig)
    n_pyslice_gids = transform(n_orig_gids)

    n_pyslice_gids_on_rank = sorted(
        n.global_id for n in nest.NodeCollection(n_pyslice_gids) if n.vp % nest.NumProcesses() == nest.Rank()
    )

    assert n_sliced._to_array()["All"] == n_pyslice_gids

    n_sliced_gids_by_rank = n_sliced._to_array("rank")
    assert len(n_sliced_gids_by_rank) == 1
    assert sorted(next(iter(n_sliced_gids_by_rank.values()))) == n_pyslice_gids_on_rank

    n_sliced_gids_by_thread = n_sliced._to_array("thread")
    assert sorted(itertools.chain(*n_sliced_gids_by_thread.values())) == n_pyslice_gids_on_rank

    for thread, tgids in n_sliced_gids_by_thread.items():
        for node in nest.NodeCollection(tgids):
            assert node.thread == thread


@pytest.mark.parametrize("n_threads", num_threads)
@pytest.mark.parametrize("stride", [1, 2, 3, 7, 12])
def test_slice_single_element_parts(n_threads, stride):
    """Same test as above, but on NC with single-element parts to check stepping over parts"""

    nest.ResetKernel()
    nest.local_num_threads = n_threads

    # Use different models to get single-element node collections
    mod_names = [f"pn{n:02d}" for n in range(91)]
    for mod in mod_names:
        nest.CopyModel("parrot_neuron", mod)

    n_orig = sum(nest.Create(mod) for mod in mod_names)
    n_orig_gids = n_orig._to_array()["All"]

    n_sliced = n_orig[::stride]
    n_pyslice_gids = n_orig_gids[::stride]

    n_pyslice_gids_on_rank = sorted(
        (n.global_id for n in nest.NodeCollection(n_pyslice_gids) if n.vp % nest.NumProcesses() == nest.Rank())
    )

    assert n_sliced._to_array()["All"] == n_pyslice_gids

    n_sliced_gids_by_rank = n_sliced._to_array("rank")
    assert len(n_sliced_gids_by_rank) == 1
    assert sorted(next(iter(n_sliced_gids_by_rank.values()))) == n_pyslice_gids_on_rank

    n_sliced_gids_by_thread = n_sliced._to_array("thread")
    assert sorted(itertools.chain(*n_sliced_gids_by_thread.values())) == n_pyslice_gids_on_rank

    for thread, tgids in n_sliced_gids_by_thread.items():
        for node in nest.NodeCollection(tgids):
            assert node.thread == thread


@pytest.mark.parametrize("n_threads", num_threads)
@pytest.mark.parametrize("stride", [1, 2, 3, 7, 12])
def test_multi_parts_slicing(n_threads, stride):
    """Same test as above, but on NC from parts of different size with different gaps"""

    nest.ResetKernel()
    nest.local_num_threads = n_threads

    n_orig = nest.Create("parrot_neuron", 97)
    n_orig_gids = n_orig._to_array()["All"]

    n_sliced = n_orig[:5] + n_orig[6:7] + n_orig[8:18] + n_orig[23:34] + n_orig[41:]
    n_pyslice_gids = n_orig_gids[:5] + n_orig_gids[6:7] + n_orig_gids[8:18] + n_orig_gids[23:34] + n_orig_gids[41:]

    n_pyslice_gids_on_rank = sorted(
        (n.global_id for n in nest.NodeCollection(n_pyslice_gids) if n.vp % nest.NumProcesses() == nest.Rank())
    )

    assert n_sliced._to_array()["All"] == n_pyslice_gids

    n_sliced_gids_by_rank = n_sliced._to_array("rank")
    assert len(n_sliced_gids_by_rank) == 1
    assert sorted(next(iter(n_sliced_gids_by_rank.values()))) == n_pyslice_gids_on_rank

    n_sliced_gids_by_thread = n_sliced._to_array("thread")
    assert sorted(itertools.chain(*n_sliced_gids_by_thread.values())) == n_pyslice_gids_on_rank

    for thread, tgids in n_sliced_gids_by_thread.items():
        for node in nest.NodeCollection(tgids):
            assert node.thread == thread


@pytest.mark.parametrize("n_threads", num_threads)
@pytest.mark.parametrize("start, step", ([[0, 1], [0, 3]] + [[2, n] for n in range(1, 9)]))
def test_get_positions_with_mpi(n_threads, start, step):
    """
    Test that correct positions can be obtained from sliced node collections.

    Two cases above for starting without offset, the remaining with a small offset.
    With the range of step values, combined with 3 and 4 MPI processes, we ensure
    that we have cases where the step is half of or a multiple of the number of
    processes.
    """

    num_neurons = 128

    nest.ResetKernel()
    nest.local_num_threads = n_threads

    # Need floats because NEST returns positions as floats
    node_pos = [(float(x), 0.0) for x in range(num_neurons)]

    layer = nest.Create(
        model="parrot_neuron",
        n=num_neurons,
        positions=nest.spatial.free(pos=node_pos, edge_wrap=False),
    )

    pos = layer[start::step].spatial["positions"]
    node_ranks = [n.vp % nest.NumProcesses() for n in layer]
    assert len(node_ranks) == num_neurons

    # pos is a tuple of tuples, so we need to create a tuple for comparison
    expected_pos = tuple(
        npos for npos, nrk in zip(node_pos[start::step], node_ranks[start::step]) if nrk == nest.Rank()
    )

    assert pos == expected_pos


@pytest.mark.parametrize("n_threads", num_threads)
@pytest.mark.parametrize("pick", range(0, 7))
def test_get_spatial_for_single_element_and_mpi(n_threads, pick):
    """
    Test that spatial information can be collected from a single layer element.

    This was an original minimal reproducer for #3108.
    """

    num_neurons = 7

    nest.ResetKernel()
    nest.local_num_threads = n_threads

    node_pos = [(float(x), 0.0) for x in range(num_neurons)]

    layer = nest.Create(
        model="parrot_neuron",
        n=num_neurons,
        positions=nest.spatial.free(pos=node_pos, edge_wrap=False),
    )

    # We want to retrieve this on all ranks to see that it does not break NEST
    sp = layer[pick].spatial["positions"]

    pick_rank = layer[pick].vp % nest.NumProcesses()
    if pick_rank == nest.Rank():
        assert sp[0] == node_pos[pick]
    else:
        assert len(sp) == 0


@pytest.mark.parametrize("n_threads", num_threads)
@pytest.mark.parametrize("pick", range(0, 5))
def test_connect_with_single_element_slice_and_mpi(n_threads, pick):
    """
    Test that connection with single-element sliced layer is possible on multiple mpi processes.

    This was an original minimal reproducer for #3108.
    """

    num_neurons = 5

    nest.ResetKernel()
    nest.local_num_threads = n_threads

    layer = nest.Create(
        model="parrot_neuron",
        n=num_neurons,
        positions=nest.spatial.free(pos=nest.random.uniform(min=-1, max=1), num_dimensions=2, edge_wrap=False),
    )

    # space-dependent syn_spec passed only to force use of ConnectLayers
    nest.Connect(layer[pick], layer, {"rule": "pairwise_bernoulli", "p": 1.0}, {"weight": nest.spatial.distance})

    local_nodes = tuple(n.global_id for n in layer if n.vp % nest.NumProcesses() == nest.Rank())

    c = nest.GetConnections()
    src = tuple(c.sources())
    tgt = tuple(c.targets())
    assert src == (layer[pick].global_id,) * len(local_nodes)
    assert sorted(tgt) == sorted(local_nodes)


@pytest.mark.parametrize("n_threads", num_threads)
@pytest.mark.parametrize("sstep", [2, 3, 4, 6])
@pytest.mark.parametrize("tstep", [2, 3, 4, 6])
def test_connect_slice_to_slice_and_mpi(n_threads, sstep, tstep):
    """
    Test that connection with stepped source and target layers is possible on multiple mpi processes.

    This was an original minimal reproducer for #3108.
    """

    num_neurons = 128

    nest.ResetKernel()
    nest.local_num_threads = n_threads

    layer = nest.Create(
        model="parrot_neuron",
        n=num_neurons,
        positions=nest.spatial.free(pos=nest.random.uniform(min=-1, max=1), num_dimensions=2, edge_wrap=False),
    )

    # space-dependent syn_spec passed only to force use of ConnectLayers
    nest.Connect(
        layer[::sstep], layer[2::tstep], {"rule": "pairwise_bernoulli", "p": 1.0}, {"weight": nest.spatial.distance}
    )

    local_nodes = tuple(n.global_id for n in layer if n.vp % nest.NumProcesses() == nest.Rank())
    local_targets = set(n.global_id for n in layer[2::tstep] if n.vp % nest.NumProcesses() == nest.Rank())

    c = nest.GetConnections()
    src = tuple(c.sources())
    tgt = tuple(c.targets())

    assert len(c) == len(layer[::sstep]) * len(local_targets)
    assert set(tgt) == local_targets  # all local neurons in layer[2::tstep] must be targets
    if local_targets:
        assert set(src) == set(layer[::sstep].global_id)  # all neurons in layer[::sstep] neurons must be sources
