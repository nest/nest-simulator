# -*- coding: utf-8 -*-
#
# test_ticket_349.py
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


@pytest.mark.skipif(not nest.ll_api.sli_func("statusdict/have_gsl ::"), reason="Requires GSL")
def test_ticket_349():
    """
    Regression test for Ticket #349.

    Ensure that a multimeter handles illegal entries in the record_from list gracefully
    and that an empty record_from list does not cause the simulation to crash.

    Author: Hans Ekkehard Plesser, 2009-07-02
    """

    # First test: Connect with illegal entry
    nest.ResetKernel()
    mm = nest.Create("multimeter")
    n = nest.Create("iaf_cond_alpha")

    r = nest.GetDefaults("iaf_cond_alpha")["recordables"]
    rfail = [f"{item}_foo" for item in r]

    mm.set(record_from=rfail)

    with pytest.raises(Exception):
        nest.Connect(mm, n)

    # Second test: Connect with illegal entry first, then retry with legal list
    nest.ResetKernel()
    mm = nest.Create("multimeter")
    n = nest.Create("iaf_cond_alpha")

    r = nest.GetDefaults("iaf_cond_alpha")["recordables"]
    rfail = [f"{item}_foo" for item in r]

    mm.set(record_from=rfail)

    try:
        nest.Connect(mm, n)
    except Exception:
        pass

    mm.set(record_from=r)
    nest.Connect(mm, n)

    # Third test: Connect multimeter with empty list, then simulate
    nest.ResetKernel()
    mm = nest.Create("multimeter")
    n = nest.Create("iaf_cond_alpha")
    mm.set(record_from=[])
    nest.Connect(mm, n)

    nest.Simulate(10.0)

    events = mm.get("events")
    assert len(events["senders"]) == 0
    assert len(events["times"]) == 0
