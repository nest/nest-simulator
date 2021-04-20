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

See Also
~~~~~~~~

:doc:`twoneurons`

"""

###############################################################################
# First, we import all necessary modules for simulation, analysis and
# plotting. Additionally, we set the verbosity to suppress info
# messages and reset the kernel.
# Resetting the kernel allows you to execute the script several
# times in a Python shell without interferences from previous NEST
# simulations. Thus, without resetting the kernel the network status
# including connections between nodes, status of neurons, devices and
# intrinsic time clocks, is kept and influences the next simulations.

import nest
import nest.voltage_trace
import matplotlib.pyplot as plt

nest.set_verbosity("M_WARNING")
nest.ResetKernel()

###############################################################################
# Second, the nodes (neurons and devices) are created using ``Create``.
# We store the returned handles in variables for later reference.
# The ``Create`` function also allow you to create multiple nodes
# e.g. ``nest.Create('iaf_psc_alpha',5)``
# Also default parameters of the model can be configured using ``Create``
# by including a list of parameter dictionaries
# e.g. `nest.Create("iaf_psc_alpha", params=[{'I_e':376.0}])`.
# In this example we will configure these parameters in an additional
# step, which is explained in the third section.

neuron = nest.Create("iaf_psc_alpha")
voltmeter = nest.Create("voltmeter")

###############################################################################
# Third, we set the external current of the neuron.

neuron.I_e = 376.0

###############################################################################
# Fourth, the neuron is connected to the voltmeter. The command
# ``Connect`` has different variants. Plain ``Connect`` just takes the
# handles of pre- and postsynaptic nodes and uses the default values
# for weight and delay. Note that the connection direction for the voltmeter is
# reversed compared to the spike recorder, because it observes the
# neuron instead of receiving events from it. Thus, ``Connect``
# reflects the direction of signal flow in the simulation kernel
# rather than the physical process of inserting an electrode into the
# neuron. The latter semantics is presently not available in NEST.

nest.Connect(voltmeter, neuron)

###############################################################################
# Now we simulate the network using ``Simulate``, which takes the
# desired simulation time in milliseconds.

nest.Simulate(1000.0)

###############################################################################
# Finally, we plot the neuron's membrane potential as a function of
# time and display the plot using pyplot.

nest.voltage_trace.from_device(voltmeter)
plt.show()
