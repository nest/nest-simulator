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
    """
    Draws samples from a uniform distribution.

    Samples are distributed uniformly in [min, max) (includes min, but excludes max).

    Parameters
    ----------
    min : float, optional
        Lower boundary of the sample interval. Default value is 0.
    max : float, optional
        Upper boundary of the sample interval. Default value is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return CreateParameter('uniform', {'min': min, 'max': max})


def normal(mean=0.0, std=1.0):
    """
    Draws samples from a normal distribution.

    Parameters
    ----------
    mean : float, optional
        Mean of the distribution. Default value is 0.
    std : float, optional
        Standard deviation of the distribution. Default value is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return CreateParameter('normal', {'mean': mean, 'std': std})


def exponential(beta=1.0):
    """
    Draws samples from an exponential distribution.

    Parameters
    ----------
    beta : float, optional
        Scale parameter the distribution. Default value is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return CreateParameter('exponential', {'beta': beta})


def lognormal(mean=0.0, std=1.0):
    """
    Draws samples from a log-normal distribution.

    Parameters
    ----------
    mean : float, optional
        Mean value of the underlying normal distribution. Default value is 0.
    std : float, optional
        Standard deviation of the underlying normal distribution. Default value is 1.0.

    Returns
    -------
    Parameter:
        Object yielding values drawn from the distribution.
    """
    return CreateParameter('lognormal', {'mean': mean, 'std': std})
