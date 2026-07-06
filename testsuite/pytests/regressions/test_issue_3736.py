# -*- coding: utf-8 -*-
#
# test_issue_3736.py
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

import math

import nest
import pytest

"""
Issue 3736: Permit std == 0 for normal and lognormal parameters
"""


@pytest.mark.parametrize("distribution, transform", [["normal", lambda x: x], ["lognormal", math.exp]])
@pytest.mark.parametrize("mean", [0, 0.1, 1])
def test_always_mean(distribution, transform, mean):
    """
    Confirm that random values obtained for std==0 are all equal to the mean.

    Transform handles the fact that the distribution mean differs from the "mean"
    parameter for some distributions, such as lognormal here.
    """

    p = nest.CreateParameter(distribution, {"mean": mean, "std": 0})
    assert all(p.GetValue() == transform(mean) for _ in range(10))
