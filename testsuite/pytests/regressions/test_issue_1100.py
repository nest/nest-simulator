# -*- coding: utf-8 -*-
#
# test_issue_1100.py
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
Regression test for Issue #1100 (GitHub).

The test checks whether the `local_spike_counter` takes multiplicity into
account when counting spikes.
"""

import nest


def test_local_spike_counter_with_spike_multiplicity():
    """
    Test that `local_spike_counter` counts spike multiplicities correctly.

    The test creates a parrot neuron, which receives one spike with multiplicity
    two from a spike generator. The parrot neuron should then emit a spike with
    multiplicity two, which should be counted as two spikes by the local spike
    counter.
    """

    parrot = nest.Create("parrot_neuron")
    sgen = nest.Create("spike_generator", params={"spike_times": [1.0], "spike_multiplicities": [2]})
    srec = nest.Create("spike_recorder")

    nest.Connect(sgen, parrot)
    nest.Connect(parrot, srec)

    nest.Simulate(10.0)

    # Ensure that spikes were delivered
    assert srec.n_events == 2

    # Check that local_spike_counter takes multiplicity into account
    assert nest.GetKernelStatus("local_spike_counter") == 2
