# -*- coding: utf-8 -*-
#
# multimeter_file.py
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
Example of multimeter recording to file
---------------------------------------

This file demonstrates recording from an ``iaf_cond_alpha`` neuron using a
multimeter and writing data to file.

"""

###############################################################################
# First, we import the necessary modules to simulate and plot this example.
# The simulation kernel is put back to its initial state using ``ResetKernel``.

import nest
import numpy
import matplotlib.pyplot as plt

nest.ResetKernel()

###############################################################################
# With ``SetKernelStatus``, global properties of the simulation kernel can be
# specified. The following properties are related to writing to file:
#
# * ``overwrite_files`` is set to True to permit overwriting of an existing file.
# * ``data_path`` is the path to which all data is written. It is given relative
#   to  the current working directory.
# * 'data_prefix' allows to specify a common prefix for all data files.

nest.SetKernelStatus({"overwrite_files": True,
                      "data_path": "",
                      "data_prefix": ""})

###############################################################################
# For illustration, the recordables of the ``iaf_cond_alpha`` neuron model are
# displayed. This model is an implementation of a spiking neuron using
# integrate-and-fire dynamics with conductance-based synapses. Incoming spike
# events induce a postsynaptic change of conductance modeled by an alpha
# function.

print("iaf_cond_alpha recordables: {0}".format(
      nest.GetDefaults("iaf_cond_alpha")["recordables"]))

###############################################################################
# A neuron, a multimeter as recording device and two spike generators for
# excitatory and inhibitory stimulation are instantiated. The command ``Create``
# expects a model type and, optionally, the desired number of nodes and a
# dictionary of parameters to overwrite the default values of the model.
#
#  * For the neuron, the rise time of the excitatory synaptic alpha function
#    (`tau_syn_ex`, in ms) and the reset potential of the membrane
#    (`V_reset`, in mV) are specified.
#  * For the ``multimeter``, the time interval for recording (`interval`, in
#    ms) and the measures to record (membrane potential `V_m` in mV and
#    excitatory and inhibitoy synaptic conductances `g_ex` and`g_in` in nS)
#    are set.
#
#  In addition, more parameters can be modified for writing to file:
#
#  - `record_to` indicates where to put recorded data. All possible values are
#    available by inspecting the keys of the `recording_backends` dictionary
#    obtained from ``GetKernelStatus()``.
#  - `label` specifies an arbitrary label for the device. If writing to files,
#    it used in the file name instead of the model name.
#
#  * For the spike generators, the spike times in ms (`spike_times`) are given
#    explicitly.

n = nest.Create("iaf_cond_alpha",
                params={"tau_syn_ex": 1.0, "V_reset": -70.0})

m = nest.Create("multimeter",
                params={"interval": 0.1,
                        "record_from": ["V_m", "g_ex", "g_in"],
                        "record_to": "ascii",
                        "label": "my_multimeter"})

s_ex = nest.Create("spike_generator",
                   params={"spike_times": numpy.array([10.0, 20.0, 50.0])})
s_in = nest.Create("spike_generator",
                   params={"spike_times": numpy.array([15.0, 25.0, 55.0])})

###############################################################################
# Next, We connect the spike generators to the neuron with ``Connect``. Synapse
# specifications can be provided in a dictionary. In this example of a
# conductance-based neuron, the synaptic weight ``weight`` is given in nS.
# Note that the values are  positive for excitatory stimulation and negative
# for inhibitor connections.

nest.Connect(s_ex, n, syn_spec={"weight": 40.0})
nest.Connect(s_in, n, syn_spec={"weight": -20.0})
nest.Connect(m, n)

###############################################################################
# A network simulation with a duration of 100 ms is started with ``Simulate``.

nest.Simulate(100.)

###############################################################################
# After the simulation, the recordings are obtained from the file the
# multimeter wrote to, accessed with the `filenames` property of the
# multimeter. After three header rows, the data is formatted in columns. The
# first column is the ID of the sender node. The second column is the ID time
# of the recording, in ms. Subsequent rows are values of properties specified
# in the `record_from` property of the multimeter.

data = numpy.loadtxt(m.filenames[0], skiprows=3)
sender, t, v_m, g_in, g_ex = data.T

###############################################################################
# Finally, the time courses of the membrane voltage and the synaptic
# conductance are displayed.

plt.clf()

plt.subplot(211)
plt.plot(t, v_m)
plt.axis([0, 100, -75, -53])
plt.ylabel("membrane potential (mV)")

plt.subplot(212)
plt.plot(t, g_ex, t, g_in)
plt.axis([0, 100, 0, 45])
plt.xlabel("time (ms)")
plt.ylabel("synaptic conductance (nS)")
plt.legend(("g_exc", "g_inh"))
plt.show()
