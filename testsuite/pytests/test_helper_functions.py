# -*- coding: utf-8 -*-
#
# test_helper_functions.py
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
import pytest
import testutil


@pytest.mark.parametrize(
    "level, value",
    [
        ("M_ALL", 0),
        ("M_DEBUG", 5),
        ("M_STATUS", 7),
        ("M_INFO", 10),
        ("M_DEPRECATED", 18),
        ("M_WARNING", 20),
        ("M_ERROR", 30),
        ("M_FATAL", 40),
        ("M_QUIET", 100),
    ],
)
def test_set_verbosity(level, value):
    nest.set_verbosity(level)
    assert nest.get_verbosity() == value


@pytest.mark.parametrize(
    "a, b, expected",
    [
        ({}, {}, True),
        ({}, {"a": 5}, True),
        ({"a": 5}, {"a": 5}, True),
        ({"a": 7}, {"a": 5}, False),
        ({"a": 5, "b": 3}, {"a": 5}, False),
    ],
)
def test_dict_is_subset(a, b, expected):
    assert testutil.dict_is_subset_of(a, b) == expected
