# -*- coding: utf-8 -*-
#
# test_issue_1140.py
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
Regression test for Issue #1140 (GitHub).

Test if parameters for `inhomogeneous_poisson_generator` can be set with empty
arrays for `rate_times` and `rate_values`.
"""

import nest
import numpy as np
import numpy.testing as nptest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_inhomogeneous_poisson_generator_set_empty_params():
    """
    Test `inhomogeneous_poisson_generator` setter with empty parameter arrays.

    The test checks if we can set parameters with empty arrays `rate_times`
    and `rate_values` without causing a segfault.
    """

    params = {"rate_times": np.array([]), "rate_values": np.array([])}
    ipg = nest.Create("inhomogeneous_poisson_generator")
    ipg.set(params)


def test_inhomogeneous_poisson_generator_params_set_implicitly():
    """
    Test `inhomogeneous_poisson_generator` parameters set on creation.

    The test checks if parameters on creation are set properly when set
    implicitly with `Create`. `Create` temporarily changes default values,
    then resets them, which would trigger the issue.
    """

    params = {"rate_times": [10.0, 110.0, 210.0], "rate_values": [400.0, 1000.0, 200.0]}
    ipg = nest.Create("inhomogeneous_poisson_generator", params=params)

    for key in params:
        actual = ipg.get(key)
        expected = params[key]
        nptest.assert_array_equal(actual, expected)
