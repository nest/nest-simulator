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

import nest
import pytest
import operator as opr


def const_param(val):
    return nest.CreateParameter('constant', {'value': val})


@pytest.mark.parametrize('op, a, b', [
    [opr.pow, const_param(31), const_param(5)],
    [opr.pow, 31, const_param(5)],
    [opr.mod, const_param(31), const_param(5)],
    [opr.mod, const_param(31), 5],
    [opr.pow, 31, const_param(5)]
])
def test_unsupported_operators(op, a, b):
    """
    Test that unsupported operator-operand combinations raise a TypeError.

    A side-purpose of this test is to document which operators are not fully supported.
    """

    with pytest.raises(TypeError):
        op(a, b)


@pytest.mark.parametrize('op', [
    opr.neg,
    opr.pos
])
def test_unary_operators(op):
    """
    Perform tests for unary operators.

    Parametrization is over operators
    """

    val_a = 31
    a = nest.CreateParameter('constant', {'value': val_a})

    assert op(a).GetValue() == op(val_a)


@pytest.mark.parametrize('op', [
    opr.add,
    opr.sub,
    opr.mul,
    opr.truediv
])
@pytest.mark.parametrize('a, b', [
    [const_param(31), const_param(5)],
    [31, const_param(5)],
    [const_param(31), 5]
])
def test_binary_operators(op, a, b):
    """
    Perform tests for binary operators.

    Outer parametrization is over operators, the inner over param-param, param-number and number-param combinations.
    """

    # Extract underlying numerical value if we are passed a parameter object.
    try:
        val_a = a.GetValue()
    except AttributeError:
        val_a = a
    try:
        val_b = b.GetValue()
    except AttributeError:
        val_b = b

    assert op(a, b).GetValue() == op(val_a, val_b)


@pytest.mark.parametrize('op, a, b', [
    [opr.pow, const_param(31), 5]])
def test_incomplete_binary_operators(op, a, b):
    """
    Perform tests for binary operators that do not support Parameter as any operand.
    """

    # Extract underlying numerical value if we are passed a parameter object.
    try:
        val_a = a.GetValue()
    except AttributeError:
        val_a = a
    try:
        val_b = b.GetValue()
    except AttributeError:
        val_b = b

    assert op(a, b).GetValue() == op(val_a, val_b)


@pytest.mark.parametrize('op', [
    opr.eq,
    opr.ne,
    opr.lt,
    opr.le,
    opr.gt,
    opr.ge
])
@pytest.mark.parametrize('a, b', [
    [const_param(31), const_param(31)],
    [const_param(31), 31],
    [31, const_param(31)],
    [const_param(31), const_param(5)],
    [const_param(5), const_param(31)],
])
def test_comparison_operators(op, a, b):
    """
    Perform tests for comparison operators.

    Outer parametrization is over operators, the inner over param-param, param-number and number-param combinations.
    """

    # Extract underlying numerical value if we are passed a parameter object.
    try:
        val_a = a.GetValue()
    except AttributeError:
        val_a = a
    try:
        val_b = b.GetValue()
    except AttributeError:
        val_b = b

    assert op(a, b).GetValue() == op(val_a, val_b)
