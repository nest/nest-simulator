# -*- coding: utf-8 -*-
#
# test_mat2_psc_exp.py
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
Test of mat2_psc_exp neuron model.

This set of tests compare the simulated response to a current step with
reference data.
"""

from collections import namedtuple

import nest
import numpy as np
import numpy.testing as nptest
import pandas as pd
import pandas.testing as pdtest
import pytest


@pytest.fixture(scope="module")
def simulation():
    """
    Fixture for simulating the mat2_psc_exp neuron model.

    A DC current is injected into the neuron using a current generator device.
    The membrane potential, the threshold potential as well as the spiking
    activity are recorded by corresponding devices.

    It can be observed how the current charges the membrane, a spike is
    emitted, the threshold potential is updated and evaluated whereas the
    membrane potential is not reset and further evaluated, too. Additionally
    the neuron becomes refractory after emitting a spike.

    The timing of the various events on the simulation grid is of particular
    interest and crucial for the consistency of the simulation scheme.

    .. note::
        We explicitly set threshold omega here to ensure that after change of
        definition of omega from relative to E_L to absolute we still get the
        original results; see #506.
    """

    nest.resolution = 0.1

    nrn = nest.Create("mat2_psc_exp", params={"omega": -51.0})
    dc_gen = nest.Create("dc_generator", params={"amplitude": 2400.0})
    mm = nest.Create(
        "multimeter", params={"interval": nest.resolution, "record_from": ["V_m", "V_th"], "time_in_steps": True}
    )
    srec = nest.Create("spike_recorder", params={"time_in_steps": True})

    nest.Connect(dc_gen, nrn, syn_spec={"weight": 1.0, "delay": nest.resolution})
    nest.Connect(mm, nrn, syn_spec={"weight": 1.0, "delay": nest.resolution})
    nest.Connect(nrn, srec, syn_spec={"weight": 1.0, "delay": nest.resolution})

    nest.Simulate(8.0)

    Devices = namedtuple("Devices", "spike_recorder, multimeter")
    return Devices(srec, mm)


def test_simulated_spike_times(simulation):
    """
    Compare recorded spike times with reference data.
    """

    # The threshold potential crosses the membrane potential at time step 31,
    # but due to the refractory period of 2 ms the neuron is not allowed to
    # fire until time step 32.
    expected_spike_times = [11, 32, 54]

    spike_data = simulation.spike_recorder.events
    actual_spike_times = spike_data["times"]

    nptest.assert_array_equal(actual_spike_times, expected_spike_times)


def test_simulated_potentials(simulation):
    """
    Compare recorded potentials with reference data.
    """

    expected_potentials = pd.DataFrame(
        [
            [1, -70, -51],  # <--- dc_gen is switched on
            [2, -70, -51],  # <--- The DC current arrives at neuron,
            [3, -67.6238, -51],  # <- but has not affected the potential yet
            [4, -65.2947, -51],  # |
            [5, -63.0117, -51],  # First evaluation of the DC current. The
            [6, -60.774, -51],  # threshold potential stays constant,
            [7, -58.5805, -51],  # because it is at its resting level.
            [8, -56.4305, -51],
            [9, -54.323, -51],
            [10, -52.2573, -51],
            [11, -50.2324, -12],  # <--- The membrane potential crossed the
            [12, -48.2477, -12.3692],  # <-   threshold potential the first time.
            [13, -46.3023, -12.7346],  # |  The threshold potential is updated and
            [14, -44.3953, -13.0965],  # |  the membrane potential is further evaluated
            [15, -42.5262, -13.4548],  # |  without resetting.
            [16, -40.694, -13.8095],  # |
            [17, -38.8982, -14.1607],  # The threshold potential decays double
            [18, -37.1379, -14.5084],  # exponential towards its resting level.
            [19, -35.4124, -14.8527],
            [20, -33.7212, -15.1935],
            [21, -32.0634, -15.531],
        ],
        columns=["times", "V_m", "V_th"],
    )

    mm_data = simulation.multimeter.events
    actual_potentials = pd.DataFrame.from_dict({key: mm_data[key] for key in ["times", "V_m", "V_th"]})

    pdtest.assert_frame_equal(actual_potentials.iloc[:21], expected_potentials, rtol=1e-05, atol=1e-08)
