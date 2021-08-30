# -*- coding: utf-8 -*-
#
# hl_api_spatial_distributions.py
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
from ..lib.hl_api_types import CreateParameter

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


def exponential(x, beta=1.0):
    """
    Applies an exponential distribution on a Parameter.

    Parameters
    ----------
    x : Parameter
        Input Parameter.
    beta : float, optional
        Scale parameter. Default is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return CreateParameter('exp_distribution', {
        'x': x,
        'beta': beta,
    })


def gaussian(x, mean=0.0, std=1.0):
    """
    Applies a gaussian distribution on a Parameter.

    Parameters
    ----------
    x : Parameter
        Input Parameter.
    mean : float, optional
        Mean of the distribution. Default is 0.0.
    std : float, optional
        Standard deviation of the distribution. Default is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return CreateParameter('gaussian', {
        'x': x,
        'mean': mean,
        'std': std,
    })


def gaussian2D(x, y, mean_x=0.0, mean_y=0.0, std_x=1.0, std_y=1.0, rho=0.0):
    """
    Applies a bivariate gaussian distribution on two Parameters, representing values in the x and y direction.

    Parameters
    ----------
    x : Parameter
        Input Parameter for the x-direction.
    y : Parameter
        Input Parameter for the y-direction.
    mean_x : float, optional
        Mean of the distribution in the x-direction. Default is 0.0.
    mean_y : float, optional
        Mean of the distribution in the y-direction. Default is 0.0.
    std_x : float, optional
        Standard deviation of the distribution in the x-direction. Default is 1.0.
    std_y : float, optional
        Standard deviation of the distribution in the y-direction. Default is 1.0.
    rho : float, optional
        Correlation of x and y. Default is 0.0

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return CreateParameter('gaussian2d', {
        'x': x,
        'y': y,
        'mean_x': mean_x,
        'mean_y': mean_y,
        'std_x': std_x,
        'std_y': std_y,
        'rho': rho,
    })


def gamma(x, kappa=1.0, theta=1.0):
    """
    Applies a gamma distribution on a Parameter.

    This function requires SciPy, and will raise an error if SciPy cannot be imported.

    Parameters
    ----------
    x : Parameter
        Input Parameter.
    kappa : float, optional
        Shape parameter. Default is 1.0.
    theta : float, optional
        Scale parameter. Default is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return CreateParameter('gamma', {
        'x': x,
        'kappa': kappa,
        'theta': theta
    })
