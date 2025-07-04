# -*- coding: utf-8 -*-
#
# sir_neuron.py
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
Susceptible (S), Infected (I), Recovered (R) model of the spread of disease
----------------------------------------------------------------------------

This scripts simulates a number of all-to-all connected ``sir`` neurons. One neuron
is infected at the beginning of the simulation, all other neurons are
susceptible. This example can be extended to the `sirs_neuron` or `sis_neuron` type by changing
the `neuron_type` variable.
"""

import matplotlib.pyplot as plt
import nest
import numpy as np

# create neuron population
num_neurons = 100
neuron_type = "sir_neuron"  # must be one of "sir_neuron" | "sirs_neuron" | "sis_neuron"
sir_neurons = nest.Create(neuron_type, num_neurons)
sir_neurons.beta_sir = 0.01  # downscale infectivity from default 0.1

# connect sir_neurons all-to-all
for i in range(num_neurons):
    for j in range(num_neurons):
        if i != j:
            nest.Connect(sir_neurons[i], sir_neurons[j])

sir_neurons[0].S = 1  # infect zeroth neuron
sir_neurons[1:].h = 1  # set number of infected neighbors of all other neurons to 1

# create recording device
multimeter = nest.Create("multimeter")
multimeter.record_from = ["S", "h"]
nest.Connect(multimeter, sir_neurons)

# simulate dynamics
simulated_time = 1000
nest.Simulate(simulated_time)

# evaluate result
state_list = multimeter.get("events")["S"]
states = np.reshape(state_list, (simulated_time - 1, num_neurons))

num_infected_over_time = np.sum(states == 1, axis=1)
num_susceptible_over_time = np.sum(states == 0, axis=1)
num_recovered_over_time = np.sum(states == 2, axis=1)

plt.plot(range(simulated_time - 1), num_infected_over_time, label="infected", color="red")
plt.plot(range(simulated_time - 1), num_susceptible_over_time, label="susceptible", color="orange")

if not neuron_type == "sis_neuron":
    plt.plot(range(simulated_time - 1), num_recovered_over_time, label="recovered", color="blue")
plt.legend()
plt.xlabel("time")
plt.ylabel("number of neurons in state")
plt.show()
