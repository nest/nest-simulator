# -*- coding: utf-8 -*-
#
# test_issue_1610.py
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
Regression test for Issue #1610 (GitHub).
"""

import nest


def test_comparing_primitive_and_composite_nodecollections():
    """
    Test comparing primitive and composite `NodeCollection`s.

    The test ensures that comparing primitive and composite `NodeCollection`s
    does not lead to a segmentation fault and gives the correct result.
    """

    prim_nc = nest.Create("iaf_psc_alpha", 10)
    comp_nc = nest.Create("iaf_psc_alpha", 5) + nest.Create("iaf_psc_alpha", 5)

    assert prim_nc != comp_nc
    assert comp_nc != prim_nc
