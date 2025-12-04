# -*- coding: utf-8 -*-
#
# brette_et_al_2007_benchmark.py
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
Brette et al. 2007 Benchmark Framework
--------------------------------------

This module provides a common framework for running the Brette et al. 2007
simulator review benchmarks. The benchmarks create sparsely coupled networks
of excitatory and inhibitory neurons which exhibit self-sustained activity
after an initial stimulus. The model is based on the Vogels & Abbott network model [1]_.

This framework is used by the individual benchmark scripts:
- ``coba.py`` (Benchmark 1: Conductance-based synapses)
- ``cuba.py`` (Benchmark 2: Current-based synapses)
- ``hh_coba.py`` (Benchmark 3: Hodgkin-Huxley neurons)
- ``cuba_ps.py`` (Benchmark 4: Precise spiking)

"""

import time

import nest


def compute_rate(spike_recorder, n_rec, simtime, num_processes=1):
    """
    Compute average firing rate per neuron from spike recorder.

    Parameters
    ----------
    spike_recorder : NodeCollection
        Spike recorder device
    n_rec : int
        Number of neurons recorded from
    simtime : float
        Simulation time [ms]
    num_processes : int
        Number of MPI processes (default: 1)

    Returns
    -------
    float
        Average firing rate per neuron [spikes/s]
    """
    # Assume evenly distributed neurons across processes, which is valid here but not in general.
    n_spikes = spike_recorder.n_events
    n_neurons = n_rec / num_processes
    rate = n_spikes / (n_neurons * simtime) * 1000.0  # convert to spikes/s
    return rate


def build_network(params):
    """
    Build the benchmark network.

    Parameters
    ----------
    params : dict
        Dictionary containing all benchmark parameters

    Returns
    -------
    tuple
        (E_neurons, I_neurons, E_stimulus, E_recorder, I_recorder, build_time)
    """
    start_time = time.time()

    # Set kernel parameters
    nest.SetKernelStatus(
        {
            "resolution": params["dt"],
            "total_num_virtual_procs": params["virtual_processes"],
            "overwrite_files": True,
        }
    )

    # Set default neuron parameters
    nest.SetDefaults(params["model"], params["model_params"])

    # Create populations
    print("Creating excitatory population...")
    E_neurons = nest.Create(params["model"], params["NE"])

    print("Creating inhibitory population...")
    I_neurons = nest.Create(params["model"], params["NI"])

    # Create stimulus generator
    print("Creating excitatory stimulus generator...")
    E_stimulus = nest.Create(params["stimulus"])
    E_stimulus.set(params["stimulus_params"])

    # Create spike recorders
    print("Creating spike recorders...")
    E_recorder = nest.Create(params["recorder"], params["recorder_params"])

    I_recorder = nest.Create(params["recorder"])
    I_recorder.set(params["recorder_params"])

    # Calculate connection counts
    CE = int(params["NE"] * params["epsilon"])  # excitatory connections per neuron
    CI = int(params["NI"] * params["epsilon"])  # inhibitory connections per neuron

    # Create custom synapse models
    nest.CopyModel("static_synapse", "syn_ex", params["E_synapse_params"])
    nest.CopyModel("static_synapse", "syn_in", params["I_synapse_params"])
    nest.SetDefaults("syn_ex", {"delay": params["delay"]})
    nest.SetDefaults("syn_in", {"delay": params["delay"]})

    # Connect populations
    print("Connecting excitatory population...")
    nest.Connect(
        E_neurons,
        E_neurons,
        conn_spec={"rule": "fixed_indegree", "indegree": CE},
        syn_spec="syn_ex",
    )

    # I -> E
    nest.Connect(
        I_neurons,
        E_neurons,
        conn_spec={"rule": "fixed_indegree", "indegree": CI},
        syn_spec="syn_in",
    )

    print("Connecting inhibitory population...")
    # E -> I
    nest.Connect(
        E_neurons,
        I_neurons,
        conn_spec={"rule": "fixed_indegree", "indegree": CE},
        syn_spec="syn_ex",
    )

    # I -> I
    nest.Connect(
        I_neurons,
        I_neurons,
        conn_spec={"rule": "fixed_indegree", "indegree": CI},
        syn_spec="syn_in",
    )

    # Connect stimulus
    print("Connecting Poisson stimulus...")
    nest.Connect(
        E_stimulus,
        E_neurons[: params["Nstim"]],
        conn_spec="all_to_all",
        syn_spec="syn_ex",
    )

    # Connect spike recorders
    print("Connecting spike recorders...")
    nest.Connect(E_neurons[: params["Nrec"]], E_recorder)
    nest.Connect(I_neurons[: params["Nrec"]], I_recorder)

    build_time = time.time() - start_time

    return E_neurons, I_neurons, E_stimulus, E_recorder, I_recorder, build_time


def run_simulation(params):
    """
    Build and run the benchmark simulation.

    Parameters
    ----------
    params : dict
        Dictionary containing all benchmark parameters

    Returns
    -------
    dict
        Dictionary containing simulation results and timing information
    """
    nest.ResetKernel()

    # Build network
    E_neurons, I_neurons, E_stimulus, E_recorder, I_recorder, build_time = build_network(params)

    # Run simulation
    print("Simulating...")
    start_sim = time.time()
    nest.Simulate(params["simtime"])
    sim_time = time.time() - start_sim

    # Calculate number of synapses
    N = len(E_neurons) + len(I_neurons)
    CE = int(params["NE"] * params["epsilon"])
    CI = int(params["NI"] * params["epsilon"])
    Nsyn = (CE + CI) * N + params["Nrec"] * 2 + params["Nstim"]

    # Compute rates
    num_processes = nest.num_processes
    E_rate = compute_rate(E_recorder, params["Nrec"], params["simtime"], num_processes)
    I_rate = compute_rate(I_recorder, params["Nrec"], params["simtime"], num_processes)

    # Print summary
    print("\n" + "=" * 60)
    print("Simulation summary")
    print("=" * 60)
    print(f"Number of Neurons : {N}")
    print(f"Number of Synapses: {Nsyn}")
    print(f"Excitatory rate   : {E_rate:.2f} spikes/s")
    print(f"Inhibitory rate   : {I_rate:.2f} spikes/s")
    print(f"Building time     : {build_time:.2f} s")
    print(f"Simulation time   : {sim_time:.2f} s")
    print("=" * 60 + "\n")

    return {
        "N": N,
        "Nsyn": Nsyn,
        "E_rate": E_rate,
        "I_rate": I_rate,
        "build_time": build_time,
        "sim_time": sim_time,
    }
