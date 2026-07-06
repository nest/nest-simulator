# -*- coding: utf-8 -*-
#
# test_threaded_multimeter.py
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
BeginDocumentation
   Name: testsuite::test_threaded_multimeter - test support for multimeter

   Synopsis: (test_threaded_multimeter.sli) run -> dies if assertion fails

   Description:
   Tests that multimeter works when we use threads.

   Test ported from SLI unit test.

   Author: Stine B. Vennemo
   FirstVersion: June 2017
   SeeAlso: test_treaded_spike_recorder.sli
"""

import nest
import numpy as np
import pytest


@pytest.mark.skipif_missing_threads
def test_threaded_multimeter():
    """
    Test that multimeter works correctly with multiple threads.
    """
    N = 10  # should not be divisible by thread number

    def run_multimeter(n_threads):
        nest.ResetKernel()
        # Distributed setting not covered
        assert nest.num_processes == 1
        nest.set(local_num_threads=n_threads)

        # Create neurons with different initial V_m
        nrns = nest.Create("iaf_psc_alpha", N, params={"I_e": 40.0})
        for i, nrn in enumerate(nrns):
            v_m = -90.0 + 30.0 * i / N
            nrn.set({"V_m": v_m})

        # Create multimeter
        ac = nest.Create("multimeter", params={"record_from": ["V_m"], "interval": 1.0})

        nest.Connect(ac, nrns)

        nest.Simulate(5.0)

        # Obtain data
        events = ac.get("events")
        return events["V_m"], events["senders"]

    # Run with 1 and 3 threads
    r1V, r1s = run_multimeter(1)
    r3V, r3s = run_multimeter(3)

    # Sort and compare - results should be identical regardless of thread count
    assert np.array_equal(np.sort(r1V), np.sort(r3V))
    assert np.array_equal(np.sort(r1s), np.sort(r3s))
