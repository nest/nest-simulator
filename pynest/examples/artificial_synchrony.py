# -*- coding: utf-8 -*-
#
# artificial_synchrony.py
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
Artificial synchrony in discrete-time simulations
-------------------------------------------------

Artificial synchrony can be introduced by discrete-time simulation
of neuronal networks, because they typically constrain spike times to a
grid determined by the computational step size. Hansel et al. (1998) [1]_
used a small all-to-all connected network of spiking neurons to demonstrate
that this artificial synchrony can be reduced by a finer resolution or by
interpolating for the correct spike times. In a further step, Morrison et
al. (2007) [2]_ showed that by interpolating the exact spike times and
distributing them, not only the artificial synchrony is avoided
but even machine precision can be obtained for small time steps.

Here, we simulate the 'Hansel' network of 128 all-to-all connected
excitatory integrate-and-fire neurons with alpha-shaped postsynaptic
currents (PSC) in two implementations:

1. Precise implementation by Morrison et al. (2007) [2]_
2. Grid-constrained implementation

The synchrony of the network can be calculated from the membrane
potentials of each neuron following Hansel et al. (1998) [1]_. By
varying the coupling weights and estimating the corresponding
synchrony of the network, the results of Hansel et al. (1998) [1]_,
Morrison et al. (2007) [2]_, and Diesmann et al. (2008) [3]_ can be
reproduced.

The synchrony measure Σ is defined as:

Σ = Δ_N / Δ

where:
- Δ_N is the variance of the mean membrane potential across neurons
- Δ is the mean variance of individual neuron membrane potentials

References
~~~~~~~~~~

.. [1] Hansel D, Mato G, Meunier C, Neltner L. 1998. On numerical
       simulations of integrate-and-fire neural networks. Neural Computation.
       10(2):467-483.
       https://doi.org/10.1162/089976698300017845

.. [2] Morrison A, Straube S, Plesser HE, Diesmann M. 2007. Exact subthreshold
       integration with continuous spike times in discrete-time neural network
       simulations. Neural Computation. 19(1):47-79.
       https://doi.org/10.1162/neco.2007.19.1.47

.. [3] Diesmann M, Hanuschkin A, Helias M, Kunkel S, Morrison A. 2008. The
       performance of solvers for integrate-and-fire models with exact spike
       timing. Frontiers in Neuroinformatics. Conference Abstract:
       Neuroinformatics 2008.

"""

###############################################################################
# First, we import all necessary modules for simulation, analysis, and
# plotting.

import matplotlib.pyplot as plt
import nest
import numpy as np

###############################################################################
# Second, we set the simulation parameters.

# Network parameters
nr = 128  # number of neurons in the network

# Simulation parameters
tics_per_ms = 2**5  # low-level time resolution in tics
dt = 1.0 / tics_per_ms  # simulation time step dt=2^-5 ms (must be multiple of tic length)
simtime = 10000.0  # simulation time [ms]

# Neuron parameters
C_m = 250.0  # membrane capacitance [pF]
E_L = 0.0  # leaky potential [mV]
I_e = 575.0  # suprathreshold current [pA]
tau_m = 10.0  # membrane time constant [ms]
V_reset = 0.0  # reset potential [mV]
V_th = 20.0  # threshold potential [mV]
t_refra = 0.25  # refractory time [ms]
tau_syn = 1.648  # synaptic time constant [ms] (3/2*ln(3))

# Initial conditions
gamma = 0.5  # parameter determining the degree of synchrony at the beginning

# Synaptic parameters
delay = 0.25  # synaptic delay [ms]
autapses = True  # allow self-connections

# Coupling strength range
strength_min = 0.0  # minimum coupling strength [pA]
strength_max = 5.0  # maximum coupling strength [pA]
strength_step = 0.2  # step size for coupling strength [pA]

# Synchrony calculation parameters
time_start = 5000.0  # start time for synchrony calculation [ms]
time_window = 5000.0  # time window for synchrony calculation [ms]

# Random number generator seed
rng_seed = 2000

###############################################################################
# Calculate spike period T (needed for setup of initial membrane potential)

R = tau_m / C_m
T = tau_m * np.log((R * I_e + E_L - V_reset) / (R * I_e + E_L - V_th))
firing_rate = 1000.0 / T  # firing rate [Hz]

print("\n" + "=" * 80)
print("Artificial Synchrony Example")
print("=" * 80)
print(f"gamma: {gamma}")
print(f"delay: {delay} ms")
print(f"tau_r: {t_refra} ms")
print(f"ISI: {T:.4f} ms")
print(f"Firing rate: {firing_rate:.2f} Hz")
print("=" * 80 + "\n")

###############################################################################
# Define function to calculate synchrony from membrane potential data


def calculate_synchrony(vm_events, time_start, time_window, neuron_gids):
    """
    Calculate synchrony measure Σ from membrane potential data.

    Parameters
    ----------
    vm_events : dict
        Dictionary containing 'times', 'senders', and 'V_m' arrays from voltmeter
    time_start : float
        Start time for synchrony calculation [ms]
    time_window : float
        Time window for synchrony calculation [ms]
    neuron_gids : list
        List of neuron GIDs (global IDs)

    Returns
    -------
    float
        Synchrony measure Σ = Δ_N / Δ
    """
    times = vm_events["times"]
    senders = vm_events["senders"]
    V_m = vm_events["V_m"]

    # Find indices for the time window
    time_end = time_start + time_window
    mask = (times >= time_start) & (times < time_end)
    times_window = times[mask]
    senders_window = senders[mask]
    V_m_window = V_m[mask]

    if len(times_window) == 0:
        return np.nan

    # Get unique time steps
    unique_times = np.unique(times_window)
    n_timesteps = len(unique_times)
    nr = len(neuron_gids)

    if n_timesteps == 0:
        return np.nan

    # Create mapping from GID to index
    gid_to_idx = {gid: idx for idx, gid in enumerate(neuron_gids)}

    # Reshape data: each row is a time step, each column is a neuron
    # Create a 2D array: (n_timesteps, nr)
    V_m_matrix = np.full((n_timesteps, nr), np.nan)

    # Fill the matrix with membrane potential values
    for i, t in enumerate(unique_times):
        time_mask = times_window == t
        time_senders = senders_window[time_mask]
        time_V_m = V_m_window[time_mask]

        # Map sender GIDs to neuron indices
        for sender, vm_val in zip(time_senders, time_V_m):
            if sender in gid_to_idx:
                neuron_idx = gid_to_idx[sender]
                V_m_matrix[i, neuron_idx] = vm_val

    # Remove any rows with NaN values (incomplete time steps)
    valid_rows = ~np.isnan(V_m_matrix).any(axis=1)
    V_m_matrix = V_m_matrix[valid_rows]

    if len(V_m_matrix) == 0:
        return np.nan

    # Calculate mean membrane potential across neurons at each time step
    mean_V_t = np.nanmean(V_m_matrix, axis=1)

    # Calculate Δ_N: variance of the mean membrane potential
    Delta_N = np.nanvar(mean_V_t)

    # Calculate Δ: mean variance of individual neuron membrane potentials
    Delta = np.nanmean(np.nanvar(V_m_matrix, axis=0))

    # Synchrony measure
    if Delta != 0:
        synchrony = Delta_N / Delta
    else:
        synchrony = np.nan

    return synchrony


###############################################################################
# Run simulations for both precise and grid-constrained models

results = {"precise": [], "grid": []}
strengths = list(np.arange(strength_min, strength_max + strength_step, strength_step))

for sim_type in ["precise", "grid"]:
    print(f"\n{'=' * 40}")
    if sim_type == "precise":
        print("Running precise simulations:")
    else:
        print("Running grid-constrained simulations:")
    print("=" * 40)

    for strength in strengths:
        print(f"  Weight: {strength:.1f} pA", end="", flush=True)

        # Reset kernel for each simulation
        nest.ResetKernel()

        # Set kernel parameters
        nest.set_verbosity("M_WARNING")
        # Set tics_per_ms first, then resolution (resolution must be multiple of tic length)
        nest.SetKernelStatus(
            {
                "tics_per_ms": tics_per_ms,
                "resolution": dt,
                "rng_seed": rng_seed,
                "overwrite_files": True,
            }
        )

        # Select neuron model (precise models have off-grid spiking enabled by default)
        if sim_type == "precise":
            model = "iaf_psc_alpha_ps"
        else:
            model = "iaf_psc_alpha"

        # Create neurons with parameters
        if sim_type == "precise":
            neurons = nest.Create(
                model,
                nr,
                params={
                    "C_m": C_m,
                    "E_L": E_L,
                    "I_e": I_e,
                    "tau_m": tau_m,
                    "tau_syn_ex": tau_syn,
                    "tau_syn_in": tau_syn,
                    "V_m": E_L,
                    "V_reset": V_reset,
                    "V_th": V_th,
                    "t_ref": t_refra,
                },
            )
        else:
            neurons = nest.Create(
                model,
                nr,
                params={
                    "C_m": C_m,
                    "E_L": E_L,
                    "I_e": I_e,
                    "tau_m": tau_m,
                    "tau_syn_ex": tau_syn,
                    "V_m": E_L,
                    "V_reset": V_reset,
                    "V_th": V_th,
                    "t_ref": t_refra,
                },
            )

        # Set initial membrane potentials (Morrison et al. 2007)
        V_0_values = []
        for i in range(nr):
            V_0 = (R * I_e) * (1.0 - np.exp(-gamma * i / nr * T / tau_m))
            V_0_values.append(V_0)
        neurons.V_m = V_0_values

        # Connect neurons all-to-all
        nest.Connect(
            neurons,
            neurons,
            conn_spec={"rule": "all_to_all", "allow_autapses": autapses},
            syn_spec={"weight": strength, "delay": delay},
        )

        # Create voltmeter
        voltmeter = nest.Create("voltmeter", params={"interval": 1.0, "record_to": "memory"})

        # Connect voltmeter to neurons
        nest.Connect(voltmeter, neurons)

        # Simulate
        nest.Simulate(simtime)

        # Get voltmeter data
        vm_events = voltmeter.events

        # Get neuron GIDs for mapping (extract from voltmeter events to ensure consistency)
        # The senders in vm_events are the neuron GIDs
        unique_senders = np.unique(vm_events["senders"])
        neuron_gids = sorted(unique_senders.tolist())

        # Calculate synchrony
        synchrony = calculate_synchrony(vm_events, time_start, time_window, neuron_gids)

        # Clean up for next iteration
        del voltmeter, neurons

        results[sim_type].append(synchrony)
        print(f" -> Synchrony: {synchrony:.4f}")

###############################################################################
# Plot results

plt.figure(figsize=(10, 6))
plt.plot(strengths, results["precise"], "o-", label="Precise (alpha_ps)", linewidth=2, markersize=6)
plt.plot(strengths, results["grid"], "s-", label="Grid-constrained (alpha)", linewidth=2, markersize=6)

plt.xlabel("Coupling strength [pA]", fontsize=14)
plt.ylabel("Synchrony Σ", fontsize=14)
plt.title(
    f"Network Synchrony of {nr} IAF neurons, γ = {gamma}, dt = 2⁻⁵ ms",
    fontsize=14,
)
plt.legend(fontsize=12)
plt.grid(True, alpha=0.3)
plt.xlim([0, strength_max])
plt.ylim([0, 1])

plt.tight_layout()
plt.show()

print("\n" + "=" * 80)
print("Simulation completed!")
print("=" * 80)
