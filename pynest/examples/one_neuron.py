# -*- coding: utf-8 -*-
#
# one_neuron.py
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
One neuron example
------------------

This script simulates a neuron driven by a constant external current
and records its membrane potential.

KEYWORDS: iaf_psc_alpha, voltmeter
"""

###############################################################################
# First, we import all necessary modules needed to simulate, analyze and
# plot our example. Additionally, we set the verbosity to only show warnings
# and reset the kernel.
# Resetting the kernel removes any nodes we may have created previously and
# resets the internal clock to zero. This allows us to execute the script
# several times in a Python shell without interference from previous NEST
# simulations.

import nest
import nest.voltage_trace

nest.set_verbosity("M_WARNING")
nest.ResetKernel()

###############################################################################
# Second, we create the nodes, in our case the neuron and measuring device,
# using the `Create()` function.
# We store the returned handles in variables for later reference.
# We can indicate the number of nodes that we want as well. For example,
# :code:`nest.Create('iaf_psc_alpha',5)`

neuron = nest.Create("iaf_psc_alpha")
voltmeter = nest.Create("voltmeter")

###############################################################################
# Third, the neuron and the voltmeter are configured using `SetStatus()`,
# which expects a list of node handles and a list of parameter dictionaries.
# In this example, we use `SetStatus()` to configure the constant current input
# to the neuron. We also want to record the global id of the observed nodes and
# set the `withgid` flag of the voltmeter to ``True``.
# Alternatively, we can add the parameters of the model as arguments to
# Create(), for example,
# :code:`nest.Create("iaf_psc_alpha", params=[{'I_e':376.0}])`
# or :code:`nest.Create("voltmeter", [{"withgid": True, "withtime": True}])`.

nest.SetStatus(neuron, "I_e", 376.0)
nest.SetStatus(voltmeter, [{"withgid": True}])

###############################################################################
# Fourth, we connect the neuron to the voltmeter. The order in which the
# arguments to `Connect()` are specified reflects the flow of events in the
# simulation kernel; in our case, the voltmeter periodically sends requests to
# the neuron to ask for its membrane potential at that point in time.

nest.Connect(voltmeter, neuron)

###############################################################################
# Now we simulate the network using `Simulate()`, which takes the
# desired simulation time in milliseconds.

nest.Simulate(1000.0)

###############################################################################
# Finally, we plot the neuron's membrane potential as a function of
# time.

nest.voltage_trace.from_device(voltmeter)
