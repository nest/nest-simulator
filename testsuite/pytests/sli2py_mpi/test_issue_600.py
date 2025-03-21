# -*- coding: utf-8 -*-
#
# test_issue_600.py
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


from mpi_test_wrapper import MPITestAssertEqual


@MPITestAssertEqual([1, 2, 4], debug=True)
def test_issue_600():
    """
    Confirm that waveform relaxation works with MPI.
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
    n1 = nest.Create("hh_psc_alpha_gap", params={"I_e": 400.0})
    n2 = nest.Create("iaf_psc_alpha")
    n3 = nest.Create("hh_psc_alpha_gap")
    n4 = nest.Create("iaf_psc_alpha")

    sr = nest.Create(
        "spike_recorder",
        params={
            "record_to": "ascii",
            "time_in_steps": True,
            "label": SPIKE_LABEL.format(nest.num_processes),  # noqa: F821
        },
    )

    # Use weights as required or sufficient to trigger spikes in n3, n2, n4
    nest.Connect(
        n1, n3, {"rule": "one_to_one", "make_symmetric": True}, {"synapse_model": "gap_junction", "weight": 10.0}
    )
    nest.Connect(n1, n2, {"rule": "one_to_one"}, {"synapse_model": "static_synapse", "weight": 1000.0})
    nest.Connect(n1, n4, {"rule": "one_to_one"}, {"synapse_model": "static_synapse", "weight": 1200.0})

    nest.Connect(n1 + n2 + n3 + n4, sr)

    nest.Simulate(50)
