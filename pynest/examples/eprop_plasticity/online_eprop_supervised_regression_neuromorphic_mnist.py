# -*- coding: utf-8 -*-
#
# online_eprop_supervised_regression_neuromorphic_mnist.py
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

r"""
Tutorial on learning mnist classification with e-prop
-------------------------------------------------------

Training a classification model using supervised e-prop plasticity to accumulate evidence.

Description
~~~~~~~~~~~

This script demonstrates supervised learning of a classification task with the eligibility propagation (e-prop)
plasticity mechanism by Bellec et al. [1]_.

This type of learning is demonstrated at the proof-of-concept task in [1]_. We based this script on their
TensorFlow script given in [2]_.

[[TODO: add documentation]]

.. image:: ../../../../pynest/examples/eprop_plasticity/eprop_supervised_classification_schematic_evidence-accumulation.png
   :width: 70 %
   :alt: See Figure 1 below.
   :align: center

Learning in the neural network model is achieved by optimizing the connection weights with e-prop plasticity.
This plasticity rule requires a specific network architecture depicted in Figure 1. The neural network model
consists of a recurrent network that receives input from Poisson generators and projects onto two readout
neurons - one for the left and one for the right turn at the end. The input neuron population consists of four
groups: one group providing background noise of a specific rate for some base activity throughout the
experiment, one group providing the input spikes of the left cues and one group providing them for the right
cues, and a last group defining the recall window, in which the network has to decide. The readout neuron
compares the network signal :math:`\pi_k` with the teacher target signal :math:`\pi_k^*`, which it receives from
a rate generator. Since the decision is at the end and all the cues are relevant, the network has to keep the
cues in memory. Additional adaptive neurons in the network enable this memory. The network's training error is
assessed by employing a cross-entropy error loss.

Details on the event-based NEST implementation of e-prop can be found in [3]_.

References
~~~~~~~~~~

.. [1] Bellec G, Scherr F, Subramoney F, Hajek E, Salaj D, Legenstein R, Maass W (2020). A solution to the
       learning dilemma for recurrent networks of spiking neurons. Nature Communications, 11:3625.
       https://doi.org/10.1038/s41467-020-17236-y

.. [2] https://github.com/IGITUGraz/eligibility_propagation/blob/master/Figure_3_and_S7_e_prop_tutorials/tutorial_evidence_accumulation_with_alif.py

.. [3] Korcsak-Gorzo A, Stapmanns J, Espinoza Valverde JA, Dahmen D, van Albada SJ, Bolten M, Diesmann M.
       Event-based implementation of eligibility propagation (in preparation)
"""  # pylint: disable=line-too-long # noqa: E501

# %% ###########################################################################################################
# Import libraries
# ~~~~~~~~~~~~~~~~
# We begin by importing all libraries required for the simulation, analysis, and visualization.

import os
import sys
import zipfile

import matplotlib as mpl
import matplotlib.pyplot as plt
import nest
import numpy as np
from cycler import cycler
from IPython.display import HTML, Image

# %% ###########################################################################################################
# Schematic of network architecture
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# This figure, identical to the one in the description, shows the required network architecture in the center,
# the input and output of the pattern generation task above, and lists of the required NEST device, neuron, and
# synapse models below. The connections that must be established are numbered 1 to 7.

try:
    Image(filename="./eprop_supervised_classification_schematic_evidence-accumulation.png")
except Exception:
    pass

# %% ###########################################################################################################
# Setup
# ~~~~~

# %% ###########################################################################################################
# Initialize random generator
# ...........................
# We seed the numpy random generator, which will generate random initial weights as well as random input and
# output.

rng_seed = 1  # numpy random seed
np.random.seed(rng_seed)  # fix numpy random seed

# %% ###########################################################################################################
# Define timing of task
# .....................
# The task's temporal structure is then defined, once as time steps and once as durations in milliseconds.
# Using a batch size larger than one aids the network in generalization, facilitating the solution to this task.
# The original number of iterations requires distributed computing.

n_batch = 32  # batch size, 64 in reference [2], 32 in the README to reference [2]
n_iter_train = 50
n_iter_test = 4

steps = {}

steps["sequence"] = 300  # time steps of one full sequence
steps["learning_window"] = 10  # time steps of window with non-zero learning signals
steps["training_time"] = n_iter_train * n_batch * steps["sequence"]
steps["testing_time"] = n_iter_test * n_batch * steps["sequence"]
steps["task"] = steps["training_time"] + steps["testing_time"]  # time steps of task

steps.update(
    {
        "offset_gen": 1,  # offset since generator signals start from time step 1
        "delay_in_rec": 1,  # connection delay between input and recurrent neurons
        "delay_rec_out": 0,  # connection delay between recurrent and output neurons
        "delay_out_norm": 0,  # connection delay between output neurons for normalization
        "extension_sim": 1 + 2,  # extra time step to close right-open simulation time interval in Simulate()
    }
)

steps["delays"] = steps["delay_in_rec"] + steps["delay_rec_out"] + steps["delay_out_norm"]  # time steps of delays

steps["total_offset"] = steps["offset_gen"] + steps["delays"]  # time steps of total offset

steps["sim"] = steps["task"] + steps["total_offset"] + steps["extension_sim"]  # time steps of sim

duration = {"step": 1.0}  # ms, temporal resolution of the simulation

duration.update({key: value * duration["step"] for key, value in steps.items()})  # ms, durations

# %% ###########################################################################################################
# Set up simulation
# .................
# As last step of the setup, we reset the NEST kernel to remove all existing NEST simulation settings and
# objects and set some NEST kernel parameters, some of which are e-prop-related.

params_setup = {
    "eprop_reset_neurons_on_update": True,  # if True, reset dynamic variables at start of each update interval
    "eprop_update_interval": duration["sequence"],  # ms, time interval for updating the synaptic weights
    "print_time": True,  # if True, print time progress bar during simulation, set False if run as code cell
    "resolution": duration["step"],
    "total_num_virtual_procs": 1,  # number of virtual processes, set in case of distributed computing
}

####################

nest.ResetKernel()
nest.set(**params_setup)
nest.set_verbosity("M_FATAL")

# %% ###########################################################################################################
# Create neurons
# ~~~~~~~~~~~~~~
# We proceed by creating a certain number of input, recurrent, and readout neurons and setting their parameters.
# Additionally, we already create an input spike generator and an output target rate generator, which we will
# configure later. Within the recurrent network, alongside a population of regular neurons, we introduce a
# population of adaptive neurons, to enhance the network's memory retention.

pixels_blacklist = np.loadtxt("./NMNIST_pixels_blacklist.txt")

n_in = 2 * 34 * 34 - len(pixels_blacklist)  # number of input neurons
n_rec = 100  # number of recurrent neurons
n_out = 10

model_nrn_rec = "eprop_iaf"

params_nrn_rec = {
    "beta": 1.0,  # widht scaling of the pseudo derivative
    "beta_fr_ema": 0.999,  # Smoothing factor of firing rate exponential moving average
    "C_m": 1.0,  # pF, membrane capacitance - takes effect only if neurons get current input (here not the case)
    "c_reg": 2.0 / duration["sequence"],  # firing rate regularization scaling
    "E_L": 0.0,  # mV, leak reversal potential
    "eprop_isi_trace_cutoff": 10**2,  # cutoff of integration of eprop trace between spikes
    "f_target": 10.0,  # spikes/s, target firing rate for firing rate regularization
    "gamma": 0.3,  # height scaling of the pseudo derivative
    "I_e": 0.0,  # pA, external current input
    "surrogate_gradient_function": "piecewise_linear",  # surrogate gradient / pseudo-derivative function
    "t_ref": 0.0,  # ms, duration of refractory period
    "tau_m": 30.0,  # ms, membrane time constant
    "V_m": 0.0,  # mV, initial value of the membrane voltage
    "V_th": 0.5,  # mV, spike threshold membrane voltage
    "V_reset": -0.5,
}

if model_nrn_rec == "eprop_iaf":
    del params_nrn_rec["V_reset"]
    params_nrn_rec["c_reg"] = 2.0 / duration["sequence"]  # firing rate regularization scaling
    params_nrn_rec["regular_spike_arrival"] = True  # postsynaptic current scale factor
    params_nrn_rec["V_th"] = 0.6  # mV, spike threshold membrane voltage

params_nrn_out = {
    "C_m": 1.0,
    "E_L": 0.0,
    "eprop_isi_trace_cutoff": 10**2,  # cutoff of integration of eprop trace between spikes
    "I_e": 0.0,
    "loss": "mean_squared_error",  # loss function
    "regular_spike_arrival": False,
    "tau_m": 100.0,
    "V_m": 0.0,
}

####################

# Intermediate parrot neurons required between input spike generators and recurrent neurons,
# since devices cannot establish plastic synapses for technical reasons

gen_spk_in = nest.Create("spike_generator", n_in)
nrns_in = nest.Create("parrot_neuron", n_in)

# The suffix _bsshslm_2020 follows the NEST convention to indicate in the model name the paper
# that introduced it by the first letter of the authors' last names and the publication year.

nrns_rec = nest.Create(model_nrn_rec, n_rec, params_nrn_rec)
nrns_out = nest.Create("eprop_readout", n_out, params_nrn_out)
gen_rate_target = nest.Create("step_rate_generator", n_out)
gen_learning_window = nest.Create("step_rate_generator")


# %% ###########################################################################################################
# Create recorders
# ~~~~~~~~~~~~~~~~
# We also create recorders, which, while not required for the training, will allow us to track various dynamic
# variables of the neurons, spikes, and changes in synaptic weights. To save computing time and memory, the
# recorders, the recorded variables, neurons, and synapses can be limited to the ones relevant to the
# experiment, and the recording interval can be increased (see the documentation on the specific recorders). By
# default, recordings are stored in memory but can also be written to file.

# n_record = 1  # number of adaptive and regular neurons each to record recordables from
# n_record_w = 3  # number of senders and targets to record weights from

# params_mm_rec = {
#     "interval": duration["step"],  # interval between two recorded time points
#     "record_from": ["V_m", "surrogate_gradient", "learning_signal"],  # recordables
#     "start": duration["offset_gen"] + duration["delay_in_rec"],  # start time of recording
#     "stop": duration["offset_gen"] + duration["delay_in_rec"] + duration["task"],  # stop time of recording
# }

params_mm_out = {
    "interval": duration["step"],
    "record_from": ["V_m", "readout_signal", "target_signal", "error_signal"],
    "start": duration["total_offset"] - 1,  # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<,,,
    "stop": duration["total_offset"] + duration["task"] - 1,
}

# params_wr = {
#     "senders": nrns_in[:n_record_w] + nrns_rec[:n_record_w],  # limit senders to subsample weights to record
#     "targets": nrns_rec[:n_record_w] + nrns_out,  # limit targets to subsample weights to record from
# }

####################

# mm_rec = nest.Create("multimeter", params_mm_rec)
mm_out = nest.Create("multimeter", params_mm_out)
# sr = nest.Create("spike_recorder")
# wr = nest.Create("weight_recorder", params_wr)

# nrns_reg_record = nrns_reg[:n_record]
# nrns_ad_record = nrns_ad[:n_record]

# %% ###########################################################################################################
# Create connections
# ~~~~~~~~~~~~~~~~~~
# Now, we define the connectivities and set up the synaptic parameters, with the synaptic weights drawn from
# normal distributions. After these preparations, we establish the enumerated connections of the core network,
# as well as additional connections to the recorders.

params_conn_all_to_all = {"rule": "all_to_all", "allow_autapses": False}
params_conn_one_to_one = {"rule": "one_to_one"}


def calculate_glorot_dist(fan_in, fan_out):
    glorot_scale = 1.0 / max(1.0, (fan_in + fan_out) / 2.0)
    glorot_limit = np.sqrt(3.0 * glorot_scale)
    glorot_distribution = np.random.uniform(low=-glorot_limit, high=glorot_limit, size=(fan_in, fan_out))
    return glorot_distribution


dtype_weights = np.float32  # data type of weights - for reproducing TF results set to np.float32
weights_in_rec = np.array(np.random.randn(n_in, n_rec).T / np.sqrt(n_in), dtype=dtype_weights)
weights_rec_rec = np.array(np.random.randn(n_rec, n_rec).T / np.sqrt(n_rec), dtype=dtype_weights)
np.fill_diagonal(weights_rec_rec, 0.0)  # since no autapses set corresponding weights to zero
weights_rec_out = np.array(calculate_glorot_dist(n_rec, n_out).T, dtype=dtype_weights)
weights_out_rec = np.array(np.random.randn(n_rec, n_out), dtype=dtype_weights)


sparsity_level_in = 0.9
mask_in = np.random.choice([0, 1], weights_in_rec.shape, p=[sparsity_level_in, 1 - sparsity_level_in])
sparsity_level_out = 0.0
mask_out = np.random.choice([0, 1], weights_rec_out.shape, p=[sparsity_level_out, 1 - sparsity_level_out])

weights_in_rec *= mask_in
weights_rec_out *= mask_out

params_common_syn_eprop = {
    "optimizer": {
        "type": "gradient_descent",  # algorithm to optimize the weights
        "batch_size": 1,
        "eta": 5e-3,  # learning rate
        "Wmin": -100.0,  # pA, minimal limit of the synaptic weights
        "Wmax": 100.0,  # pA, maximal limit of the synaptic weights
    },
}

params_syn_base = {
    "synapse_model": "eprop_synapse",
    "delay": duration["step"],  # ms, dendritic delay
    "kappa": np.exp(
        -duration["step"] / params_nrn_out["tau_m"]
    ),  # ms, for technical reasons pass a filter with the readout neuron membrane time constant
}

params_syn_in = params_syn_base.copy()
params_syn_rec = params_syn_base.copy()
params_syn_out = params_syn_base.copy()


params_syn_feedback = {
    "synapse_model": "eprop_learning_signal_connection",
    "delay": duration["step"],
    "weight": weights_out_rec,
}

params_syn_learning_window = {
    "synapse_model": "rate_connection_delayed",
    "delay": duration["step"],
    "receptor_type": 1,  # receptor type over which readout neuron receives learning window signal
}

params_syn_rate_target = {
    "synapse_model": "rate_connection_delayed",
    "delay": duration["step"],
    "receptor_type": 2,  # receptor type over which readout neuron receives target signal
}

params_syn_static = {
    "synapse_model": "static_synapse",
    "delay": duration["step"],
}

params_init_optimizer = {
    "optimizer": {
        "m": 0.0,  # initial 1st moment estimate m of Adam optimizer
        "v": 0.0,  # initial 2nd moment raw estimate v of Adam optimizer
    }
}

####################

nest.SetDefaults("eprop_synapse", params_common_syn_eprop)

nest.Connect(gen_spk_in, nrns_in, params_conn_one_to_one, params_syn_static)  # connection 1
# nest.Connect(nrns_in, nrns_rec, params_conn_all_to_all, params_syn_in)  # connection 2
for j in range(n_rec):
    for i in range(n_in):
        w = weights_in_rec[j, i]
        if np.abs(w) > 0:
            params_syn_in["weight"] = w
            nest.Connect(nrns_in[i], nrns_rec[j], params_conn_one_to_one, params_syn_in)
# nest.Connect(nrns_rec, nrns_rec, params_conn_all_to_all, params_syn_rec)  # connection 3
# nest.Connect(nrns_rec, nrns_out, params_conn_all_to_all, params_syn_out)  # connection 4
for j in range(n_out):
    for i in range(n_rec):
        w = weights_rec_out[j, i]
        if np.abs(w) > 0:
            params_syn_out["weight"] = w
            nest.Connect(nrns_rec[i], nrns_out[j], params_conn_one_to_one, params_syn_out)
nest.Connect(nrns_out, nrns_rec, params_conn_all_to_all, params_syn_feedback)  # connection 5
nest.Connect(gen_rate_target, nrns_out, params_conn_one_to_one, params_syn_rate_target)  # connection 6
nest.Connect(gen_learning_window, nrns_out, params_conn_all_to_all, params_syn_learning_window)  # connection
nest.Connect(mm_out, nrns_out, params_conn_all_to_all, params_syn_static)

# After creating the connections, we can individually initialize the optimizer's
# dynamic variables for single synapses (here exemplarily for two connections).

# nest.GetConnections(nrns_rec[0], nrns_rec[1:3]).set([params_init_optimizer] * 2)

# %% ###########################################################################################################
# Create input and output
# ~~~~~~~~~~~~~~~~~~~~~~~


def extract_dataset(zip_path, target_folder):
    with zipfile.ZipFile(zip_path) as zip_file:
        for member in zip_file.namelist():
            if not os.path.exists(os.path.join(target_folder, member)):
                zip_file.extract(member, target_folder)


def load_n_mnist(file_path, num_labels=10, shuffle=True, pixels_blacklist=None):
    assert 0 < num_labels <= 10, "num_labels must be between 1 and 10"

    samples = np.loadtxt(file_path, dtype="int")
    all_indices, all_labels = samples[:, 0], samples[:, 1]

    if num_labels < 10:
        wanted_labels = set(range(num_labels))
        filtered_samples = [(idx, lbl) for idx, lbl in zip(all_indices, all_labels) if lbl in wanted_labels]
        indices, labels = zip(*filtered_samples)
    else:
        indices, labels = all_indices, all_labels

    indices, labels = np.array(indices), np.array(labels)

    if shuffle:
        shuffled_indices = np.random.permutation(len(indices))
        indices, labels = indices[shuffled_indices], labels[shuffled_indices]

    images = [load_image(f"NMNISTsmall/{idx}.bs2", pixels_blacklist) for idx in indices]

    return images, labels


def load_image(file_path, pixels_blacklist=None):
    with open(file_path, "rb") as file:
        inputByteArray = file.read()
    byte_array = np.asarray([x for x in inputByteArray])

    x_coords = byte_array[0::5]
    y_coords = byte_array[1::5]
    polarities = byte_array[2::5] >> 7
    times = ((byte_array[2::5] << 16) | (byte_array[3::5] << 8) | byte_array[4::5]) & 0x7FFFFF
    times = np.clip(times // 1000, 1, 299)

    image_full = [[] for _ in range(2 * 34 * 34)]
    image = []

    for polarity, x, y, time in zip(polarities, y_coords, x_coords, times):
        pixel = polarity * 34 * 34 + x * 34 + y
        image_full[pixel].append(time)

    for pixel, times in enumerate(image_full):
        if pixel not in pixels_blacklist:
            image.append(times)

    return image


class DataLoader:
    def __init__(self, images, labels, n_batch):
        self.images = images
        self.labels = labels
        self.n_batch = n_batch
        self.current_index = 0

    def get_new_batch(self):
        end_index = self.current_index + self.n_batch

        if end_index <= len(self.images):
            batch_images = self.images[self.current_index : end_index]
            batch_labels = self.labels[self.current_index : end_index]
        else:
            overflow = end_index - len(self.images)
            batch_images = self.images[self.current_index :] + self.images[:overflow]
            batch_labels = np.concatenate((self.labels[self.current_index :], self.labels[:overflow]))

        self.current_index = (end_index) % len(self.images)

        return batch_images, batch_labels


extract_dataset("NMNISTsmall.zip", "./")
file_path_train = "NMNISTsmall/train1K.txt"
file_path_test = "NMNISTsmall/test100.txt"

train_images, train_labels = load_n_mnist(
    file_path_train, num_labels=n_out, shuffle=True, pixels_blacklist=pixels_blacklist
)
test_images, test_labels = load_n_mnist(
    file_path_test, num_labels=n_out, shuffle=True, pixels_blacklist=pixels_blacklist
)

train_loader = DataLoader(train_images, train_labels, n_batch=n_batch)
test_loader = DataLoader(test_images, test_labels, n_batch=n_batch)

# %% ###########################################################################################################
# Force final update
# ~~~~~~~~~~~~~~~~~~
# Synapses only get active, that is, the correct weight update calculated and applied, when they transmit a
# spike. To still be able to read out the correct weights at the end of the simulation, we force spiking of the
# presynaptic neuron and thus an update of all synapses, including those that have not transmitted a spike in
# the last update interval, by sending a strong spike to all neurons that form the presynaptic side of an eprop
# synapse. This step is required purely for technical reasons.

gen_spk_final_update = nest.Create("spike_generator", 1, {"spike_times": [duration["task"] + duration["delays"]]})

nest.Connect(gen_spk_final_update, nrns_in + nrns_rec, "all_to_all", {"weight": 1000.0})

# %% ###########################################################################################################
# Read out pre-training weights
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Before we begin training, we read out the initial weight matrices so that we can eventually compare them to
# the optimized weights.


def get_weights(pop_pre, pop_post):
    conns = nest.GetConnections(pop_pre, pop_post).get(["source", "target", "weight"])
    conns["senders"] = np.array(conns["source"]) - np.min(conns["source"])
    conns["targets"] = np.array(conns["target"]) - np.min(conns["target"])

    conns["weight_matrix"] = np.zeros((len(pop_post), len(pop_pre)))
    conns["weight_matrix"][conns["targets"], conns["senders"]] = conns["weight"]
    return conns


weights_pre_train = {
    "in_rec": get_weights(nrns_in, nrns_rec),
    # "rec_rec": get_weights(nrns_rec, nrns_rec),
    "rec_out": get_weights(nrns_rec, nrns_out),
}

# %% ###########################################################################################################
# Simulate
# ~~~~~~~~
# We train the network by simulating for a set simulation time, determined by the number of iterations and the
# batch size and the length of one sequence.

spike_times = [[] for _ in range(n_in)]
target_rates = np.zeros((n_out, steps["task"]))
target_signal_rescale_factor = 1.0

for iteration in np.arange(n_iter_train + n_iter_test):
    t_start_iteration = iteration * n_batch * steps["sequence"]
    t_end_iteration = t_start_iteration + n_batch * steps["sequence"]

    if iteration < n_iter_train:
        loader = train_loader
    else:
        loader = test_loader

    img_batch, targets_batch = loader.get_new_batch()

    for batch_elem in range(n_batch):
        t_start_batch_elem = t_start_iteration + batch_elem * steps["sequence"]
        t_end_batch_elem = t_start_batch_elem + steps["sequence"]

        target_rates[targets_batch[batch_elem], t_start_batch_elem:t_end_batch_elem] = target_signal_rescale_factor

        for n, relative_times in enumerate(img_batch[batch_elem]):
            absolute_times = t_start_batch_elem * np.ones_like(relative_times) + relative_times
            spike_times[n] += absolute_times.tolist()

params_gen_spk_in = []
for spk_times in spike_times:
    params_gen_spk_in.append({"spike_times": spk_times})

params_gen_rate_target = []
for target_rate in target_rates:
    params_gen_rate_target.append(
        {
            "amplitude_times": np.arange(duration["total_offset"], duration["total_offset"] + duration["task"]),
            "amplitude_values": target_rate,
        }
    )

amplitude_times = np.hstack(
    [
        np.array([0.0, duration["sequence"] - duration["learning_window"]])
        + duration["total_offset"]
        + i * duration["sequence"]
        for i in range(n_batch * (n_iter_train + n_iter_test))
    ]
)

amplitude_values = np.array([0.0, 1.0] * n_batch * (n_iter_train + n_iter_test))

params_gen_learning_window = {
    "amplitude_times": amplitude_times,
    "amplitude_values": amplitude_values,
}

nest.SetStatus(gen_spk_in, params_gen_spk_in)
nest.SetStatus(gen_rate_target, params_gen_rate_target)
nest.SetStatus(gen_learning_window, params_gen_learning_window)


nest.Simulate(duration["total_offset"] + duration["training_time"])
params_common_syn_eprop["optimizer"]["eta"] = 0.0
nest.SetDefaults("eprop_synapse", params_common_syn_eprop)
nest.Simulate(duration["extension_sim"] + duration["testing_time"])


"""
process data of recording devices
"""
events_mm_out = mm_out.get("events")

senders = events_mm_out["senders"]
readout_signal = events_mm_out["V_m"]
target_signal = events_mm_out["target_signal"]

readout_signal = np.array([readout_signal[senders == i] for i in set(senders)])  # nrns_out.tolist()
target_signal = np.array([target_signal[senders == i] for i in set(senders)])

readout_signal = readout_signal.reshape((n_out, n_iter_train + n_iter_test, n_batch, steps["sequence"]))
readout_signal = readout_signal[:, :, :, -steps["learning_window"] :]

target_signal = target_signal.reshape((n_out, n_iter_train + n_iter_test, n_batch, steps["sequence"]))
target_signal = target_signal[:, :, :, -steps["learning_window"] :]

"""
calculate recall errors
"""

mse = np.mean((target_signal - readout_signal) ** 2, axis=3)
distance_to_target = np.mean((target_signal_rescale_factor - readout_signal) ** 2, axis=3)

losses = np.mean(mse, axis=(0, 2))

y_prediction = np.argmin(distance_to_target, axis=0)
y_target = np.argmax(np.mean(target_signal, axis=3), axis=0)
accuracy = np.mean((y_target == y_prediction), axis=1)

print("\nTraining: ")
for i, (loss, acc) in enumerate(zip(losses[:n_iter_train], accuracy[:n_iter_train])):
    print(f"    iter: {i} loss: {loss:0.5f} acc: {acc:0.5f}")

print("\nTesting: ")
for i, (loss, acc) in enumerate(zip(losses[n_iter_train:], accuracy[n_iter_train:])):
    print(f"    iter: {i} loss: {loss:0.5f} acc: {acc:0.5f}")


exit()
# %% ###########################################################################################################
# Read out post-training weights
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# After the training, we can read out the optimized final weights.

weights_post_train = {
    "in_rec": get_weights(nrns_in, nrns_rec),
    "rec_rec": get_weights(nrns_rec, nrns_rec),
    "rec_out": get_weights(nrns_rec, nrns_out),
}

# %% ###########################################################################################################
# Read out recorders
# ~~~~~~~~~~~~~~~~~~
# We can also retrieve the recorded history of the dynamic variables and weights, as well as detected spikes.

# events_mm_reg = mm_reg.get("events")
# events_mm_ad = mm_ad.get("events")
events_mm_out = mm_out.get("events")
# events_sr = sr.get("events")
# events_wr = wr.get("events")

# %% ###########################################################################################################
# Evaluate training error
# ~~~~~~~~~~~~~~~~~~~~~~~
# We evaluate the network's training error by calculating a loss - in this case, the cross-entropy error between
# the integrated recurrent network activity and the target rate.

readout_signal = events_mm_out["readout_signal"]  # corresponds to softmax
target_signal = events_mm_out["target_signal"]
senders = events_mm_out["senders"]

readout_signal = np.array([readout_signal[senders == i] for i in set(senders)])
target_signal = np.array([target_signal[senders == i] for i in set(senders)])

readout_signal = readout_signal.reshape((n_out, n_iter, n_batch, steps["sequence"]))
readout_signal = readout_signal[:, :, :, -steps["learning_window"] :]

target_signal = target_signal.reshape((n_out, n_iter, n_batch, steps["sequence"]))
target_signal = target_signal[:, :, :, -steps["learning_window"] :]

loss = -np.mean(np.sum(target_signal * np.log(readout_signal), axis=0), axis=(1, 2))

y_prediction = np.argmax(np.mean(readout_signal, axis=3), axis=0)
y_target = np.argmax(np.mean(target_signal, axis=3), axis=0)
accuracy = np.mean((y_target == y_prediction), axis=1)
recall_errors = 1.0 - accuracy

print(accuracy)

# %% ###########################################################################################################
# Plot results
# ~~~~~~~~~~~~
# Then, we plot a series of plots.

do_plotting = False  # if True, plot the results

if not do_plotting:
    exit()

colors = {
    "blue": "#2854c5ff",
    "red": "#e04b40ff",
    "white": "#ffffffff",
}

plt.rcParams.update(
    {
        "font.sans-serif": "Arial",
        "axes.spines.right": False,
        "axes.spines.top": False,
        "axes.prop_cycle": cycler(color=[colors["blue"], colors["red"]]),
    }
)

# %% ###########################################################################################################
# Plot training error
# ...................
# We begin with two plots visualizing the training error of the network: the loss and the recall error, both
# plotted against the iterations.

fig, axs = plt.subplots(2, 1, sharex=True)

axs[0].plot(range(1, n_iter + 1), loss)
axs[0].set_ylabel(r"$E = -\sum_{t,k} \pi_k^{*,t} \log \pi_k^t$")

axs[1].plot(range(1, n_iter + 1), recall_errors)
axs[1].set_ylabel("recall errors")

axs[-1].set_xlabel("training iteration")
axs[-1].set_xlim(1, n_iter)
axs[-1].xaxis.get_major_locator().set_params(integer=True)

fig.tight_layout()

# %% ###########################################################################################################
# Plot recordables
# ................
# This plotting routine shows how to plot all of the recorded dynamic variables and spikes across time. We take
# one snapshot in the first iteration and one snapshot at the end.


def plot_recordable(ax, events, recordable, ylabel, xlims):
    for sender in set(events["senders"]):
        idc_sender = events["senders"] == sender
        idc_times = (events["times"][idc_sender] > xlims[0]) & (events["times"][idc_sender] < xlims[1])
        ax.plot(events["times"][idc_sender][idc_times], events[recordable][idc_sender][idc_times], lw=0.5)
    ax.set_ylabel(ylabel)
    margin = np.abs(np.max(events[recordable]) - np.min(events[recordable])) * 0.1
    ax.set_ylim(np.min(events[recordable]) - margin, np.max(events[recordable]) + margin)


def plot_spikes(ax, events, nrns, ylabel, xlims):
    idc_times = (events["times"] > xlims[0]) & (events["times"] < xlims[1])
    idc_sender = np.isin(events["senders"][idc_times], nrns.tolist())
    senders_subset = events["senders"][idc_times][idc_sender]
    times_subset = events["times"][idc_times][idc_sender]

    ax.scatter(times_subset, senders_subset, s=0.1)
    ax.set_ylabel(ylabel)
    margin = np.abs(np.max(senders_subset) - np.min(senders_subset)) * 0.1
    ax.set_ylim(np.min(senders_subset) - margin, np.max(senders_subset) + margin)


for xlims in [(0, steps["sequence"]), (steps["task"] - steps["sequence"], steps["task"])]:
    fig, axs = plt.subplots(14, 1, sharex=True, figsize=(8, 14), gridspec_kw={"hspace": 0.4, "left": 0.2})

    plot_spikes(axs[0], events_sr, nrns_in, r"$z_i$" + "\n", xlims)
    plot_spikes(axs[1], events_sr, nrns_reg, r"$z_j$" + "\n", xlims)

    plot_recordable(axs[2], events_mm_reg, "V_m", r"$v_j$" + "\n(mV)", xlims)
    plot_recordable(axs[3], events_mm_reg, "surrogate_gradient", r"$\psi_j$" + "\n", xlims)
    plot_recordable(axs[4], events_mm_reg, "learning_signal", r"$L_j$" + "\n(pA)", xlims)

    plot_spikes(axs[5], events_sr, nrns_ad, r"$z_j$" + "\n", xlims)

    plot_recordable(axs[6], events_mm_ad, "V_m", r"$v_j$" + "\n(mV)", xlims)
    plot_recordable(axs[7], events_mm_ad, "surrogate_gradient", r"$\psi_j$" + "\n", xlims)
    plot_recordable(axs[8], events_mm_ad, "V_th_adapt", r"$A_j$" + "\n(mV)", xlims)
    plot_recordable(axs[9], events_mm_ad, "learning_signal", r"$L_j$" + "\n(pA)", xlims)

    plot_recordable(axs[10], events_mm_out, "V_m", r"$v_k$" + "\n(mV)", xlims)
    plot_recordable(axs[11], events_mm_out, "target_signal", r"$\pi^*_k$" + "\n", xlims)
    plot_recordable(axs[12], events_mm_out, "readout_signal", r"$\pi_k$" + "\n", xlims)
    plot_recordable(axs[13], events_mm_out, "error_signal", r"$\pi_k-\pi^*_k$" + "\n", xlims)

    axs[-1].set_xlabel(r"$t$ (ms)")
    axs[-1].set_xlim(*xlims)

    fig.align_ylabels()

# %% ###########################################################################################################
# Plot weight time courses
# ........................
# Similarly, we can plot the weight histories. Note that the weight recorder, attached to the synapses, works
# differently than the other recorders. Since synapses only get activated when they transmit a spike, the weight
# recorder only records the weight in those moments. That is why the first weight registrations do not start in
# the first time step and we add the inital weights manually.


def plot_weight_time_course(ax, events, nrns_senders, nrns_targets, label, ylabel):
    for sender in nrns_senders.tolist():
        for target in nrns_targets.tolist():
            idc_syn = (events["senders"] == sender) & (events["targets"] == target)
            idc_syn_pre = (weights_pre_train[label]["source"] == sender) & (
                weights_pre_train[label]["target"] == target
            )

            times = [0.0] + events["times"][idc_syn].tolist()
            weights = [weights_pre_train[label]["weight"][idc_syn_pre]] + events["weights"][idc_syn].tolist()

            ax.step(times, weights, c=colors["blue"])
        ax.set_ylabel(ylabel)
        ax.set_ylim(-0.6, 0.6)


fig, axs = plt.subplots(3, 1, sharex=True, figsize=(3, 4))

plot_weight_time_course(axs[0], events_wr, nrns_in, nrns_rec, "in_rec", r"$W_\mathrm{in}$" + "\n(pA)")
plot_weight_time_course(axs[1], events_wr, nrns_rec, nrns_rec, "rec_rec", r"$W_\mathrm{rec}$" + "\n(pA)")
plot_weight_time_course(axs[2], events_wr, nrns_rec, nrns_out, "rec_out", r"$W_\mathrm{out}$" + "\n(pA)")

axs[-1].set_xlabel(r"$t$ (ms)")
axs[-1].set_xlim(0, steps["task"])

fig.align_ylabels()
fig.tight_layout()

# %% ###########################################################################################################
# Plot weight matrices
# ....................
# If one is not interested in the time course of the weights, it is possible to read out only the initial and
# final weights, which requires less computing time and memory than the weight recorder approach. Here, we plot
# the corresponding weight matrices before and after the optimization.

cmap = mpl.colors.LinearSegmentedColormap.from_list(
    "cmap", ((0.0, colors["blue"]), (0.5, colors["white"]), (1.0, colors["red"]))
)

fig, axs = plt.subplots(3, 2, sharex="col", sharey="row")

all_w_extrema = []

for k in weights_pre_train.keys():
    w_pre = weights_pre_train[k]["weight"]
    w_post = weights_post_train[k]["weight"]
    all_w_extrema.append([np.min(w_pre), np.max(w_pre), np.min(w_post), np.max(w_post)])

args = {"cmap": cmap, "vmin": np.min(all_w_extrema), "vmax": np.max(all_w_extrema)}

for i, weights in zip([0, 1], [weights_pre_train, weights_post_train]):
    axs[0, i].pcolormesh(weights["in_rec"]["weight_matrix"].T, **args)
    axs[1, i].pcolormesh(weights["rec_rec"]["weight_matrix"], **args)
    cmesh = axs[2, i].pcolormesh(weights["rec_out"]["weight_matrix"], **args)

    axs[2, i].set_xlabel("recurrent\nneurons")

axs[0, 0].set_ylabel("input\nneurons")
axs[1, 0].set_ylabel("recurrent\nneurons")
axs[2, 0].set_ylabel("readout\nneurons")
fig.align_ylabels(axs[:, 0])

axs[0, 0].text(0.5, 1.1, "pre-training", transform=axs[0, 0].transAxes, ha="center")
axs[0, 1].text(0.5, 1.1, "post-training", transform=axs[0, 1].transAxes, ha="center")

axs[2, 0].yaxis.get_major_locator().set_params(integer=True)

cbar = plt.colorbar(cmesh, cax=axs[1, 1].inset_axes([1.1, 0.2, 0.05, 0.8]), label="weight (pA)")

fig.tight_layout()

plt.show()
