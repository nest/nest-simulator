# -*- coding: utf-8 -*-
#
# test_parameter.py
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

@note These tests only confirm that operators on parameters work in principle. Therefore,
      we can use constant parameters for simplicity. We need to test elsewhere that
      other types of Parameters produce correct values.
"""

import nest
import pytest
import operator


@pytest.mark.parametrize('op', [operator.pos,
                                operator.neg])
def test_binary_operators(op):
    """
    Perform tests for unary operators.

    Parametrization is over operators
    """

    val_a = 31
    nest.CreateParameter('constant', {'value': val_a})
        
    assert op(a).GetValue() == op(a)


@pytest.mark.parametrize('op', [operator.add,
                                operator.sub,
                                operator.mul,
                                operator.truediv,
                                operator.pow])
@pytest.mark.parametrize('a, b', [[nest.CreateParameter('constant', {'value': 31}), nest.CreateParameter('constant', {'value':  5})],
                                  [31, nest.CreateParameter('constant', {'value':  5})],
                                  [nest.CreateParameter('constant', {'value': 31}), 5]])
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
        
    assert op(a, b).GetValue() == op(a, b)


@pytest.mark.parametrize('op', [operator.eq,
                                operator.ne,
                                operator.lt,
                                operator.le,
                                operator.gt,
                                operator.ge])
@pytest.mark.parametrize('a, b', [[nest.CreateParameter('constant', {'value': 31}), nest.CreateParameter('constant', {'value':  31})],
                                  [nest.CreateParameter('constant', {'value': 31}), nest.CreateParameter('constant', {'value':  5})],
                                  [nest.CreateParameter('constant', {'value': 5}), nest.CreateParameter('constant', {'value':  31})],
                                  [31, nest.CreateParameter('constant', {'value':  5})],
                                  [nest.CreateParameter('constant', {'value': 31}), 5]])
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
        
    assert op(a, b).GetValue() == op(a, b)
