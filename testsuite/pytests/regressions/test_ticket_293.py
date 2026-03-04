# -*- coding: utf-8 -*-
#
# test_ticket_293.py
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


@pytest.mark.skipif_missing_threads
def test_ticket_293():
    """
    Regression test for Ticket #293.

    This test verifies that NEST handles unknown keys in the status dictionary
    gracefully when the number of threads is changed. It ensures that an error
    is raised for the unknown key, but NEST does not crash.

    Author: Hans E Plesser, 2008-10-20
    """

    with pytest.raises(Exception):
        nest.SetKernelStatus({"local_num_threads": 2, "foo": 5})
