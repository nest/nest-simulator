# -*- coding: utf-8 -*-
#
# test_iaf_ps_dc_accuracy.py
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

import math

import nest
import pytest


@pytest.mark.parametrize(
    "model",
    [
        "iaf_psc_alpha_ps",
        "iaf_psc_delta_ps",
        "iaf_psc_exp_ps",
        "iaf_psc_exp_ps_lossless",
    ],
)
@pytest.mark.parametrize("resolution", [2**i for i in range(0, -14, -1)])
@pytest.mark.parametrize(
    "params",
    [
        {
            "E_L": 0.0,  # resting potential in mV
            "V_m": 0.0,  # initial membrane potential in mV
            "V_th": 2000.0,  # spike threshold in mV
            "I_e": 1000.0,  # DC current in pA
            "tau_m": 10.0,  # membrane time constant in ms
            "C_m": 250.0,  # membrane capacity in pF
        }
    ],
)
@pytest.mark.parametrize("duration, tolerance", [(5.0, 1e-13), (500.0, 1e-9)])
def test_iaf_ps_dc_accuracy(model, resolution, params, duration, tolerance):
    """
    A DC current is injected for a finite duration. The membrane potential at
    the end of the simulated interval is compared to the theoretical value for
    different computation step sizes.

    Computation step sizes are specified as base 2 values.

    Two different intervals are tested. At the end of the first interval the membrane
    potential still steeply increases. At the end of the second, the membrane
    potential has within double precision already reached the limit for large t.

    The high accuracy of the neuron models is achieved by the use of Exact Integration [1]
    and an appropriate arrangement of the terms [2]. For small computation step sizes the
    accuracy at large simulation time decreases because of the accumulation of errors.

    Reference output is documented at the end of the script.

    Individual simulation results can be inspected by uncommented the call
    to function print_details.
    """
    nest.ResetKernel()
    nest.SetKernelStatus({"tics_per_ms": 2**14, "resolution": resolution})

    neuron = nest.Create(model, params=params)
    nest.Simulate(duration)

    V_m = nest.GetStatus(neuron, "V_m")[0]
    expected_V_m = params["I_e"] * params["tau_m"] / params["C_m"] * (1.0 - math.exp(-duration / params["tau_m"]))

    assert math.fabs(V_m - expected_V_m) < tolerance


@pytest.mark.parametrize(
    "model",
    [
        "iaf_psc_alpha_ps",
        "iaf_psc_delta_ps",
        "iaf_psc_exp_ps",
        "iaf_psc_exp_ps_lossless",
    ],
)
@pytest.mark.parametrize("resolution", [2**i for i in range(0, -14, -1)])
@pytest.mark.parametrize(
    "params",
    [
        {
            "E_L": 0.0,  # resting potential in mV
            "V_m": 0.0,  # initial membrane potential in mV
            "V_th": 15.0,  # spike threshold in mV
            "I_e": 1000.0,  # DC current in pA
            "tau_m": 10.0,  # membrane time constant in ms
            "C_m": 250.0,  # membrane capacity in pF
        }
    ],
)
@pytest.mark.parametrize("duration, tolerance", [(5.0, 1e-13)])
def test_iaf_ps_dc_t_accuracy(model, resolution, params, duration, tolerance):
    """
    A DC current is injected for a finite duration. The time of the first
    spike is compared to the theoretical value for different computation
    step sizes.

    Computation step sizes are specified as base 2 values.

    The high accuracy of the neuron models is achieved by the use of
    Exact Integration [1] and an appropriate arrangement of the terms
    [2]. For small computation step sizes the accuracy at large
    simulation time decreases because of the accumulation of errors.

    The expected output is documented at the end of the script.
    Individual simulation results can be inspected by uncommented the
    call to function print_details.
    """
    nest.ResetKernel()
    nest.SetKernelStatus({"tics_per_ms": 2**14, "resolution": resolution})

    neuron = nest.Create(model, params=params)
    spike_recorder = nest.Create("spike_recorder")
    nest.Connect(neuron, spike_recorder)

    nest.Simulate(duration)

    spike_times = nest.GetStatus(spike_recorder, "events")[0]["times"]
    assert len(spike_times) == 1, "Neuron did not spike exactly once."

    t_spike = spike_times[0]
    expected_t = -params["tau_m"] * math.log(1.0 - (params["C_m"] * params["V_th"]) / (params["tau_m"] * params["I_e"]))

    assert math.fabs(t_spike - expected_t) < tolerance
