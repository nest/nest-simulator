# -*- coding: utf-8 -*-
#
# test_ticket_681.py
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

import tempfile

import nest
import pytest

"""
Regression test for Ticket #681.

Test ported from SLI regression test.
Ensure NEST reports errors during node preparation without crashing when files already exist.

Author: Hans Ekkehard Plesser, 2014-11-25
"""


@pytest.mark.skipif_missing_threads
def test_ticket_681_handles_recording_backend_errors_without_crash():
    """
    Ensure that running NEST twice with overwrite disabled raises an error but does not crash.

    Test ported from SLI regression test.
    Ensures that NEST handles errors during calibration gracefully when files already exist.
    """

    if nest.ll_api.sli_func("statusdict/have_mpi ::"):
        pytest.skip("Test requires a serial NEST build without MPI support.")

    with tempfile.TemporaryDirectory() as data_path:
        try:
            # First simulation: should succeed
            nest.ResetKernel()
            nest.SetKernelStatus({"local_num_threads": 4, "overwrite_files": False, "data_path": data_path})

            neurons = nest.Create("iaf_psc_alpha", 4, params={"I_e": 1500.0})
            recorder = nest.Create("spike_recorder", params={"record_to": "ascii"})
            nest.Connect(neurons, recorder)
            nest.Simulate(1000.0)

            # Second simulation: should fail because files exist and overwrite is disabled
            nest.ResetKernel()
            nest.SetKernelStatus({"local_num_threads": 4, "overwrite_files": False, "data_path": data_path})

            neurons = nest.Create("iaf_psc_alpha", 4, params={"I_e": 1500.0})
            recorder = nest.Create("spike_recorder", params={"record_to": "ascii"})
            nest.Connect(neurons, recorder)

            # Expect IOError when trying to simulate with existing files
            with pytest.raises(nest.kernel.NESTErrors.IOError):
                nest.Simulate(1000.0)
        finally:
            # Always reset kernel to clean state, restoring defaults
            nest.ResetKernel()
