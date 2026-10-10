# -*- coding: utf-8 -*-
#
# test_issue_3780.py
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


def test_issue_3780():
    """
    Slicing a spatial NodeCollection (which carries metadata) turns it into
    a composite collection that keeps the original, shared parts_ and only
    narrows the visible range via first_part_/last_part_/first_elem_/
    last_elem_. Comparing two such slices must take that narrowed range into
    account, not just the shared underlying parts_.
    """

    nc = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid(shape=[3, 3], extent=(3.0, 3.0)))

    nc1 = nc[1]
    nc2 = nc[2]

    # Different single-element slices of the same spatial NodeCollection must
    # not compare as equal.
    assert nc1 != nc2

    # The same slice, computed twice, must compare as equal.
    assert nc1 == nc[1]

    # A larger, overlapping-looking slice must still be distinguished from a
    # different one of the same size.
    assert nc[0:3] != nc[3:6]
    assert nc[0:3] == nc[0:3]


def test_issue_3780_index_array_slicing_unaffected():
    """
    Slicing with an explicit index array (rather than a Python slice) drops
    the metadata and already compared correctly before this fix; guard
    against a regression here too.
    """

    nc = nest.Create("iaf_psc_alpha", positions=nest.spatial.grid(shape=[3, 3], extent=(3.0, 3.0)))

    assert nc[[1]] != nc[[2]]
    assert nc[[1]] == nc[[1]]
