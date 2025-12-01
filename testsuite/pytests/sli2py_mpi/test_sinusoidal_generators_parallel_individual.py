# -*- coding: utf-8 -*-
#
# test_sinusoidal_generators_parallel_individual.py
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

This file tests the case that all targets receive individual spike trains.
We cannot parametrize over the checker function, therefore we need two files.
"""

import nest
import numpy as np
import pytest
from mpi_test_wrapper import MPITestAssertEqual


def assert_all_spike_trains_different(all_res):
    """Assert that for each number of processes, all spike trains are pairwise different."""

    for spikes in all_res["spike"]:
        ix_by_sender = list(spikes.groupby("sender").groups.values())
        assert all(
            not np.array_equal(spikes.iloc[lhs_ix].time_step, spikes.iloc[rhs_ix].time_step)
            for idx, lhs_ix in enumerate(ix_by_sender[:-1])
            for rhs_ix in ix_by_sender[(idx + 1) :]
        )


@pytest.mark.skipif_incompatible_mpi
@pytest.mark.skipif_missing_threads
@pytest.mark.parametrize("gen_model", ["sinusoidal_poisson_generator", "sinusoidal_gamma_generator"])
@pytest.mark.parametrize("num_threads", [1, 2])
@MPITestAssertEqual([1, 2, 4], debug=False, specific_assert=assert_all_spike_trains_different)
def test_sinusoidal_generator_with_spike_recorder(gen_model, num_threads):
    """Test spike recording for individual spike trains.

    The test builds a network with ``num_vp x 3`` parrot neurons that
    receives spikes from the specified sinusoidal generator. The test
    ensures that different targets receive different spike trains.
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
            "individual_spike_trains": True,
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
