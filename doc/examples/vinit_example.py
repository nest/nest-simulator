# -*- coding: utf-8 -*-
#
# vinit_example.py
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

"""Initial membrane voltage
----------------------------

Plot several runs of the ``iaf_cond_exp_sfa_rr`` neuron without input for various
initial values of the membrane potential.

References
~~~~~~~~~~~~

.. [1] Dayan, P. and Abbott, L.F. (2001) Theoretical neuroscience,
       MIT Press, page 166

"""

###############################################################################
# First, the necessary modules for simulation and plotting are imported.

import nest
import numpy
import pylab

###############################################################################
# A loop runs over a range of initial membrane voltages.
#
# In the beginning of each iteration, the simulation kernel is put back to
# its initial state using `ResetKernel`.
#
# Next, a neuron is instantiated with ``Create``. The used neuron model
# ``iaf_cond_exp_sfa_rr`` is an implementation of a spiking neuron with
# integrate-and-fire dynamics, conductance-based synapses, an additional
# spike-frequency adaptation and relative refractory mechanisms as described
# in [1]_. Incoming spike events induce a post-synaptic change of
# conductance  modeled  by an  exponential  function. ``SetStatus`` allows to
# assign the initial membrane voltage of the current loop run to the neuron.
#
# ``Create`` is used once more to instantiate a ``voltmeter`` as recording device
# which is subsequently connected to the neuron with ``Connect``.
#
# Then, a simulation with a duration of 75 ms is started with ``Simulate``.
#
# When the simulation has finished, the recorded times and membrane voltages
# are read from the voltmeter via ``GetStatus`` where they can be accessed
# through the key ``events`` of the status dictionary.
#
# Finally, the time course of the membrane voltages is plotted for each of
# the different inital values.

for vinit in numpy.arange(-100, -50, 10, float):

    nest.ResetKernel()

    cbn = nest.Create("iaf_cond_exp_sfa_rr")

    nest.SetStatus(cbn, "V_m", vinit)

    voltmeter = nest.Create("voltmeter")
    nest.Connect(voltmeter, cbn)

    nest.Simulate(75.0)

    t = nest.GetStatus(voltmeter, "events")[0]["times"]
    v = nest.GetStatus(voltmeter, "events")[0]["V_m"]

    pylab.plot(t, v, label="initial V_m = %.2f mV" % vinit)

###############################################################################
# Set the legend and the labels for the plot outside of the loop.

pylab.legend(loc=4)
pylab.xlabel("time (ms)")
pylab.ylabel("V_m (mV)")
