# -*- coding: utf-8 -*-
#
# pynest_examples1_template.py
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

""" The Summary Line - Balanced Neuron Example
----------------------------------------------------------------

Example text for extended summary section.

This model implements X according to the Y model described by [1]_ and [2]_.

This script simulates a neuron driven by an excitatory and an inhibitory
population of neurons firing Poisson spike trains. The aim is to find a firing
rate for the inhibitory population that will make the neuron fire at the same
rate as the excitatory population.

Optimization is performed using the `bisection` method from Scipy,
simulating the network repeatedly.

See Also
---------
intrinisic_current_spiking
intrisic_current_subthreshold

Notes
------
Additional information can be included here regarding background theory etc.

The value of :math:`\omega` is X.
For the population and time-averaged from the spiking simulation:

.. math::

    X(e^{j\omega } ) = x(n)e^{ - j\omega n}

 * use the asterisk for bullet items
 * second item

References
------------
.. [1] Sander M., et al. 2011. Biology of the sauropod dinosaurs: The
       evolution of gigantism. Biological Reviews. 86(1):117-155.
       https://doi.org/10.111/j.1469-185x.2010.00137.x

.. [2] Francillon-Vieillot H, et al. 1990. Microstructure and mineralization of
       vertebrate skeletal tissues. In: Carter J ed. Skeletal Biomineralization
       Patterns, Processes and Evolutionary Trends. New York: Van Nostrand
       Reinhold, 471–530.

:Authors:
    D Adams, N Gaiman

KEYWORDS: scipy, poisson spike train, precise
"""

import nest  # begin with imports

###############################################################################
# The excitatory `poisson_generator` (`noise[0]`) is configured using
# `SetStatus`, which expects a list of node handles and a list of parameter
# dictionaries.

nest.SetStatus(noise, [{"rate": n_ex * r_ex}, {"rate": n_in * r_in}])

##############################################################################
# We plot the target neuron's membrane potential as a function of time

nest.voltage_trace.from_device(voltmeter)  # end with output
