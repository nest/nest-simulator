import math
import os.path
import random

import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

import nest


tau_syn = 0.32582722403722841
neuron_params = {
    "E_L": 0.0,
    "C_m": 250.0,
    "tau_m": 10.0,
    "t_ref": 0.5,
    "V_th": 20.0,
    "V_reset": 0.0,
    "tau_syn_ex": tau_syn,
    "tau_syn_in": tau_syn,
    "tau_minus": 30.0,
    "V_m": 5.7
}

stdp_params = {
    "delay": 5.0,
    "axonal_delay": 0.0,
    "alpha": 0.0513,
    "mu": 0.4,
    "tau_plus": 15.0,
    "weight": 45.
}

T = 10000.
NE = 9000
eta = 4.92


def rate_cv(sr):
    times = sr.events["times"]
    isi = np.ediff1d(times)
    cv = isi.std() / isi.mean()
    return len(times) / T * 1000., cv


def get_weights(post_neuron):
    return nest.GetConnections(target=post_neuron, synapse_model="stdp_pl_synapse_hom_ax_delay").get("weight")


def run(enable_stdp):
    if enable_stdp:
        stdp_params['lambda'] = 0.1
    else:
        stdp_params['lambda'] = 0.

    nest.ResetKernel()
    nest.local_num_threads = 8
    nest.resolution = 0.1
    nest.rng_seed = 42
    nest.set_verbosity("M_ERROR")

    E_ext = nest.Create("poisson_generator", 1, {"rate": eta * 1000.0})
    E_pg = nest.Create("poisson_generator", 1, params={"rate": 10.0})
    I_pg = nest.Create("poisson_generator", 1, params={"rate": 10.0 * NE / 5})
    E_neurons = nest.Create("parrot_neuron", NE)
    I_neurons = nest.Create("parrot_neuron", 1)
    post_neuron = nest.Create("iaf_psc_alpha", 1, params=neuron_params)
    sr = nest.Create("spike_recorder")

    nest.SetDefaults("stdp_pl_synapse_hom_ax_delay", stdp_params)

    nest.Connect(E_pg, E_neurons)
    nest.Connect(I_pg, I_neurons)
    nest.Connect(E_neurons, post_neuron, syn_spec={"synapse_model": "stdp_pl_synapse_hom_ax_delay"})
    nest.Connect(I_neurons, post_neuron, syn_spec={"weight": 45. * -5})
    nest.Connect(E_ext, post_neuron, syn_spec={"weight": 45.})
    nest.Connect(post_neuron, sr)

    nest.Simulate(T)

    return sr, post_neuron


def find_equilibrium():
    global eta
    rate = 0
    cv = 0
    while not (9.9 < rate < 10.1 and 0.99 < cv < 1.01):
        sr, _ = run(enable_stdp=False)
        rate, cv = rate_cv(sr)
        print(f"rate: {rate:.3f}, cv: {cv:.3f}, eta: {eta}")
        if rate >= 10.1:
            eta *= 1. - 0.03 * abs(rate - 10.) * max(0.7, random.random())
        else:
            eta *= 1. + 0.02 * abs(rate - 10.) * max(0.7, random.random())


def find_equilibrium_stdp():
    rate = 0
    cv = 0
    syn_weight = 0
    while not (44. < syn_weight < 46. and 9.9 < rate < 10.1 and 0.99 < cv < 1.01):
        sr, post_neuron = run(enable_stdp=False)
        rate, cv = rate_cv(sr)
        weights = get_weights(post_neuron)
        syn_weight = np.mean(weights)
        print(f"rate: {rate:.3f}, cv: {cv:.3f}, weight: {syn_weight:.3f}, alpha: {stdp_params['alpha']}")
        if syn_weight >= 46.:
            stdp_params['alpha'] *= 1. - 0.03 * abs(syn_weight - 45.) * max(0.7, random.random())
        else:
            stdp_params['alpha'] *= 1. + 0.02 * abs(syn_weight - 45.) * max(0.7, random.random())


def plot_corrections():
    ax_perc = np.arange(0.0, 1.01, 0.01, dtype=np.float32)
    if os.path.exists("num_corrections.npy"):
        num_corrections = np.load("num_corrections.npy")
    else:
        num_corrections = np.empty(101, dtype=np.int32)
        for i, p in enumerate(ax_perc):
            print(i)
            stdp_params["delay"] = 5. * (1 - p)
            stdp_params["axonal_delay"] = 5. * p
            run(enable_stdp=False)
            num_corrections[i] = nest.kernel_status["num_corrections"]
        np.save("num_corrections.npy", num_corrections)
    plt.plot(ax_perc, num_corrections)
    plt.show()


def plot_weights():
    if os.path.exists("weights_correction.npy") and os.path.exists("weights_no_correction.npy"):
        weights_correction = np.load("weights_correction.npy")
        weights_no_correction = np.load("weights_no_correction.npy")
        sns.distplot(weights_correction, hist=False, kde=True, kde_kws={'shade': True, 'linewidth': 3}, label="correction")
        sns.distplot(weights_no_correction, hist=False, kde=True, kde_kws={'shade': True, 'linewidth': 3},
                     label="no correction")
        plt.legend()
        plt.show()
    else:
        stdp_params["delay"] = 0.
        stdp_params["axonal_delay"] = 5.
        _, post_neuron = run(enable_stdp=True)
        weights = get_weights(post_neuron)
        np.save("weights_no_correction.npy", weights)


# find_equilibrium()
# find_equilibrium_stdp()
# plot_corrections()
plot_weights()
