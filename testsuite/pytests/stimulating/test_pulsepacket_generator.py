# -*- coding: utf-8 -*-
#
# test_pulsepacket_generator.py
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
Test parameter setting and correct number of spikes emitted by `pulsepacket_generator`.
"""

import nest
import numpy as np
import pandas as pd
import pandas.testing as pdtest
import pytest


@pytest.fixture(autouse=True)
def reset():
    nest.ResetKernel()


def test_set_params_on_instance_and_model():
    """
    Test setting `pulsepacket_generator` parameters on instance and model.
    """

    params = {"pulse_times": [1.0, 2.5, 4.6], "activity": 150, "sdev": 0.1234}

    # Set params on instance
    ppg1 = nest.Create("pulsepacket_generator")
    ppg1.set(params)

    # Set params on model
    nest.SetDefaults("pulsepacket_generator", params)
    ppg2 = nest.Create("pulsepacket_generator")

    # Verify that both ways to set params give same result
    df_pgg1 = pd.DataFrame.from_dict(ppg1.get(params.keys()))
    df_pgg2 = pd.DataFrame.from_dict(ppg2.get(params.keys()))
    pdtest.assert_frame_equal(df_pgg1, df_pgg2)


@pytest.mark.parametrize("params", [{"sdev": -5.0}, {"activity": -5}])
def test_set_illegal_values(params):
    """
    Test that an error is raised if `sdev` or `activity` is set to negative value.
    """

    with pytest.raises(nest.NESTErrors.BadProperty):
        nest.SetDefaults("pulsepacket_generator", params)


def test_valid_to_pass_empty_pulse_times():
    """
    Assure that a `pulsepacket_generator` with empty `pulse_times` can be simulated.
    """

    params = {"pulse_times": np.array([]), "activity": 0, "sdev": 0.0}
    ppg = nest.Create("pulsepacket_generator")
    ppg.set(params)
    nest.Simulate(1.0)


def test_number_of_spikes():
    """
    Test that `pulsepacket_generator` emits expected number of spikes.

    The test builds and simulates a system with `pulsepacket_generator`
    connected to a `spike_recorder`. The test checks the number of
    recorded spikes against our expectation.
    """

    nest.resolution = 0.1

    tstart = 75.0
    tstop = 225.0
    nspk = 10
    pulset = np.array([10.0, 125.0, 175.0, 275.0])
    stddev = 5.0

    # Find number of pulse centers in [tstart, tstop]
    npulseff = np.count_nonzero(np.logical_and(pulset >= tstart, pulset <= tstop))

    # Since tstart, tstop are far from pulse times, it is highly likely that
    # only spikes belonging to the pulses with centers in [tstart, tstop] are
    # fired and then we get for the total spike number
    npsktot = nspk * npulseff

    # Build and simulate system
    params = {
        "start": tstart,
        "stop": tstop,
        "pulse_times": pulset,
        "activity": nspk,
        "sdev": stddev,
    }
    ppg = nest.Create("pulsepacket_generator", params=params)
    sr = nest.Create("spike_recorder")

    nest.Connect(ppg, sr)

    nest.Simulate(300.0)

    actual_spikes = sr.events["times"]

    # Check that min and max are inside [tstart, tstop]
    assert np.min(actual_spikes) >= tstart
    assert np.max(actual_spikes) <= tstop

    # Check number of spikes
    assert len(actual_spikes) == npsktot
