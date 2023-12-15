# -*- coding: utf-8 -*-
#
# test_issue_1212.py
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
Regression test for Issue #1212 (GitHub).

Test that a weight recorder can be used with a probabilistic synapse.
"""

import nest


def test_weight_recorder_with_probabilistic_synapse():
    """
    Ensure that a `weight_recorder` can be used with the probabilistic `bernoulli_synapse`.

    This test ensures that using a weight recorder with a probabilistic synapse
    that drops a spike doesn't result in a segfault.
    """

    sgen = nest.Create("spike_generator")
    src_parrot = nest.Create("parrot_neuron")
    tgt_parrot = nest.Create("parrot_neuron")
    wrec = nest.Create("weight_recorder")

    nest.CopyModel("bernoulli_synapse", "bernoulli_synapse_wrec", {"weight_recorder": wrec})

    nest.Connect(sgen, src_parrot)
    nest.Connect(
        src_parrot,
        tgt_parrot,
        conn_spec={"rule": "one_to_one"},
        syn_spec={
            "synapse_model": "bernoulli_synapse_wrec",
            "p_transmit": 0.0,
            "weight": 1.0,
        },
    )

    nest.Simulate(20.0)
