# -*- coding: utf-8 -*-
#
# test_iaf_psc_alpha.py
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
import testutil


def test_iaf_psc_alpha_basic():
    """
    An overall test of the iaf_psc_alpha model connected
    to some useful devices.

    A DC current is injected into the neuron using a current generator
    device. The membrane potential as well as the spiking activity are
    recorded by corresponding devices.

    It can be observed how the current charges the membrane, a spike
    is emitted, the neuron becomes absolute refractory, and finally
    starts to recover.

    The timing of the various events on the simulation grid is of
    particular interest and crucial for the consistency of the
    simulation scheme.

    Although 0.1 cannot be represented in the IEEE double data type, it
    is safe to simulate with a resolution (computation step size) of 0.1
    ms because by default nest is built with a timebase enabling exact
    representation of 0.1 ms.
    """
    nest.ResetKernel()

    # Simulation variables
    duration = 8.0
    resolution = 0.1
    delay = 1.0

    # Create neuron model
    neuron = nest.Create("iaf_psc_alpha")

    # Create devices
    voltmeter = nest.Create("voltmeter")
    dc_generator = nest.Create("dc_generator")

    voltmeter.interval = resolution
    dc_generator.amplitude = 1000.0

    # Connect devices
    nest.Connect(voltmeter, neuron, syn_spec={"weight": 1.0, "delay": delay})
    nest.Connect(dc_generator, neuron, syn_spec={"weight": 1.0, "delay": resolution})

    # Simulate
    nest.Simulate(duration)

    # Collect spikes from spike_recorder
    voltage = np.column_stack((voltmeter.events["times"], voltmeter.events["V_m"]))

    # Compare actual and expected spike times
    actual, expected = testutil.get_comparable_timesamples(
        resolution,
        voltage,
        np.array(
            [
                [0.1, -70],
                [0.2, -70],
                [0.3, -69.602],
                [0.4, -69.2079],
                [0.5, -68.8178],
                [0.6, -68.4316],
                [0.7, -68.0492],
                [0.8, -67.6706],
                [0.9, -67.2958],
                [1.0, -66.9247],
                [4.5, -56.0204],
                [4.6, -55.7615],
                [4.7, -55.5051],
                [4.8, -55.2513],
                [4.9, -55.0001],
                [5.0, -70],
                [5.1, -70],
                [5.2, -70],
                [5.3, -70],
                [5.4, -70],
                [5.5, -70],
                [5.6, -70],
                [5.7, -70],
                [5.8, -70],
                [5.9, -70],
                [6.0, -70],
                [6.1, -70],
                [6.2, -70],
                [6.3, -70],
                [6.4, -70],
                [6.5, -70],
                [6.6, -70],
                [6.7, -70],
                [6.8, -70],
                [6.9, -70],
                [7.0, -70],
                [7.1, -69.602],
                [7.2, -69.2079],
                [7.3, -68.8178],
                [7.4, -68.4316],
                [7.5, -68.0492],
                [7.6, -67.6706],
                [7.7, -67.2958],
                [7.8, -66.9247],
                [7.9, -66.5572],
            ]
        ),
    )

    # Assert approximate equality, based on number of digits in reference data
    np.testing.assert_allclose(actual, expected, rtol=1e-6)


def test_iaf_psc_alpha_i0():
    """
    Test of a specific feature of the iaf_psc_alpha
    model. It is tested whether an internal DC current that is present
    from the time of neuron initialization, correctly affects the membrane
    potential.

    This is probably the simplest setup in which we can study how the
    dynamics develops from an initial condition.

    When the DC current is supplied by a device external to the neuron
    the situation is more complex because additional delays are introduced.
    """
    nest.ResetKernel()

    # Simulation variables
    resolution = 0.1
    delay = resolution
    duration = 8.0

    # Create neuron and devices
    neuron = nest.Create("iaf_psc_alpha", params={"I_e": 1000.0})
    voltmeter = nest.Create("voltmeter")
    voltmeter.interval = resolution
    spike_recorder = nest.Create("spike_recorder")

    # Connect devices
    nest.Connect(voltmeter, neuron, syn_spec={"weight": 1.0, "delay": delay})
    nest.Connect(neuron, spike_recorder, syn_spec={"weight": 1.0, "delay": delay})

    # Simulate
    nest.Simulate(duration)

    # Get voltmeter output
    results = np.column_stack((voltmeter.events["times"], voltmeter.events["V_m"]))

    # Get spike times
    spikes = np.column_stack(
        (
            spike_recorder.events["senders"],
            spike_recorder.events["times"],
        )
    )

    # Compare voltmeter output to expected
    actual, expected = testutil.get_comparable_timesamples(
        resolution,
        results,
        np.array(
            [
                [0.1, -69.602],
                [0.2, -69.2079],
                [0.3, -68.8178],
                [0.4, -68.4316],
                [0.5, -68.0492],
                [4.3, -56.0204],
                [4.4, -55.7615],
                [4.5, -55.5051],
                [4.6, -55.2513],
                [4.7, -55.0001],
                [4.8, -70],
                [4.9, -70],
                [5.0, -70],
            ]
        ),
    )
    np.testing.assert_allclose(actual, expected, rtol=1e-6)

    # Compare spike times
    assert spikes == pytest.approx(
        np.array(
            [
                [1, 4.8],
            ]
        )
    )


@pytest.mark.parametrize("resolution", [0.1, 0.2, 0.5, 1.0])
def test_iaf_psc_alpha_i0_refractory(resolution):
    """
    Test a specific feature of the iaf_psc_alpha model.

    It is tested whether the voltage traces of simulations
    carried out at different resolutions (computation step sizes) are well
    aligned and identical when the neuron recovers from refractoriness.

    In grid based simulation a prerequisite is that the spike is reported at
    a grid position shared by all the resolutions compared.

    Here, we compare resolutions 0.1, 0.2, 0.5, and 1.0 ms. Therefore, the
    internal DC current is adjusted such (1450.0 pA) that the spike is
    reported at time 3.0 ms, corresponding to computation step 30, 15, 6,
    and 3, respectively.

    The results are consistent with those of iaf_psc_alpha_ps capable of
    handling off-grid spike timing when the interpolation order is set to
    0.
    """
    # Simulation variables
    delay = resolution
    duration = 8.0

    nest.ResetKernel()
    nest.resolution = resolution

    neuron = nest.Create("iaf_psc_alpha", params={"I_e": 1450.0})
    voltmeter = nest.Create("voltmeter", params={"interval": resolution})
    spike_recorder = nest.Create("spike_recorder")

    nest.Connect(voltmeter, neuron, syn_spec={"weight": 1.0, "delay": delay})
    nest.Connect(neuron, spike_recorder, syn_spec={"weight": 1.0, "delay": delay})

    nest.Simulate(duration)

    times = voltmeter.events["times"]
    V_m = voltmeter.events["V_m"]
    results = np.column_stack((times, V_m))

    actual, expected = testutil.get_comparable_timesamples(
        resolution,
        results,
        np.array(
            [
                [0.1, -69.4229],
                [0.2, -68.8515],
                [0.3, -68.2858],
                [0.4, -67.7258],
                [0.5, -67.1713],
                [0.6, -66.6223],
                [0.7, -66.0788],
                [0.8, -65.5407],
                [0.9, -65.008],
                [1.0, -64.4806],
                [1.1, -63.9584],
                [1.2, -63.4414],
                [1.3, -62.9295],
                [1.4, -62.4228],
                [1.5, -61.9211],
                [1.6, -61.4243],
                [1.7, -60.9326],
                [1.8, -60.4457],
                [1.9, -59.9636],
                [2.0, -59.4864],
                [2.1, -59.0139],
                [2.2, -58.5461],
                [2.3, -58.0829],
                [2.4, -57.6244],
                [2.5, -57.1704],
                [2.6, -56.721],
                [2.7, -56.276],
                [2.8, -55.8355],
                [2.9, -55.3993],
                [3.0, -70],
                [3.1, -70],
                [3.2, -70],
                [3.3, -70],
                [3.4, -70],
                [3.5, -70],
                [3.6, -70],
                [3.7, -70],
                [3.8, -70],
                [3.9, -70],
                [4.0, -70],
                [4.1, -70],
                [4.2, -70],
                [4.3, -70],
                [4.4, -70],
                [4.5, -70],
                [4.6, -70],
                [4.7, -70],
                [4.8, -70],
                [4.9, -70],
                [5.0, -70],
                [5.1, -69.4229],
                [5.2, -68.8515],
                [5.3, -68.2858],
                [5.4, -67.7258],
                [5.5, -67.1713],
                [5.6, -66.6223],
                [5.7, -66.0788],
                [5.8, -65.5407],
                [5.9, -65.008],
                [6.0, -64.4806],
                [6.1, -63.9584],
                [6.2, -63.4414],
                [6.3, -62.9295],
                [6.4, -62.4228],
                [6.5, -61.9211],
                [6.6, -61.4243],
                [6.7, -60.9326],
                [6.8, -60.4457],
                [6.9, -59.9636],
            ]
        ),
    )
    np.testing.assert_allclose(actual, expected, rtol=1e-6)
