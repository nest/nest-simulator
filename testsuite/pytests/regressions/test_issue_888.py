# -*- coding: utf-8 -*-
#
# test_issue_888.py
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


def test_issue_888():
    """
    Ensure that the resolution is set as a multiple of tics.

    This test verifies that setting the resolution without specifying the tics per ms
    results in an error. It also checks that setting both the resolution and tics per ms
    to incompatible values results in an error.

    Author: Håkon Mørk, 2018-03-05
    """

    # Only setting resolution should fail
    with pytest.raises(Exception):
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": 0.03125})

    # Setting both resolution and tics per ms to incompatible values should fail
    with pytest.raises(Exception):
        nest.ResetKernel()
        nest.SetKernelStatus({"tics_per_ms": 1000.0, "resolution": 0.03125})
