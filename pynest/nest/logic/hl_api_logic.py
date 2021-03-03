# -*- coding: utf-8 -*-
#
# hl_api_logic.py
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
from ..lib.hl_api_types import CreateParameter

__all__ = [
    'conditional',
]


def conditional(condition, param_if_true, param_if_false):
    """
    Yields one value or another, based on the condition.

    Parameters
    ----------
    condition : Parameter
        A comparing Parameter, created with the usual comparators.
    param_if_true : [Parameter | float]
        Value or Parameter used to get a value used if the condition evaluates to true.
    param_if_false : [Parameter | float]
        Value or Parameter used to get a value used if the condition evaluates to false.

    Returns
    -------
    Parameter:
        Object representing the conditional.
    """
    if isinstance(param_if_true, (int, float)):
        param_if_true = CreateParameter(
            'constant', {'value': float(param_if_true)})
    if isinstance(param_if_false, (int, float)):
        param_if_false = CreateParameter(
            'constant', {'value': float(param_if_false)})
    return sli_func("conditional", condition, param_if_true, param_if_false)
