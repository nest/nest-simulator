# -*- coding: utf-8 -*-
#
# test_ticket_716.py
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


def test_stdp_dopamine_synapse_weight_constant_without_presynaptic_spikes():
    """
    Regression test for Ticket #716.

    Ensure that the weight of an stdp_dopamine_synapse is constant in the absence of presynaptic spiking.
    """
    nest.ResetKernel()
    vt = nest.Create("volume_transmitter")
    nest.SetDefaults("stdp_dopamine_synapse", {"volume_transmitter": vt[0]})

    n_pre = nest.Create("parrot_neuron")  # does not fire
    n_post = nest.Create("parrot_neuron")
    n_dopa = nest.Create("parrot_neuron")

    sg_post = nest.Create("spike_generator", params={"spike_times": [0.5, 1.1, 3.4]})
    sg_dopa = nest.Create("spike_generator", params={"spike_times": [1.4, 2.3, 4.6]})

    nest.Connect(n_pre, n_post, syn_spec="stdp_dopamine_synapse")
    nest.Connect(sg_dopa, n_dopa)
    nest.Connect(n_dopa, vt)
    nest.Connect(sg_post, n_post)

    # Get initial weight
    conns = nest.GetConnections(synapse_model="stdp_dopamine_synapse")
    w0 = conns.get("weight")
    nest.Simulate(10.0)
    w1 = conns.get("weight")

    assert abs(w0 - w1) <= 1e-13, f"Weight changed: w0={w0}, w1={w1}"
