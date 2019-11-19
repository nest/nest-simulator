# -*- coding: utf-8 -*-
#
# testiaf.py
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

"""IAF Neuron example
------------------

A DC current is injected into the neuron using a current generator
device. The membrane potential as well as the spiking activity are
recorded by corresponding devices.

It can be observed how the current charges the membrane, a spike
is emitted, the neuron becomes absolute refractory, and finally
starts to recover.

"""

###############################################################################
# First, we import all necessary modules for simulation and plotting

import nest
import pylab

###############################################################################
# Second the function ``build_network`` is defined to build the network and
# return the handles of the ``spike_detector`` and the ``voltmeter``. The
# function takes the simulation resolution as argument
#
# The function first resets the simulation kernel and sets the number of
# threads and the simulation resolution.  The ``iaf_psc_alpha`` neuron is
# created and the handle is stored in the variable `neuron`. The status of
# the neuron is changed so it receives an external current. Next a
# ``voltmeter`` and a ``spike_detector`` are created and their handles stored
# in the variables `vm` and `sd` respectively.
#
# The voltmeter and spike detector are then connected to the neuron. ``Connect``
# takes the device and neuron handles as input. The voltmeter is connected to the
# neuron and the neuron to the spike detector because the neuron sends spikes
# to the detector and the voltmeter 'observes' the neuron.


def build_network(dt):

    nest.ResetKernel()
    nest.SetKernelStatus({"local_num_threads": 1, "resolution": dt})

    neuron = nest.Create('iaf_psc_alpha')
    nest.SetStatus(neuron, "I_e", 376.0)

    vm = nest.Create('voltmeter')
    sd = nest.Create('spike_detector')

    nest.Connect(vm, neuron)
    nest.Connect(neuron, sd)

    return vm, sd


###############################################################################
# The neuron is simulated for three different resolutions and then the
# voltage trace is plotted

for dt in [0.1, 0.5, 1.0]:
    print("Running simulation with dt=%.2f" % dt)
    vm, sd = build_network(dt)

    nest.Simulate(1000.0)

###########################################################################
# The network is simulated using ``Simulate``, which takes the desired
# simulation time in milliseconds and advances the network state by this
# amount of time. During simulation, the ``spike_detector`` counts the
# spikes of the target neuron and the total number is read out at the
# end of the simulation period.
#
# The values of the voltage recorded by the voltmeter are read out and
# the values for the membrane potential are stored in potential and the
# corresponding times in the times array

    potentials = nest.GetStatus(vm, "events")[0]["V_m"]
    times = nest.GetStatus(vm, "events")[0]["times"]

###########################################################################
# Using the pylab library the voltage trace is plotted over time

    pylab.plot(times, potentials, label="dt=%.2f" % dt)
    print("  Number of spikes: {0}".format(nest.GetStatus(sd, "n_events")[0]))

###########################################################################
# Finally the axis are labelled and a legend is generated

    pylab.legend(loc=3)
    pylab.xlabel("time (ms)")
    pylab.ylabel("V_m (mV)")
