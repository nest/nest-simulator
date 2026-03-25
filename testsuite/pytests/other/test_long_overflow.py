# -*- coding: utf-8 -*-
#
# test_long_overflow.py
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
Confirm that out-of-bounds Python integers raise exceptions.
"""

import nest
import pytest

LONG_MIN = -(2**63)
LONG_MAX = 2**63 - 1

LONG_BELOW_MIN = LONG_MIN - 1
LONG_ABOVE_MAX = LONG_MAX + 1


@pytest.fixture
def neuron():
    return nest.Create("iaf_psc_alpha")


@pytest.mark.parametrize("python_int", [LONG_MIN, LONG_MAX])
def test_within_bounds(neuron, python_int):
    neuron.V_m = python_int


@pytest.mark.parametrize("python_int", [LONG_BELOW_MIN, LONG_ABOVE_MAX])
def test_outside_bounds(neuron, python_int):
    with pytest.raises(OverflowError):
        neuron.V_m = python_int
