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
Multimeter to file example
--------------------------

This file demonstrates recording from an `iaf_cond_alpha` neuron using a
multimeter and writing data to file.


KEYWORDS: iaf_cond_alpha
"""

###############################################################################
# First, we import the necessary modules to simulate and plot this example.
# The simulation kernel is put back to its initial state using `ResetKernel`.

import nest
import numpy
import pylab

nest.ResetKernel()

###############################################################################
# With `SetKernelStatus`, global properties of the simulation kernel can be
# specified. The following properties are related to writing to file:
#
# * `overwrite_files` is set to True to permit overwriting of an existing file.
# * `data_path` is the path to which all data is written. It is given relative
#    to  the current working directory.
# * 'data_prefix' allows to specify a common prefix for all data files.

nest.SetKernelStatus({"overwrite_files": True,
                      "data_path": "",
                      "data_prefix": ""})

###############################################################################
# For illustration, the recordables of the `iaf_cond_alpha` neuron model are
# displayed. This model is an implementation of a spiking neuron using
# integrate-and-fire dynamics with conductance-based synapses. Incoming spike
# events induce a post-synaptic change of conductance modeled by an alpha
# function.

print("iaf_cond_alpha recordables: {0}".format(
      nest.GetDefaults("iaf_cond_alpha")["recordables"]))

###############################################################################
# A neuron, a multimeter as recording device and two spike generators for
# excitatory and inhibitory stimulation are instantiated. The command `Create`
# expects a model type and, optionally, the desired number of nodes and a
# dictionary of parameters to overwrite the default values of the model.
#
#* For the neuron, the rise time of the excitatory synaptic alpha function
# in ms `tau_syn_ex` and the reset potential of the membrane in mV `V_reset`
#  are specified.
#* For the multimeter, the time interval for recording in ms `interval` and a
#  selection of measures to record (the membrane voltage in mV `V_m` and the
#  excitatory `g_ex` and inhibitoy `g_in` synaptic conductances in nS) are set.
#
#  In addition, more parameters can be modified for writing to file:
#
#  - `withgid` is set to True to record the global id of the observed node(s).
#    (default: False).
#  - `to_file` indicates whether to write the recordings to file and is set
#    to True.
#  - `label` specifies an arbitrary label for the device. It is used instead of
#   the name of the model in the output file name.
#
#* For the spike generators, the spike times in ms `spike_times` are given
#  explicitly.

n = nest.Create("iaf_cond_alpha",
                params={"tau_syn_ex": 1.0, "V_reset": -70.0})

m = nest.Create("multimeter",
                params={"interval": 0.1,
                        "record_from": ["V_m", "g_ex", "g_in"],
                        "withgid": True,
                        "to_file": True,
                        "label": "my_multimeter"})

s_ex = nest.Create("spike_generator",
                   params={"spike_times": numpy.array([10.0, 20.0, 50.0])})
s_in = nest.Create("spike_generator",
                   params={"spike_times": numpy.array([15.0, 25.0, 55.0])})

###############################################################################
# Next, We connect the spike generators to the neuron with `Connect`. Synapse
# specifications can be provided in a dictionary. In this example of a
# conductance-based neuron, the synaptic weight `weight` is given in nS.
# Note that the values are  positive for excitatory stimulation and negative
# for inhibitor connections.

nest.Connect(s_ex, n, syn_spec={"weight": 40.0})
nest.Connect(s_in, n, syn_spec={"weight": -20.0})
nest.Connect(m, n)

###############################################################################
# A network simulation with a duration of 100 ms is started with `Simulate`.

nest.Simulate(100.)

###############################################################################
# After the simulation, the recordings are obtained from the multimeter via the
# key `events` of the status dictionary accessed by `GetStatus`. `times`
# indicates the recording times stored for each data point. They are recorded
# if the parameter `withtime` of the multimeter is set to True which is the
# default case.

events = nest.GetStatus(m)[0]["events"]
t = events["times"]

###############################################################################
# Finally, the time courses of the membrane voltage and the synaptic
# conductance are displayed.

pylab.clf()

pylab.subplot(211)
pylab.plot(t, events["V_m"])
pylab.axis([0, 100, -75, -53])
pylab.ylabel("membrane potential (mV)")

pylab.subplot(212)
pylab.plot(t, events["g_ex"], t, events["g_in"])
pylab.axis([0, 100, 0, 45])
pylab.xlabel("time (ms)")
pylab.ylabel("synaptic conductance (nS)")
pylab.legend(("g_exc", "g_inh"))
