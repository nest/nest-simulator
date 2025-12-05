# -*- coding: utf-8 -*-
#
# test_localonly.py
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

import nest
import pytest


@pytest.fixture
def network():
    nest.ResetKernel()
    nodes = nest.Create("parrot_neuron", 13) + nest.Create("spike_generator", 5)
    lnodes = nest.GetNodes(local_only=True)
    yield nodes, lnodes


def test_all_are_local(network):
    """
    Test that local_only node collection works correctly.
    """

    _, lnodes = network
    assert all(lnodes.local)


def test_all_locals_included(network):
    """
    Test that all local nodes are included in GetNodes() result.
    """

    nodes, lnodes = network
    nl_ids = set(n.global_id for n in nodes if n.local)
    ln_ids = set(n.global_id for n in lnodes)

    assert nl_ids == ln_ids


def test_consistency(network):
    """
    Test that we get the same results as with GetLocalNodeCollection.
    """

    nodes, lnodes = network
    lc = nest.GetLocalNodeCollection(nodes)
    assert set(n.global_id for n in lc) == set(n.global_id for n in lnodes)
