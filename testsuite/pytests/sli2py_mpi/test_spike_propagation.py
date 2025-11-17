# -*- coding: utf-8 -*-
#
# test_spike_propagation.py
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

import pytest
from mpi_test_wrapper import MPITestAssertEqual


@pytest.mark.skipif_incompatible_mpi
@MPITestAssertEqual([1, 2, 4], debug=False)
def test_spike_propagation():
    """
    Confirm that spikes travel along a chain of neurons independent of how many ranks they are distributed over.

    The test compares data written by spike_recorder to SPIKE_LABEL.
    """

    import nest

    N = 7

    sg = nest.Create("spike_generator", params={"spike_times": [1]})
    nrns = nest.Create("parrot_neuron", N)
    srec = nest.Create(
        "spike_recorder",
        params={
            "label": SPIKE_LABEL.format(nest.num_processes),  # noqa: F821
            "record_to": "ascii",
            "time_in_steps": True,
        },
    )

    nest.Connect(sg, nrns[0])
    nest.Connect(nrns[:-1], nrns[1:], "one_to_one")
    nest.Connect(nrns, srec)

    nest.Simulate(10)
