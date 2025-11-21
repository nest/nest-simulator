# -*- coding: utf-8 -*-
#
# test_ticket_516.py
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
@pytest.mark.parametrize("on_source", [True, False])
@MPITestAssertEqual([1, 2, 4], debug=False)
def test_ticket_516(on_source):
    """
    Confirm that simulations of a spatial network yield consistent spike trains.

    The test compares data written by spike_recorder to SPIKE_LABEL.
    """

    import nest

    nest.rng_seed = 1234567
    nest.total_num_virtual_procs = 4

    # Drive network with DC current, random connectivity and weights
    # lead to variation in spike times between neurons
    layer = nest.Create("iaf_psc_exp", params={"I_e": 500}, positions=nest.spatial.grid(shape=[5, 5], edge_wrap=False))
    sr = nest.Create(
        "spike_recorder",
        params={
            "record_to": "ascii",
            "time_in_steps": True,
            "label": SPIKE_LABEL.format(nest.num_processes),  # noqa: F821
        },
    )

    nest.Connect(
        layer,
        layer,
        {"rule": "pairwise_bernoulli", "use_on_source": on_source, "mask": {"circular": {"radius": 0.5}}, "p": 0.7},
        {"weight": nest.random.uniform(-5, 15), "delay": 1},
    )

    nest.Connect(layer, sr)

    nest.Simulate(200)
