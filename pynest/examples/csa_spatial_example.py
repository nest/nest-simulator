# -*- coding: utf-8 -*-
#
# csa_spatial_example.py
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
Using CSA with spatial populations
----------------------------------

This example shows a brute-force way of specifying connections between
NEST populations with spatial data using Connection Set Algebra instead of
the built-in connection routines.

Using the CSA requires NEST to be compiled with support for
libneurosim [1]_.

See Also
~~~~~~~~~

:doc:`csa_example`

References
~~~~~~~~~~~~

.. [1] Djurfeldt M, Davison AP and Eppler JM (2014). Efficient generation of connectivity in neuronal networks
       from simulator-independent descriptions. Front. Neuroinform.
       http://dx.doi.org/10.3389/fninf.2014.00043
"""

###############################################################################
# First, we import all necessary modules.


import nest
import matplotlib.pyplot as plt

###############################################################################
# Next, we check for the availability of the CSA Python module. If it does
# not import, we exit with an error message.

try:
    import csa
    haveCSA = True
except ImportError:
    print("This example requires CSA to be installed in order to run.\n" +
          "Please make sure you compiled NEST using\n" +
          "  -Dwith-libneurosim=[OFF|ON|</path/to/libneurosim>]\n" +
          "and CSA and libneurosim are available.")
    import sys
    sys.exit(1)

###############################################################################
# We define a factory that returns a CSA-style geometry function for
# the given layer. The function returned will return for each CSA-index
# the position in space of the given neuron as a 2- or 3-element list.
#
# This function stores a copy of the neuron positions internally, entailing
# memory overhead.


def geometryFunction(population):

    positions = nest.GetPosition(population)

    def geometry_function(idx):
        return positions[idx]

    return geometry_function


nest.ResetKernel()

###############################################################################
# We create two spatial populations that have 20x20 neurons of type
# ``iaf_psc_alpha``.

pop1 = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid([20, 20]))
pop2 = nest.Create('iaf_psc_alpha', positions=nest.spatial.grid([20, 20]))

###############################################################################
# For each population, we create a CSA-style geometry function and a CSA metric
# based on them.

g1 = geometryFunction(pop1)
g2 = geometryFunction(pop2)
d = csa.euclidMetric2d(g1, g2)

###############################################################################
# The connection set `cs` describes a Gaussian connectivity profile with
# ``sigma = 0.2`` and cutoff at 0.5, and two values (10000.0 and 1.0) used as
# ``weight`` and ``delay``, respectively.

cs = csa.cset(csa.random * (csa.gaussian(0.2, 0.5) * d), 10000.0, 1.0)

###############################################################################
# We can now connect the populations using the ``CGConnect`` function. It
# takes the IDs of pre- and postsynaptic neurons (`pop` and `pop2`),
# the connection set (`cs`) and a dictionary that map the parameters
# weight and delay to positions in the value set associated with the
# connection set.

nest.CGConnect(pop1, pop2, cs, {"weight": 0, "delay": 1})

###############################################################################
# Finally, we use the ``PlotTargets`` function to show all targets in `pop2`
# starting at the center neuron of `pop1`.

cntr = nest.FindCenterElement(pop1)
nest.PlotTargets(cntr, pop2)
plt.show()
