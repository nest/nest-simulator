# -*- coding: utf-8 -*-
#
# test_gap_junctions_mpi.py
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
@pytest.mark.skipif_missing_gsl
@MPITestAssertEqual([1, 2, 4], debug=False)
def test_gap_junctions_mpi():
    """
    Test gap junction functionality in parallel.

    This is an overall test of the hh_psc_alpha_gap model connected by gap_junction.
    The test checks if the gap junction functionality works in parallel.
    """

    import nest
    import pandas as pd

    # We can only test here if GSL is available
    if not nest.ll_api.sli_func("statusdict/have_gsl ::"):
        return

    total_vps = 4
    h = 0.1

    nest.SetKernelStatus(
        {
            "total_num_virtual_procs": total_vps,
            "resolution": h,
            "use_wfr": True,
            "wfr_tol": 0.0001,
            "wfr_interpolation_order": 3,
            "wfr_max_iterations": 10,
            "wfr_comm_interval": 1.0,
        }
    )
    n = nest.Create("hh_psc_alpha_gap", n=4)
    n[0].I_e = 400.0

    sr = nest.Create(
        "spike_recorder",
        params={
            "record_to": "ascii",
            "time_in_steps": True,
            "label": SPIKE_LABEL.format(nest.num_processes),  # noqa: F821
        },
    )

    for nrn, w in zip(n[1:], [10, 8, 12]):
        nest.Connect(
            n[0], nrn, {"rule": "one_to_one", "make_symmetric": True}, {"synapse_model": "gap_junction", "weight": w}
        )

    nest.Connect(n, sr)

    nest.Simulate(50)
