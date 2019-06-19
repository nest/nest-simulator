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
    'power',
]

# TODO: Special cases when argument is a number?


def exp(parameter):
    return sli_func("exp", parameter)


def sin(parameter):
    return sli_func("sin", parameter)


def cos(parameter):
    return sli_func("cos", parameter)


def power(parameter, exponent):
    return sli_func("pow", parameter, float(exponent))
