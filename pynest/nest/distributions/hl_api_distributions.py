# -*- coding: utf-8 -*-
#
# hl_api_distributions.py
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

from ..math import exp

try:
    import scipy.special
    HAVE_SCIPY = True
except ImportError:
    HAVE_SCIPY = False


__all__ = [
    'exponential',
    'gaussian',
    'gaussian2D',
    'gamma',
]


def exponential(x, a=1.0, tau=1.0):
    return a * exp(-x/tau)


def gaussian(x, p_center=1.0, mean=0.0, std_deviation=1.0):
    return p_center * exp(-(x-mean)**2/(2*std_deviation**2))


def gaussian2D(x, y, p_center=1.0, mean_x=0.0, mean_y=0.0, std_deviation_x=1.0,
               std_deviation_y=1.0, rho=0.0):
    x_term = (x - mean_x)**2/std_deviation_x**2
    y_term = (y - mean_y)**2/std_deviation_y**2
    xy_term = (x - mean_x)*(y - mean_y)/(std_deviation_x*std_deviation_y)
    return p_center * exp(- (x_term + y_term - 2*rho*xy_term)/(2*(1-rho**2)))


def gamma(x, alpha=1.0, theta=1.0):
    if not HAVE_SCIPY:
        raise ImportError('gamma distribution requires scipy')
    return (x**(alpha - 1) * exp(- x / theta) /
            (theta**alpha * scipy.special.gamma(alpha)))
