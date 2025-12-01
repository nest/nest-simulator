# -*- coding: utf-8 -*-
#
# test_ticket_481.py
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
import pytest


def run_simulation(model, syn1, syn2, debugging=False):
    """
    Helper function to simulate and check if neuron voltages exceed the reference.

    This function creates two neurons using the specified model and sets their initial voltage to Vref.
    It then simulates a network with a poisson generator, voltmeters, and the specified synapses.

    Args:
        model (str): The neuron model to use.
        syn1 (str): The first synapse model to connect the poisson generator to the first neuron.
        syn2 (str): The second synapse model to connect the poisson generator to the second neuron.
        debugging (bool): If True, prints additional information about the simulation results.

    Returns:
        bool: True if the maximum voltage of both neurons exceeds Vref, False otherwise.
    """
    
    # Define time step for precise simulation
    dt = 0.125  # ms
    nest.ResetKernel()
    nest.SetKernelStatus({"tics_per_ms": 1.0 / dt, "resolution": dt})

    Vref = -70.0
    # Create neurons with specified model and initial parameters
    n1 = nest.Create(model, params={"V_m": Vref, "E_L": Vref, "V_th": 1e10, "I_e": 0.0})
    n2 = nest.Create(model, params={"V_m": Vref, "E_L": Vref, "V_th": 1e10, "I_e": 0.0})

    # Create a poisson generator with a high rate
    pg = nest.Create("poisson_generator_ps", params={"rate": 10000.0, "start": 0.0})
    # Create voltmeters to record neuron voltages
    vm1 = nest.Create("voltmeter", params={"interval": dt})
    vm2 = nest.Create("voltmeter", params={"interval": dt})

    # Connect poisson generator to neurons with specified synapse models
    nest.Connect(pg, n1, syn_spec={"synapse_model": syn1})
    nest.Connect(pg, n2, syn_spec={"synapse_model": syn2})
    # Connect voltmeters to neurons to record their membrane potentials
    nest.Connect(vm1, n1)
    nest.Connect(vm2, n2)

    # Simulate the network for 1000 ms
    nest.Simulate(1000.0)

    # Retrieve the maximum voltage recorded by each voltmeter
    vm1_events = nest.GetStatus(vm1, "events")[0]
    vm2_events = nest.GetStatus(vm2, "events")[0]

    max_v1 = max(vm1_events["V_m"])
    max_v2 = max(vm2_events["V_m"])

    if debugging:
        print(f"Max Voltage 1: {max_v1}, Max Voltage 2: {max_v2}")

    # Return True if both neurons' voltages exceeded Vref
    return max_v1 > Vref and max_v2 > Vref
    """
    Set up the simulation kernel with specified resolution and time settings.

    This function resets the kernel and sets the resolution and ticks per millisecond for precise simulations.
    """
    nest.ResetKernel()
    nest.SetKernelStatus({"tics_per_ms": 1.0 / dt, "resolution": dt})


# Define time step
dt = 0.125  # ms


def test_ticket_481():
    """
    Ensure that poisson_generator_ps delivers spikes to more than one node.

    Test ported from SLI regression test
    This test checks if a poisson generator can correctly deliver spikes to multiple neurons
    through synapses of identical or different types. The key assertion is that neuron membrane
    potentials exceed a reference voltage in all valid configurations, indicating successful spike delivery.
    The test expects exceptions when mixed synapse models are used inappropriately.

    Author: Hans Ekkehard Plesser, 2010-11-03; based on original reproducer by Peiran Gao
    """
    # Test 1: Both neurons connected with identical synapse models, should pass
    result = run_simulation("iaf_psc_delta_ps", "static_synapse", "static_synapse")
    assert result, "Both neurons should receive spikes with identical synapse models."

    # Test 2: Neurons connected with different named copies of the same synapse model
    nest.CopyModel("static_synapse", "hoo")
    result = run_simulation("iaf_psc_delta_ps", "hoo", "hoo")
    assert result, "Both neurons should receive spikes with identical synapse models, even if renamed."

    # Test 3: Neurons connected with different synapse models, expected to fail
    nest.CopyModel("static_synapse", "foo")
    nest.CopyModel("static_synapse", "goo")
    with pytest.raises(Exception):
        run_simulation("iaf_psc_delta_ps", "foo", "goo")
