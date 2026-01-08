# -*- coding: utf-8 -*-
#
# test_ticket_941.py
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


"""This test ensures that GetConnections returns correct results when neurons are connected
with different synapse models."""

import nest
import numpy as np
import pytest


def check_connection(source, n_expected, expected):
    """
    Helper method to check GetConnections results against expected values, where expected is a tuple of (target,
    synapse_model)
    """
    target_expected, syn_type_expected = expected
    conns = nest.GetConnections(source=source)

    assert len(conns) == n_expected

    for node in conns:
        assert node.get("source") == source.tolist()[0]
        assert node.get("target") == target_expected.tolist()[0]
        assert node.get("synapse_model") == syn_type_expected


def test_different_connections():
    """
    Create various connections between parrot populations and check them against GetConnections results
    via helper method.
    """
    nest.ResetKernel()
    nest.set_verbosity("M_ERROR")

    spike_generator = nest.Create("spike_generator", {"spike_times": [1.0]})
    spike_recorder = nest.Create("spike_recorder")

    pn1 = nest.Create("parrot_neuron")
    pn2 = nest.Create("parrot_neuron")

    nest.Connect(spike_generator, pn1, syn_spec="static_synapse", conn_spec={"rule": "one_to_one"})
    nest.Connect(pn2, spike_recorder, syn_spec="static_synapse", conn_spec={"rule": "one_to_one"})
    assert nest.num_connections == 2

    check_connection(spike_generator, 1, (pn1, "static_synapse"))
    check_connection(pn2, 1, (spike_recorder, "static_synapse"))

    nest.Connect(pn1, pn2, syn_spec={"synapse_model": "static_synapse", "delay": 1.0}, conn_spec={"rule": "one_to_one"})
    assert nest.num_connections == 3

    check_connection(pn1, 1, (pn2, "static_synapse"))

    nest.Connect(pn1, pn2, syn_spec={"synapse_model": "static_synapse", "delay": 2.0}, conn_spec={"rule": "one_to_one"})
    assert nest.num_connections == 4

    check_connection(pn1, 2, (pn2, "static_synapse"))

    nest.Connect(
        pn1, pn2, syn_spec={"synapse_model": "static_synapse_hom_w", "delay": 3.0}, conn_spec={"rule": "one_to_one"}
    )
    assert nest.num_connections == 5

    nest.Simulate(10.0)
    spike_recs = spike_recorder.get("events", ["times"])
    assert np.all(
        spike_recs["times"]
        == pytest.approx(
            [
                3.0,
                4.0,
                5.0,
            ]
        )
    )

    synapses = nest.GetConnections(source=pn1, target=pn2).get("synapse_model")
    expected_synapses = ["static_synapse", "static_synapse", "static_synapse_hom_w"]
    assert np.all(np.isin(expected_synapses, synapses))
