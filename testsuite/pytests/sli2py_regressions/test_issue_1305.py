# -*- coding: utf-8 -*-
#
# test_issue_1305.py
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
Regression test for Issue #1305 (GitHub).

This set of tests ensures that NEST can set small resolutions and deals with
rounding errors correctly.
"""

import nest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_resolution_rounding_valid():
    """Test setting valid resolution."""

    target_resolution = 0.102
    nest.set(resolution=target_resolution, tics_per_ms=1000)

    assert nest.resolution == pytest.approx(target_resolution)


def test_resolution_rounding_invalid():
    """Test setting invalid resolution."""

    target_resolution = 0.1002

    with pytest.raises(nest.NESTError):
        nest.set(resolution=target_resolution, tics_per_ms=1000)
