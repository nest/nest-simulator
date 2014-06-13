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

import nest
import numpy
import pylab

def extract_events(data, time=None, sel=None):
    """
    Extracts all events within a given time interval or are from a
    given set of neurons.
    - data is a matrix such that
      data[:,0] is a vector of all gids and
      data[:,1] a vector with the corresponding time stamps.
    - time is a list with at most two entries such that
      time=[t_max] extracts all events with t< t_max
      time=[t_min, t_max] extracts all events with t_min <= t < t_max
    - sel is a list of gids such that
      sel=[gid1, ... , gidn] extracts all events from these gids.
      All others are discarded.
    Both time and sel may be used at the same time such that all
    events are extracted for which both conditions are true.
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
        gid = v[0]
        if time and (t < t_min or t >= t_max):
            continue
        if not sel or gid in sel:
            val.append(v)

    return numpy.array(val)

def from_data(data, title=None, hist=False, hist_binwidth=5.0, grayscale=False, sel=None):
    """
    Plot raster from data array
    """

    ts = data[:, 1]
    d = extract_events(data, sel=sel)
    ts1 = d[:, 1]
    gids = d[:, 0]

    return _make_plot(ts, ts1, gids, data[:, 0], hist, hist_binwidth, grayscale, title)

def from_file(fname, title=None, hist=False, hist_binwidth=5.0, grayscale=False):
    """
    Plot raster from file
    """

    if nest.is_iterable(fname):
        data = None
        for f in fname:
            if data is None:
                data = numpy.loadtxt(f)
            else:
                data = numpy.concatenate((data, numpy.loadtxt(f)))
    else:
        data = numpy.loadtxt(fname)

    return from_data(data, title, hist, hist_binwidth, grayscale)

def from_device(detec, title=None, hist=False, hist_binwidth=5.0, grayscale=False, plot_lid=False):
    """
    Plot raster from spike detector
    """

    if not nest.GetStatus(detec)[0]["model"] == "spike_detector":
        raise nest.NESTError("Please provide a spike_detector.")

    if nest.GetStatus(detec, "to_memory")[0]:

        ts, gids = _from_memory(detec)

        if not len(ts):
            raise nest.NESTError("No events recorded!")

        if plot_lid:
            gids = [nest.GetLID([x]) for x in gids]

        if title is None:
            title = "Raster plot from device '%i'" % detec[0]

        if nest.GetStatus(detec)[0]["time_in_steps"]:
            xlabel = "Steps"
        else:
            xlabel = "Time (ms)"

        return _make_plot(ts, ts, gids, gids, hist, hist_binwidth, grayscale, title, xlabel)

    elif nest.GetStatus(detec, "to_file")[0]:
        fname = nest.GetStatus(detec, "filenames")[0]
        return from_file(fname, title, hist, hist_binwidth, grayscale)

    else:
        raise nest.NESTError("No data to plot. Make sure that either to_memory or to_file are set.")

def _from_memory(detec):
    ev = nest.GetStatus(detec, "events")[0]
    return ev["times"], ev["senders"]

def _make_plot(ts, ts1, gids, neurons, hist, hist_binwidth, grayscale, title, xlabel=None):
    """
    Generic plotting routine that constructs a raster plot along with
    an optional histogram (common part in all routines above)
    """

    pylab.figure()

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
        ax1 = pylab.axes([0.1, 0.3, 0.85, 0.6])
        plotid = pylab.plot(ts1, gids, color_marker)
        pylab.ylabel(ylabel)
        pylab.xticks([])
        xlim = pylab.xlim()

        pylab.axes([0.1, 0.1, 0.85, 0.17])
        t_bins = numpy.arange(numpy.amin(ts), numpy.amax(ts), float(hist_binwidth))
        n, bins = _histogram(ts, bins=t_bins)
        num_neurons = len(numpy.unique(neurons))
        heights = 1000 * n / (hist_binwidth * num_neurons)
        pylab.bar(t_bins, heights, width=hist_binwidth, color=color_bar, edgecolor=color_edge)
        pylab.yticks([int(x) for x in numpy.linspace(0.0, int(max(heights) * 1.1) + 5, 4)])
        pylab.ylabel("Rate (Hz)")
        pylab.xlabel(xlabel)
        pylab.xlim(xlim)
        pylab.axes(ax1)
    else:
        plotid = pylab.plot(ts1, gids, color_marker)
        pylab.xlabel(xlabel)
        pylab.ylabel(ylabel)

    if title is None:
        pylab.title("Raster plot")
    else:
        pylab.title(title)

    pylab.draw()

    return plotid


def _histogram(a, bins=10, bin_range=None, normed=False):
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

def show():
    """
    Call pylab.show() to show all figures and enter the GUI main loop.
    Python will block until all figure windows are closed again.
    You should call this function only once at the end of a script.

    See also: http://matplotlib.sourceforge.net/faq/howto_faq.html#use-show
    """

    pylab.show()
