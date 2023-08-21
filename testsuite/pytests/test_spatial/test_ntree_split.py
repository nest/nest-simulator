# -*- coding: utf-8 -*-
#
# test_ntree_split.py
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
a
"""

import nest
import numpy as np


def test_ntree_split():
    """
    a
    """

    # Generate positions. The parameters pre_n_x and r should be defined such
    # that we get roundoff errors. pre_n_x must be larger than 100 to make the
    # ntree split.
    pre_n_x = 110
    r = 0.45
    positions = np.zeros((pre_n_x, 3))

    low_xy = -r
    high_xy = r
    dx = (high_xy - low_xy) / (pre_n_x - 1)

    positions[:, 0] = np.arange(low_xy, high_xy + dx, dx)
    print(positions)
