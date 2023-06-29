# -*- coding: utf-8 -*-
#
# tsodyks_nrn_benchmark.py
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

import numpy as np

import nest

nest.set_verbosity(level="M_QUIET")  # make NEST less chatty


def brunel_tsodyks_network(
    n_procs=1, n_threads=1, rng_seed=143202461, kernel_params=None, model_params=None, sim_params=None
):
    if kernel_params is None:
        raise TypeError("Pass kernel_params as dict")
    if not isinstance(kernel_params, dict):
        raise TypeError("Pass kernel_params as dict")

    if model_params is None:
        raise TypeError("Pass model_params as dict")
    if not isinstance(model_params, dict):
        raise TypeError("Pass model_params as dict")

    if sim_params is None:
        raise TypeError("Pass sim_params as dict")
    if not isinstance(sim_params, dict):
        raise TypeError("Pass sim_params as dict")

    # Start a new NEST session
    nest.ResetKernel()

    # Set global kernel parameters
    nest.set(rng_seed=rng_seed, total_num_virtual_procs=n_procs * n_threads, **kernel_params)

    # Unpack a few variables for convenience
    JE = model_params["JE"]
    neuron_params_ex = model_params["neuron_params_ex"]
    neuron_params_in = model_params["neuron_params_in"]
    T_presim = sim_params["T_presim"]

    # COMPUTE DEPENDENT VARIABLES
    JI = -model_params["g"] * JE  # peak of IPSP [mV]

    # Threshold rate; the external rate needed for a neuron to reach
    # threshold in absence of feedback]
    nu_thresh = neuron_params_ex["V_th"] / (JE * model_params["CE"] * neuron_params_ex["tau_m"])

    # External firing rate; firing rate of a neuron in the external
    # population
    nu_ext = model_params["eta"] * nu_thresh

    # CREATE NETWORK ELEMENTS
    # Create excitatory nodes
    nodes_exc = nest.Create("iaf_tsodyks", model_params["NE"], params=neuron_params_ex)

    # Create inhibitory nodes
    nodes_inh = nest.Create("iaf_tsodyks", model_params["NI"], params=neuron_params_in)

    # Create Poisson generator; the excitatory stimuli from the external
    # population. The firing rate of the whole external population is given
    # by the product of nu_ext and the in-degree CE. The factor 1000.0 in
    # the product changes the units from spikes per ms to spikes per second.
    ext_stim = nest.Create(
        "poisson_generator", 1, params={"rate": 25000.0}  # {"rate": nu_ext * model_params["CE"] * 1000.}
    )

    # Create excitatory spike recorder
    espikes = nest.Create("spike_recorder", 1)

    # Create inhibitory spike recorder
    ispikes = nest.Create("spike_recorder", 1)

    # Randomize initial membrane potentials
    random_vm = nest.random.normal(model_params["mean_potential"], model_params["sigma_potential"])
    nest.GetLocalNodeCollection(nodes_exc).V_m = random_vm
    nest.GetLocalNodeCollection(nodes_inh).V_m = random_vm

    # Create static synapses by copying the pre-defined 'static_synapse' model
    nest.SetDefaults("static_synapse", {"delay": model_params["D"]})

    # Excitatory synapse model
    nest.CopyModel("static_synapse", "syn_exc", params={"weight": JE})

    # Inhibitory synapse model
    nest.CopyModel("static_synapse", "syn_inh", params={"weight": JI})

    # CONNECT NETWORK ELEMENTS
    nest.Connect(pre=ext_stim, post=nodes_exc, conn_spec="all_to_all", syn_spec="syn_exc")

    nest.Connect(pre=ext_stim, post=nodes_inh, conn_spec="all_to_all", syn_spec="syn_exc")

    # Connect nodes to the spike recorder using excitatory synapses.
    nest.Connect(pre=nodes_exc, post=espikes, conn_spec="all_to_all", syn_spec="syn_exc")

    nest.Connect(pre=nodes_inh, post=ispikes, conn_spec="all_to_all", syn_spec="syn_exc")

    # Connect the excitatory population
    nest.Connect(
        pre=nodes_exc,
        post=nodes_exc + nodes_inh,
        conn_spec={
            "rule": "fixed_indegree",
            "indegree": model_params["CE"],
            "allow_autapses": False,
            "allow_multapses": True,
        },
        syn_spec="syn_exc",
    )

    # Connect the inhibitory population
    nest.Connect(
        pre=nodes_inh,
        post=nodes_exc + nodes_inh,
        conn_spec={
            "rule": "fixed_indegree",
            "indegree": model_params["CI"],
            "allow_autapses": False,
            "allow_multapses": True,
        },
        syn_spec="syn_inh",
    )

    # SIMULATE
    # Pre-simulation to avoid transient effects
    nest.Simulate(T_presim)

    # Simulate network dynamics
    nest.Simulate(sim_params["T_sim"])

    # Read out recordings from subset of neurons and store spike trains
    # We remove spikes from pre-simulation and shift time so that
    # recorded spike times reflect the start and end of the data
    # gathering simulation

    events_exc = nest.GetStatus(espikes, "events")[0]
    events_inh = nest.GetStatus(ispikes, "events")[0]

    sts_exc = []  # excitatory spike trains
    sts_inh = []  # inhibitory spike trains

    for sender_exc in nodes_exc[: sim_params["N_rec_exc"]]:
        try:
            st_exc = events_exc["times"][events_exc["senders"] == sender_exc]
            st_exc = st_exc[st_exc >= T_presim] - T_presim
            sts_exc.append(st_exc.tolist())
        except IndexError:
            sts_exc.append([])

    for i, sender_inh in enumerate(nodes_inh[: sim_params["N_rec_inh"]]):
        try:
            st_inh = events_inh["times"][events_inh["senders"] == sender_inh]
            st_inh = st_inh[st_inh >= T_presim] - T_presim
            sts_inh.append(st_inh.tolist())
        except IndexError:
            sts_inh.append([])

    return sts_exc, sts_inh


if __name__ == "__main__":
    import os
    import platform
    import sys
    import time
    from pprint import pprint

    n_procs = 1
    n_threads = 8
    # n_procs = int(os.environ["SLURM_JOB_NUM_NODES"]) * int(os.environ["SLURM_NTASKS_PER_NODE"])
    # n_threads = int(os.environ["SLURM_CPUS_PER_TASK"])

    # Set network size
    order = int(1e4)
    epsilon = 0.2
    NE = 4 * order  # no. of excitatory neurons
    NI = 1 * order  # no. of inhibitory neurons
    CE = int(epsilon * NE)  # no. of excitatory synapses per neuron
    CI = int(epsilon * NI)  # no. of inhibitory synapses per neuron

    # Set scaling of memory print
    sys_scale = 1e6  # Used to scale memory usage for Linux systems
    if platform.system() == "Darwin":
        # Change memory scale if system is Darwin
        sys_scale = 1e9

    def memory_thisjob(scale=1.0):
        """Wrapper to obtain current memory usage

        `nest.ll_api.sr('memory_thisjob')` returns memory usage in B on
        Darwin and kB on Linux. The scale kw arg can be used to scale the
        returned memory accordlingly.
        """

        nest.ll_api.sr("memory_thisjob")
        return nest.ll_api.spp() / scale

    mem_ini = memory_thisjob(scale=sys_scale)

    kernel_params = {
        "resolution": 0.1,
        "rng_type": "mt19937",
        "print_time": True,
    }

    model_params = {
        "NE": NE,  # number of excitatory neurons
        "NI": NI,  # number of inhibitory neurons
        "CE": CE,  # number of excitatory synapses per neuron
        "CI": CI,  # number of inhibitory synapses per neuron
        "neuron_params_ex": {  # Set parameters for excitatory iaf_tsodyks
            "tau_m": 20,  # membrance time constant [ms]
            "t_ref": 2.0,  # refractory period [ms]
            "C_m": 250.0,  # membrane capacitance [pF]
            "E_L": 0.0,  # resting membrane potential [mV]
            "V_th": 20.0,  # threshold potential [mV]
            "V_reset": 0.0,  # reset potential [mV]
            "V_m": 9.5,  # mean membrane potential [mV]
            "tau_psc": 2.0,
            "tau_rec": 100.0,
            "tau_fac": 400.0,
            "U": 0.5,
            "u": 0.0,
            "x": 0.0,
            "y": 0.0,
        },
        "neuron_params_in": {  # Set parameters for excitatory iaf_tsodyks
            "tau_m": 20,  # membrance time constant [ms]
            "t_ref": 2.0,  # refractory period [ms]
            "C_m": 250.0,  # membrane capacitance [pF]
            "E_L": 0.0,  # resting membrane potential [mV]
            "V_th": 20.0,  # threshold potential [mV]
            "V_reset": 0.0,  # reset potential [mV]
            "V_m": 9.5,  # mean membrane potential [mV]
            "tau_psc": 2.0,
            "tau_rec": 100.0,
            "tau_fac": 400.0,
            "U": 0.05,
            "u": 0.0,
            "x": 0.0,
            "y": 0.0,
        },
        # network params
        "eta": 2.0,
        "g": 4.5,
        "D": 1.5,  # synaptic delay, all connections [ms]
        "JE": 5.0,  # peak of EPSP [mV]
        # initial membrane potentials are drawn from a normal distribution with:
        "mean_potential": 9.5,  # [mV]
        "sigma_potential": 5.0,  # [mV]
    }

    sim_params = {
        "T_sim": 500.0,  # total simulation time [ms]
        "T_presim": 0.0,  # equilibrating simulation time [ms]
        "N_rec_exc": 100,  # number of excitatory neurons to record
        "N_rec_inh": 25,  # number of inhibitory neurons to record
    }

    start_time = time.time()

    sts_exc, sts_inh = brunel_tsodyks_network(
        n_procs=n_procs,
        n_threads=n_threads,
        kernel_params=kernel_params,
        model_params=model_params,
        sim_params=sim_params,
    )

    end_time = time.time() - start_time
    mem_sim = memory_thisjob(scale=sys_scale)

    print(f"[Rank {nest.Rank()}] number of processes: {nest.GetKernelStatus('num_processes')}")
    print(f"[Rank {nest.Rank()}] number of threads: {nest.GetKernelStatus('local_num_threads')}")
    print(f"[Rank {nest.Rank()}] total simulation time: {sim_params['T_sim'] + sim_params['T_presim']} ms")
    print(f"[Rank {nest.Rank()}] number of neurons: {nest.GetKernelStatus('network_size'):,}")
    print(f"[Rank {nest.Rank()}] number of connections: {nest.GetKernelStatus('num_connections'):,}")
    print(f'[Rank {nest.Rank()}] number of spikes: {nest.GetKernelStatus("local_spike_counter"):,}')
    print(f"[Rank {nest.Rank()}] elapsed wall time: {end_time} s")
    print(f"[Rank {nest.Rank()}] total memory usage: {mem_sim - mem_ini} GB")

    """
    sts_exc_count = {}
    for i, st_exc in enumerate(sts_exc):
        sts_exc_count[f"node {i+1}"] = len(st_exc)

    sts_inh_count = {}
    for i, st_inh in enumerate(sts_inh):
        sts_inh_count[f"node {i+1}"] = len(st_inh)
    print("STS_EXC")
    pprint(sts_exc_count)
    print("\nSTS_INH")
    pprint(sts_inh_count)
    """
