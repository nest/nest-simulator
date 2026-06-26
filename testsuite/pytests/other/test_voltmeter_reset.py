# -*- coding: utf-8 -*-
#
# test_voltmeter_reset.py
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


def test_voltmeter_reset():
    """
    Test if resetting works on voltmeter.

    The voltmeter records from iaf_psc_alpha to memory, checks if the proper number
    of data points is acquired, deleted on reset, and recorded again on further
    simulation. This test ascertains that also data stored in the derived recorder
    class, not only in RecordingDevice, is reset.
    """

    nest.ResetKernel()

    n = nest.Create("iaf_psc_alpha")
    vm = nest.Create("voltmeter")
    vm.set({"interval": 1.0})

    nest.Connect(vm, n)

    # Simulate and check initial recording
    nest.Simulate(10.5)
    events = vm.events
    assert vm.n_events == 10
    assert len(events["times"]) == 10
    assert len(events["V_m"]) == 10

    # Reset voltmeter and check
    vm.set({"n_events": 0})
    events = vm.events
    assert vm.n_events == 0
    assert len(events["times"]) == 0
    assert len(events["V_m"]) == 0

    # Simulate more and check recording
    nest.Simulate(5.0)
    events = vm.events
    assert vm.n_events == 5
    assert len(events["times"]) == 5
    assert len(events["V_m"]) == 5
