# -*- coding: utf-8 -*-
#
# eprop_supervised_classification.py
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
Supervised learning of a classification task with e-prop plasticity
-------------------------------------------------------------------

This script demonstrates supervised learning of a classification task with the
eligibility propagation (e-prop) plasticity mechanism by Bellec et al. [1]_.

This so-called evidence accumulation task is inspired by behavioral tasks, where
a lab animal (e.g., a so-called evidence accumulation task with mouse) runs
along a track, gets cues on the left and right, and has to decide t the end of
the track between taking a left and a right turn of which one is correct.  After
a number of iterations, the animal is able to infer the underlying principle of
the task. Here, the solution is to turn to the side of which a higher number of
cues was presented.

Learning in the neural network model is achieved by optimizing its weights with
e-prop plasticity. The neural network model consists of a recurrent network
which receives input from Poisson generators and projects onto two readout
neurons - one for the left and one for the right turn at the end. The input
neurons are divided into 4 groups: one group providing background noise of a
certain rate for some base activity throughout the experiment, one group
providing the input spikes of the left cues and one group providing them for the
right cues, and a last group defining the recall window, in which the network
has to decide. The teacher target signal indicating the correct solution is
provided by a rate generator that projects onto the readout neurons. Since the
decision is at the end and all the cues are relevant for it, the network has to
keep the cues in memory. This memory is enabled by additional adaptive neurons
in the network. The performance of the network is evaluated by a cross-entropy
loss.

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


def calculate_glorot_dist(fan_in, fan_out):
    glorot_scale = 1.0 / max(1.0, (fan_in + fan_out) / 2.0)
    glorot_limit = np.sqrt(3.0 * glorot_scale)
    glorot_distribution = np.random.uniform(low=-glorot_limit, high=glorot_limit, size=(fan_in, fan_out))
    return glorot_distribution


def calculate_adapt_beta(adapt_tau, tau):
    adapt_beta = 1.7 * (1.0 - np.exp(-1.0 / adapt_tau)) / (1.0 - np.exp(-1.0 / tau))
    return adapt_beta


def generate_task_input_output(
    n_batch, n_in, p_group, input_spike_rate, n_cues, n_input_symbols, steps_sequence, steps_spacing, steps_cue
):
    n_pop_nrn = n_in // n_input_symbols

    prob_choices = np.array([p_group, 1 - p_group], dtype=np.float32)
    idx = np.random.choice([0, 1], n_batch)
    probs = np.zeros((n_batch, 2), dtype=np.float32)
    probs[:, 0] = prob_choices[idx]
    probs[:, 1] = prob_choices[1 - idx]

    batched_cues = np.zeros((n_batch, n_cues), dtype=int)
    for b_idx in range(n_batch):
        batched_cues[b_idx, :] = np.random.choice([0, 1], n_cues, p=probs[b_idx])

    input_spike_prob = np.zeros((n_batch, steps_sequence, n_in))

    for b_idx in range(n_batch):
        for c_idx in range(n_cues):
            cue = batched_cues[b_idx, c_idx]

            t_start = c_idx * (steps_cue + steps_spacing) + steps_spacing
            t_stop = t_start + steps_cue

            pop_nrn_start = cue * n_pop_nrn
            pop_nrn_stop = pop_nrn_start + n_pop_nrn

            input_spike_prob[b_idx, t_start:t_stop, pop_nrn_start:pop_nrn_stop] = input_spike_rate

    input_spike_prob[:, -steps_recall:, 2 * n_pop_nrn : 3 * n_pop_nrn] = input_spike_rate
    input_spike_prob[:, :, 3 * n_pop_nrn :] = input_spike_rate / 4.0
    input_spike_bools = input_spike_prob > np.random.rand(input_spike_prob.size).reshape(input_spike_prob.shape)
    input_spike_bools[:, [0, -1], :] = 0  # remove spikes in first and last time step due to technical reasons

    target_cues = np.zeros(n_batch, dtype=int)
    target_cues[:] = np.sum(batched_cues, axis=1) > int(n_cues / 2)

    return input_spike_bools, target_cues


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
n_ref = 5  # number of refractory time steps
t_ref = n_ref * time_step  # ms, duration of refractory period
tau_m_rec = 20.0  # ms, membrane time constant of recurrent neurons
tau_m_out = 20.0  # ms, membrane time constant of readout neurons
V_th = 0.6  # mV, spike threshold membrane voltage

V_m_init = 0.0  # mV, initial value of the membrane voltage
adaptation_init = 0.0  # initial value of the spike threshold adaptation

adapt_tau = 2000.0  # ms, time constant of adaptive threshold
adapt_beta = calculate_adapt_beta(adapt_tau, tau_m_rec)  # prefactor of adaptive threshold

# === synapses ===
dtype_weights = np.float32  # data type of weights - for reproducing TF results set to np.float32
tau_decay = tau_m_out  # ms, decay time constant - has to match tau_m_out for technical reasons
weight_out_out = 1.0  # pA, weight connection readout neurons, 1.0 required for correct softmax computation
delay = time_step  # ms, dendritic delay of all synapses
receptor_type_out_out = 1  # receptor type of readout neuron to receive other readout neuron's signals for softmax
receptor_type_target_rate = 2  # receptor type over which readout neuron receives target signal

# === network ===
n_in = 40  # number of input neurons
n_ad = 50  # number of adaptive neurons
n_reg = 50  # number of regular neurons
n_rec = n_ad + n_reg  # number of recurrent neurons
n_out = 2  # number of readout neurons

# === task ===
regression = False  # if True, regression; if False, classification

n_input_symbols = 4  # number of input populations, e.g. 4 = left, right, recall, noise
n_cues = 7  # number of cues given before decision
p_group = 0.3  # probability with which one input group is present

n_batch = 1  # batch size
n_iter = 5  # number of iterations

steps_cue = 100  # time steps in one cue presentation
steps_spacing = 50  # time steps of break between two cues
steps_bg_noise = 1050  # time steps of background noise
steps_recall = 150  # time steps of recall
steps_cues = n_cues * (steps_cue + steps_spacing)  # time steps of all cues
steps_recall_onset = steps_cues + steps_bg_noise  # time steps until recall onset
steps_sequence = steps_recall_onset + steps_recall  # time steps of one full sequence
steps_task = n_iter * n_batch * steps_sequence  # time steps of task
steps_shift = 4  # time steps of shift - as generators and multimeter act on timesteps > 0 + signal traveling time

duration_sequence = steps_sequence * time_step  # ms, time duration of one sequence
duration_recall_onset = steps_recall_onset * time_step  # ms, time duration until recall onset
duration_recall = steps_recall * time_step  # ms, time duration of recall
duration_task = steps_task * time_step  # ms, time duration of task
duration_shift = steps_shift * time_step  # ms, time duration of shift
duration_sim = duration_task + duration_shift  # ms, time duration of simulation

# === input ===
input_spike_rate = 0.04  # kHz, firing rate of frozen input noise
dtype_in_spks = np.float32  # data type of input spikes - for reproducing TF results set to np.float32

# === plasticity ===
eta = 5e-3  # learning rate
update_interval = duration_sequence  # ms, interval for updating the synapse weights
update_interval_reset = True  # if True, reset dynamic variables at the beginning of each update interval

c_reg = 2.0  # firing rate regularization scaling - double the TF c_reg for classification for technical reasons
target_firing_rate = 10.0  # Hz, target firing rate for firing rate regularization

adam = True  # if True, use Adam optimizer, if False gradient descent
adam_beta1 = 0.9  # exponential decay rate for 1st moment estimate of Adam optimizer
adam_beta2 = 0.999  # exponential decay rate for 2nd moment raw estimate of Adam optimizer
adam_epsilon = 1e-8  # small numerical stabilization constant of Adam optimizer
adam_m = 0.0  # initial 1st moment estimate m of Adam optimizer
adam_v = 0.0  # initial 2nd moment raw estimate v of Adam optimizer

Wmin = -100.0  # pA, minimal limit of the synaptic weights
Wmax = 100.0  # pA, maximal limit of the synaptic weights

# === recording ===
n_record = 1  # number of recorded adaptive and regular neurons each
n_sample_wr = 10  # number of senders and targets to record weights from
record_to = "memory"  # where to record to: 'ascii' or 'memory'
origin = 0.0  # start point of recording

record_from_reg = ["V_m", "V_m_pseudo_deriv", "learning_signal"]  # recordables regular neurons
record_from_ad = record_from_reg + ["adapting_threshold", "adaptation"]  # recordables adaptive neurons
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
model_nrn_reg = "eprop_iaf_psc_delta"
model_nrn_ad = "eprop_iaf_psc_delta_adapt"
model_nrn_out = "eprop_readout"
model_gen_rate_target = "step_rate_generator"

model_syn_eprop_in = "eprop_synapse_wr_in"
model_syn_eprop_rec = "eprop_synapse_wr_rec"
model_syn_eprop_out = "eprop_synapse_wr_out"

model_syn_feedback = "eprop_learning_signal_connection"
model_syn_out_out = "rate_connection_delayed"
model_syn_target_rate = "rate_connection_delayed"
model_syn_static = "static_synapse"

#####################
# neuron parameters #
#####################

params_nrn_reg = {
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

params_nrn_ad = {
    "adapt_beta": adapt_beta,
    "adapt_tau": adapt_tau,
    "adaptation": adaptation_init,
    "C_m": C_m,
    "E_L": E_L,
    "gamma": gamma,
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
weights_out = np.array(calculate_glorot_dist(n_rec, n_out).T, dtype=dtype_weights)
weights_feedback = np.array(np.random.randn(n_rec, n_out), dtype=dtype_weights)

params_common_syn_eprop = {
    "adam": adam,
    "adam_beta1": adam_beta1,
    "adam_beta2": adam_beta2,
    "adam_epsilon": adam_epsilon,
    "batch_size": n_batch,
    "recall_duration": duration_recall,
}

params_syn_in = {
    "adam_m": adam_m,
    "adam_v": adam_v,
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
    "adam_m": adam_m,
    "adam_v": adam_v,
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
    "adam_m": adam_m,
    "adam_v": adam_v,
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

params_syn_out_out = {
    "delay": delay,
    "receptor_type": receptor_type_out_out,
    "synapse_model": model_syn_out_out,
    "weight": weight_out_out,
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

input_spike_bools_list = []
target_cues_list = []

for iteration in range(n_iter):
    input_spike_bools, target_cues = generate_task_input_output(
        n_batch, n_in, p_group, input_spike_rate, n_cues, n_input_symbols, steps_sequence, steps_spacing, steps_cue
    )
    input_spike_bools_list.append(input_spike_bools)
    target_cues_list.extend(target_cues.tolist())

input_spike_bools_arr = np.array(input_spike_bools_list).reshape(steps_task, n_in)

timeline_task = np.arange(0, duration_task, time_step)

params_gen_spk_in = [
    {"spike_times": timeline_task[input_spike_bools_arr[:, nrn_in_idx]].astype(dtype_in_spks).tolist()}
    for nrn_in_idx in range(n_in)
]

target_rate_changes = np.zeros((n_out, n_batch * n_iter))
target_rate_changes[np.array(target_cues_list), np.arange(n_batch * n_iter)] = 1

params_gen_rate_target = [
    {
        # shift by one time step since NEST does not allow rate generation in 0th time step
        "amplitude_times": np.arange(time_step, duration_task + time_step, duration_sequence).tolist(),
        "amplitude_values": target_rate_changes[nrn_out_idx].tolist(),
    }
    for nrn_out_idx in range(n_out)
]

#######################
# recorder parameters #
#######################

# note that multimeter only records from time step 1

params_recorder = {"record_to": record_to, "origin": origin}

params_mm = dict(params_recorder, **{"interval": time_step})

params_mm_reg = dict(params_mm, **{"label": "multimeter_reg", "record_from": record_from_reg})
params_mm_ad = dict(params_mm, **{"label": "multimeter_ad", "record_from": record_from_ad})
params_mm_out = dict(params_mm, **{"label": "multimeter_out", "record_from": record_from_out})

params_sr_in = dict(params_recorder, **{"label": "spike_recorder_in"})
params_sr_reg = dict(params_recorder, **{"label": "spike_recorder_reg"})
params_sr_ad = dict(params_recorder, **{"label": "spike_recorder_ad"})

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
nrns_reg = nest.Create(model_nrn_reg, n_reg, params_nrn_reg)
nrns_ad = nest.Create(model_nrn_ad, n_ad, params_nrn_ad)
nrns_out = nest.Create(model_nrn_out, n_out, params_nrn_out)
gen_rate_target = nest.Create(model_gen_rate_target, n_out, params_gen_rate_target)

nrns_rec = nrns_reg + nrns_ad

###################
# create recorder #
###################

mm_reg = nest.Create("multimeter", params_mm_reg)
mm_ad = nest.Create("multimeter", params_mm_ad)
mm_out = nest.Create("multimeter", params_mm_out)

sr_in = nest.Create("spike_recorder", params_sr_in)
sr_reg = nest.Create("spike_recorder", params_sr_reg)
sr_ad = nest.Create("spike_recorder", params_sr_ad)

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
nest.Connect(nrns_out, nrns_out, params_conn_all_to_all, params_syn_out_out)
nest.Connect(gen_rate_target, nrns_out, params_conn_one_to_one, params_syn_target_rate)

nest.Connect(nrns_reg, sr_reg, params_conn_all_to_all, params_syn_static)
nest.Connect(nrns_ad, sr_ad, params_conn_all_to_all, params_syn_static)
nest.Connect(nrns_in, sr_in, params_conn_all_to_all, params_syn_static)

nest.Connect(mm_reg, nrns_reg[:n_record], params_conn_all_to_all, params_syn_static)
nest.Connect(mm_ad, nrns_ad[:n_record], params_conn_all_to_all, params_syn_static)
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
sender = events_out["senders"]
vms = events_out["V_m"]

readout = np.array([vms[sender == i] for i in np.unique(sender)])
readout = readout[:, 2:-1]  # shift readout by 2 time steps to align it with the target signal

events_reshaped = {
    "V_m": readout.reshape(n_out, n_iter, n_batch, steps_sequence)[:, :, :, -int(duration_recall) :],
    "target_signal": np.repeat(np.reshape(target_rate_changes, (n_out, n_iter, n_batch, 1)), duration_recall, axis=-1),
}

y_prediction = np.argmax(np.mean(events_reshaped["V_m"], axis=3), axis=0)
y_target = np.argmax(np.mean(events_reshaped["target_signal"], axis=3), axis=0)
softmax = np.exp(events_reshaped["V_m"]) / np.sum(np.exp(events_reshaped["V_m"]), axis=0)
loss = -np.mean(np.sum(events_reshaped["target_signal"] * np.log(softmax), axis=0), axis=(1, 2))
accuracy = np.mean((y_target == y_prediction), axis=1)
recall_errors = 1.0 - accuracy

performance_df = pd.DataFrame.from_dict(
    {
        "iterations": range(n_iter),
        "loss": loss.tolist(),
        "recall_errors": recall_errors.tolist(),
    }
)

################
# verification #
################

losses = performance_df.loss.tolist()

losses_verification = [
    0.7411525500061912,
    0.7403877948465877,
    0.6657860139398386,
    0.6637053390959563,
    0.7294282459362229,
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
ax.set_ylabel(r"$E = -\sum_{t,k} \pi_k^{*,t} \log \pi_k^t$")
ax.set_xlabel("training iteration")
ax.spines["top"].set_visible(False)
ax.spines["right"].set_visible(False)
ax.set_xticks(performance_df.iterations)
ax.grid()
fig.tight_layout()

if is_show_plot:
    plt.show()
