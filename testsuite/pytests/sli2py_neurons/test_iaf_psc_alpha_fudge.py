# -*- coding: utf-8 -*-
#
# test_iaf_psc_alpha_fudge.py
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
import numpy as np
import nest
import pytest
import testutil
from scipy.special import lambertw
def test_iaf_psc_alpha_fudge():
    """
    The peak time of the postsynaptic potential (PSP) is calculated using
    the LambertW function. The theoretical peak voltage amplitude for a
    postsynaptic current of amplitude 1pA is then used to adjust the
    synaptic weight such that a PSP of amplitude 1mV is generated.  The
    success of this approach is verified by comparing the theoretical
    value with the result of a simulation where a single spike is sent to
    the neuron.

    The name of this test script has a historical explanation. Prior to
    July 2009 the analytical expression for the peak time of the PSP was
    not known to the NEST developers. Therefore the normalization factor
    required to adjust the PSP amplitude was computed by root finding
    outside of NEST. The factor was called "fudge" in examples and
    application code. The root finding was not done in NEST because infix
    mathematical notation only become available in SLI in January
    2009. The name "fudge" indicated that the origin of this value was not
    obvious from the simulation scripts and usage was inherently dangerous
    because a change of the time constants of the neuron model would
    invalidate the value of "fudge".
    """
    # Simulation variables
    resolution = 0.1
    delay = resolution
    duration = 20.0

    # Create neuron and devices
    neuron = nest.Create("iaf_psc_alpha")
    voltmeter = nest.Create("voltmeter")
    voltmeter.interval = resolution

    # Connect voltmeter
    nest.Connect(voltmeter, neuron, syn_spec={"weight": 1.0, "delay": delay})

    # Biophysical parameters
    tau_m = 20.0
    tau_syn = 0.5
    C_m = 250.0

    # Set neuron parameters
    nest.SetStatus(neuron, {
        "tau_m": tau_m,
        "tau_syn_ex": tau_syn,
        "tau_syn_in": tau_syn,
        "C_m": C_m
    })

    # Compute fudge factors
    a = tau_m / tau_syn
    b = 1.0 / tau_syn - 1.0 / tau_m
    t_max = (1.0 / b) * (-lambertw(-math.exp(-1.0 / a) / a, k=-1) - 1.0 / a).real

    V_max = (
            math.exp(1)
            / (tau_syn * C_m * b)
            * ((math.exp(-t_max / tau_m) - math.exp(-t_max / tau_syn)) / b - t_max * math.exp(-t_max / tau_syn))
    )

    # Create spike generator to fire once at resolution
    spike_gen = nest.Create("spike_generator", params={
        "precise_times": False,
        "spike_times": [resolution],
    })

    # Connect spike generator to neuron
    nest.Connect(spike_gen, neuron, syn_spec={
        "weight": float(1.0 / V_max),
        "delay": delay
    })

    # Simulate
    nest.Simulate(duration)

    # Extract membrane potential trace
    volt_data = voltmeter.events
    times = volt_data["times"]
    V_m = volt_data["V_m"]
    results = np.column_stack((times, V_m))

    # Find time of peak voltage
    actual_t_max = results[np.argmax(results[:, 1]), 0]

    # Assert the peak is close to theoretical t_max + 0.2
    assert actual_t_max == pytest.approx(t_max + 0.2, abs=0.05)