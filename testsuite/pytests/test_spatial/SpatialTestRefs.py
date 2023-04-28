# -*- coding: utf-8 -*-
#
# SpatialTestRefs.py
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

import numpy as np


class SpatialTestRefs:
    """
    Provides common fixtures for source and target references used by the spatial tests.
    """

    src_layer_ref = np.array(
        [[1., -0.5, 0.5], [2., -0.5, 0.25], [3., -0.5, 0.], [4., -0.5, -0.25], [5., -0.5, -0.5], [6., -0.25, 0.5],
         [7., -0.25, 0.25], [8., -0.25, 0.], [9., -0.25, -0.25], [10., -0.25, -0.5], [11., 0., 0.5],
         [12., 0., 0.25], [13., 0., 0.], [14., 0., -0.25], [15., 0., -0.5], [16., 0.25, 0.5], [17., 0.25, 0.25],
         [18., 0.25, 0.], [19., 0.25, -0.25], [20., 0.25, -0.5], [21., 0.5, 0.5], [22., 0.5, 0.25], [23., 0.5, 0.],
         [24., 0.5, -0.25], [25., 0.5, -0.5]])

    target_layer_ref = np.array(
        [[26.0, -0.5, 0.5], [27, -0.5, 0.25], [28, -0.5, 0], [29, -0.5, -0.25], [30, -0.5, -0.5], [31, -0.25, 0.5],
         [32, -0.25, 0.25], [33, -0.25, 0], [34, -0.25, -0.25], [35, -0.25, -0.5], [36, 0, 0.5], [37, 0, 0.25],
         [38, 0, 0], [39, 0, -0.25], [40, 0, -0.5], [41, 0.25, 0.5], [42, 0.25, 0.25], [43, 0.25, 0],
         [44, 0.25, -0.25], [45, 0.25, -0.5], [46, 0.5, 0.5], [47, 0.5, 0.25], [48, 0.5, 0], [49, 0.5, -0.25],
         [50, 0.5, -0.5]])
