# -*- coding: utf-8 -*-
#
# test_spike_poisson_ps.py
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
Test that spike times are independent of resolution.

This test connects a `poisson_generator_ps` to a `spike_recorder` and
performs simulations for different time resolutions. The difference in spike
times should be exactly 0.0 between all resolutions. The test checks whether
the spike times indeed are independent of the resolution.
"""

import nest
import numpy as np
import numpy.testing as nptest
import pytest


def simulator(resolution):
    """
    Simulate the system with provided time resolution and return spike times.
    """

    nest.ResetKernel()

    nest.resolution = resolution

    # End of stimulation period
    stim_time = 30.0

    # We need to ensure that all spikes arrive at the spike recorder.
    # Thus, the simulation time needs to exceed the end of stimulation
    # by at least two multiples of the delay, which is 1 ms by default.
    # We also need to ensure that the simulation time is a multiple of
    # the resolution.
    # To be on the safe side, we add 3 ms and then round up to the
    # nearest multiple of the resolution.
    sim_time = stim_time + np.ceil(3.0 / resolution) * resolution

    pgen_ps = nest.Create("poisson_generator_ps", params={"rate": 128921.3, "stop": stim_time})
    srec = nest.Create("spike_recorder")
    nest.Connect(pgen_ps, srec)

    nest.Simulate(sim_time)

    return srec.events["times"]


@pytest.fixture(scope="module")
def reference_spikes():
    """
    Fixture that simulates reference spikes.
    """

    ref_spikes = simulator(0.1)
    return ref_spikes


@pytest.mark.parametrize("resolution", [0.03, 0.02, 0.01, 0.001])
def test_spikes_independent_of_resolution(reference_spikes, resolution):
    """
    Ensure that spike times are independent of resolution.

    The `poisson_generator_ps` generates spikes without attention to resolution.
    This means that exactly the same sequence of spike times is generated
    internally in the generator independent of resolution. Rounding errors occur,
    if at all, only when the spike time is split into a multiple of the
    resolution and the offset. In this calculation, the resolution enters as a
    fixed multiple of tics. The calculation is reversed in the spike recorder,
    when step-multiple and offset are recombined. This splitting and
    recombination apparently gives reproducible results.

    The test simulates the system for different resolutions and compares the
    spike times against reference spikes. To account for rounding errors, the
    error tolerance is set to 1e-14.
    """

    actual_spikes = simulator(resolution)
    nptest.assert_almost_equal(actual_spikes, reference_spikes, decimal=14)
