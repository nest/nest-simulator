# -*- coding: utf-8 -*-
#
# csa_example.py
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

'''
Using CSA for connection setup
------------------------------

This example sets up a simple network in NEST using the Connection Set
Algebra (CSA). The network uses random connectivity with a connection 
probability of 10%.

See also Djurfeldt M, Davison AP and Eppler JM (2014) **Efficient
generation of connectivity in neuronal networks from
simulator-independent descriptions**, *Front. Neuroinform.*
http://dx.doi.org/10.3389/fninf.2014.00043
'''

'''
First, we import necessary modules for simulation and plotting.
'''

import sys
import nest
from nest import voltage_trace
from nest import visualization

'''
We now check for the availability of the CSA Python module and
exit with an error message, if we don't have it.
'''

try:
    import csa
    haveCSA = True
except ImportError:
    print("This example requires CSA to be installed in order to run!")
    sys.exit()

'''
We create a ``random`` connection set with a probability of 0.1
and two values (10000.0 and 1.0) used as weight and delay,
respectively.
'''

cs = csa.cset(csa.random(0.1), 10000.0, 1.0)

'''
The neurons of the pre- and postsynaptic populations are
created. Each of them contains 16 neurons.
'''

pre = nest.Create("iaf_neuron", 16)
post = nest.Create("iaf_neuron", 16)

'''
The populations are connected using the `CGConnect` function, which
takes the ids of pre- and postsynaptic neurons (``pre`` and ``post``),
the connection set (``cs``) and a dictionary that maps the parameters
weight and delay to positions in the value set associated with the
connection set.
'''

nest.CGConnect(pre, post, cs, {"weight": 0, "delay": 1})

'''
A `poisson_generator` is created and set to fire with a rate of
80000.0 spikes per second. It is connected to the neurons of the
pre-synaptic population with a weight of 1.2 and a delay of 1.0ms.
'''

pg = nest.Create("poisson_generator", params={"rate": 80000.0})
nest.DivergentConnect(pg, pre, 1.2, 1.0)

'''
A `voltmeter` is created and connected to all post-synaptic nodes.
'''

vm = nest.Create("voltmeter")
nest.DivergentConnect(vm, post)

'''
We save the whole connection graph as an image using the
`plot_network` function of the `visualization` submodule of PyNEST.

'''

allnodes = pg + pre + post + vm
visualization.plot_network(allnodes, "csa_example_graph.png")

'''
We simulate the network for 50 ms. The voltage traces of the
post-synaptic nodes are plotted.
'''

nest.Simulate(50.0)
voltage_trace.from_device(vm)
