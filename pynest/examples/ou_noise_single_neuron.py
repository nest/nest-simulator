# -*- coding: utf-8 -*-
#
# ou_noise_single_neuron.py
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
One neuron with Ornstein-Uhlenbeck noise
----------------------------------------

This script simulates a neuron with input from the ``ou_noise_generator`` and
records the neuron's membrane potential.
"""

###############################################################################
# First, we import the modules needed to simulate, analyze, and plot the
# example. We also reset the kernel.

import matplotlib.pyplot as plt
import nest

nest.ResetKernel()
nest.resolution = 0.1


###############################################################################
# Second, we create the neuron, noise generator and voltmeter.

neuron = nest.Create("iaf_psc_alpha")
ou_noise = nest.Create("ou_noise_generator")
voltmeter = nest.Create("voltmeter")

###############################################################################
# Third, we configure the OU-noise generator. The parameters are:
# ``mean`` and ``std`` in `pA`, ``tau`` (correlation time) and ``dt`` in `ms`.
# The update interval ``dt`` must be a multiple of the simulation resolution.

ou_noise.set(
    {
        "mean": 250.0,  # pA
        "std": 50.0,  # pA
        "tau": 50.0,  # ms
        "dt": nest.resolution,  # ms
    }
)

###############################################################################
# Fourth, we connect the generator to the neuron and the voltmeter to the
# neuron to record the membrane potential.

nest.Connect(ou_noise, neuron)
nest.Connect(voltmeter, neuron)

###############################################################################
# Now we simulate the network for 1000 ms.

nest.Simulate(1000.0)

###############################################################################
# Finally, we plot the neuron's membrane potential as a function of time.

nest.voltage_trace.from_device(voltmeter)
plt.show()
