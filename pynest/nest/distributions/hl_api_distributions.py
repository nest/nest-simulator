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
    """
    Applies an exponential distribution on a Parameter.

    Parameters
    ----------
    x : Parameter
        Input Parameter.
    a : float, optional
        Coefficient of the exponential term. Default is 1.0.
    tau : float, optional
        Inverse rate. Default is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return a * exp(-x/tau)


def gaussian(x, p_center=1.0, mean=0.0, std_deviation=1.0):
    """
    Applies a gaussian distribution on a Parameter.

    Parameters
    ----------
    x : Parameter
        Input Parameter.
    p_center : float, optional
        Value at the center of the gaussian. Default is 1.0.
    mean : float, optional
        Mean of the distribution. Default is 0.0.
    std_deviation : float, optional
        Standard deviation of the distribution. Default is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return p_center * exp(-(x-mean)**2/(2*std_deviation**2))


def gaussian2D(x, y, p_center=1.0, mean_x=0.0, mean_y=0.0, std_deviation_x=1.0,
               std_deviation_y=1.0, rho=0.0):
    """
    Applies a bivariate gaussian distribution on two Parameters, representing values in the x and y direction.

    Parameters
    ----------
    x : Parameter
        Input Parameter for the x-direction.
    y : Parameter
        Input Parameter for the y-direction.
    p_center : float, optional
        Value at the center of the gaussian. Default is 1.0.
    mean_x : float, optional
        Mean of the distribution in the x-direction. Default is 0.0.
    mean_y : float, optional
        Mean of the distribution in the y-direction. Default is 0.0.
    std_deviation_x : float, optional
        Standard deviation of the distribution in the x-direction. Default is 1.0.
    std_deviation_y : float, optional
        Standard deviation of the distribution in the y-direction. Default is 1.0.
    rho : float, optional
        Correlation of x and y. Default is 0.0

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    x_term = (x - mean_x)**2/std_deviation_x**2
    y_term = (y - mean_y)**2/std_deviation_y**2
    xy_term = (x - mean_x)*(y - mean_y)/(std_deviation_x*std_deviation_y)
    return p_center * exp(- (x_term + y_term - 2*rho*xy_term)/(2*(1-rho**2)))


def gamma(x, alpha=1.0, theta=1.0):
    """
    Applies a gamma distribution on a Parameter.

    This function requires SciPy, and will raise an error if SciPy cannot be imported.

    Parameters
    ----------
    x : Parameter
        Input Parameter.
    alpha : float, optional
        Shape parameter. Default is 1.0.
    theta : float, optional
        Scale parameter. Default is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    if not HAVE_SCIPY:
        raise ImportError('gamma distribution requires scipy')
    return (x**(alpha - 1) * exp(- x / theta) /
            (theta**alpha * scipy.special.gamma(alpha)))
