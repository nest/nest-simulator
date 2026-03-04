# -*- coding: utf-8 -*-
#
# test_multiplicity.py
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

import numpy as np
import pytest


@pytest.mark.skipif_missing_threads
def test_multiplicity():
    """
    Confirm that spike multiplicity is handled correctly.

    Creates two parrot neurons and connects the first one to the second one. The
    first parrot neuron receives one spike with multiplicity two from a spike
    generator, which should be communicated as two spikes to the second parrot
    neuron. Each parrot neuron is connected to a spike recorder, which should
    record two spikes.
    """

    import nest

    nest.total_num_virtual_procs = 2

    sg = nest.Create("spike_generator", params={"spike_times": [1.0], "spike_multiplicities": [2]})
    p_source, p_target = nest.Create("parrot_neuron", 2)
    sr_source, sr_target = nest.Create("spike_recorder", 2)

    nest.Connect(sg, p_source)
    nest.Connect(p_source, p_target)
    nest.Connect(p_source, sr_source)
    nest.Connect(p_target, sr_target)

    nest.Simulate(10)

    # When running this on two MPI ranks, we need to make sure we check for spike times
    # only on the rank responsible for the neuron recorded by the recorder.
    assert not p_source.local or np.array_equal(sr_source.events["times"], [2, 2])
    assert not p_target.local or np.array_equal(sr_target.events["times"], [3, 3])
