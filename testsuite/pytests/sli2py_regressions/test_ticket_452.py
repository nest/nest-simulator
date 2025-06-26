# -*- coding: utf-8 -*-
#
# test_ticket_452.py
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


def run_simulation(model, simcommand):
    """
    Helper function to run simulations based on the model and simulation command.
    Returns the spike times recorded by the spike recorder.
    """
    nest.ResetKernel()
    nest.SetKernelStatus({"tics_per_ms": 8, "resolution": 0.125})  # 0.125 ms resolution

    neuron = nest.Create(model, params={"I_e": 500.0})
    pg = nest.Create("poisson_generator_ps", params={"rate": 10000.0, "start": 0.0})
    sr = nest.Create("spike_recorder")

    nest.Connect(pg, neuron, syn_spec={"delay": 1.0, "weight": 1.0})
    nest.Connect(neuron, sr)

    simcommand()

    # Retrieve spike times from the spike recorder
    events = nest.GetStatus(sr, "events")[0]
    return events["times"]


def test_ticket_452():
    """
    Ensure that simulations with precise spike timing yield identical results
    independent of whether one simulates the full simulation time without interruption,
    or splits the simulation into many small pieces.

    Note: The problem was unrelated to the setting of kernel property  /off_grid_spiking
    and the spike_recorder property  /precise_times.

    Author: Hans Ekkehard Plesser, 2010-09-30; based on original reproduced code by Alexander Hanuschkin
    """
    models = ["iaf_psc_delta_ps", "iaf_psc_alpha_ps"]

    # Define two different simulation approaches
    def simulate_continuous():
        nest.Simulate(500.0)

    def simulate_stepped():
        for _ in range(500):  # Simulate 500 steps of 1.0 ms each
            nest.Simulate(1.0)

    # Test each model with both simulation approaches
    for model in models:
        spike_times_continuous = run_simulation(model, simulate_continuous)
        spike_times_stepped = run_simulation(model, simulate_stepped)

        # Check if the spike times are identical for both simulation approaches
        np.testing.assert_array_equal(spike_times_continuous, spike_times_stepped)
