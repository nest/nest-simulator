# -*- coding: utf-8 -*-
#
# test_ticket_156.py
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
import nest
import numpy as np
import pytest


def test_ticket_156():
    """
    Verify that poisson_generator_ps generates identical spike trains for given
    start and stop times across different simulation resolutions and different targets.

    Protocol:
    1. poisson_generator_ps projects to two spike_recorders.
    2. Run for different resolutions, record precise spike times in ms.
    3. Test all spikes are in (start, stop].
    4. Test for different results between spike_recorders.
    5. Test for identical results across resolutions.
    """

    ps_params = {"origin": 0.0, "start": 1.0, "stop": 2.0, "rate": 12345.0}  # Expect ~ 12.3 spikes

    resolutions = [0.01, 0.1, 0.2, 0.5, 1.0]
    simtime = ps_params["stop"] + 2.0

    def check_limits(spks):
        """Check if all spikes are within the specified limits."""
        ori = ps_params["origin"]
        return np.min(spks) > ps_params["start"] + ori and np.max(spks) <= ps_params["stop"] + ori

    def single_trial(h):
        """Run a single trial with a given resolution."""
        nest.ResetKernel()
        nest.SetKernelStatus({"resolution": h})

        pg = nest.Create("poisson_generator_ps", params=ps_params)
        srs = [nest.Create("spike_recorder") for _ in range(2)]

        for sr in srs:
            nest.Connect(pg, sr, syn_spec={"delay": 1.0, "weight": 1.0})

        nest.Simulate(simtime)
        return [nest.GetStatus(sr, "events")[0]["times"] for sr in srs]

    # Run trials for each resolution
    results = [single_trial(h) for h in resolutions]

    # First test: Check limits
    assert all(check_limits(np.concatenate(trial)) for trial in results), "Spike times out of bounds"

    # Second test: Different results between targets
    assert all(not np.array_equal(trial[0], trial[1]) for trial in results), "Spike recorders have identical results"

    # Third test: Equality among runs
    # Compare spike times across resolutions for each recorder separately
    for i in range(2):
        spike_times_across_resolutions = [trial[i] for trial in results]
        reference_times = spike_times_across_resolutions[0]
        assert all(
            np.allclose(reference_times, times, atol=1e-15) for times in spike_times_across_resolutions[1:]
        ), "Spike times not consistent across resolutions"
