# -*- coding: utf-8 -*-
#
# cuba_stdp.py
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
Benchmark 5 of the simulator review (CUBA-STDP)
------------------------------------------------

The fifth simulator review benchmark is implemented as a variation of the
Brunel Network. This script creates a sparsely coupled network of excitatory
and inhibitory neurons. Connections within and across both populations are
created at random. Both neuron populations receive Poisson background
input. The spike output of 500 neurons are recorded. Neurons are modeled as
leaky integrate-and-fire neurons with current-injecting synapses (exponential
functions). Excitatory-excitatory synapses implement multiplicative STDP.

This is Benchmark 5 of the FACETS simulator review (Brette et al. 2007):
- Neuron model: integrate-and-fire (i&f)
- Synapse model: STDP-current (excitatory-excitatory), static-current (others)
- Synapse time course: exponential
- Spike times: grid-constrained

This benchmark demonstrates the capabilities of NEST in simulating large
networks with heterogeneous synaptic dynamics.

References
~~~~~~~~~~

.. [1] Brunel N. 2000. Dynamics of sparsely connected networks of excitatory
       and inhibitory spiking neurons. Journal of Computational Neuroscience.
       8:183-208.
       https://doi.org/10.1023/A:1008925309027

.. [2] Brette R, Rudolph M, Carnevale T, Hines M, Beeman D, Bower JM, et al.
       2007. Simulation of networks of spiking neurons: a review of tools and
       strategies. Journal of Computational Neuroscience. 23(3):349-398.
       https://doi.org/10.1007/s10827-007-0038-6

"""

import nest
import numpy as np
from brette_et_al_2007_benchmark import compute_rate

###############################################################################
# Set benchmark parameters

virtual_processes = 1  # number of virtual processes to use
simtime = 2000.0  # simulated time [ms]
dt = 0.1  # simulation step [ms]

NE = 9000  # number of excitatory neurons
NI = 2250  # number of inhibitory neurons

model = "iaf_psc_exp"  # neuron model
model_params = {
    "E_L": 0.0,  # resting membrane potential [mV]
    "V_m": 0.0,  # initial membrane potential [mV] (randomly initialized below)
    "V_th": 20.0,  # threshold [mV]
    "V_reset": 0.0,  # reset potential [mV]
    "C_m": 250.0,  # capacity of the membrane [pF]
    "tau_m": 20.0,  # membrane time constant [ms]
}

mean_potential = 7.0  # mean initial membrane potential [mV]
sigma_potential = 5.0  # standard deviation of initial membrane potential [mV]

epsilon = 0.1  # connection probability
delay = 1.5  # synaptic delay [ms]
JE = 35.0  # peak amplitude of PSC [pA]
g = 5.0  # relative strength of inhibitory connections

plastic_synapses = True
stdp_params = {
    "delay": 1.5,
    "alpha": 1.05,  # STDP asymmetry parameter
    "weight": JE,
    "Wmax": 2.0 * JE,  # max strength of E->E synapses [pA]
}

sigma_w = 3.0  # initial standard deviation of E->E synapses [pA]

stimulus_params = {
    "rate": 900.0 * 4.5,  # rate of initial Poisson stimulus [spikes/s]
}

recorder_params = {
    "record_to": "ascii",
    "label": "cuba_stdp",
}

Nrec = 500  # number of neurons per population to record from

###############################################################################
# Build and run simulation

nest.ResetKernel()
nest.set_verbosity("M_INFO")

# Set kernel parameters
nest.SetKernelStatus(
    {
        "resolution": dt,
        "total_num_virtual_procs": virtual_processes,
        "overwrite_files": True,
        "rng_seed": 238,
    }
)

# Set default neuron parameters
nest.SetDefaults(model, model_params)

print("Creating excitatory population ...")
E_neurons = nest.Create(model, NE)

print("Creating inhibitory population ...")
I_neurons = nest.Create(model, NI)

print("Creating stimulus generators ...")
E_stimulus = nest.Create("poisson_generator")
E_stimulus.set(stimulus_params)

I_stimulus = nest.Create("poisson_generator")
I_stimulus.set(stimulus_params)

print("Creating spike recorders ...")
E_recorder = nest.Create("spike_recorder")
E_recorder.set(recorder_params)

I_recorder = nest.Create("spike_recorder")
I_recorder.set(recorder_params)

# Set initial membrane potentials (randomly distributed)
print("Configuring neuron parameters ...")
all_neurons = E_neurons + I_neurons
V_m_values = np.random.normal(mean_potential, sigma_potential, len(all_neurons))
# Ensure values are below threshold
V_m_values = np.clip(V_m_values, -np.inf, model_params["V_th"] - 0.1)
all_neurons.V_m = V_m_values.tolist()

# Calculate connection counts
CE = int(NE * epsilon)  # excitatory connections per neuron
CI = int(NI * epsilon)  # inhibitory connections per neuron

# Create custom synapse models
nest.CopyModel("static_synapse", "syn_ex", {"weight": JE})
nest.CopyModel("static_synapse", "syn_in", {"weight": -JE * g})
nest.SetDefaults("syn_ex", {"delay": delay})
nest.SetDefaults("syn_in", {"delay": delay})

# Set up STDP synapse
nest.SetDefaults("stdp_synapse", stdp_params)

# Determine synapse model for E->E connections
if plastic_synapses:
    synapse_model = "stdp_synapse"
else:
    synapse_model = "syn_ex"

print("Connecting stimulus generators ...")
nest.Connect(E_stimulus, E_neurons, conn_spec="all_to_all", syn_spec="syn_ex")
nest.Connect(I_stimulus, I_neurons, conn_spec="all_to_all", syn_spec="syn_ex")

print("Connecting excitatory -> excitatory population ...")
# E->E with STDP and random initial weights
nest.Connect(
    E_neurons,
    E_neurons,
    conn_spec={"rule": "fixed_indegree", "indegree": CE},
    syn_spec={"synapse_model": synapse_model, "weight": nest.random.normal(JE, sigma_w)},
)

print("Connecting excitatory -> inhibitory population ...")
nest.Connect(
    E_neurons,
    I_neurons,
    conn_spec={"rule": "fixed_indegree", "indegree": CE},
    syn_spec="syn_ex",
)

print("Connecting inhibitory -> excitatory population ...")
nest.Connect(
    I_neurons,
    E_neurons,
    conn_spec={"rule": "fixed_indegree", "indegree": CI},
    syn_spec="syn_in",
)

print("Connecting inhibitory -> inhibitory population ...")
nest.Connect(
    I_neurons,
    I_neurons,
    conn_spec={"rule": "fixed_indegree", "indegree": CI},
    syn_spec="syn_in",
)

print("Connecting spike recorders ...")
nest.Connect(E_neurons[:Nrec], E_recorder)
nest.Connect(I_neurons[:Nrec], I_recorder)

###############################################################################
# Run simulation

print("Simulating ...")
nest.Simulate(simtime)

# Get timing from kernel status
kernel_status = nest.GetKernelStatus()
build_time = kernel_status["network_build_time"]
sim_time = kernel_status["simulation_time"]

# Calculate number of synapses
N = NE + NI
Nsyn = (CE + CI) * N + Nrec * 2 + N

# Compute rates
num_processes = nest.num_processes
E_rate = compute_rate(E_recorder, Nrec, simtime, num_processes)
I_rate = compute_rate(I_recorder, Nrec, simtime, num_processes)

# Print summary
print("\n" + "=" * 60)
print("Brunel Network Simulation")
print("=" * 60)
print(f"Building time     : {build_time:.2f} s")
print(f"Simulation time   : {sim_time:.2f} s")
print(f"Number of Neurons : {N}")
print(f"Number of Synapses: {Nsyn}")
print(f"Excitatory rate   : {E_rate:.2f} spikes/s")
print(f"Inhibitory rate   : {I_rate:.2f} spikes/s")
print("=" * 60 + "\n")
