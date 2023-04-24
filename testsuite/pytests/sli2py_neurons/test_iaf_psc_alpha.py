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

"""
    test_iaf_psc_alpha.py is an overall test of the iaf_psc_alpha model connected
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

    The expected output is documented and briefly commented at the end of 
    the script.

    Other test programs discuss the various aspects of this script in detail,
    see iaf_psc_alpha, testsuite/test_iaf_i0, testsuite/test_iaf_i0_refractory,
    testsuite/test_iaf_dc
"""
import pytest

import nest
import numpy as np


def test_iaf():
    h = 0.1
    nest.ResetKernel()
    nest.local_num_threads = 1
    nest.resolution = h
    neuron = nest.Create("iaf_psc_alpha")
    dc_gen = nest.Create("dc_generator")
    dc_gen.amplitude= 1000
    vm = nest.Create("voltmeter")
    vm.time_in_steps = True
    vm.interval = h
    sr = nest.Create("spike_recorder")
    sr.time_in_steps = True
    nest.Connect(dc_gen, neuron, syn_spec={"weight": 1.0, "delay": h})
    nest.Connect(vm, neuron, syn_spec={"weight": 1.0, "delay": h})
    nest.Connect(neuron, sr, syn_spec={"weight": 1.0, "delay": h})
    nest.Simulate(8)

    expected = np.array([
        [ 1           , -70          ], # <----- The earliest time dc_gen can be switched on.
        [ 2           , -70          ], # <----- The DC current arrives at the neuron, it is
        [ 3           , -69.602      ], # <-     reflected in the neuron's state variable y0,
        [ 4           , -69.2079     ], #   |    (initial condition) but has not yet affected
        [ 5           , -68.8178     ], #   |    the membrane potential.
        [ 6           , -68.4316     ], #   |
        [ 7           , -68.0492     ], #    --- the effect of the DC current is visible in the
        [ 8           , -67.6706     ], #        membrane potential
        [ 9           , -67.2958     ], #
        [ 10          , -66.9247     ], #
        [ 45          , -56.0204     ], #
        [ 46          , -55.7615     ], #
        [ 47          , -55.5051     ], #
        [ 48          , -55.2513     ], #
        [ 49          , -55.0001     ], #
        [ 50          , -70          ], #  <---- The membrane potential crossed threshold in the
        [ 51          , -70          ], #        step 4.9 ms -> 5.0 ms. The membrane potential is
        [ 52          , -70          ], #        reset (no super-threshold values can be observed).
        [ 53          , -70          ], #        The spike is reported at 5.0 ms
        [ 54          , -70          ], #
        [ 55          , -70          ], #
        [ 56          , -70          ], #
        [ 57          , -70          ], #
        [ 58          , -70          ], #
        [ 59          , -70          ], #
        [ 60          , -70          ], #
        [ 61          , -70          ], #
        [ 62          , -70          ], #
        [ 63          , -70          ], #
        [ 64          , -70          ], #
        [ 65          , -70          ], #
        [ 66          , -70          ], #
        [ 67          , -70          ], #
        [ 68          , -70          ], #
        [ 69          , -70          ], #
        [ 70          , -70          ], #  <---- The last point in time at which the membrane potential
        [ 71          , -69.602      ], #  <-    is clamped. The fact that the neuron is not refractory
        [ 72          , -69.2079     ], #    |   anymore is reflected in the state variable r==0.
        [ 73          , -68.8178     ], #    |   The neuron was refractory for 2.0 ms.
        [ 74          , -68.4316     ], #    |
        [ 75          , -68.0492     ], #    --- The membrane potential starts to increase
        [ 76          , -67.6706     ], #        immediately afterwards and the neuron can generate
        [ 77          , -67.2958     ], #        spikes again (at this resolution reported with time
        [ 78          , -66.9247     ], #        stamp 7.1 ms on the grid)
        [ 79          , -66.5572     ], #  <--
                                        #     |
                                        #     |
                                        #     - The simulation was run for 8.0 ms.However, in the step
                                        #       7.9 ms -> 8.0 ms the voltmeter necessarily receives the
                                        #       voltages that occurred at time 7.9 ms (delay h).This
                                        #       results in different end times of the recorded voltage
                                        #       traces at different resolutions.In the current
                                        #       simulation kernel there is no general cure for this
                                        #       problem.One workaround is to end the simulation script
                                        #       with "h Simulate", thereby making the script resolution
                                        #       dependent.
    ])

    tested = np.isin(vm.events["times"], expected[:, 0])
    times = vm.events["times"][tested]
    V_m = vm.events["V_m"][tested]
    actual = np.column_stack((times, V_m))
    assert actual == pytest.approx(expected)
