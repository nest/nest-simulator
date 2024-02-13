import os.path
import random

import numpy as np
import pandas as pd
from scipy.spatial.distance import jensenshannon
import matplotlib.pyplot as plt
import seaborn as sns


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


def run(enable_stdp):
    import nest
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
    import nest
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
    import nest
    rate = 0
    cv = 0
    syn_weight = 0
    while not (44. < syn_weight < 46. and 9.9 < rate < 10.1 and 0.99 < cv < 1.01):
        sr, post_neuron = run(enable_stdp=False)
        rate, cv = rate_cv(sr)
        weights = nest.GetConnections(target=post_neuron, synapse_model="stdp_pl_synapse_hom_ax_delay").get("weight")
        syn_weight = np.mean(weights)
        print(f"rate: {rate:.3f}, cv: {cv:.3f}, weight: {syn_weight:.3f}, alpha: {stdp_params['alpha']}")
        if syn_weight >= 46.:
            stdp_params['alpha'] *= 1. - 0.03 * abs(syn_weight - 45.) * max(0.7, random.random())
        else:
            stdp_params['alpha'] *= 1. + 0.02 * abs(syn_weight - 45.) * max(0.7, random.random())


def set_font_sizes(small=8, medium=10, large=12, family='Helvetica'):
    plt.rc('font', size=small)  # controls default text sizes
    plt.rc('axes', titlesize=small)  # fontsize of the axes title
    plt.rc('axes', labelsize=medium)  # fontsize of the x and y labels
    plt.rc('xtick', labelsize=small)  # fontsize of the tick labels
    plt.rc('ytick', labelsize=small)  # fontsize of the tick labels
    plt.rc('legend', fontsize=small)  # legend fontsize
    plt.rc('figure', titlesize=large)  # fontsize of the figure title
    plt.rc('font', family=family)


def save_grayscale(filename):
    from PIL import Image
    img = Image.open(filename).convert('L')
    name, ending = filename.split('.')
    img.save(name + '_g.' + ending)


def plot_corrections():
    ax_perc = np.arange(0.0, 1.01, 0.01, dtype=np.float32)
    if os.path.exists("num_corrections.npy"):
        num_corrections = np.load("num_corrections.npy")
    else:
        import nest
        num_corrections = np.empty(101, dtype=np.int32)
        for i, p in enumerate(ax_perc):
            print(i)
            stdp_params["delay"] = 5. * (1 - p)
            stdp_params["axonal_delay"] = 5. * p
            run(enable_stdp=False)
            num_corrections[i] = nest.kernel_status["num_corrections"] / NE
        np.save("num_corrections.npy", num_corrections)
    fig, ax = plt.subplots(dpi=300)
    plt.plot(ax_perc, num_corrections, color="black")
    plt.setp(ax.spines.values(), linewidth=2)
    ax.set_xlabel("Axonal delay ratio")
    ax.set_ylabel("Number of corrections")  # average per neuron
    ax.tick_params(width=2)
    set_font_sizes()
    fig.savefig('num_corrections.tif')
    save_grayscale('num_corrections.tif')
    plt.show()


def plot_weights():
    if os.path.exists("weights_correction.npy") and os.path.exists("weights_no_correction.npy"):
        fig, ax = plt.subplots(dpi=300)
        weights_correction = np.load("weights_correction.npy")
        weights_no_correction = np.load("weights_no_correction.npy")
        sns.distplot(weights_correction, hist=False, kde=True, kde_kws={'fill': True, 'linewidth': 3},
                     label="correction", ax=ax)
        sns.distplot(weights_no_correction, hist=False, kde=True, kde_kws={'fill': True, 'linewidth': 3},
                     label="no correction", ax=ax)
        plt.setp(ax.spines.values(), linewidth=2)
        ax.set_xlabel("Synaptic weight")
        ax.tick_params(width=2)
        plt.legend()
        set_font_sizes()
        fig.savefig('weight_distributions.tif')
        save_grayscale('weight_distributions.tif')
        plt.show()
    else:
        import nest
        stdp_params["delay"] = 0.
        stdp_params["axonal_delay"] = 5.
        _, post_neuron = run(enable_stdp=True)
        weights = nest.GetConnections(target=post_neuron, synapse_model="stdp_pl_synapse_hom_ax_delay").get("weight")
        np.save("weights_no_correction.npy", weights)


def jensen_shannon_divergence():
    weights_correction = np.load("weights_correction.npy")
    weights_no_correction = np.load("weights_no_correction.npy")
    p = np.histogram(weights_correction, bins=100000, density=True)[0]
    q = np.histogram(weights_no_correction, bins=100000, density=True)[0]
    print(round(jensenshannon(p, q)**2, 3))


# find_equilibrium()
# find_equilibrium_stdp()
plot_corrections()
# plot_weights()
# jensen_shannon_divergence()


def plot_benchmark_static():
    df_master = pd.read_csv("/home/vogelsang1/Documents/ax-delay-paper-results/hpc_axonal_delay_master_static.txt", delimiter=',').drop('rng_seed', axis=1).groupby("num_nodes").agg("mean")
    df_corr = pd.read_csv("/home/vogelsang1/Documents/ax-delay-paper-results/hpc_axonal_delay_correction_static.txt", delimiter=',').drop('rng_seed', axis=1).groupby("num_nodes").agg("mean")
    df_adj = pd.read_csv("/home/vogelsang1/Documents/ax-delay-paper-results/hpc_axonal_delay_adjacency_static.txt", delimiter=',').drop('rng_seed', axis=1).groupby("num_nodes").agg("mean")
    plt.plot(df_master.index, df_master["time_simulate"], label="master")
    plt.plot(df_corr.index, df_corr["time_simulate"], label="corr")
    plt.plot(df_adj.index, df_adj["time_simulate"], label="adj")
    plt.legend()
    plt.show()


def plot_benchmark_stdp():
    df_corr = pd.read_csv("/home/vogelsang1/Documents/ax-delay-paper-results/hpc_axonal_delay_correction_stdp.txt", delimiter=',').drop('rng_seed', axis=1)
    df_adj = pd.read_csv("/home/vogelsang1/Documents/ax-delay-paper-results/hpc_axonal_delay_adjacency_stdp.txt", delimiter=',').drop('rng_seed', axis=1)
    plt.show()

    df_corr_1 = df_corr[df_corr["num_nodes"] == 1]
    df_corr_32 = df_corr[df_corr["num_nodes"] == 32]
    df_adj_1 = df_adj[df_adj["num_nodes"] == 1]
    df_adj_32 = df_adj[df_adj["num_nodes"] == 32]
    plt.plot(df_corr_1["axonal_delay"], df_corr_1["time_simulate"])
    plt.plot(df_corr_32["axonal_delay"], df_corr_32["time_simulate"])
    plt.plot(df_adj_1["axonal_delay"], df_adj_1["time_simulate"])
    plt.plot(df_adj_32  ["axonal_delay"], df_adj_32["time_simulate"])


# plot_benchmark_static()
# plot_benchmark_stdp()
