# -*- coding: utf-8 -*-
#
# test_ginzburg_neuron.py
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


@pytest.mark.skipif_missing_threads
@pytest.mark.skipif_incompatible_mpi
@MPITestAssertEqual([1, 2, 4])
def test_gamma_sup_generator():
    """
    Creates ginzburg neurons and sends spikes to spike recorder. Assert invariant results for fixed VP number.
    This is a partial response to #551. May be adapted to other generators.

    The test compares data written by spike_recorder to SPIKE_LABEL.
    """

    import nest

    TOTAL_VPS = 4

    nest.set(total_num_virtual_procs=TOTAL_VPS, overwrite_files=True)

    sg = nest.Create("spike_generator", params={"spike_times": [1, 1]})
    ginz = nest.Create("ginzburg_neuron", TOTAL_VPS, params={"tau_m": 1})
    pnet = nest.Create("parrot_neuron", TOTAL_VPS)
    sr = nest.Create(
        "spike_recorder",
        params={
            "label": SPIKE_LABEL.format(nest.num_processes),  # noqa: F821
            "record_to": "ascii",
            "time_in_steps": True,
        },
    )

    nest.Connect(sg, ginz)
    nest.Connect(ginz, pnet, "one_to_one")
    nest.Connect(pnet, sr)

    nest.Simulate(20)

    # Uncomment next line to provoke test failure
    # nest.Simulate(20 if nest.num_processes == 1 else 40)
