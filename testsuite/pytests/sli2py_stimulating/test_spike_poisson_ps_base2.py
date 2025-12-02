# -*- coding: utf-8 -*-
#
# test_spike_poisson_ps_base2.py
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
Name: testsuite::test_spike_poisson_ps_base2.sli

Synopsis: (test_spike_poisson_ps_base2) run

Description:
In NEST spike times are represented by an integer part in units of h
and an offset in units of milliseconds. This helps to maintain a
uniform absolute accuracy of spike times even for long simulation
times.

The user has access to the two components of the spike if the spike
recorder is set to /time_in_steps true.
In this case the spike_recorder returns events with the properties
/times and /offsets, where /times are the integer parts s in units of h
and offsets are the fractional parts o in milliseconds. According to
NEST's definition of a grid-constrained spike as a spike occuring
somewhere in (t-h,t], the precise spike time is
   t = s*h - o

Access to spike times with a uniform absolute accuracy is benefitial
when testing and comparing integrators for neuron models, see appendix
A.2 of [1] for details.

This script tests whether the accuracy of spike times is maintained
independent of the choice of computation step size h.

This assumes that also the poisson generator is capable of maintaining
the accuracy independent of computation step size.
If this test fails go back to
 test_spike_poisson_ps.sli
to check whether poisson_generator_ps can emit spike times at double
precision or whether spike times are limited to the precision of a
tic.

Test ported from SLI regression test.

References:
[1] Morrison A, Straube S, Plesser H E, Diesmann M (2007) Exact
subthreshold integration with continuous spike times in discrete time
neural network simulations. Neural Computation 19: 47-79

Author: May 2010, adapted to NEST2, Diesmann
SeeAlso: testsuite::test_spike_poisson_ps
"""

import nest
import numpy as np
import pytest


def test_spike_poisson_ps_base2():
    """
    Test that spike time accuracy is maintained independent of computation step size.
    """
    min_exponent = -20
    spike_absolute_accuracy = 1e-15
    T = 4.0

    h_min = 2.0**min_exponent

    def transmission(h):
        """Run simulation with given resolution and return spike times and offsets."""
        nest.ResetKernel()
        tics_per_ms = 2.0 ** (-min_exponent)
        nest.set(tics_per_ms=tics_per_ms, resolution=h)

        sp = nest.Create("spike_recorder", params={"time_in_steps": True})
        pn = nest.Create("poisson_generator_ps", params={"rate": 16384.0})

        nest.Connect(pn, sp)

        nest.Simulate(T)

        events = sp.get("events")
        times = events["times"]
        offsets = events["offsets"]
        return times, offsets

    # Test with different resolutions
    # Note: resolution must be >= one tic (h_min), so we stop at min_exponent
    results = []
    for exp_offset in range(0, min_exponent - 1, -1):
        # ratio (integer) of h to smallest h
        ratio = 2.0 ** (exp_offset - min_exponent)
        h = 2.0**exp_offset
        times, offsets = transmission(h)

        # Convert all time steps to units of smallest h
        times_scaled = times * ratio
        results.append((times_scaled, offsets))

    # Compare results with the one at smallest h as reference
    reference_times, reference_offsets = results[-1]

    # Compute differences: (s*hmin-o)-(sr*hmin-or)
    differences = []
    for times_scaled, offsets in results[:-1]:  # All except reference
        for i, (s, o) in enumerate(zip(times_scaled, offsets)):
            if i < len(reference_times):
                sr = reference_times[i]
                or_val = reference_offsets[i]
                # Compute difference: (s-sr)*hmin - o + or
                diff = (s - sr) * h_min - o + or_val
                differences.append(abs(diff))

    # Check that all differences are within absolute accuracy
    max_diff = max(differences) if differences else 0.0
    assert max_diff <= spike_absolute_accuracy, (
        f"Maximum difference {max_diff} exceeds accuracy " f"{spike_absolute_accuracy}"
    )
