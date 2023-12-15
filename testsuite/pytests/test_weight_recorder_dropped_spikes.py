# -*- coding: utf-8 -*-
#
# test_weight_recorder_dropped_spikes.py
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
Test that weight recorded records correctly also for synapses which drop spikes.
"""


import nest
import numpy as np


def test_weight_recorder_with_bernoulli():
    """
    Confirm that dropped spikes are not recorded and all recorded spikes have individual target.
    """

    # Choose parameters that yield an astromomically small probability
    # of n_targets spikes to be transmitted
    n_targets = 100
    p_bernoulli = 0.2

    sg = nest.Create("spike_generator", params={"spike_times": [1.0]})
    sender = nest.Create("parrot_neuron")
    targets = nest.Create("parrot_neuron", n=n_targets)
    sr = nest.Create("spike_recorder")
    wr = nest.Create("weight_recorder")

    nest.SetDefaults("bernoulli_synapse", {"p_transmit": p_bernoulli, "weight_recorder": wr})

    nest.Connect(sg, sender)
    nest.Connect(sender, targets, syn_spec={"synapse_model": "bernoulli_synapse"})
    nest.Connect(targets, sr)

    nest.Simulate(5)

    # Test logic:
    # 1. sender sends a single spike to each neuron in the targets population
    # 2. The bernoulli_synapse transmits only some of these spikes
    # 3. A subset of the neurons in the targets population thus emit a spike,
    #    namely those who receive an input spike via the bernoulli_synapse
    # 4. Each output spike from the targets population recorded by the spike recorder
    #    (with the targets neuron as "sender") must therefore have exactly one corresponding
    #    entry in weight_recorder data with that neuron as "target".

    # Ensure we have some spikes but fewer than n_targets, otherwise test would be meaningless
    sr_targets = sr.get("events", "senders")
    assert 0 < len(sr_targets) < n_targets

    # Weight recorder shall have picked up the same spikes as the spike recorder
    wr_targets = wr.get("events", "targets")
    np.testing.assert_array_equal(wr_targets, sr_targets, strict=True)
