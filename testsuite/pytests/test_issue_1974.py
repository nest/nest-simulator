# -*- coding: utf-8 -*-
#
# test_issue_1974.py
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
    nrns = nest.Create("parrot_neuron", 2)

    local_nrns = [n.global_id for n in nrns if n.local]

    yield local_nrns, nrns


@pytest.mark.skipif_missing_music
def test_issue_1974_in(network):
    """
    For each of the two parrot neurons, exactly one connection from/to the in/out MUSIC proxy must be created,
    specifically to the proxy on the rank on which the parrot neuron exists. Therefore, the pooled connections across
    ranks must be identical independent of number of ranks.
    """

    local_nrns, nrns = network

    music_in = nest.Create("music_event_in_proxy", 1, {"port_name": "in_spikes"})

    nest.Connect(music_in, nrns)

    in_targets = nest.GetConnections(source=music_in).get("target")
    if isinstance(in_targets, int):
        in_targets = [in_targets]
    assert sorted(in_targets) == local_nrns


@pytest.mark.skipif_missing_music
def test_issue_1974_out(network):
    """
    For each of the two parrot neurons, exactly one connection from/to the in/out MUSIC proxy must be created,
    specifically to the proxy on the rank on which the parrot neuron exists. Therefore, the pooled connections across
    ranks must be identical independent of number of ranks.
    """

    local_nrns, nrns = network

    music_out = nest.Create("music_event_out_proxy", 1, {"port_name": "out_spikes"})

    nest.Connect(nrns, music_out)

    out_targets = nest.GetConnections(target=music_out).get("source")
    if isinstance(out_targets, int):
        out_targets = [out_targets]
    assert sorted(out_targets) == local_nrns
