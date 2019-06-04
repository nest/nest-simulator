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

from ..lib.hl_api_types import Parameter, CreateParameter
from ..ll_api import sli_func
import numpy as np

__all__ = [
    'exponential',
    'lognormal',
    'normal',
    'uniform',
]

# TODO: Can ParameterWrapper be removed?


class ParameterWrapper(Parameter):
    def __init__(self, parametertype, specs):
        self._parameter = CreateParameter(parametertype, specs)

    def get_value(self):
        return self._parameter.GetValue()

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


def uniform(min=0.0, max=1.0, dimension=None):
    if isinstance(min, (list, tuple)):
        if not isinstance(max, (list, tuple)):
            raise ValueError('Max must be list/tuple.')
        if not len(min) == len(max):
            raise ValueError(
                'Min and max must have same number of dimensions.')
        parameters = [
            CreateParameter('uniform', {'min': min_val, 'max': max_val})
            for min_val, max_val in zip(min, max)]
        if len(parameters) == 2:
            return sli_func('dimension2d', *parameters)
        elif len(parameters) == 3:
            return sli_func('dimension3d', *parameters)
        else:
            raise ValueError('Must be 2 or 3 dimensional.')
    elif dimension:
        parameters = [
            CreateParameter('uniform', {'min': min, 'max': max})
            for _ in range(dimension)]
        if dimension == 2:
            return sli_func('dimension2d', *parameters)
        elif dimension == 3:
            return sli_func('dimension3d', *parameters)
        else:
            raise ValueError('Must be 2 or 3 dimensional.')
    else:
        return CreateParameter('uniform', {'min': min, 'max': max})


def normal(loc=0.0, scale=1.0, min=None, max=None,
           redraw=False, dimension=None):
    if redraw:
        raise NotImplementedError('Redraw is not supported yet')
    parameters = {'mean': loc, 'sigma': scale}
    if min:
        parameters.update({'min': min})
    if max:
        parameters.update({'max': max})

    if isinstance(loc, (list, tuple)):
        if not isinstance(scale, (list, tuple)):
            raise ValueError('scale must be list/tuple.')
        if not len(loc) == len(scale):
            raise ValueError(
                'loc and scale must have same number of dimensions.')

        params = [CreateParameter('normal', dict(zip(parameters, t)))
                  for t in zip(*parameters.values())]

        if len(params) == 2:
            return sli_func('dimension2d', *params)
        elif len(params) == 3:
            return sli_func('dimension3d', *params)
        else:
            raise ValueError('Must be 2 or 3 dimensional.')
    elif dimension:
        parameters = [
            CreateParameter('normal', parameters) for _ in range(dimension)]
        if dimension == 2:
            return sli_func('dimension2d', *parameters)
        elif dimension == 3:
            return sli_func('dimension3d', *parameters)
        else:
            raise ValueError('Must be 2 or 3 dimensional.')
    else:
        return CreateParameter('normal', parameters)


def exponential(scale=1.0, dimension=None):
    if isinstance(scale, (list, tuple)):
        parameters = [
            CreateParameter('exponential', {'scale': scale_val})
            for scale_val in scale]
        if len(parameters) == 2:
            return sli_func('dimension2d', *parameters)
        elif len(parameters) == 3:
            return sli_func('dimension3d', *parameters)
        else:
            raise ValueError('Must be 2 or 3 dimensional.')
    elif dimension:
        parameters = [
            CreateParameter('exponential', {'scale': scale})
            for _ in range(dimension)]
        if dimension == 2:
            return sli_func('dimension2d', *parameters)
        elif dimension == 3:
            return sli_func('dimension3d', *parameters)
        else:
            raise ValueError('Must be 2 or 3 dimensional.')
    else:
        return CreateParameter('exponential', {'scale': scale})


def lognormal(mean=0.0, sigma=1.0, min=None, max=None, dimension=None):
    # TODO: mean not the same as mu?
    parameters = {'mu': mean, 'sigma': sigma}
    if min:
        parameters.update({'min': min})
    if max:
        parameters.update({'max': max})

    if isinstance(mean, (list, tuple)):
        if not isinstance(sigma, (list, tuple)):
            raise ValueError('sigma must be list/tuple.')
        if not len(mean) == len(sigma):
            raise ValueError(
                'mean and sigma must have same number of dimensions.')

        params = [CreateParameter('lognormal', dict(zip(parameters, t)))
                  for t in zip(*parameters.values())]

        if len(params) == 2:
            return sli_func('dimension2d', *params)
        elif len(params) == 3:
            return sli_func('dimension3d', *params)
        else:
            raise ValueError('Must be 2 or 3 dimensional.')
    elif dimension:
        parameters = [
            CreateParameter('lognormal', parameters) for _ in range(dimension)]
        if dimension == 2:
            return sli_func('dimension2d', *parameters)
        elif dimension == 3:
            return sli_func('dimension3d', *parameters)
        else:
            raise ValueError('Must be 2 or 3 dimensional.')
    else:
        return CreateParameter('lognormal', parameters)
