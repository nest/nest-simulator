# -*- coding: utf-8 -*-
#
# test_issue_311.py
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
Regression test for Issue #311 (GitHub).

This test ensures that NEST behaves properly when a model triggers an exception
during update.
"""

import nest
import pytest


def test_nest_behaves_well_after_exception_during_update():
    # Pathological parameters to trigger numerical exception
    nrn = nest.Create("aeif_cond_alpha", params={"I_e": 10000000.0, "g_L": 0.01})

    # Execute in try-except context so exception does not propagate out
    did_crash = False
    try:
        nest.Simulate(100.0)
    except nest.NESTErrors.NumericalInstability:
        did_crash = True
        pass

    # did_crash must be true, otherwise no exception was triggered
    assert did_crash

    # Test that we still can inspect the kernel after an exception
    nest.GetKernelStatus()

    # Test that we cannot continue simulation after an exception.
    # Set neuron parameters to values that should stabilize numerics
    nrn.set({"V_m": -70.0, "w": 0.0, "I_e": 0.0})

    with pytest.raises(nest.NESTErrors.KernelException):
        nest.Simulate(0.1)

    # Test that we can simulate again after a ResetKernel
    nest.ResetKernel()
    nest.Create("aeif_cond_alpha", params={"I_e": 1000.0})
    nest.Simulate(100.0)
