# -*- coding: utf-8 -*-
#
# testutil.py
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

import sys

import numpy as np
import pytest


def dict_is_subset_of(small, big):
    """
    Return true if dict `small` is subset of dict `big`.

    `small` must contain all keys in `big` with the same values.
    """

    # See
    # https://stackoverflow.com/questions/20050913/python-unittests-assertdictcontainssubset-recommended-alternative
    # https://peps.python.org/pep-0584/
    #
    # Note: | is **not** a symmetric operator for dicts. `small` must be the second operand to | as it determines
    #       the value of joint keys in the merged dictionary.

    return big == big | small


def get_comparable_timesamples(actual, expected):
    simulated_points = isin_approx(actual[:, 0], expected[:, 0])
    expected_points = isin_approx(expected[:, 0], actual[:, 0])
    assert len(actual[simulated_points]) > 0, "The recorded data did not contain any relevant timesamples"
    return actual[simulated_points], pytest.approx(expected[expected_points])
