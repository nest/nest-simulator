# -*- coding: utf-8 -*-
#
# raster_plot.py
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

""" Functions for raster plotting."""

import nest
import numpy

__all__ = [
    'extract_events',
    'from_data',
    'from_device',
    'from_file',
    'from_file_numpy',
    'from_file_pandas'
]


def extract_events(data, time=None, sel=None):
    """Extract all events within a given time interval.

    Both time and sel may be used at the same time such that all
    events are extracted for which both conditions are true.

    Parameters
    ----------
    data : list
        Matrix such that
        data[:,0] is a vector of all node_ids and
        data[:,1] a vector with the corresponding time stamps.
    time : list, optional
        List with at most two entries such that
        time=[t_max] extracts all events with t< t_max
        time=[t_min, t_max] extracts all events with t_min <= t < t_max
    sel : list, optional
        List of node_ids such that
        sel=[node_id1, ... , node_idn] extracts all events from these node_ids.
        All others are discarded.

    Returns
    -------
    numpy.array
        List of events as (node_id, t) tuples
    """
    val = []

    if time:
        t_max = time[-1]
        if len(time) > 1:
            t_min = time[0]
        else:
            t_min = 0

    for v in data:
        t = v[1]
        node_id = v[0]
        if time and (t < t_min or t >= t_max):
            continue
        if not sel or node_id in sel:
            val.append(v)

    return numpy.array(val)


def from_data(data, sel=None, **kwargs):
    """Plot raster plot from data array.

    Parameters
    ----------
    data : list
        Matrix such that
        data[:,0] is a vector of all node_ids and
        data[:,1] a vector with the corresponding time stamps.
    sel : list, optional
        List of node_ids such that
        sel=[node_id1, ... , node_idn] extracts all events from these node_ids.
        All others are discarded.
    kwargs:
        Parameters passed to _make_plot
    """
    if len(data) == 0:
        raise nest.kernel.NESTError("No data to plot.")
    ts = data[:, 1]
    d = extract_events(data, sel=sel)
    ts1 = d[:, 1]
    node_ids = d[:, 0]

    return _make_plot(ts, ts1, node_ids, data[:, 0], **kwargs)


def from_file(fname, **kwargs):
    """Plot raster from file.

    Parameters
    ----------
    fname : str or tuple(str) or list(str)
        File name or list of file names

        If a list of files is given, the data from them is concatenated as if
        it had been stored in a single file - useful when MPI is enabled and
        data is logged separately for each MPI rank, for example.
    kwargs:
        Parameters passed to _make_plot
    """
    if isinstance(fname, str):
        fname = [fname]

    if isinstance(fname, (list, tuple)):
        try:
            global pandas
            pandas = __import__('pandas')
            from_file_pandas(fname, **kwargs)
        except ImportError:
            from_file_numpy(fname, **kwargs)
    else:
        print('fname should be one of str/list(str)/tuple(str).')


def from_file_pandas(fname, **kwargs):
    """Use pandas."""
    data = None
    for f in fname:
        dataFrame = pandas.read_table(f, header=2, skipinitialspace=True)
        newdata = dataFrame.values

        if data is None:
            data = newdata
        else:
            data = numpy.concatenate((data, newdata))

    return from_data(data, **kwargs)


def from_file_numpy(fname, **kwargs):
    """Use numpy."""
    data = None
    for f in fname:
        newdata = numpy.loadtxt(f, skiprows=3)

        if data is None:
            data = newdata
        else:
            data = numpy.concatenate((data, newdata))

    return from_data(data, **kwargs)


def from_device(detec, **kwargs):
    """
    Plot raster from a spike recorder.

    Parameters
    ----------
    detec : TYPE
        Description
    kwargs:
        Parameters passed to _make_plot

    Raises
    ------
    nest.kernel.NESTError
    """

    type_id = nest.GetDefaults(detec.get('model'), 'type_id')
    if not type_id == "spike_recorder":
        raise nest.kernel.NESTError("Please provide a spike_recorder.")

    if detec.get('record_to') == "memory":

        ts, node_ids = _from_memory(detec)

        if not len(ts):
            raise nest.kernel.NESTError("No events recorded!")

        if "title" not in kwargs:
            kwargs["title"] = "Raster plot from device '%i'" % detec.get('global_id')

        if detec.get('time_in_steps'):
            xlabel = "Steps"
        else:
            xlabel = "Time (ms)"

        return _make_plot(ts, ts, node_ids, node_ids, xlabel=xlabel, **kwargs)

    elif detec.get("record_to") == "ascii":
        fname = detec.get("filenames")
        return from_file(fname, **kwargs)

    else:
        raise nest.kernel.NESTError("No data to plot. Make sure that \
            record_to is set to either 'ascii' or 'memory'.")


def _from_memory(detec):
    ev = detec.get("events")
    return ev["times"], ev["senders"]


def _make_plot(ts, ts1, node_ids, neurons, hist=True, hist_binwidth=5.0,
               grayscale=False, title=None, xlabel=None):
    """Generic plotting routine.

    Constructs a raster plot along with an optional histogram (common part in
    all routines above).

    Parameters
    ----------
    ts : list
        All timestamps
    ts1 : list
        Timestamps corresponding to node_ids
    node_ids : list
        Global ids corresponding to ts1
    neurons : list
        Node IDs of neurons to plot
    hist : bool, optional
        Display histogram
    hist_binwidth : float, optional
        Width of histogram bins
    grayscale : bool, optional
        Plot in grayscale
    title : str, optional
        Plot title
    xlabel : str, optional
        Label for x-axis
    """
    import matplotlib.pyplot as plt

    plt.figure()

    if grayscale:
        color_marker = ".k"
        color_bar = "gray"
    else:
        color_marker = "."
        color_bar = "blue"

    color_edge = "black"

    if xlabel is None:
        xlabel = "Time (ms)"

    ylabel = "Neuron ID"

    if hist:
        ax1 = plt.axes([0.1, 0.3, 0.85, 0.6])
        plotid = plt.plot(ts1, node_ids, color_marker)
        plt.ylabel(ylabel)
        plt.xticks([])
        xlim = plt.xlim()

        plt.axes([0.1, 0.1, 0.85, 0.17])
        t_bins = numpy.arange(
            numpy.amin(ts), numpy.amax(ts),
            float(hist_binwidth)
        )
        n, _ = _histogram(ts, bins=t_bins)
        num_neurons = len(numpy.unique(neurons))
        heights = 1000 * n / (hist_binwidth * num_neurons)

        plt.bar(t_bins, heights, width=hist_binwidth, color=color_bar,
                edgecolor=color_edge)
        plt.yticks([
            int(x) for x in
            numpy.linspace(0.0, int(max(heights) * 1.1) + 5, 4)
        ])
        plt.ylabel("Rate (Hz)")
        plt.xlabel(xlabel)
        plt.xlim(xlim)
        plt.axes(ax1)
    else:
        plotid = plt.plot(ts1, node_ids, color_marker)
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)

    if title is None:
        plt.title("Raster plot")
    else:
        plt.title(title)

    plt.draw()

    return plotid


def _histogram(a, bins=10, bin_range=None, normed=False):
    """Calculate histogram for data.

    Parameters
    ----------
    a : list
        Data to calculate histogram for
    bins : int, optional
        Number of bins
    bin_range : TYPE, optional
        Range of bins
    normed : bool, optional
        Whether distribution should be normalized

    Raises
    ------
    ValueError
    """
    from numpy import asarray, iterable, linspace, sort, concatenate

    a = asarray(a).ravel()

    if bin_range is not None:
        mn, mx = bin_range
        if mn > mx:
            raise ValueError("max must be larger than min in range parameter")

    if not iterable(bins):
        if bin_range is None:
            bin_range = (a.min(), a.max())
        mn, mx = [mi + 0.0 for mi in bin_range]
        if mn == mx:
            mn -= 0.5
            mx += 0.5
        bins = linspace(mn, mx, bins, endpoint=False)
    else:
        if (bins[1:] - bins[:-1] < 0).any():
            raise ValueError("bins must increase monotonically")

    # best block size probably depends on processor cache size
    block = 65536
    n = sort(a[:block]).searchsorted(bins)
    for i in range(block, a.size, block):
        n += sort(a[i:i + block]).searchsorted(bins)
    n = concatenate([n, [len(a)]])
    n = n[1:] - n[:-1]

    if normed:
        db = bins[1] - bins[0]
        return 1.0 / (a.size * db) * n, bins
    else:
        return n, bins
