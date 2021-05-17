# -*- coding: utf-8 -*-
#
# one_neuron_with_noise.py
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
One neuron with noise
---------------------

This script simulates a neuron with input from the ``poisson_generator``, and
records the neuron's membrane potential.

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
import matplotlib.pyplot as plt

nest.set_verbosity("M_WARNING")
nest.ResetKernel()

###############################################################################
# Second, the nodes (the neuron, poisson generator (two of them), and the
# voltmeter) are created using  the ``Create`` function.
# We store the returned handles in variables for later reference.

neuron = nest.Create("iaf_psc_alpha")
noise = nest.Create("poisson_generator", 2)
voltmeter = nest.Create("voltmeter")

###############################################################################
# Third, the Poisson generator is configured using ``SetStatus``, which expects
# a list of node handles and a list of parameter dictionaries. We set the
# Poisson generators to 8,000 Hz and 15,000 Hz, respectively. Note that we do
# not need to set parameters for the neuron and the voltmeter, since they have
# satisfactory defaults.

noise[0].rate = 80000.0
noise[1].rate = 15000.0

###############################################################################
# Fourth, the neuron is connected to the ``poisson_generator`` and to the
# ``voltmeter``. We also specify the synaptic weight and delay in this step.

nest.Connect(noise, neuron, syn_spec={'weight': [[1.2, -1.0]], 'delay': 1.0})
nest.Connect(voltmeter, neuron)

###############################################################################
# Now we simulate the network using ``Simulate``, which takes the
# desired simulation time in milliseconds.

nest.Simulate(1000.0)

###############################################################################
# Finally, we plot the neuron's membrane potential as a function of
# time.

nest.voltage_trace.from_device(voltmeter)
plt.show()
