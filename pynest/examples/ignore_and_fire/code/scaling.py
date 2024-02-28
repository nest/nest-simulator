"""
Scaling experiment
==================

* demonstrates scaling experiments with a plastic two-population network,
* illustrates problems arising with standard ``integrate-and-fire`` dynamics, and
* describes how to perform exact scaling experiments by using ``ignore_and_fire`` neurons.
"""

import os
import sys
import time

import json

import matplotlib.pyplot as plt
import model
import numpy
from matplotlib import gridspec

##########################################################
## parameters
neuron_models = ["iaf_psc_alpha", "ignore_and_fire"]  ## neuron models
Ns = numpy.arange(1250, 15000, 1250)                  ## network sizes

data_path_root = "./data"                             ## root of path to simulation data

simulate = True
# simulate = False
analyse = True
# analyse = False


##########################################################
## network construction and simulation
def run_model(neuron_model, N, data_path_root):
    """
    Builds an instance of the model for a given network size and neuron model,
    simulates it, records spiking activity and synaptic weight data, measures
    model construction and simulation times, and stores this data in a json file.
    """

    parameters = model.get_default_parameters()

    ## overwrite default parameters
    parameters["record_spikes"] = True
    parameters["neuron_model"] = neuron_model  ## choose model flavor
    parameters["N"] = N  ## specify scale (network sizes)
    parameters["data_path"] = data_path_root + "/N" + str(N)

    ## create and simulate network
    model_instance = model.Model(parameters)

    start = time.time()    
    model_instance.create()
    model_instance.connect()
    stop = time.time()
    build_time = stop - start

    start = time.time()
    model_instance.simulate(model_instance.pars["T"])
    stop = time.time()
    sim_time = stop - start

    ## store model instance parameters
    model_instance.save_parameters("model_instance_parameters", model_instance.pars["data_path"])

    ## record connectivity at end of simulation
    subset_size = 200  ## number of pre- and post-synaptic neurons weights are extracted from
    pop_pre = model_instance.nodes["pop_E"][:subset_size]
    pop_post = model_instance.nodes["pop_E"][:subset_size]
    C = model_instance.get_connectivity(
        pop_pre, pop_post, model_instance.pars["data_path"] + "/" + "connectivity_postsim.dat"
    )

    ## data analysis
    ### time and population averaged firing rate
    spikes = model.load_spike_data(
        model_instance.pars["data_path"], "spikes-%d" % (numpy.array(model_instance.nodes["spike_recorder"])[0])
    )
    rate = time_and_population_averaged_spike_rate(
        spikes, (0.0, model_instance.pars["T"]), model_instance.pars["N_rec_spikes"]
    )

    ### synaptic weight statistics after simulation
    connectivity_postsim = model.load_connectivity_data(model_instance.pars["data_path"], "connectivity_postsim")
    weight_stats = data_statistics(connectivity_postsim[:, 2])
    # weights = numpy.arange(0.,150.1,0.5)
    # whist_postsim = model.get_weight_distribution(connectivity_postsim,weights)
    # print(whist_postsim)

    ##############
    print()
    print("model: %s" % model_instance.pars["neuron_model"])
    print("\tN = %d" % model_instance.pars["N"])
    print("\t\taverage firing rate: nu=%.2f/s" % (rate))
    print(
        "\t\tweight distribution [min,mean,median,sd,max] = [%.1f, %.1f, %.1f, %.1f, %.1f] pA"
        % (weight_stats["min"], weight_stats["mean"], weight_stats["median"], weight_stats["sd"], weight_stats["max"])
    )
    print()
    ##############

    data = {}
    data["build_time"] = build_time
    data["sim_time"] = sim_time
    data["rate"] = rate
    data["weight_mean"] = weight_stats["mean"]
    data["weight_sd"] = weight_stats["sd"]
    data["weight_min"] = weight_stats["min"]
    data["weight_max"] = weight_stats["max"]
    data["weight_median"] = weight_stats["median"]

    save_dict_as_json(data, "data", model_instance.pars["data_path"])

    return data


##########################################################


def time_and_population_averaged_spike_rate(spikes, time_interval, pop_size):
    """
    Calculates the time and population averaged firing rate for an ensemble
    of spike trains within a specified time interval.
    """
    D = time_interval[1] - time_interval[0]
    n_events = sum((spikes[:, 1] >= time_interval[0]) * (spikes[:, 1] <= time_interval[1]))
    rate = n_events / D * 1000.0 / pop_size
    return rate


##########################################################
def data_statistics(data):
    """
    Calculates and collet simple statistics for a list of data samples.
    """
    stats = {}
    stats["mean"] = numpy.nanmean(data)
    stats["sd"] = numpy.nanstd(data)
    stats["min"] = numpy.nanmin(data)
    stats["max"] = numpy.nanmax(data)
    stats["median"] = numpy.nanmedian(data)
    return stats


##############################################
def save_dict_as_json(data_dict, filename_root, path):
    """
    Save python dictionary as json file.

    Arguments
    ---------
    data_dict: dict
               Data dictionary.

    filename_root: str
                   Root of file name.

    path:          str
                   File path

    """

    import json

    json.dump(data_dict, open("%s/%s.json" % (path, filename_root), "w"))

    return


##########################################################
def load_data(neuron_model, N, data_path_root):
    """
    Loads a data dictionary from a json file.
    """
    data = {}

    ## read data from json file
    data_path = data_path_root + "/N" + str(N)  + "/" + neuron_model
    with open(data_path + "/data.json") as f:
        data = json.load(f)

    return data


##########################################################

if analyse:
    build_time = numpy.zeros((len(neuron_models), len(Ns)))
    sim_time = numpy.zeros_like(build_time)
    rate = numpy.zeros_like(build_time)
    weight_mean = numpy.zeros_like(build_time)
    weight_sd = numpy.zeros_like(build_time)
    weight_min = numpy.zeros_like(build_time)
    weight_max = numpy.zeros_like(build_time)
    weight_median = numpy.zeros_like(build_time)

for cm, neuron_model in enumerate(neuron_models):
    print("%s\n" % neuron_model)

    for cN, N in enumerate(Ns):
        print("N = %d" % N)

        if simulate:
            data = run_model(neuron_model, int(N), data_path_root)

        if analyse:
            data = load_data(neuron_model, int(N), data_path_root)

            build_time[cm, cN] = data["build_time"]
            sim_time[cm, cN] = data["sim_time"]
            rate[cm, cN] = data["rate"]
            weight_mean[cm, cN] = data["weight_mean"]
            weight_sd[cm, cN] = data["weight_sd"]
            weight_min[cm, cN] = data["weight_min"]
            weight_max[cm, cN] = data["weight_max"]
            weight_median[cm, cN] = data["weight_median"]

        print()
    print()

if analyse:
    ## plotting
    fig = plt.figure(num=3, figsize=(3, 4), dpi=300)
    plt.clf()

    plt.rcParams.update(
        {
            "text.usetex": True,
            "font.size": 8,
            "axes.titlesize": 8,
            "legend.fontsize": 8,
        }
    )

    gs = gridspec.GridSpec(3, 1)

    ## build and sim times
    ax1 = fig.add_subplot(gs[0, 0])

    ## firing rate
    ax2 = fig.add_subplot(gs[1, 0])

    ## weight stat
    ax3 = fig.add_subplot(gs[2, 0])

    ms = 4
    lw = 2
    clrs = ["0", "0.8"]

    for cm, neuron_model in enumerate(neuron_models):
        ## sim time
        ax1.plot(
            Ns,
            sim_time[cm, :],
            "-o",
            mfc=clrs[cm],
            mec=clrs[cm],
            ms=ms,
            lw=lw,
            color=clrs[cm],
            label=r"%s" % neuron_model,
        )
        ax1.set_xlim(Ns[0], Ns[-1])
        ax1.set_xticklabels([])
        ax1.set_ylabel(r"simulation time (s)")
        ax1.set_title(r"fixed in-degree $K=1250$")

        ## firing rate
        ax2.plot(
            Ns, rate[cm, :], "-o", mfc=clrs[cm], mec=clrs[cm], ms=ms, lw=lw, color=clrs[cm], label=r"%s" % neuron_model
        )
        ax2.set_xlim(Ns[0], Ns[-1])
        ax2.set_xticklabels([])
        ax2.set_ylim(0.5, 2)
        ax2.set_ylabel(r"firing rate (1/s)")
        ax2.legend(loc=1)

        ## weight stat
        if cm == 0:
            lbl1 = "mean"
            lbl2 = "mean + SD"
        else:
            lbl1 = ""
            lbl2 = ""

        ax3.plot(Ns, weight_mean[cm, :], "-o", mfc=clrs[cm], mec=clrs[cm], lw=lw, color=clrs[cm], ms=ms, label=lbl1)
        ax3.plot(
            Ns,
            weight_mean[cm, :] + weight_sd[cm, :],
            "--",
            mfc=clrs[cm],
            mec=clrs[cm],
            lw=lw,
            color=clrs[cm],
            ms=ms,
            label=lbl2,
        )
        ax3.set_xlim(Ns[0], Ns[-1])
        ax3.set_ylim(10, 100)
        ax3.set_xlabel(r"network size $N$")
        ax3.set_ylabel(r"synaptic weight (pA)")
        ax3.legend(loc=1)

    plt.subplots_adjust(left=0.17, bottom=0.1, right=0.95, top=0.95, hspace=0.1)
    plt.savefig("../figures/scaling.png")
