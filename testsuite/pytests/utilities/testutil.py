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

import nest
import pandas as pd
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


def get_comparable_timesamples(resolution, actual, expected):
    """
    Return result of inner join on time stamps for actual and expected given resolution.

    `actual` and `expected` must be arrays with columns representing times (in ms) and values.
    Times will be converted to steps given the resolution and the data will then be inner-
    joined on the steps, i.e., rows with equal steps will be extracted.

    Returns two one-dimensional arrays containing the values at the joint points from
    actual and expected, respectively.
    """

    tics = nest.tics_per_ms

    actual = pd.DataFrame(actual, columns=["t", "val_a"])
    expected = pd.DataFrame(expected, columns=["t", "val_e"])

    actual["tics"] = (actual.t * tics).round().astype(int)
    expected["tics"] = (expected.t * tics).round().astype(int)

    common = pd.merge(actual, expected, how="inner", on="tics")

    return common.val_a.values, common.val_e.values
