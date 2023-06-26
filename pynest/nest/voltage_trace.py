# -*- coding: utf-8 -*-
#
# voltage_trace.py
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
Functions to plot voltage traces.
"""

import nest
import numpy

__all__ = [
    "from_device",
    "from_file",
]


def from_file(fname, title=None, grayscale=False):
    """Plot voltage trace from file.

    Parameters
    ----------
    fname : str or list
        Filename or list of filenames to load from
    title : str, optional
        Plot title
    grayscale : bool, optional
        Plot in grayscale

    Raises
    ------
    ValueError
    """
    import matplotlib.pyplot as plt

    if isinstance(fname, (list, tuple)):
        data = None
        for file in fname:
            if data is None:
                data = numpy.loadtxt(file)
            else:
                data = numpy.concatenate((data, numpy.loadtxt(file)))
    else:
        data = numpy.loadtxt(fname)

    if grayscale:
        line_style = "k"
    else:
        line_style = ""

    if len(data.shape) == 1:
        print(
            "INFO: only found 1 column in the file. \
            Assuming that only one neuron was recorded."
        )
        plotid = plt.plot(data, line_style)
        plt.xlabel("Time (steps of length interval)")

    elif data.shape[1] == 2:
        print(
            "INFO: found 2 columns in the file. Assuming \
            them to be node ID, pot."
        )

        plotid = []
        data_dict = {}
        for dat in data:
            if not dat[0] in data_dict:
                data_dict[dat[0]] = [dat[1]]
            else:
                data_dict[dat[0]].append(dat[1])

        for dat in data_dict:
            plotid.append(plt.plot(data_dict[dat], line_style, label="Neuron %i" % dat))

        plt.xlabel("Time (steps of length interval)")
        plt.legend()

    elif data.shape[1] == 3:
        plotid = []
        data_dict = {}
        g = data[0][0]
        t = []
        for d in data:
            if not d[0] in data_dict:
                data_dict[d[0]] = [d[2]]
            else:
                data_dict[d[0]].append(d[2])
            if d[0] == g:
                t.append(d[1])

        for d in data_dict:
            plotid.append(plt.plot(t, data_dict[d], line_style, label="Neuron %i" % d))

        plt.xlabel("Time (ms)")
        plt.legend()

    else:
        raise ValueError("Inappropriate data shape %i!" % data.shape)

    if not title:
        title = "Membrane potential from file '%s'" % fname

    plt.title(title)
    plt.ylabel("Membrane potential (mV)")
    plt.draw()

    return plotid


def from_device(detec, neurons=None, title=None, grayscale=False, timeunit="ms"):
    """Plot the membrane potential of a set of neurons recorded by
    the given voltmeter or multimeter.

    Parameters
    ----------
    detec : list
        Global id of voltmeter or multimeter in a list, e.g. [1]
    neurons : list, optional
        Indices of of neurons to plot
    title : str, optional
        Plot title
    grayscale : bool, optional
        Plot in grayscale
    timeunit : str, optional
        Unit of time

    Raises
    ------
    nest.kernel.NESTError
        Description
    """
    import matplotlib.pyplot as plt

    if len(detec) > 1:
        raise nest.kernel.NESTError("Please provide a single voltmeter.")

    type_id = nest.GetDefaults(detec.get("model"), "type_id")
    if type_id not in ("voltmeter", "multimeter"):
        raise nest.kernel.NESTError(
            "Please provide a voltmeter or a \
            multimeter measuring V_m."
        )
    elif type_id == "multimeter":
        if "V_m" not in detec.get("record_from"):
            raise nest.kernel.NESTError(
                "Please provide a multimeter \
                measuring V_m."
            )
        elif not detec.get("record_to") == "memory" and len(detec.get("record_from")) > 1:
            raise nest.kernel.NESTError(
                "Please provide a multimeter \
                measuring only V_m or record to memory!"
            )

    if detec.get("record_to") == "memory":
        timefactor = 1.0
        if not detec.get("time_in_steps"):
            if timeunit == "s":
                timefactor = 1000.0
            else:
                timeunit = "ms"

        times, voltages = _from_memory(detec)

        if not len(times):
            raise nest.NESTError("No events recorded!")

        if neurons is None:
            neurons = voltages.keys()

        plotids = []
        for neuron in neurons:
            time_values = numpy.array(times[neuron]) / timefactor

            if grayscale:
                line_style = "k"
            else:
                line_style = ""

            try:
                plotids.append(plt.plot(time_values, voltages[neuron], line_style, label="Neuron %i" % neuron))
            except KeyError:
                print("INFO: Wrong ID: {0}".format(neuron))

        if not title:
            title = "Membrane potential"
        plt.title(title)

        plt.ylabel("Membrane potential (mV)")

        if nest.GetStatus(detec)[0]["time_in_steps"]:
            plt.xlabel("Steps")
        else:
            plt.xlabel("Time (%s)" % timeunit)

        plt.legend(loc="best")
        plt.draw()

        return plotids

    elif detec.get("record_to") == "ascii":
        fname = detec.get("filenames")
        return from_file(fname, title, grayscale)
    else:
        raise nest.kernel.NESTError(
            "Provided devices neither record to \
            ascii file, nor to memory."
        )


def _from_memory(detec):
    """Get voltage traces from memory.
    ----------
    detec : list
        Global id of voltmeter or multimeter
    """
    import array

    ev = detec.get("events")
    potentials = ev["V_m"]
    senders = ev["senders"]

    v = {}
    t = {}

    if "times" in ev:
        times = ev["times"]
        for s, currentsender in enumerate(senders):
            if currentsender not in v:
                v[currentsender] = array.array("f")
                t[currentsender] = array.array("f")

            v[currentsender].append(float(potentials[s]))
            t[currentsender].append(float(times[s]))
    else:
        # reconstruct the time vector, if not stored explicitly
        origin = detec.get("origin")
        start = detec.get("start")
        interval = detec.get("interval")
        senders_uniq = numpy.unique(senders)
        num_intvls = len(senders) / len(senders_uniq)
        times_s = origin + start + interval + interval * numpy.array(range(num_intvls))

        for s, currentsender in enumerate(senders):
            if currentsender not in v:
                v[currentsender] = array.array("f")
                t[currentsender] = times_s
            v[currentsender].append(float(potentials[s]))

    return t, v
