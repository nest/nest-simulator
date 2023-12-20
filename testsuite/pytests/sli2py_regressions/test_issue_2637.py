# -*- coding: utf-8 -*-
#
# test_issue_2637.py
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
Regression test for Issue #2637 (GitHub).

This set of tests ensures that a ``NodeCollection`` can be sliced with
a Python ``list`` of NumPy integers.
"""

import nest
import numpy as np
import pytest


@pytest.mark.parametrize("dtype", [int, np.int32, np.int64])
def test_nc_slice_list_of_numpy_ints(dtype):
    """
    Ensure that a list of NumPy integers can slice a ``NodeCollection``.
    """

    nc = nest.Create("iaf_psc_alpha", 2)
    slice_arr = np.array([0, 1], dtype=dtype)
    slice_lst = list(slice_arr)
    nc_slice = nc[slice_lst]
