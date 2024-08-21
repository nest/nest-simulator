# -*- coding: utf-8 -*-
#
# test_amat2_psc_exp.py
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
Test of amat2_psc_exp neuron model.

This set of tests compare the simulated response to a current step with
reference data.

.. note::
    The `simulation` fixture is run once and the results are used by both
    `test_simulated_spike_times` and `test_simulated_potentials`. The last
    test `test_simulation_with_voltage_dependent_component` resets the kernel
    and runs its own simulation. Therefore, the order of tests in this module
    should not be randomized.
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
    Fixture for simulating the amat2_psc_exp neuron as a mat2_psc_exp.

    This simulation is identical with the one in test_mat2_psc_exp.py and
    ensures that both models behave identically provided the voltage-dependent
    threshold component is deactivated (beta = 0). Since the models have
    different default parameter values, we need to obtain default values from
    mat2_psc_exp.

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

    mat2p = nest.GetDefaults("mat2_psc_exp")
    # must differ from tau_m but is irrelvant for beta=0
    tau_v = mat2p["tau_m"] + 10.0

    nrn = nest.Create(
        "amat2_psc_exp",
        params={
            "omega": -51.0,
            "beta": 0.0,
            "E_L": mat2p["E_L"],
            "I_e": mat2p["I_e"],
            "C_m": mat2p["C_m"],
            "tau_m": mat2p["tau_m"],
            "tau_syn_ex": mat2p["tau_syn_ex"],
            "tau_syn_in": mat2p["tau_syn_in"],
            "t_ref": mat2p["t_ref"],
            "tau_1": mat2p["tau_1"],
            "tau_2": mat2p["tau_2"],
            "alpha_1": mat2p["alpha_1"],
            "alpha_2": mat2p["alpha_2"],
            "tau_v": tau_v,
        },
    )

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

    actual_spike_times = simulation.spike_recorder.events["times"]

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
            # <-   threshold potential the first time.
            [12, -48.2477, -12.3692],
            # |  The threshold potential is updated and
            [13, -46.3023, -12.7346],
            # |  the membrane potential is further evaluated
            [14, -44.3953, -13.0965],
            [15, -42.5262, -13.4548],  # |  without resetting.
            [16, -40.694, -13.8095],  # |
            [17, -38.8982, -14.1607],  # The threshold potential decays double
            # exponential towards its resting level.
            [18, -37.1379, -14.5084],
            [19, -35.4124, -14.8527],
            [20, -33.7212, -15.1935],
            [21, -32.0634, -15.531],
        ],
        columns=["times", "V_m", "V_th"],
    )

    mm_data = simulation.multimeter.events
    actual_potentials = pd.DataFrame.from_dict({key: mm_data[key] for key in ["times", "V_m", "V_th"]})

    pdtest.assert_frame_equal(actual_potentials.iloc[:21], expected_potentials, rtol=1e-05, atol=1e-08)


def test_simulation_with_voltage_dependent_component():
    """
    Compare simulation with beta > 0 to reference data.

    From the other tests we know that the model works including spike detection
    except for the yet untested voltage-dependent component. We study this in
    subthreshold mode where we can obtain exact solutions. We set alpha_1 and
    alpha_2 to non-zero values. As there is no spike, they should not affect
    the result. We also set a constant input current.

    The reference data was obtained with Mathematica.
    """

    nest.ResetKernel()
    nest.resolution = 0.1

    nrn = nest.Create(
        "amat2_psc_exp",
        params={
            "omega": 2.0,
            "beta": 2.0,
            "E_L": 0.0,
            "I_e": 10.0,
            "V_m": -2.0,  # V_m < V_th for t < 10 ms -> no spikes
            "alpha_1": 10.0,
            "alpha_2": 10.0,
        },
    )
    mm = nest.Create(
        "multimeter",
        params={"interval": nest.resolution, "record_from": ["V_m", "V_th", "V_th_v"], "time_in_steps": True},
    )
    nest.Connect(mm, nrn, syn_spec={"weight": 1.0, "delay": nest.resolution})
    nest.Simulate(10.1)

    expected_potentials = pd.DataFrame(
        [
            [10, -1.76209, 2.21168, 0.211679],
            [20, -1.54683, 2.71733, 0.717335],
            [30, -1.35205, 3.36815, 1.36815],
            [40, -1.1758, 4.06297, 2.06297],
            [50, -1.01633, 4.73557, 2.73557],
            [60, -0.872029, 5.34504, 3.34504],
            [70, -0.741463, 5.86852, 3.86852],
            [80, -0.623322, 6.29576, 4.29576],
            [90, -0.516424, 6.62509, 4.62509],
            [100, -0.419699, 6.86044, 4.86044],
        ],
        columns=["times", "V_m", "V_th", "V_th_v"],
    )

    actual_potentials = pd.DataFrame.from_dict({key: mm.events[key] for key in ["times", "V_m", "V_th", "V_th_v"]})

    pdtest.assert_frame_equal(
        actual_potentials.iloc[9:100:10].reset_index(drop=True), expected_potentials, rtol=1e-05, atol=1e-08
    )
