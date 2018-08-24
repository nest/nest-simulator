# -*- coding: utf-8 -*-
#
# hl_api_random.py
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

from nest.topology import CreateParameter
from nest import Parameter
import numpy as np


class ParameterWrapper(Parameter):
    def __init__(self, parametertype, specs):
        self._parameter = CreateParameter(parametertype, specs)

    def get_value(self):
        return self._parameter.GetValue([0.0, 0.0])

    def __add__(self, other):
        self._parameter += other
        return self

    def __sub__(self, other):
        self._parameter -= other
        return self

    def __mul__(self, other):
        self._parameter *= other
        return self

    def __div__(self, other):
        self._parameter /= other
        return self

    def __truediv__(self, other):
        self._parameter /= other
        return self


class Exponential(ParameterWrapper):
    def __init__(self, scale=1.0):
        self._scale = scale

    def __add__(self, other):
        return np.random.exponential(self._scale) + other

    def __sub__(self, other):
        return np.random.exponential(self._scale) - other

    def __mul__(self, other):
        return np.random.exponential(self._scale) * other

    def __div__(self, other):
        return np.random.exponential(self._scale) / other

    def get_value(self):
        return np.random.exponential(self._scale)


def uniform(min=0.0, max=1.0):
    return ParameterWrapper('uniform', {'min': min, 'max': max})


def normal(loc=0.0, scale=1.0, min=None, max=None, redraw=False):
    if redraw:
        raise NotImplementedError('Redraw is not supported yet')
    parameters = {'mean': loc, 'sigma': scale}
    if min:
        parameters.update({'min': min})
    if max:
        parameters.update({'max': max})
    return ParameterWrapper('normal', parameters)


def exponential(scale=1.0):
    return Exponential(scale=scale)


def lognormal(mean=0.0, sigma=1.0, min=None, max=None):
    # TODO: mean not the same as mu?
    parameters = {'mu': mean, 'sigma': sigma}
    if min:
        parameters.update({'min': min})
    if max:
        parameters.update({'max': max})
    return ParameterWrapper('lognormal', parameters)
