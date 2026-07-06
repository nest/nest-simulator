# -*- coding: utf-8 -*-
#
# test_ticket_464.py
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
import nest
import pytest


# Placeholder decorator to simulate MPI check
def skip_if_have_mpi(func):
    try:
        # Simulates checking for MPI (not directly feasible in Python)
        import MPI

        return pytest.mark.skip(reason="Test skipped if NEST has MPI support")(func)
    except ImportError:
        return func


@skip_if_have_mpi
def test_ticket_464():
    """
    Ensure that NEST triggers an error with a frozen multimeter connected to a node.

    Test ported from SLI regression test
    This test has been modified to reflect that NEST now protects multimeter against being frozen.
    The first test triggers an error instead of an assertion, and crash_or_die has been replaced by fail_or_die.

    Remarks:
    This test has been modified (2011-02-11) to reflect the fact that NEST now protects
    multimeter against being frozen. Thus, the first test triggers an error instead of
    and assertion, and crash_or_die has been replaced by fail_or_die.

    Author: Hans Ekkehard Plesser, 2010-10-04
    """

    # Test: multimeter frozen, should trigger error
    nest.ResetKernel()
    # Attempt to create and freeze a multimeter
    mm = nest.Create("multimeter", params={"record_from": ["V_m"]})
    with pytest.raises(Exception):  # Expect an error due to frozen multimeter
        mm.set(frozen=True)  # Set frozen status
        iaf_neuron = nest.Create("iaf_psc_alpha")
        nest.Connect(mm, iaf_neuron)
        nest.Simulate(3.0)

    # Test: multimeter thawed, should run fine
    nest.ResetKernel()
    mm = nest.Create("multimeter", params={"record_from": ["V_m"]})
    mm.set(frozen=False)  # Ensure multimeter is not frozen
    iaf_neuron = nest.Create("iaf_psc_alpha")
    nest.Connect(mm, iaf_neuron)
    nest.Simulate(3.0)  # Should run without issues

    # If we reach here, the second test has passed
