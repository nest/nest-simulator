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

"""
Regression test for Ticket #349.

Ensure that a multimeter handles illegal entries in the record_from list gracefully
and that an empty record_from list does not cause the simulation to crash.

Author: Hans Ekkehard Plesser, 2009-07-02
"""


def test_exception_on_bad_recordables():
    nest.ResetKernel()

    good_recordables = nest.GetDefaults("iaf_psc_alpha")["recordables"]
    not_recordables = [f"{item}_foo" for item in good_recordables]

    mm = nest.Create("multimeter")
    nrn = nest.Create("iaf_psc_alpha")

    with pytest.raises(nest.NESTErrors.IllegalConnection):
        mm.record_from = not_recordables
        nest.Connect(mm, nrn)

    # If we get here, error was raised properly above, so we can test
    # if trying again with good values will succeed.
    mm.record_from = good_recordables
    nest.Connect(mm, nrn)

    # No assert require, we just check that Connect() succeeds


def test_empty_recordables():
    nest.ResetKernel()
    mm = nest.Create("multimeter", params={"record_from": []})
    nrn = nest.Create("iaf_psc_alpha")
    nest.Connect(mm, nrn)

    nest.Simulate(10.0)

    assert len(mm.events["senders"]) == 0
    assert len(mm.events["times"]) == 0
