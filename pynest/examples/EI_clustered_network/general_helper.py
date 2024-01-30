# -*- coding: utf-8 -*-
#
# general_helper.py
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

"""PyNEST EI-clustered network: general helper functions
--------------------------------------------------------

Helper functions for merging dictionaries and some plotting functions.
"""
import copy
from types import ModuleType

import matplotlib.pyplot as plt
import numpy as np


def merge_params(params, default_values):
    """Merge default values with params dictionary.

    Updates default Values defined in a module with params-dictionary.

    Parameters
    ----------
    params: dict
         Override parameters.
    default_values: module
        Parameters with default values.

    Returns
    -------
    dict
        Dictionary with default values updated by params.
    """
    return nested_update(module_to_dict(default_values), params)


def module_to_dict(module):
    """Create dict of module variables.

    Creates dict of explicit variables in module which are not imported modules.
    Generates for each variable name an entry in the dictionary.

    Parameters
    ----------
    module: module
        Module to be converted to dict.

    Returns
    -------
    dict
        Dictionary with module variables.
    """
    module_dict = {}
    if module:
        module_dict = {
            key: value
            for key, value in module.__dict__.items()
            if not (
                key.startswith("__")
                or key.startswith("_")
                or isinstance(value, ModuleType)
            )
        }
    return module_dict


def nested_update(d, d2):
    """Update nested dictionary.

    Update values in one dictionary with values of second dictionary. Adds new keys if they are not present.

    Parameters
    ----------
    d: dict
        Dictionary to be updated.
    d2: dict
        Dictionary with update values.

    Returns
    -------
    dict
        Updated dictionary.
    """
    d_local = copy.deepcopy(d)
    for key in d2:
        if isinstance(d2[key], dict) and key in d_local:
            d_local[key] = nested_update(d_local[key], d2[key])
        else:
            d_local[key] = d2[key]
    return d_local


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
