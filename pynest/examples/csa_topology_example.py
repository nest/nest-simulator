# -*- coding: utf-8 -*-
#
# csa_topology_example.py
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
Using CSA with Topology layers
------------------------------

This example shows a brute-force way of specifying connections
between NEST Topology layers using Connection Set Algebra.
"""

"""
First, we import necessary modules.
"""

import nest
import nest.topology as topo

"""
We now check for the availability of the CSA Python module and
exit with an error message, if we don't have it.
"""

try:
    import csa
    haveCSA = True
except ImportError:
    print("This example requires CSA to be installed in order to run!")
    sys.exit()


"""
We define a factory that returns a CSA-style geometry function for
the given layer. The function returned will return for each CSA-index
the position in space of the given neuron as a 2- or 3-element list.

This function stores a copy of the neuron positions internally,
entailing memory overhead.
"""

def geometryFunction(topologyLayer):

    positions = topo.GetPosition(nest.GetLeaves(topologyLayer)[0])

    def geometry_function(idx):
        return positions[idx]
    
    return geometry_function


"""
We create two layers that have 20x20 neurons of type `iaf_neuron`.
"""

pop1 = topo.CreateLayer({'elements': 'iaf_neuron',
                         'rows': 20, 'columns': 20})
pop2 = topo.CreateLayer({'elements': 'iaf_neuron',
                         'rows': 20, 'columns': 20})

"""
For each layer, we create a CSA-style geometry function and a CSA
metric based on them.
"""

g1 = geometryFunction(pop1)
g2 = geometryFunction(pop2)
d = csa.euclidMetric2d(g1, g2)

"""
The connection set describes Gaussian connectivity profile with
sigma = 0.2 and cutoff at 0.5, and two values (10000.0 and 1.0) used
as weight and delay, respectively.
"""

cs = csa.cset(csa.random * (csa.gaussian(0.2, 0.5) * d), 10000.0, 1.0)

"""
The populations are connected using the `CGConnect` function, which
takes the ids of pre- and postsynaptic neurons (``pop1`` and ``pop2``),
the connection set (``cs``) and a dictionary that maps the parameters
weight and delay to positions in the value set associated with the
connection set.
"""

nest.CGConnect(pop1, pop2, cs, {"weight": 0, "delay": 1})

"""
We use the `PlotTargets` function to show all targets of the
center neuron from `pop1`.
"""

topo.PlotTargets(topo.FindCenterElement(pop1), pop2)
