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

from ..lib.hl_api_types import CreateParameter

__all__ = [
    'exponential',
    'lognormal',
    'normal',
    'uniform',
]


def uniform(min=0.0, max=1.0):
    return CreateParameter('uniform', {'min': min, 'max': max})


def normal(loc=0.0, scale=1.0):
    return CreateParameter('normal', {'mean': loc, 'sigma': scale})


def exponential(scale=1.0):
    return CreateParameter('exponential', {'scale': scale})


def lognormal(mu=0.0, sigma=1.0):
    return CreateParameter('lognormal', {'mu': mu, 'sigma': sigma})
