# -*- coding: utf-8 -*-
#
# test_parameter_operators.py
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
Test that Parameters support arithmetic operations correctly.

This set of tests confirms that arithmetic operations on NEST Parameter objects yield
correct results by comparing with operations on the undelying numeric values.
It also confirms that operations on Parameter objects and plain numbers work.

.. note::

   These tests only confirm that operators on parameters work in principle. Therefore,
   we can use constant parameters for simplicity.
"""

import operator as ops

import nest
import pytest


def _const_param(val):
    return nest.CreateParameter("constant", {"value": val})


def _to_numeric(item):
    return item.GetValue() if hasattr(item, "GetValue") else item


@pytest.mark.xfail(raises=TypeError, strict=True)
@pytest.mark.parametrize(
    "op, a, b",
    [[ops.mod, _const_param(31), _const_param(5)], [ops.mod, _const_param(31), 5], [ops.mod, 31, _const_param(5)]],
)
def test_unsupported_operators(op, a, b):
    """
    Test that unsupported operator-operand combinations raise a TypeError.

    A side-purpose of this test is to document unsupported operators.
    """

    op(a, b)


@pytest.mark.parametrize("op", [ops.neg, ops.pos])
def test_unary_operators(op):
    """
    Perform tests for unary operators.

    Parametrization is over operators.
    """

    val_a = 31
    a = _const_param(val_a)

    assert op(a).GetValue() == op(val_a)


@pytest.mark.parametrize("op", [ops.add, ops.sub, ops.mul, ops.truediv])
@pytest.mark.parametrize("a, b", [[_const_param(31), _const_param(5)], [31, _const_param(5)], [_const_param(31), 5]])
def test_binary_operators(op, a, b):
    """
    Perform tests for binary operators.

    Outer parametrization is over operators, the inner over param-param, param-number and number-param combinations.
    """

    val_a = _to_numeric(a)
    val_b = _to_numeric(b)

    assert op(a, b).GetValue() == op(val_a, val_b)


def _unsupported_binary_op(op, a, b):
    """
    Represent test cases where an op is not supported for at least one of a or b.

    The syntax for representing individual test cases with expected failure is much more
    verbose than a simple `[op, a, b]`. This helper function returns the correct representation,
    ensuring that unexpected passing will be marked as error. For consistency with general
    Python behavior, we require that a `TypeError` is raised.
    """

    return pytest.param(op, a, b, marks=pytest.mark.xfail(raises=TypeError, strict=True))


@pytest.mark.parametrize(
    "op, a, b",
    [
        [ops.pow, _const_param(31), 5],
        _unsupported_binary_op(ops.pow, _const_param(31), _const_param(5)),
        _unsupported_binary_op(ops.pow, 31, _const_param(5)),
    ],
)
def test_incomplete_binary_operators(op, a, b):
    """
    Perform tests for binary operators that do not support parameters as all operands.
    """

    val_a = _to_numeric(a)
    val_b = _to_numeric(b)

    assert op(a, b).GetValue() == op(val_a, val_b)


@pytest.mark.parametrize("op", [ops.eq, ops.ne, ops.lt, ops.le, ops.gt, ops.ge])
@pytest.mark.parametrize(
    "a, b",
    [
        [_const_param(31), _const_param(31)],
        [_const_param(31), 31],
        [31, _const_param(31)],
        [_const_param(31), _const_param(5)],
        [_const_param(5), _const_param(31)],
    ],
)
def test_comparison_operators(op, a, b):
    """
    Perform tests for comparison operators.

    Outer parametrization is over operators, the inner over param-param, param-number and number-param combinations.
    """

    val_a = _to_numeric(a)
    val_b = _to_numeric(b)

    assert op(a, b).GetValue() == op(val_a, val_b)
