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
Simple Parameter tests
"""

import nest
import pytest

va, vb = 5, 7
a = nest.CreateParameter('constant', {'value': va})
b = nest.CreateParameter('constant', {'value': vb})


@pytest.fixture(autouse=True)
def reset_kernel():
    nest.ResetKernel()


@pytest.mark.parametrize('op_a, op_b', [[a, b], [a, vb], [vb, a]])
def test_add(op_a, op_b):

    try:
        val_a = op_a.GetValue()
    except AttributeError:
        val_a = op_a
    try:
        val_b = op_b.GetValue()
    except AttributeError:
        val_b = op_b
        
    assert (op_a + op_b).GetValue() == val_a + val_b

    
@pytest.mark.parametrize('op_a, op_b', [[a, b], [a, vb], [vb, a]])
def test_sub(op_a, op_b):

    try:
        val_a = op_a.GetValue()
    except AttributeError:
        val_a = op_a
    try:
        val_b = op_b.GetValue()
    except AttributeError:
        val_b = op_b
        
    assert (op_a - op_b).GetValue() == val_a - val_b


@pytest.mark.parametrize('op_a, op_b', [[a, b], [a, vb], [vb, a]])
def test_mul(op_a, op_b):

    try:
        val_a = op_a.GetValue()
    except AttributeError:
        val_a = op_a
    try:
        val_b = op_b.GetValue()
    except AttributeError:
        val_b = op_b
        
    assert (op_a * op_b).GetValue() == val_a * val_b


@pytest.mark.parametrize('op_a, op_b', [[a, b], [a, vb], [vb, a]])
def test_div(op_a, op_b):

    try:
        val_a = op_a.GetValue()
    except AttributeError:
        val_a = op_a
    try:
        val_b = op_b.GetValue()
    except AttributeError:
        val_b = op_b
        
    assert (op_a / op_b).GetValue() == val_a / val_b


@pytest.mark.parametrize('op_a, op_b', [[a, b], [a, vb], [vb, a]])
def test_mod(op_a, op_b):

    try:
        val_a = op_a.GetValue()
    except AttributeError:
        val_a = op_a
    try:
        val_b = op_b.GetValue()
    except AttributeError:
        val_b = op_b
        
    assert (op_a % op_b).GetValue() == val_a % val_b


@pytest.mark.parametrize('op_a, op_b', [[a, b], [a, vb], [vb, a]])
def test_pow(op_a, op_b):

    try:
        val_a = op_a.GetValue()
    except AttributeError:
        val_a = op_a
    try:
        val_b = op_b.GetValue()
    except AttributeError:
        val_b = op_b
        
    assert (op_a ** op_b).GetValue() == val_a ** val_b
