# -*- coding: utf-8 -*-
#
# test_issue_1242.py
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
Regression test for Issue #1242 (GitHub).
"""

import nest
import pytest


def test_volume_transmitter_illegal_connection():
    """
    Test that volume transmitter throws an error for illegal connection.

    The test ensures that a connection from a node without proxies to
    a node without proxies that has global targets throws an error
    (instead of failing silently or resulting in inconsistent results).
    """

    sg = nest.Create("spike_generator")
    vt = nest.Create("volume_transmitter")

    with pytest.raises(nest.kernel.NESTErrors.IllegalConnection):
        nest.Connect(sg, vt)
