# -*- coding: utf-8 -*-
#
# test_iaf_psc_alpha_1to2.py
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
    test_iaf_psc_alpha_1to2 checks the spike interaction of two iaf_psc_alpha model
    neurons.

    In order to obtain identical results for different computation step
    sizes h, the SLI script needs to be independent of h.  This is
    achieved by specifying all time parameters in milliseconds (ms). In
    particular the time of spike emission and the synaptic delay need to
    be integer multiples of the computation step sizes to be
    tested. test_iaf_dc_aligned_delay demonstrates the strategy for the
    case of DC current input.

    A DC current in the pre-synaptic neuron is adjusted to cause a spike
    at a grid position (t=3.0 ms) joined by all computation step sizes to
    be tested.

    Note that in a neuron model where synaptic events are modeled by a
    truncated exponential the effect of the incoming spike would be
    visible at the time of impact (here, t=4.0 ms). This is because the
    initial condition for the postsynaptic potential (PSP) has a
    non-zero voltage component. For PSPs with finite rise time the
    situation is different. In this case the voltage component of the
    initial condition is zero (see documentation of
    test_iaf_psp). Therefore, at the time of impact the PSP is only
    visible in other components of the state vector.

    The expected output is documented and briefly commented at the end of
    the script.
"""
import pytest

import nest
import numpy as np


def test_iaf_psc_alpha_1to2():
    delay = 1.0
    simtime = 8.0

    def run_sim(h):
        nest.ResetKernel()
        nest.local_num_threads = 1
        nest.resolution = h
        neuron1, neuron2 = nest.Create("iaf_psc_alpha", 2)
        neuron1.I_e = 1450.0
        vm = nest.Create("voltmeter")
        vm.interval = h
        sr = nest.Create("spike_recorder")
        nest.Connect(neuron1, neuron2, syn_spec={"weight": 100.0, "delay": delay})
        nest.Connect(vm, neuron2)
        nest.Connect(neuron1, sr)
        nest.Connect(neuron2, sr)
        nest.Simulate(simtime)
        return np.column_stack((vm.events["times"], vm.events["V_m"]))

    expected = np.array([
        [ 2.5          , -70          ], #          <-- Voltage trace of the postsynaptic neuron
        [ 2.6          , -70          ], #               (neuron2), at rest until a spike arrives.
        [ 2.7          , -70          ], #
        [ 2.8          , -70          ], #
        [ 2.9          , -70          ], #
        [ 3.0          , -70          ], #
        [ 3.1          , -70          ], #              spike at t=3.0 ms.
        [ 3.2          , -70          ], #
        [ 3.3          , -70          ], #
        [ 3.4          , -70          ], #
        [ 3.5          , -70          ], #          <--  Synaptic delay of 1.0 ms.
        [ 3.6          , -70          ], #
        [ 3.7          , -70          ], #
        [ 3.8          , -70          ], #
        [ 3.9          , -70          ], #
        [ 4.0          , -70          ], #  <------  Spike arrives at the postsynaptic neuron
        [ 4.1          , -69.9974     ], #     <-    (neuron2) and changes the state vector of
        [ 4.2          , -69.9899     ], #       |   the neuron, not visible in voltage because
        [ 4.3          , -69.9781     ], #       |   voltage of PSP initial condition is 0.
        [ 4.4          , -69.9624     ], #       |
        [ 4.5          , -69.9434     ], #        -  Arbitrarily close to the time of impact
        [ 4.6          , -69.9213     ], #           (t=4.0 ms) the effect of the spike (PSP)
        [ 4.7          , -69.8967     ], #           is visible in the voltage trace.
        [ 4.8          , -69.8699     ], #
        [ 4.9          , -69.8411     ], #
        [ 5.0          , -69.8108     ], #
        [ 5.1          , -69.779      ], #
        [ 5.2          , -69.7463     ], #     <---  The voltage trace is independent
        [ 5.3          , -69.7126     ], #           of the computation step size h.
        [ 5.4          , -69.6783     ], #           Larger step sizes only have fewer
        [ 5.5          , -69.6435     ], #           sample points.
        [ 5.6          , -69.6084     ], #
        [ 5.7          , -69.5732     ], #
    ])

    for h in (.1, .2, .5, 1.0):
        actual = run_sim(h)
        simulated_points = np.isin(actual[:, 0], expected[:, 0])
        expected_points = np.isin(expected[:, 0], actual[:, 0])
        assert actual[simulated_points] == pytest.approx(expected[expected_points])

