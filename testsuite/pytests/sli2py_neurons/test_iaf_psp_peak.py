# -*- coding: utf-8 -*-
#
# test_iaf_psp_peak.py
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

"""
Name: testsuite::test_iaf_psp_peak - test of closed form expression for peak

Synopsis: (test_iaf_psp_peak) run -> compare expression with numerics

Description:

Several NEST neuron models have an alpha-shaped postsynaptic current (PSC).
In these models the PSC is normalized to unit amplitude. Thus, a synaptic weight
w leads to a PSC with amplitude w in units of pA.
In order to adjust the amplitude of the postsynaptic potential (PSP) of a
neuron model with an alpha-shaped postsynaptic current (PSC) to a particular
amplitude we need to first find the location of the maximum tmax of the PSP.
Here, this is done in two different ways:
1. We numerically search for the root of the derivative of the PSP
2. We used a closed form expression to compute the position of the maximum
The test verifies that the methods lead to the same result. The test file
test_iaf_psp_normalized shows how this value is used to specify w such that a
PSP with a desired amplitude u in units of mV results.

The closed form expression can be found by first transforming the expression
   d psp(t) / dt = 0
into the normal form
   exp(s) = 1 + a * s,
where s is the scaled time s=bt and a and b depend on the time constants
a = tau_m/tau_alpha, b = 1/tau_alpha - 1/tau_m .

The solution for s can then be expressed with the help of the Lambert W-function W
which is the inverse of x=W*exp(W) and reads

  s = 1/a * ( -a W(-exp(-1/a)/a) - 1 )

Test ported from SLI regression test.

References:
  [1] Weisstein, Lambert W-function

Author:  July 2009, Diesmann
SeeAlso: testsuite::test_lambertw, testsuite::test_iaf_psp_normalized, LambertWm1
"""

import numpy as np
import pytest
from scipy.optimize import brentq
from scipy.special import lambertw


def test_iaf_psp_peak():
    """
    Test that closed form expression for PSP peak time matches numerical solution.
    """
    # Parameters of the Brunel network examples
    tau_m = 20.0  # membrane time constant in ms
    tau_syn = 0.5  # synaptic time constant in ms
    C_m = 1.0  # membrane capacity in pF

    # In NEST neuron models with alpha-shaped PSC, the PSCs are normalized to
    # unit amplitude. Thus, a weight w results in a PSC with amplitude w.
    E = 1.0  # unit amplitude

    def psp(t):
        """Postsynaptic potential function."""
        return (
            E
            / tau_syn
            * 1.0
            / C_m
            * (
                (np.exp(-t / tau_m) - np.exp(-t / tau_syn)) / (1.0 / tau_syn - 1.0 / tau_m) ** 2
                - t * np.exp(-t / tau_syn) / (1.0 / tau_syn - 1.0 / tau_m)
            )
        )

    def dpsp(t):
        """Derivative of the postsynaptic potential."""
        return (
            E
            / tau_syn
            * 1.0
            / C_m
            * (
                (-1.0 / tau_m * np.exp(-t / tau_m) + 1.0 / tau_syn * np.exp(-t / tau_syn))
                / (1.0 / tau_syn - 1.0 / tau_m) ** 2
                - (np.exp(-t / tau_syn) - 1.0 / tau_syn * t * np.exp(-t / tau_syn)) / (1.0 / tau_syn - 1.0 / tau_m)
            )
        )

    # Numerical solution: find root of derivative for strictly positive t
    t0 = brentq(dpsp, 0.1, 5.0, xtol=1e-11)

    # Closed form solution using Lambert W function
    a = tau_m / tau_syn
    # Use the -1 branch of Lambert W (lambertw with k=-1)
    w_val = lambertw(-np.exp(-1.0 / a) / a, k=-1)
    t_peak = (-a * w_val - 1.0) / a / (1.0 / tau_syn - 1.0 / tau_m)
    # Convert to real (lambertw returns complex)
    t_peak = float(t_peak.real)

    # Assert that peak times from direct root finding and
    # closed form solution are the same
    assert abs(t_peak - t0) < 1e-10
