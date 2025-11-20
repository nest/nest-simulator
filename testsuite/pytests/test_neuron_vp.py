# -*- coding: utf-8 -*-
#
# test_neuron_vp.py
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
Test that neurons are assigned to the correct ranks and VPs.

This test should be run for 1, 2, and 3 MPI processes.
"""


import nest
import numpy as np
import pytest

pytestmark = pytest.mark.skipif_missing_threads


@pytest.fixture
def nodes():
    nest.total_num_virtual_procs = 6
    yield nest.Create("parrot_neuron", 6)


@pytest.fixture
def expected_vp():
    nest.total_num_virtual_procs = 6
    nodes = nest.Create("parrot_neuron", 6)

    yield np.array(nodes.global_id) % nest.total_num_virtual_procs


@pytest.fixture
def expected_thread(expected_vp):
    yield expected_vp // nest.NumProcesses()


@pytest.fixture
def expected_rank(expected_vp):
    yield expected_vp % nest.NumProcesses()


@pytest.fixture
def expected_local(expected_rank):
    yield expected_rank == nest.Rank()


def test_vp(nodes, expected_vp):
    """Confirm we got correct VPs for neurons."""
    assert np.array_equal(nodes.vp, expected_vp)


def test_local(nodes, expected_local):
    """Confirm that nodes have correct rank-locality."""
    assert np.array_equal(nodes.local, expected_local)


def test_thread(nodes, expected_thread):
    """
    Confirm that nodes are placed on correct threads.

    Here, we have information only for local nodes, all other values in nodes.thread are None.
    """

    threads = np.array(nodes.thread)
    local = nodes.local
    assert np.array_equal(threads[local], expected_thread[local])
