# -*- coding: utf-8 -*-
#
# test_ac_generator.py
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
This testscript checks tests that the ac_generator provides the correct
current by comparing with a neuron driven by a step-current input
corresponding to the current expected from the ac_generator.
"""

import math

import nest
import numpy as np
import pytest


def test_ac_generaor():
    # variables
    dc = 1000.0
    ac = 550.0
    freq = 100.0
    phi = 0.0
    start = 5.0
    stop = 6.0
    dt = 0.1

    # Reset the NEST kernel and set the resolution
    nest.ResetKernel()
    nest.resolution = 0.1

    om = freq / 1000.0 * 2 * math.pi

    step_curr_times = np.arange(start, stop, dt)
    step_curr_amps = [(math.sin(x * om + phi) * ac + dc) for x in step_curr_times]

    # Create neurons
    n_ac = nest.Create("iaf_psc_alpha")
    n_sc = nest.Create("iaf_psc_alpha")

    # ac generator
    ac_gen = nest.Create(
        "ac_generator",
        params={"amplitude": ac, "offset": dc, "frequency": freq, "phase": phi, "start": start, "stop": stop},
    )

    # step current generator
    sc_gen = nest.Create(
        "step_current_generator",
        params={"amplitude_times": step_curr_times, "amplitude_values": step_curr_amps, "start": start, "stop": stop},
    )

    # voltmeter
    vm_ac = nest.Create("voltmeter", params={"interval": 0.1})
    vm_sc = nest.Create("voltmeter", params={"interval": 0.1})

    # multimeter
    mm_ac = nest.Create("multimeter", params={"record_from": ["I"], "interval": 0.1})
    mm_sc = nest.Create("multimeter", params={"record_from": ["I"], "interval": 0.1})

    # Connect everything
    nest.Connect(ac_gen, n_ac)
    nest.Connect(vm_ac, n_ac)
    nest.Connect(mm_ac, ac_gen)

    nest.Connect(sc_gen, n_sc)
    nest.Connect(vm_sc, n_sc)
    nest.Connect(mm_sc, sc_gen)

    # Simulate
    nest.Simulate(10.0)

    # Assert that the v_m of the neuron with the ac_generator
    # is equal to that with the step-current generator
    v_m_ac = vm_ac.events["V_m"]
    v_m_sc = vm_sc.events["V_m"]

    assert v_m_ac == pytest.approx(v_m_sc)

    # Assert that the I (current) of the neuron with the ac_generator
    # is equal to that with the step-current generator
    I_ac = mm_ac.events["I"]
    I_sc = mm_sc.events["I"]

    assert I_ac == pytest.approx(I_sc)
