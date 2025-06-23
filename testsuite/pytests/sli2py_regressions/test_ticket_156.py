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

    def single_trial(h):
        """
        Returns concatenated spike times recorded by the two spike recorders.

        Asserts that single spike trains fulfill requirements.
        """

        nest.ResetKernel()
        nest.resolution = h

        pg = nest.Create("poisson_generator_ps", params=ps_params)
        srs = nest.Create("spike_recorder", n=2)
        nest.Connect(pg, srs, syn_spec={"delay": 1.0, "weight": 1.0})

        nest.Simulate(simtime)

        spike_times = srs.get("events", "times")

        # Test that the two recorders recorded different times
        assert not np.array_equal(*spike_times)

        # Test that all spike times are in (start, stop]
        all_spike_times = np.concatenate(spike_times)
        assert all(all_spike_times > ps_params["origin"] + ps_params["start"])
        assert all(all_spike_times <= ps_params["origin"] + ps_params["stop"])

        return all_spike_times

    # Run trials for each resolution and check all resolutions yield identical spike trains
    results = [single_trial(h) for h in resolutions]
    for train in results[1:]:
        assert np.allclose(train, results[0], atol=1e-15)
