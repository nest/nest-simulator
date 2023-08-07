# -*- coding: utf-8 -*-
#
# eprop_supervised_regression.py
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
Supervised learning of a regression task with e-prop plasticity
---------------------------------------------------------------

This script demonstrates supervised learning of a regression task with the
eligibility propagation (e-prop) plasticity mechanism by Bellec et al. [1]_.

This so-called pattern generation task exemplifies learning of arbitrary
temporal patterns. Here, the network learns to reproduce a target signal of
superimposed sine waves with its network activity.

Learning in the neural network is achieved by optimizing the weights in the
network with e-prop plasticity. The neural network model consists of a recurrent
network which receives frozen noise input from Poisson generators and projects
onto one readout neuron. The readout neuron compares the network signal with the
teacher target signal which it receives from a rate generator. The performance
of the network is evaluated by a mean-squared error loss.

More details on the event-based NEST implementation of e-prop can be found in [2]_.

References
~~~~~~~~~~

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R,
       Maass W (2020). A solution to the learning dilemma for recurrent
       networks of spiking neurons. Nature Communications, 11:3625.
       DOI: https://doi.org/10.1038/s41467-020-17236-y
.. [2] Korcsak-Gorzo A, Stapmanns J, Espinoza Valverde JA, Dahmen D,
       van Albada SJ, Bolten M, Diesmann M. Event-based implementation of
       eligibility propagation (in preparation)
"""
import matplotlib.pyplot as plt
import nest
import numpy as np
import pandas as pd

####################
# helper functions #
####################


def get_weights(pop_pre, pop_post):
    conns = nest.GetConnections(pop_pre, pop_post)
    conns_dict = {
        "sources": list(conns.sources()),
        "targets": list(conns.targets()),
        "weights": conns.get("weight"),
    }
    return conns_dict


def subsample(nrns, n_sample):
    n_nrns = len(nrns)
    nrns_chosen = nrns[:: int(n_nrns / n_sample)] if n_nrns > n_sample else nrns
    return nrns_chosen


def calculate_superimposed_sines(steps_sequence, periods):
    n_sines = len(periods)

    weights = np.random.uniform(low=0.5, high=2.0, size=n_sines)
    phases = np.random.uniform(low=0.0, high=2.0 * np.pi, size=n_sines)

    sines = [
        w * np.sin(np.linspace(phi, phi + 2.0 * np.pi * (steps_sequence // T), steps_sequence))
        for w, phi, T in zip(weights, phases, periods)
    ]

    superposition = sum(sines)
    superposition -= superposition[0]
    superposition /= max(np.abs(superposition).max(), 1e-6)
    return superposition


##############
# parameters #
##############

# === simulation ===
seed = 1  # numpy random seed and NEST random seed
time_step = 1.0  # ms, temporal resolution of the simulation
nvp = 1  # number of virtual processes
print_time = False  # if True, print time progress bar during simulation
overwrite_files = True  # if True, overwrite saved files

# === neurons ===
C_m = 1.0  # pF, membrane capacitance - takes effect only if neurons get current input - here not the case
gamma = 0.3  # scaling of the pseudo derivative
E_L = 0.0  # mV, leak reversal potential
I_e = 0.0  # pA, external current input
n_ref = 0  # number of refractory time steps
t_ref = n_ref * time_step  # ms, duration of refractory period
tau_m_rec = 30.0  # ms, membrane time constant of recurrent neurons
tau_m_out = 30.0  # ms, membrane time constant of readout neuron
V_th = 0.03  # mV, spike threshold membrane voltage

V_m_init = 0.0  # mV, initial value of the membrane voltage

# === synapses ===
dtype_weights = np.float32  # data type of weights - for reproducing TF results set to np.float32
tau_decay = tau_m_out  # ms, decay time constant - has to match tau_m_out for technical reasons
delay = time_step  # ms, dendritic delay of all synapses
receptor_type_target_rate = 2  # receptor type over which readout neuron receives target signal

# === network ===
n_in = 100  # number of input neurons
n_rec = 100  # number of recurrent neurons
n_out = 1  # number of readout neurons

# === task ===
regression = True  # if True, regression; if False, classification

sine_periods = [1000.0, 500.0, 333.0, 200.0]  # ms, periods of superimposed sine waves composing the target signal

n_batch = 1  # batch size
n_iter = 5  # number of iterations

steps_recall = 1  # time steps of recall
steps_recall_onset = 0  # time steps until recall onset
steps_sequence = 1000  # time steps of one full sequence
steps_task = n_iter * n_batch * steps_sequence  # time steps of task
steps_shift = 4  # time steps of shift - as generators and multimeter act on timesteps > 0 + signal traveling time

duration_sequence = steps_sequence * time_step  # ms, time duration of one sequence
duration_recall_onset = steps_recall_onset * time_step  # ms, time duration until recall onset
duration_recall = steps_recall * time_step  # ms, time duration of recall
duration_task = steps_task * time_step  # ms, time duration of task
duration_shift = steps_shift * time_step  # ms, time duration of shift
duration_sim = duration_task + duration_shift  # ms, time duration of simulation

# === input ===
input_spike_rate = 0.05  # kHz, firing rate of frozen input noise
spike_probability = time_step * input_spike_rate  # spike probability of frozen input noise
dtype_in_spks = np.float32  # data type of input spikes - for reproducing TF results set to np.float32

# === plasticity ===
eta = 1e-4  # learning rate
update_interval = duration_sequence  # ms, interval for updating the synapse weights
update_interval_reset = True  # if True, reset dynamic variables at the beginning of each update interval

c_reg = 300.0  # firing rate regularization scaling
target_firing_rate = 10.0  # Hz, target firing rate for firing rate regularization

adam = False  # if True, use Adam optimizer, if False gradient descent

Wmin = -100.0  # pA, minimal limit of the synaptic weights
Wmax = 100.0  # pA, maximal limit of the synaptic weights

# === recording ===
n_record = 1  # number of recorded adaptive and regular neurons each
n_sample_wr = 10  # number of senders and targets to record weights from
record_to = "memory"  # where to record to: 'ascii' or 'memory'
origin = 0.0  # start point of recording

record_from_rec = ["V_m", "V_m_pseudo_deriv", "learning_signal"]  # recordables recurrent neurons
record_from_out = ["V_m", "readout_signal", "target_signal", "error_signal"]  # recordables readout neurons

# === plotting ===

is_show_plot = True  # if True, show plot of results

#########################
# fix numpy random seed #
#########################

np.random.seed(seed)

###############
# model names #
###############

model_gen_spk_in = "spike_generator"
model_nrn_in = "parrot_neuron"  # parrot neurons required for plastic synapses between devices and neurons
model_nrn_rec = "eprop_iaf_psc_delta"
model_nrn_out = "eprop_readout"
model_gen_rate_target = "step_rate_generator"

model_syn_eprop_in = "eprop_synapse_wr_in"
model_syn_eprop_rec = "eprop_synapse_wr_rec"
model_syn_eprop_out = "eprop_synapse_wr_out"

model_syn_feedback = "eprop_learning_signal_connection"
model_syn_target_rate = "rate_connection_delayed"
model_syn_static = "static_synapse"

#####################
# neuron parameters #
#####################

params_nrn_rec = {
    "C_m": C_m,
    "gamma": gamma,
    "E_L": E_L,
    "I_e": I_e,
    "regression": regression,
    "t_ref": t_ref,
    "tau_m": tau_m_rec,
    "V_m": V_m_init,
    "V_th": V_th,
}

params_nrn_out = {
    "C_m": C_m,
    "E_L": E_L,
    "I_e": I_e,
    "regression": regression,
    "start_learning": duration_recall_onset,
    "tau_m": tau_m_out,
    "V_m": V_m_init,
}

#########################
# connection parameters #
#########################

params_conn_all_to_all = {"rule": "all_to_all", "allow_autapses": False}
params_conn_one_to_one = {"rule": "one_to_one"}

######################
# synapse parameters #
######################

weights_in = np.array(np.random.randn(n_in, n_rec).T / np.sqrt(n_in), dtype=dtype_weights)
weights_rec = np.array(np.random.randn(n_rec, n_rec).T / np.sqrt(n_rec), dtype=dtype_weights)
np.fill_diagonal(weights_rec, 0.0)  # remove potential autapses
weights_out = np.array(np.random.randn(n_rec, n_out).T / np.sqrt(n_rec), dtype=dtype_weights)
weights_feedback = np.array(np.random.randn(n_rec, n_out) / np.sqrt(n_rec), dtype=dtype_weights)

params_common_syn_eprop = {
    "adam": adam,
    "batch_size": n_batch,
    "recall_duration": duration_recall,
}

params_syn_in = {
    "c_reg": c_reg,
    "delay": delay,
    "eta": eta,
    "synapse_model": model_syn_eprop_in,
    "target_firing_rate": target_firing_rate,
    "tau_decay": tau_decay,
    "weight": weights_in,
    "Wmax": Wmax,
    "Wmin": Wmin,
}

params_syn_rec = {
    "c_reg": c_reg,
    "delay": delay,
    "eta": eta,
    "synapse_model": model_syn_eprop_rec,
    "target_firing_rate": target_firing_rate,
    "tau_decay": tau_decay,
    "weight": weights_rec,
    "Wmax": Wmax,
    "Wmin": Wmin,
}

params_syn_out = {
    "delay": delay,
    "eta": eta,
    "synapse_model": model_syn_eprop_out,
    "tau_decay": tau_decay,
    "weight": weights_out,
    "Wmax": Wmax,
    "Wmin": Wmin,
}

params_syn_feedback = {
    "delay": delay,
    "synapse_model": model_syn_feedback,
    "weight": weights_feedback,
}

params_syn_target_rate = {
    "delay": delay,
    "receptor_type": receptor_type_target_rate,
    "synapse_model": model_syn_target_rate,
}

params_syn_static = {
    "delay": delay,
    "synapse_model": model_syn_static,
}

########################
# generator parameters #
########################

input_spike_bools = np.random.rand(n_batch, steps_sequence, n_in) < spike_probability
input_spike_bools = np.hstack(input_spike_bools.swapaxes(1, 2))
input_spike_bools[:, 0] = 0  # NEST does not allow spike emission in first time step

sequence_starts = np.arange(0.0, duration_task, duration_sequence)
params_gen_spk_in = []
for input_spike_bool in input_spike_bools:
    input_spike_times = np.arange(duration_sequence * n_batch)[input_spike_bool]
    input_spike_times_all = np.hstack([input_spike_times + start for start in sequence_starts]).astype(np.float32)
    params_gen_spk_in.append({"spike_times": input_spike_times_all.tolist()})

params_gen_rate_target = {
    # shift by one time step since NEST does not allow rate generation in 0th time step
    "amplitude_times": np.arange(time_step, duration_task + time_step, time_step).tolist(),
    "amplitude_values": np.tile(calculate_superimposed_sines(steps_sequence, sine_periods), n_iter * n_batch).tolist(),
}

#######################
# recorder parameters #
#######################

# note that multimeter only records from time step 1

params_recorder = {"record_to": record_to, "origin": origin}

params_mm = dict(params_recorder, **{"interval": time_step})

params_mm_rec = dict(params_mm, **{"label": "multimeter_rec", "record_from": record_from_rec})
params_mm_out = dict(params_mm, **{"label": "multimeter_out", "record_from": record_from_out})

params_sr_in = dict(params_recorder, **{"label": "spike_recorder_in"})
params_sr_rec = dict(params_recorder, **{"label": "spike_recorder_rec"})

params_wr_in = dict(params_recorder, **{"label": "weight_recorder_in"})
params_wr_rec = dict(params_recorder, **{"label": "weight_recorder_rec"})
params_wr_out = dict(params_recorder, **{"label": "weight_recorder_out"})

#########
# setup #
#########

nest.ResetKernel()

nest.SetKernelStatus(
    {
        "resolution": time_step,
        "total_num_virtual_procs": nvp,
        "print_time": print_time,
        "overwrite_files": overwrite_files,
        "rng_seed": seed,
        "eprop_update_interval": update_interval,
        "eprop_update_interval_reset": update_interval_reset,
    }
)

#################################
# create neurons and generators #
#################################

gen_spk_in = nest.Create(model_gen_spk_in, n_in, params_gen_spk_in)
nrns_in = nest.Create(model_nrn_in, n_in)
nrns_rec = nest.Create(model_nrn_rec, n_rec, params_nrn_rec)
nrns_out = nest.Create(model_nrn_out, n_out, params_nrn_out)
gen_rate_target = nest.Create(model_gen_rate_target, n_out, params_gen_rate_target)

###################
# create recorder #
###################

mm_rec = nest.Create("multimeter", params_mm_rec)
mm_out = nest.Create("multimeter", params_mm_out)

sr_in = nest.Create("spike_recorder", params_sr_in)
sr_rec = nest.Create("spike_recorder", params_sr_rec)

params_wr_in.update({"senders": subsample(nrns_in, n_sample_wr), "targets": subsample(nrns_rec, n_sample_wr)})
params_wr_rec.update({"senders": subsample(nrns_rec, n_sample_wr), "targets": subsample(nrns_rec, n_sample_wr)})
params_wr_out.update({"senders": subsample(nrns_rec, n_sample_wr), "targets": subsample(nrns_out, n_sample_wr)})

wr_in = nest.Create("weight_recorder", params_wr_in)
wr_rec = nest.Create("weight_recorder", params_wr_rec)
wr_out = nest.Create("weight_recorder", params_wr_out)

######################
# create connections #
######################

nest.SetDefaults("eprop_synapse", params_common_syn_eprop)

nest.CopyModel("eprop_synapse", model_syn_eprop_in)
nest.CopyModel("eprop_synapse", model_syn_eprop_rec)
nest.CopyModel("eprop_synapse", model_syn_eprop_out)

nest.SetDefaults(model_syn_eprop_in, {"weight_recorder": wr_in})
nest.SetDefaults(model_syn_eprop_rec, {"weight_recorder": wr_rec})
nest.SetDefaults(model_syn_eprop_out, {"weight_recorder": wr_out})

nest.Connect(gen_spk_in, nrns_in, params_conn_one_to_one, params_syn_static)
nest.Connect(nrns_in, nrns_rec, params_conn_all_to_all, params_syn_in)
nest.Connect(nrns_rec, nrns_rec, params_conn_all_to_all, params_syn_rec)
nest.Connect(nrns_rec, nrns_out, params_conn_all_to_all, params_syn_out)
nest.Connect(nrns_out, nrns_rec, params_conn_all_to_all, params_syn_feedback)
nest.Connect(gen_rate_target, nrns_out, params_conn_one_to_one, params_syn_target_rate)

nest.Connect(nrns_rec, sr_rec, params_conn_all_to_all, params_syn_static)
nest.Connect(nrns_in, sr_in, params_conn_all_to_all, params_syn_static)

nest.Connect(mm_rec, nrns_rec[:n_record], params_conn_all_to_all, params_syn_static)
nest.Connect(mm_out, nrns_out, params_conn_all_to_all, params_syn_static)

##################
# run simulation #
##################

nest.Simulate(duration_sim)

##################
# postprocessing #
##################

# note that for technical reasons, after simulation, the correct weights can only be read out for those
# synapses which were updated, i.e., which transmitted a spike, in the last update update interval

weights_post_train_dict = {
    "in_rec": get_weights(nrns_in, nrns_rec),
    "rec_rec": get_weights(nrns_rec, nrns_rec),
    "rec_out": get_weights(nrns_rec, nrns_out),
    "out_rec": get_weights(nrns_out, nrns_rec),
}

########################
# evaluate performance #
########################

events_out = nest.GetStatus(mm_out, "events")[0]
vms = events_out["V_m"]

readout = vms[2:-1]  # shift readout by 2 time steps to align it with the target signal
target = params_gen_rate_target["amplitude_values"]
error = (readout - target) ** 2
loss = 0.5 * np.add.reduceat(error, np.arange(0, steps_task, steps_sequence))

performance_df = pd.DataFrame.from_dict(
    {
        "iterations": range(n_iter),
        "loss": loss.tolist(),
    }
)

################
# verification #
################

losses = performance_df.loss.tolist()

losses_verification = [
    101.96435699904158,
    103.47003486967037,
    103.34261776087175,
    103.68305568057912,
    104.41600561835052,
]

if losses[:5] == losses_verification:
    print("\nverification successful\n")
else:
    print("\nverification FAILED !!! \n")


############
# plotting #
############

fig, ax = plt.subplots()

ax.plot(performance_df.iterations, performance_df.loss, c="#2854c5ff")
ax.set_ylabel(r"$E = \frac{1}{2} \sum_{t,k} \left( y_k^t -y_k^{*,t}\right)^2$")
ax.set_xlabel("training iteration")
ax.spines["top"].set_visible(False)
ax.spines["right"].set_visible(False)
ax.set_xticks(performance_df.iterations)
ax.grid()
fig.tight_layout()

if is_show_plot:
    plt.show()
