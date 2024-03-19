# -*- coding: utf-8 -*-
#
# test_regression_issue-3156.py
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

import operator as ops

import nest
import pytest

"""
Tests to ensure that random parameters can be used with all operators that support parameters.

We expect that each parameter value drawn will differ from all others.
"""


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


num_neurons = 10
operators = [ops.add, ops.sub, ops.mul, ops.truediv, ops.pow, ops.mod]


@pytest.mark.parametrize("lhs", [1, 2, 2.0])
@pytest.mark.parametrize("op", operators)
def test_params_random_denominator(lhs, op):
    num_neurons = 10
    try:
        n = nest.Create("iaf_psc_alpha", num_neurons, {"V_m": op(lhs, nest.random.uniform(1, 2))})
    except TypeError:
        pass  # operation not supported for parameter
    else:
        assert len(set(n.V_m)) == num_neurons


@pytest.mark.parametrize("op", operators)
@pytest.mark.parametrize("rhs", [1, 2, 2.0])
def test_params_random_numerator(op, rhs):
    num_neurons = 10

    try:
        n = nest.Create("iaf_psc_alpha", num_neurons, {"V_m": op(nest.random.uniform(1, 2), rhs)})
    except TypeError:
        pass  # operation not supported for parameter
    else:
        assert len(set(n.V_m)) == num_neurons


def test_random_numer_and_denom():
    """
    For random parameters in numerator and denominator, we make the denominator uniform
    on the set {-1, +1}. For 50 neurons, the probability that the denominator has the same
    sign (either positive or negative) is 2 * 2^-50 ≈ 2e-15.
    """

    num_neurons = 50
    try:
        n = nest.Create(
            "iaf_psc_alpha", num_neurons, {"V_m": nest.random.uniform(1, 2) / (1.0 - 2.0 * nest.random.uniform_int(2))}
        )
    except TypeError:
        pass
    else:
        V_m = n.V_m
        assert len(set(n.V_m)) == num_neurons
        assert sum(V < 0 for V in V_m) > 0
        assert sum(V >= 0 for V in V_m) > 0
