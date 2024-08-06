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

This script simulates the network modeled in [1]_.
An excitatory and an inhibitory population receives input
from an external population modeled as a Poisson process.
Two different subsets of the excitatory population,
comprising 15% of the total population each, receive additional
inputs from a time-inhomogeneous Poisson process, where the
coherence between the two signals can be varied. Local inhibition
mediates a winner-takes-all competition, and the activity of
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

# Use approximate model, can be replaced by "iaf_bw_2001_exact"
model = "iaf_bw_2001"

dt = 0.1

# number of neurons in each population
NE = 1600
NI = 400


def run_sim(coherence, seed=123):
    nest.ResetKernel()
    nest.set(resolution=dt, print_time=True, rng_seed=seed)
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

    # Signals to the two different excitatory sub-populations
    # the signal is given by a time-inhomogeneous Poisson process,
    # where the expectations are constant over intervals of 50ms,
    # and then change. The values for each interval are normally
    # distributed, with means `mu_a`and `mu_b`, and standard deviation
    # `sigma`.
    signal_start = 1000.0
    signal_duration = 1000.0
    signal_update_interval = 50.0
    f = 0.15  # proportion of neurons receiving signal inputs
    # compute expectations of the time-inhomogeneous Poisson processes
    mu_0 = 40.0  # base rate
    rho_a = mu_0 / 100  # scaling factors coherence
    rho_b = rho_a
    sigma = 4.0  # standard deviation
    mu_a = mu_0 + rho_a * coherence  # expectation for pop A
    mu_b = mu_0 - rho_b * coherence  # expectation for pop B

    # sample values for the Poisson process
    num_updates = int(signal_duration / signal_update_interval)
    update_times = np.arange(0, signal_duration, signal_update_interval)
    update_times[0] = 0.1

    # We could have written the generator expressions passed to `np.fromiter()` below as
    #
    #    ( nest.CreateParameter(...).GetValue() for _ in range(num_updates) )
    #
    # but that would create a new Parameter object for each iteration. We avoid this by
    # wrapping the iterator in an outer loop in which `p` assumes only a single value.
    rates_a = np.fromiter(
        (
            p.GetValue()
            for _ in range(num_updates)
            for p in [nest.CreateParameter("normal", {"mean": mu_a, "std": sigma})]
        ),
        float,
    )
    rates_b = np.fromiter(
        (
            p.GetValue()
            for _ in range(num_updates)
            for p in [nest.CreateParameter("normal", {"mean": mu_b, "std": sigma})]
        ),
        float,
    )

    # synaptic weights
    w_plus = 1.7  # strong connections in selective populations
    w_minus = 1 - f * (w_plus - 1) / (1 - f)  # weak connections between selective populations
    # and from nonselective to selective populations

    delay = 0.5

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

    sr_nonselective = nest.Create("spike_recorder", {"time_in_steps": True})
    sr_selective1 = nest.Create("spike_recorder", {"time_in_steps": True})
    sr_selective2 = nest.Create("spike_recorder", {"time_in_steps": True})
    sr_inhibitory = nest.Create("spike_recorder", {"time_in_steps": True})

    sr_selective1_raster = nest.Create("spike_recorder", 100, {"time_in_steps": True})
    sr_selective2_raster = nest.Create("spike_recorder", 100, {"time_in_steps": True})

    ##################################################
    # Define synapse specifications

    receptor_types = selective_pop1[0].receptor_types

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
        "weight": 1.0 * g_GABA_ex,
        "delay": delay,
        "receptor_type": receptor_types["GABA"],
    }

    ii_syn_spec = {
        "synapse_model": "static_synapse",
        "weight": 1.0 * g_GABA_in,
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
    nest.Connect(selective_pop1[:100], sr_selective1_raster, "one_to_one")
    nest.Connect(selective_pop2, sr_selective2)
    nest.Connect(selective_pop2[:100], sr_selective2_raster, "one_to_one")

    # from inhibitory pop
    nest.Connect(
        inhibitory_pop, selective_pop1 + selective_pop2 + nonselective_pop, conn_spec="all_to_all", syn_spec=ie_syn_spec
    )
    nest.Connect(inhibitory_pop, inhibitory_pop, conn_spec="all_to_all", syn_spec=ii_syn_spec)

    nest.Connect(inhibitory_pop, sr_inhibitory)

    ##################################################
    # Run simulation
    nest.Simulate(4000.0)

    ##################################################
    # Collect data from simulation
    spikes_nonselective = sr_nonselective.events["times"]
    spikes_selective1 = sr_selective1.events["times"]
    spikes_selective2 = sr_selective2.events["times"]
    spikes_inhibitory = sr_inhibitory.events["times"]

    spikes_selective1_raster = sr_selective1_raster.get("events", "times")
    spikes_selective2_raster = sr_selective2_raster.get("events", "times")

    return {
        "nonselective": spikes_nonselective,
        "selective1": spikes_selective1,
        "selective2": spikes_selective2,
        "inhibitory": spikes_inhibitory,
        "selective1_raster": spikes_selective1_raster,
        "selective2_raster": spikes_selective2_raster,
    }


coherences = [51.2, 12.8, 0.0]
results = []
for c in coherences:
    results.append(run_sim(c, seed=1234))

##################################################
# Plots

# bins for histograms
res = 1.0
bins = np.arange(0, 4001, res) - 0.001
fig, ax = plt.subplots(ncols=2, nrows=8, sharex=True, sharey=False, height_ratios=[1, 0.7, 0.3, 1, 0.7, 0.3, 1, 0.7])

fig.subplots_adjust(hspace=0.0)
ax[0, 0].set_xlim(0, 800)
ax[0, 0].set_xticks([])
phi = np.arctan(1080 / 1920)
sz = (14 * np.cos(phi), 14 * np.sin(phi))
fig.set_size_inches(sz)

# selective populations
num = 0.15 * NE

for j in range(3):
    # compute firing rates as moving averages over 50 ms windows with 5 ms strides
    hist1, _ = np.histogram(
        results[j]["selective1"] * dt, bins=bins
    )  # spikes are recorded in steps, multiply på dt to get time
    hist1 = hist1.reshape((-1, 5)).sum(-1)
    hist2, _ = np.histogram(results[j]["selective2"] * dt, bins=bins)
    hist2 = hist2.reshape((-1, 5)).sum(-1)

    pop1_rate = np.convolve(hist1, np.ones(10) * 0.1, mode="same") / num / 5 * 1000
    pop2_rate = np.convolve(hist2, np.ones(10) * 0.1, mode="same") / num / 5 * 1000

    ax[j * 3 + 1, 0].bar(np.arange(len(pop1_rate)), pop1_rate, width=1.0, color="black")
    ax[j * 3 + 1, 1].bar(np.arange(len(pop2_rate)), pop2_rate, width=1.0, color="black")
    ax[j * 3 + 1, 0].vlines([200, 400], 0, 40, colors="black", linewidths=2.4)
    ax[j * 3 + 1, 1].vlines([200, 400], 0, 40, colors="black", linewidths=2.4)
    ax[j * 3 + 1, 0].set_ylim(0, 40)
    ax[j * 3 + 1, 1].set_ylim(0, 40)
    for k in range(100):
        sp = results[j]["selective1_raster"][k] * dt / 5.0
        ax[j * 3, 0].scatter(sp, np.ones_like(sp) * k, s=1.0, marker="|", c="black")
        ax[j * 3, 0].vlines([200, 400], 0, 100, colors="black", linewidths=1.0)
        ax[j * 3, 0].set_yticks([])
        ax[j * 3, 0].set_ylim(0, 99)
        sp = results[j]["selective2_raster"][k] * dt / 5.0
        ax[j * 3, 1].scatter(sp, np.ones_like(sp) * k, s=1.0, marker="|", c="black")
        ax[j * 3, 1].vlines([200, 400], 0, 100, colors="black", linewidths=1.0)
        ax[j * 3, 1].set_yticks([])
        ax[j * 3, 1].set_ylim(0, 99)
    ax[j * 3, 0].set_title(f"coherence = {coherences[j]}")
    ax[j * 3, 1].set_title(f"coherence = {coherences[j]}")

ax[-1, 0].set_xticks([0, 200, 400, 600, 800])
ax[-1, 0].set_xticklabels(["0", "1000", "2000", "3000", "4000"])
ax[-1, 1].set_xticks([0, 200, 400, 600, 800])
ax[-1, 1].set_xticklabels(["0", "1000", "2000", "3000", "4000"])
ax[-1, 0].set_xlabel("t (ms)")
ax[-1, 1].set_xlabel("t (ms)")
ax[0, 0].text(0.32, 1.5, "Selective pop A", transform=ax[0, 0].transAxes, fontsize=15)
ax[0, 1].text(0.32, 1.5, "Selective pop B", transform=ax[0, 1].transAxes, fontsize=15)
ax[2, 0].axis("off")
ax[2, 1].axis("off")
ax[5, 0].axis("off")
ax[5, 1].axis("off")
plt.show()
