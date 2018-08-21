# -*- coding: utf-8 -*-
#
# hl_api.py
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

import nest.topology as tp
import numpy as np


class Exponential(tp.nest.Parameter):
    def __init__(self, scale=1.0):
        self.scale = scale

    def GetValue(self, point):
        return np.random.exponential(self.scale)


def uniform(min=0.0, max=1.0):
    return tp.CreateParameter('uniform', {'min': min, 'max': max})


def normal(loc=0.0, scale=1.0, min=None, max=None, redraw=False):
    if redraw:
        raise NotImplementedError('Redraw is not supported yet')
    parameters = {'mean': loc, 'sigma': scale}
    if min:
        parameters.update({'min': min})
    if max:
        parameters.update({'max': max})
    return tp.CreateParameter('normal', parameters)


def exponential(scale=1.0):
    return Exponential(scale=scale)


def lognormal(mean=0.0, sigma=1.0, min=None, max=None):
    # TODO: mean not the same as mu?
    parameters = {'mu': mean, 'sigma': sigma}
    if min:
        parameters.update({'min': min})
    if max:
        parameters.update({'max': max})
    return tp.CreateParameter('lognormal', parameters)
