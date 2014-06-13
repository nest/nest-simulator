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
This example shows a brute-force way of specifying connections
between NEST Topology layers using Connection Set Algebra.

We are working on better ways to do this.
"""

import nest
import nest.topology as topo
import matplotlib.pyplot as plt

try:
    import csa
    haveCSA = True
except ImportError:
    haveCSA = False


def geometryFunction(topologyLayer):
    """
    This factory returns a CSA-style geometry function for the given layer.

    The function returned will return for each CSA-index the position in
    space of the given neuron as a 2- or 3-element list.

    Note: This function stores a copy of the neuron positions internally,
          entailing memory overhead.
    """

    positions = topo.GetPosition(nest.GetLeaves(topologyLayer)[0])

    def geometry_function(idx):
        "Return position of neuron with given CSA-index."
        return positions[idx]

    return geometry_function


def csa_topology_example():

    # layers have 20x20 neurons and extent 1 x 1
    pop1 = topo.CreateLayer({'elements': 'iaf_neuron',
                             'rows': 20, 'columns': 20})
    pop2 = topo.CreateLayer({'elements': 'iaf_neuron',
                             'rows': 20, 'columns': 20})

    # create CSA-style geometry functions and metric
    g1 = geometryFunction(pop1)
    g2 = geometryFunction(pop2)
    d = csa.euclidMetric2d(g1, g2)

    # Gaussian connectivity profile, sigma = 0.2, cutoff at 0.5
    cs = csa.cset(csa.random * (csa.gaussian(0.2, 0.5) * d), 10000.0, 1.0)

    # create connections
    nest.CGConnect(pop1, pop2, cs, {"weight": 0, "delay": 1})

    # show targets of center neuron
    topo.PlotTargets(topo.FindCenterElement(pop1), pop2)


if __name__ == "__main__":

    if haveCSA:
        nest.ResetKernel()
        csa_topology_example()
    else:
        print("This example requires CSA to be installed in order to run!")
