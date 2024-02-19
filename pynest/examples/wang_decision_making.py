# -*- coding: utf-8 -*-
#
# wang_decision_making.py
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
Decision making in recurrent network with NMDA-dynamics
------------------------------------------------------------

This script simulates the network modelled in [1]_.
An excitatory and an inhibitory population receives input
from an external population modelled as a Poisson process.
Two different subsets of the excitatory population,
comprising 15% of the total population each, receive additional
inputs from a time-inhomogeneous Poisson process, where the
coherence between the two signals can be varied. Local inhibition
mediates a winner-takes-all comptetion, and the activity of
one of the sub-population is suppressed.

References
~~~~~~~~~~
.. [1] Wang X-J (2002). Probabilistic Decision Making by Slow Reverberation in
Cortical Circuits. Neuron, Volume 36, Issue 5, Pages 955-968.
https://doi.org/10.1016/S0896-6273(02)01092-9.

"""

import matplotlib.pyplot as plt
import nest
import numpy as np
from matplotlib.gridspec import GridSpec

np.random.seed(1234)
rng = np.random.default_rng()

# Use approximate model, can be replaced by "iaf_wang_2002_exact"
model = "iaf_wang_2002"

dt = 0.1
nest.set(resolution=dt, print_time=True)

##################################################
# Set parameter values, taken from [1]_.

# conductances excitatory population
# fmt: off
g_AMPA_ex = 0.05                 # recurrent AMPA conductance
g_AMPA_ext_ex = 2.1              # external AMPA conductance
g_NMDA_ex = 0.165                # recurrent GABA conductance
g_GABA_ex = 1.3                  # recurrent GABA conductance

# conductances inhibitory population
g_AMPA_in = 0.04                 # recurrent AMPA conductance
g_AMPA_ext_in = 1.62             # external AMPA conductance
g_NMDA_in = 0.13                 # recurrent GABA conductance
g_GABA_in = 1.0                  # recurrent GABA conductance

# neuron parameters
epop_params = {
    "tau_GABA": 5.0,             # GABA decay time constant
    "tau_AMPA": 2.0,             # AMPA decay time constant
    "tau_decay_NMDA": 100.0,     # NMDA decay time constant
    "tau_rise_NMDA": 2.0,        # NMDA rise time constant
    "alpha": 0.5,                # NMDA parameter
    "conc_Mg2": 1.0,             # Magnesium concentration
    "g_L": 25.0,                 # leak conductance
    "E_L": -70.0,                # leak reversal potential
    "E_ex": 0.0,                 # excitatory reversal potential
    "E_in": -70.0,               # inhibitory reversal potential
    "V_reset": -55.0,            # reset potential
    "V_th": -50.0,               # threshold
    "C_m": 500.0,                # membrane capacitance
    "t_ref": 2.0,                # refreactory period
}

ipop_params = {
    "tau_GABA": 5.0,             # GABA decay time constant
    "tau_AMPA": 2.0,             # AMPA decay time constant
    "tau_decay_NMDA": 100.0,     # NMDA decay time constant
    "tau_rise_NMDA": 2.0,        # NMDA rise time constant
    "alpha": 0.5,                # NMDA parameter
    "conc_Mg2": 1.0,             # Magnesium concentration
    "g_L": 20.0,                 # leak conductance
    "E_L": -70.0,                # leak reversal potential
    "E_ex": 0.0,                 # excitatory reversal potential
    "E_in": -70.0,               # inhibitory reversal potential
    "V_reset": -55.0,            # reset potential
    "V_th": -50.0,               # threshold
    "C_m": 200.0,                # membrane capacitance
    "t_ref": 1.0,                # refreactory period
}
# fmt: on

# signals to the two different excitatory sub-populations
# the signal is given by a time-inhomogeneous Poisson process,
# where the expectations are constant over intervals of 50ms,
# and then change. The values for each interval are normally
# distributed, with means mu_a and mu_b, and standard deviation
# sigma.
signal_start = 1000.0
signal_duration = 2000.0
signal_update_interval = 50.0
f = 0.15  # proportion of neurons receiving signal inputs
# compute expectations of the time-inhomogeneous Poisson processes
mu_0 = 40.0  # base rate
rho_a = mu_0 / 100  # scaling factors coherence
rho_b = rho_a
c = 0.0  # coherence
sigma = 4.0  # standard deviation
mu_a = mu_0 + rho_a * c  # expectation for pop A
mu_b = mu_0 - rho_b * c  # expectation for pop B

# sample values for the Poisson process
num_updates = int(signal_duration / signal_update_interval)
update_times = np.arange(0, signal_duration, signal_update_interval)
update_times[0] = 0.1
rates_a = np.random.normal(mu_a, sigma, size=num_updates)
rates_b = np.random.normal(mu_b, sigma, size=num_updates)

# synaptic weights
w_plus = 1.7
w_minus = 1 - f * (w_plus - 1) / (1 - f)


delay = 0.5

# number of neurons in each population
NE = 1600
NI = 400


##################################################
# Create neurons and devices

selective_pop1 = nest.Create(model, int(f * NE), params=epop_params)
selective_pop2 = nest.Create(model, int(f * NE), params=epop_params)
nonselective_pop = nest.Create(model, int((1 - 2 * f) * NE), params=epop_params)
inhibitory_pop = nest.Create(model, NI, params=ipop_params)

poisson_a = nest.Create(
    "inhomogeneous_poisson_generator",
    params={
        "origin": signal_start - 0.1,
        "start": 0.0,
        "stop": signal_duration,
        "rate_times": update_times,
        "rate_values": rates_a,
    },
)

poisson_b = nest.Create(
    "inhomogeneous_poisson_generator",
    params={
        "origin": signal_start - 0.1,
        "start": 0.0,
        "stop": signal_duration,
        "rate_times": update_times,
        "rate_values": rates_b,
    },
)

poisson_0 = nest.Create("poisson_generator", params={"rate": 2400.0})

sr_nonselective = nest.Create("spike_recorder")
sr_selective1 = nest.Create("spike_recorder")
sr_selective2 = nest.Create("spike_recorder")
sr_inhibitory = nest.Create("spike_recorder")

mm_selective1 = nest.Create("multimeter", {"record_from": ["V_m", "s_NMDA", "s_AMPA", "s_GABA"]})
mm_selective2 = nest.Create("multimeter", {"record_from": ["V_m", "s_NMDA", "s_AMPA", "s_GABA"]})
mm_nonselective = nest.Create("multimeter", {"record_from": ["V_m", "s_NMDA", "s_AMPA", "s_GABA"]})
mm_inhibitory = nest.Create("multimeter", {"record_from": ["V_m", "s_NMDA", "s_AMPA", "s_GABA"]})


##################################################
# Define synapse specifications

receptor_types = selective_pop1[0].get("receptor_types")

syn_spec_pot_AMPA = {
    "synapse_model": "static_synapse",
    "weight": w_plus * g_AMPA_ex,
    "delay": delay,
    "receptor_type": receptor_types["AMPA"],
}
syn_spec_pot_NMDA = {
    "synapse_model": "static_synapse",
    "weight": w_plus * g_NMDA_ex,
    "delay": delay,
    "receptor_type": receptor_types["NMDA"],
}

syn_spec_dep_AMPA = {
    "synapse_model": "static_synapse",
    "weight": w_minus * g_AMPA_ex,
    "delay": delay,
    "receptor_type": receptor_types["AMPA"],
}

syn_spec_dep_NMDA = {
    "synapse_model": "static_synapse",
    "weight": w_minus * g_NMDA_ex,
    "delay": delay,
    "receptor_type": receptor_types["NMDA"],
}

ie_syn_spec = {
    "synapse_model": "static_synapse",
    "weight": -1.0 * g_GABA_ex,
    "delay": delay,
    "receptor_type": receptor_types["GABA"],
}

ii_syn_spec = {
    "synapse_model": "static_synapse",
    "weight": -1.0 * g_GABA_in,
    "delay": delay,
    "receptor_type": receptor_types["GABA"],
}

ei_syn_spec_AMPA = {
    "synapse_model": "static_synapse",
    "weight": 1.0 * g_AMPA_in,
    "delay": delay,
    "receptor_type": receptor_types["AMPA"],
}

ei_syn_spec_NMDA = {
    "synapse_model": "static_synapse",
    "weight": 1.0 * g_NMDA_in,
    "delay": delay,
    "receptor_type": receptor_types["NMDA"],
}

ee_syn_spec_AMPA = {
    "synapse_model": "static_synapse",
    "weight": 1.0 * g_AMPA_ex,
    "delay": delay,
    "receptor_type": receptor_types["AMPA"],
}

ee_syn_spec_NMDA = {
    "synapse_model": "static_synapse",
    "weight": 1.0 * g_NMDA_ex,
    "delay": delay,
    "receptor_type": receptor_types["NMDA"],
}

exte_syn_spec = {
    "synapse_model": "static_synapse",
    "weight": g_AMPA_ext_ex,
    "delay": 0.1,
    "receptor_type": receptor_types["AMPA"],
}

exti_syn_spec = {
    "synapse_model": "static_synapse",
    "weight": g_AMPA_ext_in,
    "delay": 0.1,
    "receptor_type": receptor_types["AMPA"],
}


##################################################
# Create connections

# from external
nest.Connect(
    poisson_0, nonselective_pop + selective_pop1 + selective_pop2, conn_spec="all_to_all", syn_spec=exte_syn_spec
)
nest.Connect(poisson_0, inhibitory_pop, conn_spec="all_to_all", syn_spec=exti_syn_spec)

nest.Connect(poisson_a, selective_pop1, conn_spec="all_to_all", syn_spec=exte_syn_spec)
nest.Connect(poisson_b, selective_pop2, conn_spec="all_to_all", syn_spec=exte_syn_spec)

# from nonselective pop
nest.Connect(nonselective_pop, selective_pop1 + selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_dep_AMPA)
nest.Connect(nonselective_pop, selective_pop1 + selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_dep_NMDA)

nest.Connect(nonselective_pop, nonselective_pop, conn_spec="all_to_all", syn_spec=ee_syn_spec_AMPA)
nest.Connect(nonselective_pop, nonselective_pop, conn_spec="all_to_all", syn_spec=ee_syn_spec_NMDA)

nest.Connect(nonselective_pop, inhibitory_pop, conn_spec="all_to_all", syn_spec=ei_syn_spec_AMPA)
nest.Connect(nonselective_pop, inhibitory_pop, conn_spec="all_to_all", syn_spec=ei_syn_spec_NMDA)

nest.Connect(nonselective_pop, sr_nonselective)

# from selective pops
nest.Connect(selective_pop1, selective_pop1, conn_spec="all_to_all", syn_spec=syn_spec_pot_AMPA)
nest.Connect(selective_pop1, selective_pop1, conn_spec="all_to_all", syn_spec=syn_spec_pot_NMDA)

nest.Connect(selective_pop2, selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_pot_AMPA)
nest.Connect(selective_pop2, selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_pot_NMDA)

nest.Connect(selective_pop1, selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_dep_AMPA)
nest.Connect(selective_pop1, selective_pop2, conn_spec="all_to_all", syn_spec=syn_spec_dep_NMDA)

nest.Connect(selective_pop2, selective_pop1, conn_spec="all_to_all", syn_spec=syn_spec_dep_AMPA)
nest.Connect(selective_pop2, selective_pop1, conn_spec="all_to_all", syn_spec=syn_spec_dep_NMDA)

nest.Connect(selective_pop1 + selective_pop2, nonselective_pop, conn_spec="all_to_all", syn_spec=ee_syn_spec_AMPA)
nest.Connect(selective_pop1 + selective_pop2, nonselective_pop, conn_spec="all_to_all", syn_spec=ee_syn_spec_NMDA)

nest.Connect(selective_pop1 + selective_pop2, inhibitory_pop, conn_spec="all_to_all", syn_spec=ei_syn_spec_AMPA)
nest.Connect(selective_pop1 + selective_pop2, inhibitory_pop, conn_spec="all_to_all", syn_spec=ei_syn_spec_NMDA)

nest.Connect(selective_pop1, sr_selective1)
nest.Connect(selective_pop2, sr_selective2)

# from inhibitory pop
nest.Connect(
    inhibitory_pop, selective_pop1 + selective_pop2 + nonselective_pop, conn_spec="all_to_all", syn_spec=ie_syn_spec
)
nest.Connect(inhibitory_pop, inhibitory_pop, conn_spec="all_to_all", syn_spec=ii_syn_spec)

nest.Connect(inhibitory_pop, sr_inhibitory)

# multimeters record from single neuron from each population.
# since the network is fully connected, it's the same for all
# neurons in the same population.
nest.Connect(mm_selective1, selective_pop1[0])
nest.Connect(mm_selective2, selective_pop2[0])
nest.Connect(mm_nonselective, nonselective_pop[0])
nest.Connect(mm_inhibitory, inhibitory_pop[0])

##################################################
# Run simulation
nest.Simulate(5000.0)


##################################################
# Collect data from simulation
spikes_nonselective = sr_nonselective.get("events", "times")
spikes_selective1 = sr_selective1.get("events", "times")
spikes_selective2 = sr_selective2.get("events", "times")
spikes_inhibitory = sr_inhibitory.get("events", "times")

vm_nonselective = mm_nonselective.get("events", "V_m")
s_AMPA_nonselective = mm_nonselective.get("events", "s_AMPA")
s_GABA_nonselective = mm_nonselective.get("events", "s_GABA")
s_NMDA_nonselective = mm_nonselective.get("events", "s_NMDA")

vm_selective1 = mm_selective1.get("events", "V_m")
s_AMPA_selective1 = mm_selective1.get("events", "s_AMPA")
s_GABA_selective1 = mm_selective1.get("events", "s_GABA")
s_NMDA_selective1 = mm_selective1.get("events", "s_NMDA")

vm_selective2 = mm_selective2.get("events", "V_m")
s_AMPA_selective2 = mm_selective2.get("events", "s_AMPA")
s_GABA_selective2 = mm_selective2.get("events", "s_GABA")
s_NMDA_selective2 = mm_selective2.get("events", "s_NMDA")

vm_inhibitory = mm_inhibitory.get("events", "V_m")
s_AMPA_inhibitory = mm_inhibitory.get("events", "s_AMPA")
s_GABA_inhibitory = mm_inhibitory.get("events", "s_GABA")
s_NMDA_inhibitory = mm_inhibitory.get("events", "s_NMDA")


##################################################
# Plots

# bins for histograms
res = 1.0
bins = np.arange(0, 4001, res) - 0.001

fig, ax = plt.subplots(ncols=2, nrows=2, sharex=True, sharey=True)
fig.tight_layout()

# selective populations
num = NE * f * (res / 1000)
hist1, _ = np.histogram(spikes_selective1, bins=bins)
hist2, _ = np.histogram(spikes_selective2, bins=bins)
ax[0, 0].plot(hist1 / num)
ax[0, 0].set_title("Selective pop A")
ax[0, 1].plot(hist2 / num)
ax[0, 1].set_title("Selective pop B")

# nonselective population
num = NE * (1 - 2 * f) * res / 1000
hist, _ = np.histogram(spikes_nonselective, bins=bins)
ax[1, 0].plot(hist / num)
ax[1, 0].set_title("Nonselective pop")

# inhibitory population
num = NI * res / 1000
hist, _ = np.histogram(spikes_inhibitory, bins=bins)
ax[1, 1].plot(hist / num)
ax[1, 1].set_title("Inhibitory pop")


fig, ax = plt.subplots(ncols=4, nrows=4, sharex=True, sharey="row")
fig.tight_layout()

# AMPA conductances
ax[0, 0].plot(s_AMPA_selective1)
ax[0, 1].plot(s_AMPA_selective2)
ax[0, 2].plot(s_AMPA_nonselective)
ax[0, 3].plot(s_AMPA_inhibitory)

# NMDA conductances
ax[1, 0].plot(s_NMDA_selective1)
ax[1, 1].plot(s_NMDA_selective2)
ax[1, 2].plot(s_NMDA_nonselective)
ax[1, 3].plot(s_NMDA_inhibitory)


# GABA conductances
ax[2, 0].plot(s_GABA_selective1)
ax[2, 1].plot(s_GABA_selective2)
ax[2, 2].plot(s_GABA_nonselective)
ax[2, 3].plot(s_GABA_inhibitory)

# Membrane potential
ax[3, 0].plot(vm_selective1)
ax[3, 1].plot(vm_selective2)
ax[3, 2].plot(vm_nonselective)
ax[3, 3].plot(vm_inhibitory)


ax[0, 0].set_ylabel("S_AMPA")
ax[1, 0].set_ylabel("S_NMDA")
ax[2, 0].set_ylabel("S_GABA")
ax[3, 0].set_ylabel("V_m")

ax[0, 0].set_title("Selective pop1")
ax[0, 1].set_title("Selective pop2")
ax[0, 2].set_title("Nonselective pop")
ax[0, 3].set_title("Inhibitory pop")

ax[0, 0].set_title("Selective pop1")
ax[0, 1].set_title("Selective pop2")
ax[0, 2].set_title("Nonselective pop")
ax[0, 3].set_title("Inhibitory pop")


plt.show()
