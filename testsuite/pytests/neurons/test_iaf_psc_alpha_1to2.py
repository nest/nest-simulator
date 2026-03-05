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


import nest
import numpy as np
import pandas as pd
import pytest


def Vm_theory(t, nrn_params, syn_weight):
    """
    Returns the value of the membrane potential at time t, assuming
    alpha-shaped post-synaptic currents and an incoming spike at t=0.
    """

    assert (t >= 0).all()

    C_m = nrn_params["C_m"]
    tau_m = nrn_params["tau_m"]
    tau_syn = nrn_params["tau_syn_ex"]
    E_L = nrn_params["E_L"]

    prefactor = syn_weight * np.exp(1) / (tau_syn * C_m)
    term1 = (np.exp(-t / tau_m) - np.exp(-t / tau_syn)) / (1 / tau_syn - 1 / tau_m) ** 2
    term2 = t * np.exp(-t / tau_syn) / (1 / tau_syn - 1 / tau_m)
    return E_L + prefactor * (term1 - term2)


@pytest.mark.parametrize(
    "resolution, min_delay", [(0.1, 1.0), (0.2, 1.0), (0.5, 1.0), (1.0, 1.0), (0.1, 0.1), (0.1, 0.2), (0.1, 0.5)]
)
def test_1to2(resolution, min_delay):
    """
    Checks the spike interaction of two iaf_psc_alpha model neurons.

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

    See end of file for commented table of expected output.
    """

    nest.ResetKernel()
    nest.set(resolution=resolution, min_delay=min_delay, max_delay=1)

    # Parameters for network and some pre-calculated results
    I_ext = 1450
    time_to_threshold = 3  # for iaf_psc_alpha with default params driven by I_ext
    delay = 1
    weight = 100

    # Create neurons
    n1, n2 = nest.Create("iaf_psc_alpha", 2)
    n1.I_e = I_ext

    # Spike recorder for neuron 1
    sr = nest.Create("spike_recorder", params={"time_in_steps": True})
    nest.Connect(n1, sr)

    # Voltmeter for n2
    vm = nest.Create("voltmeter", params={"interval": resolution, "time_in_steps": True})
    nest.Connect(vm, n2)

    nest.Connect(n1, n2, syn_spec={"weight": weight, "delay": delay})

    # Expected spike times of first neuron for default parameters
    t_expected = [time_to_threshold, 2 * time_to_threshold + n1.t_ref]

    # Run the simulation until time of second spike of n1, so it does not affect n2
    nest.Simulate(t_expected[1])

    spikes = sr.events["times"]
    voltages = pd.DataFrame.from_records(vm.events)

    assert list(spikes) == [round(t / resolution) for t in t_expected]

    # Effect of first spike on membrane potential of second. Begins with arrival of first spike of n1 at n2
    t_arrive = t_expected[0] + delay
    t_arrive_steps = round(t_arrive / resolution)

    # Must have resting membrane potential up to spike arrival at n2
    v_pre_spike = voltages.loc[voltages.times <= t_arrive_steps].V_m
    np.testing.assert_array_equal(v_pre_spike, n2.E_L)

    # Compare membrane potential from spike arrival onward to analytical solution
    t_post_spike = voltages.loc[voltages.times >= t_arrive_steps].times * resolution - t_arrive
    v_post_spike = voltages.loc[voltages.times >= t_arrive_steps].V_m

    v_expected = Vm_theory(t_post_spike, n2.get(), weight)

    np.testing.assert_allclose(v_post_spike, v_expected, rtol=1e-12)


# ------------------------------------------------------------------------------------------
#
# Expected output of this simulation
#
# The output sent to std::cout is a superposition of the output of
# the voltmeter and the spike recorder. Both, voltmeter and spike
# recorder are connected to the same neuron.
#
#
#   h=   (in ms)
# [ 0.1   0.2    0.5   1.0]
#
#   time                    voltage
# [
#   ...
# [ 25           5           -70]%         <-- Voltage trace of the postsynaptic neuron
# [ 26    13                 -70]%              (neuron2), at rest until a spike arrives.
# [ 27                       -70]
# [ 28    14                 -70]
# [ 29                       -70]
# [ 30    15     6     3     -70]
#   1       30                   %         <-- The pre-synaptic neuron (neuron1) emits a
# [ 31                       -70]%             spike at t=3.0 ms.
# [ 32    16                 -70]
# [ 33                       -70]
# [ 34    17                 -70]
# [ 35           7           -70]%         <--  Synaptic delay of 1.0 ms.
# [ 36    18                 -70]
# [ 37                       -70]
# [ 38    19                 -70]
# [ 39                       -70]
# [ 40    20     8    4      -70]% <-----------  Spike arrives at the postsynaptic neuron
# [ 41                       -69.9974]%    <-    (neuron2) and changes the state vector of
# [ 42    21                 -69.9899]%      |   the neuron, not visible in voltage because
# [ 43                       -69.9781]%      |   voltage of PSP initial condition is 0.
# [ 44    22                 -69.9624]%      |
# [ 45           9           -69.9434]%       -  Arbitrarily close to the time of impact
# [ 46    23                 -69.9213]%          (t=4.0 ms) the effect of the spike (PSP)
# [ 47                       -69.8967]%          is visible in the voltage trace.
# [ 48    24                 -69.8699]
# [ 49                       -69.8411]
# [ 50    25    10     5     -69.8108]
# [ 51                       -69.779 ]
# [ 52    26                 -69.7463]%    <---  The voltage trace is independent
# [ 53                       -69.7126]%          of the computation step size h.
# [ 54    27                 -69.6783]%          Larger step sizes only have fewer
# [ 55          11           -69.6435]%          sample points.
# [ 56    28                 -69.6084]
# [ 57                       -69.5732]
#   ...
# ]
#
# ------------------------------------------------------------------------------------------
