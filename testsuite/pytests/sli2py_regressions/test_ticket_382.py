# -*- coding: utf-8 -*-
#
# test_ticket_382.py
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


def test_ticket_382():
    """
    Regression test for Ticket #382.

    This test asserts that one can connect a voltmeter and a multimeter after having
    simulated for some time first.

    Author: Hans E Plesser, 2010-04-22
    """

    # Test for connecting voltmeter after simulation
    nest.ResetKernel()
    vm = nest.Create("voltmeter")
    n = nest.Create("iaf_psc_alpha")
    nest.Simulate(10.0)
    nest.Connect(vm, n, syn_spec={"delay": 0.1})  # Corrected delay value
    nest.Simulate(10.0)

    # Test for connecting multimeter after simulation
    nest.ResetKernel()
    mm = nest.Create("multimeter", params={"record_from": ["V_m"]})
    n = nest.Create("iaf_psc_alpha")
    nest.Simulate(10.0)
    nest.Connect(mm, n, syn_spec={"delay": 0.1})  # Corrected delay value
    nest.Simulate(10.0)

    # Assert that the test completes without exceptions
    assert True
