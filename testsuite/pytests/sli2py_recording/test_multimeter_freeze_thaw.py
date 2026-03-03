# -*- coding: utf-8 -*-
#
# test_multimeter_freeze_thaw.py
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
import numpy.testing as nptest
import pytest


def build_net(num_neurons):
    """
    Create and connect network components.
    """

    nest.ResetKernel()
    nest.verbosity = nest.VerbosityLevel.WARNING

    nrns = nest.Create("iaf_psc_alpha", num_neurons, params={"I_e": 50.0})
    mm = nest.Create("multimeter", params={"interval": 0.5, "record_from": ["V_m"], "time_in_steps": True})

    nest.Connect(mm, nrns)

    return nrns, mm


def simulate_freeze_thaw(num_neurons):
    """
    Build network and simulate with freeze and thaw cycles.

    Only the first neuron in a NodeCollection will be frozen/thawed.
    """

    nrns, mm = build_net(num_neurons)

    nrns[0].frozen = True
    nest.Simulate(5.0)
    nrns[0].frozen = False
    nest.Simulate(5.0)
    nrns[0].frozen = True
    nest.Simulate(5.0)
    nrns[0].frozen = False
    nest.Simulate(5.0)

    return mm


def test_multimeter_freeze():
    """
    Ensure that frozen parameter can be set to False but not True on multimeter.
    """

    nest.Create("multimeter", params={"frozen": False})
    with pytest.raises(Exception):
        nest.Create("multimeter", params={"frozen": True})


def test_freeze_thaw_simulation_against_only_thawed_simulation():
    """
    Verify identical results from freeze/thaw and non-freeze simulation.

    This test first simulates with freeze/thaw cycles. Then a simulation
    for equivalent time, i.e., thawed time only, is performed. Both
    simulations should give the same membrane potentials.
    """

    mm = simulate_freeze_thaw(num_neurons=1)
    Vm_with_freeze = mm.events["V_m"]

    _, mm = build_net(num_neurons=1)
    nest.Simulate(10.0)
    Vm_thawed_only = mm.events["V_m"]

    nptest.assert_array_equal(Vm_with_freeze, Vm_thawed_only)


def test_freeze_thaw_neuron_against_only_thawed_neuron():
    """
    Verify identical results from freeze/thaw and only thawed neuron.

    This test simultaneously records from both a neuron with freeze/thaw
    cycles and a thawed neuron in simulation. The data points collected
    from the freeze/thaw neuron should be identical to the first data points
    from the thawed neuron.
    """

    mm = simulate_freeze_thaw(num_neurons=2)
    Vm = mm.events["V_m"]
    senders = mm.events["senders"]
    Vm_with_freeze = Vm[senders == 1]
    Vm_thawed_only = Vm[senders == 2]

    n_frozen = Vm_with_freeze.size
    nptest.assert_array_equal(Vm_with_freeze, Vm_thawed_only[:n_frozen])
