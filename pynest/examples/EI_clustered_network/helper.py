# -*- coding: utf-8 -*-
#
# helper.py
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

"""PyNEST EI-clustered network: Helper Functions
------------------------------------------------

Helper functions to calculate synaptic weights and first passage times for exponential synapses to construct
random balanced networks and plot raster plot with color groups.
"""

import matplotlib.pyplot as plt
import numpy as np

small = 1e-10


def postsynaptic_current_to_potential(tau_m, tau_syn, c_m=1.0, e_l=0.0):
    """Maximum post-synaptic potential amplitude for exponential synapses and a synaptic efficacy J of 1 pA.

    Parameters
    ----------
    tau_m: float
        Membrane time constant [ms]
    tau_syn: float
        Synapse time constant  [ms]
    c_m: float (optional)
        Membrane capacity [pF] (default: 1.0)
    e_l: float (optional)
        Resting potential [mV] (default: 0.0)

    Returns
    -------
    float
        maximum psp amplitude (for a 1 pA current spike) [mV]
    """
    tmax = np.log(tau_syn / tau_m) / (1 / tau_m - 1 / tau_syn)  # time of maximum deflection of the psp  [ms]
    pre = tau_m * tau_syn / c_m / (tau_syn - tau_m)  # we assume here the current spike is 1 pA, otherwise [mV/pA]
    return (e_l - pre) * np.exp(-tmax / tau_m) + pre * np.exp(-tmax / tau_syn)


def calculate_RBN_weights(params):
    """Calculate synaptic weights for exponential synapses before clustering, for a random balanced network.

    Parameters
    ----------
    params: dict
        Dictionary of network parameters

    Returns
    -------
    ndarray
        synaptic weights 2x2 matrix [[EE, EI], [IE, II]]
    """
    N_E = params.get("N_E")  # excitatory units
    N_I = params.get("N_I")  # inhibitory units
    N = N_E + N_I  # total units
    baseline_conn_prob = params.get("baseline_conn_prob")  # connection probs
    gei = params.get("gei")
    gii = params.get("gii")
    gie = params.get("gie")
    V_th_E = params.get("V_th_E")  # threshold voltage
    V_th_I = params.get("V_th_I")
    tau_E = params.get("tau_E")
    tau_I = params.get("tau_I")
    E_L = params.get("E_L")
    tau_syn_ex = params.get("tau_syn_ex")
    tau_syn_in = params.get("tau_syn_in")
    amp_EE = postsynaptic_current_to_potential(tau_E, tau_syn_ex)
    amp_EI = postsynaptic_current_to_potential(tau_E, tau_syn_in)
    amp_IE = postsynaptic_current_to_potential(tau_I, tau_syn_ex)
    amp_II = postsynaptic_current_to_potential(tau_I, tau_syn_in)

    js = np.zeros((2, 2))
    K_EE = N_E * baseline_conn_prob[0, 0]
    js[0, 0] = (V_th_E - E_L) * (K_EE**-0.5) * N**0.5 / amp_EE
    js[0, 1] = -gei * js[0, 0] * baseline_conn_prob[0, 0] * N_E * amp_EE / (baseline_conn_prob[0, 1] * N_I * amp_EI)
    K_IE = N_E * baseline_conn_prob[1, 0]
    js[1, 0] = gie * (V_th_I - E_L) * (K_IE**-0.5) * N**0.5 / amp_IE
    js[1, 1] = -gii * js[1, 0] * baseline_conn_prob[1, 0] * N_E * amp_IE / (baseline_conn_prob[1, 1] * N_I * amp_II)
    return js


def rheobase_current(tau_m, e_l, v_th, c_m):
    """Rheobase current for a given membrane time constant and resting potential.

    Parameters
    ----------
    tau_m: float
        Membrane time constant [ms]
    e_l: float
        Resting potential [mV]
    v_th: float
        Threshold potential [mV]
    c_m: float
        Membrane capacity [pF]

    Returns
    -------
    float
        rheobase current [pA]
    """
    return (v_th - e_l) * c_m / tau_m


def raster_plot(spiketimes, tlim=None, colorgroups=None, ax=None, markersize=0.5):
    """Raster plot of spiketimes.

    Plots raster plot of spiketimes withing given time limits and colors neurons according to colorgroups.

    Parameters
    ----------
    spiketimes: ndarray
        2D array [2xN_Spikes] of spiketimes with spiketimes in row 0 and neuron IDs in row 1.
    tlim: list of floats (optional)
        Time limits of plot: [tmin, tmax], if None: [min(spiketimes), max(spiketimes)]
    colorgroups: list of tuples (optional)
        Each element is a tuple in the format (color, start_neuron_ID, stop_neuron_ID]) for coloring neurons in
        given range, if None is given all neurons are black.
    ax: axis object (optional)
        If None a new figure is created, else the plot is added to the given axis.
    markersize: float (optional)
        Size of markers. (default: 0.5)

    Returns
    -------
    axis object
        Axis object with raster plot.
    """
    if ax is None:
        fig, ax = plt.subplots()
    if tlim is None:
        tlim = [min(spiketimes[0]), max(spiketimes[0])]
    if colorgroups is None:
        colorgroups = [("k", 0, max(spiketimes[1]))]
    for color, start, stop in colorgroups:
        ax.plot(
            spiketimes[0][np.logical_and(spiketimes[1] >= start, spiketimes[1] < stop)],
            spiketimes[1][np.logical_and(spiketimes[1] >= start, spiketimes[1] < stop)],
            color=color,
            marker=".",
            linestyle="None",
            markersize=markersize,
        )
    ax.set_xlim(tlim)
    ax.set_ylim([0, max(spiketimes[1])])
    ax.set_xlabel("Time [ms]")
    ax.set_ylabel("Neuron ID")
    return ax
