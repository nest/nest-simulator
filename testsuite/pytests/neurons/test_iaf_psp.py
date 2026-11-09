# -*- coding: utf-8 -*-
#
# test_iaf_psp.py
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
Name: testsuite::test_iaf_psp - sli script for test of iaf_psc_alpha spike input

Synopsis: (test_iaf_psp) run -> compare response with reference data

Description:

 test_iaf_psp.sli checks the voltage response of the iaf_psc_alpha
 model neuron to a single incoming spike. The voltage excursion is
 called postsynaptic potential (PSP). In the iaf_psc_alpha model neuron
 the postsynaptic current is described by an alpha-function
 (see [1] and references therein). The resulting PSP has a finite
 rise-time, with voltage and current being zero in the initial
 condition (see [1]).

 The dynamics is tested by connecting a device that emits spikes
 at individually configurable times (see test_spike_generator) to
 a model neuron.

 The weight of the connection specifies the peak value (amplitude)
 of the postsynaptic current (PSC) in pA.

 The subthreshold dynamics of the iaf_psc_alpha is integrated exactly.
 Therefore, it is suitable to check whether the simulation kernel
 produces results independent of the computation step size
 (resolution).

 In order to obtain identical results for different computation
 step sizes h, the SLI script needs to be independent of h.
 This is achieved by specifying all time parameters in milliseconds
 (ms). In particular the time of spike emission and the synaptic
 delay need to be integer multiples of the computation step sizes
 to be tested. test_iaf_dc_aligned_delay demonstrates the strategy
 for the case of DC current input.

 The expected output is documented and briefly commented at the end of
 the script.

Test ported from SLI unittest.

References:
  [1] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
      systems with applications to neuronal modeling. Biologial Cybernetics
      81:381-402.

Author:  July 2004, Diesmann
SeeAlso: testsuite::test_iaf, testsuite::test_iaf_dc_aligned_delay, testsuite::test_spike_generator
"""

import nest
import numpy as np
import pytest
import testutil

# Reference data extracted from SLI test comments
# Times are in steps and will be converted to milliseconds by multiplying by resolution
REFERENCE_DATA = np.array(
    [
        [1, -70.0],
        [2, -70.0],
        [3, -70.0],
        [4, -70.0],
        [5, -70.0],
        [6, -70.0],
        [7, -70.0],
        [8, -70.0],
        [9, -70.0],
        [10, -70.0],
        [11, -70.0],
        [12, -70.0],
        [13, -70.0],
        [14, -70.0],
        [15, -70.0],
        [16, -70.0],
        [17, -70.0],
        [18, -70.0],
        [19, -70.0],
        [20, -70.0],
        [21, -70.0],
        [22, -70.0],
        [23, -70.0],
        [24, -70.0],
        [25, -70.0],
        [26, -70.0],
        [27, -70.0],
        [28, -70.0],
        [29, -70.0],
        [30, -70.0],
        [31, -69.9974],
        [32, -69.9899],
        [33, -69.9781],
        [34, -69.9624],
        [35, -69.9434],
        [36, -69.9213],
        [37, -69.8967],
        [38, -69.8699],
        [39, -69.8411],
        [40, -69.8108],
        [41, -69.779],
        [42, -69.7463],
        [43, -69.7126],
        [44, -69.6783],
        [45, -69.6435],
        [46, -69.6084],
        [47, -69.5732],
        [48, -69.538],
        [49, -69.5029],
        [50, -69.4681],
        [51, -69.4336],
        [52, -69.3995],
        [53, -69.366],
        [54, -69.333],
        [55, -69.3008],
        [56, -69.2692],
        [57, -69.2383],
    ]
)
REFERENCE_DATA[:, 0] *= 0.1  # Reference data is based on 0.1ms time steps


@pytest.mark.parametrize("h", [0.1, 0.2, 0.5, 1.0])
def test_iaf_psp_aligned_impact(h):
    """
    Test PSP response at different resolutions.
    The voltage trace should be independent of computation step size h.
    """
    delay = 1.0  # in ms

    nest.ResetKernel()
    nest.set(local_num_threads=1, resolution=h)

    sg = nest.Create("spike_generator")
    sg.set(
        {
            "precise_times": False,
            "origin": 0.0,  # in ms
            "spike_times": [2.0],  # in ms
            "start": 1.0,  # in ms
            "stop": 3.0,  # in ms
        }
    )

    neuron = nest.Create("iaf_psc_alpha")

    vm = nest.Create("voltmeter", params={"time_in_steps": False, "interval": h})

    nest.Connect(sg, neuron, syn_spec={"weight": 100.0, "delay": delay})
    nest.Connect(vm, neuron)

    simulation_time = 7.0  # in ms
    nest.Simulate(simulation_time)

    # Get recorded data
    events = vm.get("events")
    times = events["times"]
    voltages = events["V_m"]

    actual_data = np.column_stack([times, voltages])

    actual, expected = testutil.get_comparable_timesamples(h, actual_data, REFERENCE_DATA)

    # Check that the function did not return empty arrays
    assert len(actual) > 0, (
        f"get_comparable_timesamples() returned empty arrays - " f"no matching time points found for resolution h={h}"
    )

    # Assert approximate equality
    # Reference data has 4-6 decimal places, so use appropriate tolerance
    np.testing.assert_allclose(actual, expected, rtol=1e-6)
