# -*- coding: utf-8 -*-
#
# test_ticket_85.py
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

"""
Regression test for Ticket #85.

Test ported from SLI regression test.
Ensure precise neuron models interact correctly with plastic synapses.

Author: Hans Ekkehard Plesser, 2015-11-23
"""


PRECISE_MODELS = ("iaf_psc_alpha_ps", "iaf_psc_delta_ps", "iaf_psc_exp_ps", "parrot_neuron_ps")


@pytest.mark.parametrize("model", PRECISE_MODELS)
def test_ticket_85_can_create_plastic_connections(model):
    """
    Ensure stdp_synapse connections can be created for precise neuron models.
    """

    nest.ResetKernel()

    pre = nest.Create(model)
    post = nest.Create(model)

    nest.Connect(pre, post, conn_spec={"rule": "one_to_one"}, syn_spec="stdp_synapse")


@pytest.mark.parametrize("model", PRECISE_MODELS)
def test_ticket_85_archiving_node_exposes_tau_minus(model):
    """
    Ensure ArchivingNode exposes tau_minus via GetStatus for precise models.
    """

    nest.ResetKernel()
    neuron = nest.Create(model)
    status = nest.GetStatus(neuron)[0]

    assert "tau_minus" in status


@pytest.mark.parametrize("model", PRECISE_MODELS)
def test_ticket_85_archiving_node_tracks_last_spike(model):
    """
    Ensure last spike time tracked by ArchivingNode matches recorded spike time.
    """

    nest.ResetKernel()

    spike_generator = nest.Create("spike_generator", params={"spike_times": [10.0]})
    neuron = nest.Create(model)
    recorder = nest.Create("spike_recorder")

    nest.Connect(spike_generator, neuron, syn_spec={"weight": 10000.0, "delay": 1.0})
    nest.Connect(neuron, recorder)

    nest.Simulate(20.0)

    events = nest.GetStatus(recorder, "events")[0]
    last_recorded_time = events["times"][-1]
    last_spike_time = neuron.get("t_spike")

    assert last_recorded_time == last_spike_time


def test_ticket_85_plasticity_changes_weight():
    """
    Ensure stdp_synapse induces a weight change for parrot_neuron_ps connections.
    """

    nest.ResetKernel()

    generator = nest.Create("poisson_generator_ps", params={"rate": 100.0})
    source = nest.Create("parrot_neuron_ps")
    target = nest.Create("parrot_neuron_ps")

    initial_weight = 1000.0

    nest.Connect(generator, source)
    nest.Connect(
        source,
        target,
        syn_spec={"synapse_model": "stdp_synapse", "weight": initial_weight, "delay": 1.0},
    )

    nest.Simulate(100.0)

    connections = nest.GetConnections(source=source)
    final_weight = nest.GetStatus(connections, "weight")[0]

    assert final_weight != pytest.approx(initial_weight)
