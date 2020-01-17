# -*- coding: utf-8 -*-
#
# hl_api_math.py
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

from ..ll_api import sli_func

__all__ = [
    'exp',
    'sin',
    'cos',
    'min',
    'max',
    'redraw',
]

# TODO: Special cases when argument is a number?


def exp(parameter):
    """
    Calculate the exponential of the parameter

    Parameters
    ----------
    parameter : Parameter
        Input Parameter.

    Returns
    -------
    Parameter:
        Object representing the exponential of the parameter.
    """
    return sli_func("exp", parameter)


def sin(parameter):
    """
    Calculate the sine of the parameter

    Parameters
    ----------
    parameter : Parameter
        Input Parameter.

    Returns
    -------
    Parameter:
        Object representing the sine of the parameter.
    """
    return sli_func("sin", parameter)


def cos(parameter):
    """
    Calculate the cosine of the parameter

    Parameters
    ----------
    parameter : Parameter
        Input Parameter.

    Returns
    -------
    Parameter:
        Object representing the cosine of the parameter.
    """
    return sli_func("cos", parameter)


def min(parameter, value):
    """
    Yields the smallest value of the value of a parameter and a given value

    Parameters
    ----------
    parameter : Parameter
        Input Parameter.
    value : float
        Value to compare against.

    Returns
    -------
    Parameter:
        Object yielding the smallest value.
    """
    return sli_func("min", parameter, float(value))


def max(parameter, value):
    """
    Yields the largest value of the value of a parameter and a given value

    Parameters
    ----------
    parameter : Parameter
        Input Parameter.
    value : float
        Value to compare against.

    Returns
    -------
    Parameter:
        Object yielding the largest value.
    """
    return sli_func("max", parameter, float(value))


def redraw(parameter, min, max):
    """
    Redraws the value of the parameter if it is outside of the given limits

    Both min and max values are included in the limit. If the number of redraws exceeds 1000, an error is thrown.

    Parameters
    ----------
    parameter : Parameter
        Input Parameter.
    min : float
        Lower bound of the value.
    max : float
        Upper bound of the value.

    Returns
    -------
    Parameter:
        Object redrawing the parameter until it can yield a value within the given limits.
    """
    return sli_func("redraw", parameter, float(min), float(max))
