# -*- coding: utf-8 -*-
#
# test_sinusoidal_generators_parallel_same.py
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
Test that of sinusoidal generators work correctly also in parallel.

This file tests the case that all targets receive the same spike train.
We cannot parametrize over the checker function, therefore we need two files.
"""

import nest
import numpy as np
import pytest
from mpi_test_wrapper import MPITestAssertEqual


def assert_all_spike_trains_equal(all_res):
    """Assert that for each number of process, we have identical spike trains from all senders."""

    for spikes in all_res["spike"]:
        ix_by_sender = list(spikes.groupby("sender").groups.values())
        ref = spikes.iloc[ix_by_sender[0]].time_step
        assert all(np.array_equal(ref, spikes.iloc[ix].time_step) for ix in ix_by_sender[1:])


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("gen_model", ["sinusoidal_poisson_generator", "sinusoidal_gamma_generator"])
@pytest.mark.parametrize("num_threads", [1, 2])
@MPITestAssertEqual([1, 2, 4], debug=False, specific_assert=assert_all_spike_trains_equal)
def test_sinusoidal_generator_with_spike_recorder(gen_model, num_threads):
    """Test spike recording with ``individual_spike_trains == False``.

    The test builds a network with ``num_vp x 3`` parrot neurons that
    receives spikes from the specified sinusoidal generator. The test
    ensures that different targets receive identical spike trains.
    """

    nest.total_num_virtual_procs = 4
    nrns_per_vp = 3
    total_num_nrns = nest.total_num_virtual_procs * nrns_per_vp

    parrots = nest.Create("parrot_neuron", total_num_nrns)
    gen = nest.Create(
        gen_model,
        params={
            "rate": 100,
            "amplitude": 50.0,
            "frequency": 10.0,
            "individual_spike_trains": False,
        },
    )
    srec = nest.Create(
        "spike_recorder",
        params={
            "record_to": "ascii",
            "time_in_steps": True,
            "label": SPIKE_LABEL.format(nest.num_processes),  # noqa: F821
        },
    )

    nest.Connect(gen, parrots)
    nest.Connect(parrots, srec)

    nest.Simulate(200.0)
