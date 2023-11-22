# -*- coding: utf-8 -*-
#
# test_iaf_psc_exp.py
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
Name: testsuite::test_iaf_psc_exp

Synopsis: (test_iaf_psc_exp) run -> compares response to current step with analytical solution

Description:
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

Author:  July 2004, Diesmann
     March 2006, Moritz Helias
"""

import nest
import numpy as np
import numpy.testing as nptest
import pytest


def test_iaf_psc_exp_dc_input():
    dt = 0.1
    dc_amp = 1000.0

    nest.ResetKernel()
    nest.set(resolution=dt, local_num_threads=1)

    dc_gen = nest.Create("dc_generator", {"amplitude": dc_amp})
    nrn = nest.Create("iaf_psc_exp", 1)
    vm = nest.Create("voltmeter", {"interval": 0.1})

    syn_spec = {"synapse_model": "static_synapse", "weight": 1.0, "delay": dt}
    nest.Connect(dc_gen, nrn, syn_spec=syn_spec)
    nest.Connect(vm, nrn, syn_spec=syn_spec)

    nest.Simulate(8.0)

    times = vm.get("events", "times")
    times -= dt  # account for delay to multimeter

    tau_m = nrn.get("tau_m")
    R = tau_m / nrn.get("C_m")

    # array for analytical solution
    V_m_analytical = np.empty_like(times)
    V_m_analytical[:] = nrn.get("E_L")

    # first index for which the DC current is received by neuron.
    # DC current will be integrated from this time step
    start_index = 1

    # analytical solution without delay and threshold
    vm_soln = nrn.get("E_L") + (1 - np.exp(-times / tau_m)) * R * dc_amp

    # rise until threshold
    V_m_analytical[start_index:] = vm_soln[:-start_index]

    # set refractory potential
    crossing_ind = (V_m_analytical > nrn.get("V_th")).argmax()
    num_ref = int(nrn.get("t_ref") / dt)
    V_m_analytical[crossing_ind : crossing_ind + num_ref] = nrn.get("V_reset")

    # rise after refractory period
    num_inds = len(times) - crossing_ind - num_ref
    V_m_analytical[crossing_ind + num_ref :] = vm_soln[:num_inds]

    nptest.assert_array_almost_equal(V_m_analytical, vm.get("events", "V_m"))
