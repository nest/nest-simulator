# -*- coding: utf-8 -*-
#
# test_spike_train_injector_mpi.py
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
Test ``spike_train_injector`` model in MPI parallel simulations.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


@pytest.mark.parametrize("times, multiplicities, precise", [[[1, 2], [3, 5], False], [[1.007, 1.993], [2, 7], True]])
def test_spike_train_injector_multiplicities(times, multiplicities, precise):
    """
    Test spike train multiplicity with spike_train_injector model.

    This test verifies the behavior of the spike train injector neuron in
    MPI parallelsimulations by using parrot neuron's spike repetition
    properties.
    """

    inj = nest.Create(
        "spike_train_injector",
        params={"spike_times": times, "spike_multiplicities": multiplicities, "precise_times": precise},
    )

    parrots = nest.Create("parrot_neuron" if not precise else "parrot_neuron_ps", 4)
    srec = nest.Create("spike_recorder", 4)

    delay = 1.3
    nest.Connect(inj, parrots, syn_spec={"delay": delay})
    nest.Connect(parrots, srec, "one_to_one")

    nest.Simulate(4.0)

    expected = []
    for t, m in zip(times, multiplicities):
        expected.extend([t + delay] * m)

    for p, ev in zip(parrots, srec.events):
        if p.local:
            assert ev["senders"] == pytest.approx(p.global_id)
            assert ev["times"] == pytest.approx(expected)
