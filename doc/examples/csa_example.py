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

"""Using CSA for connection setup
------------------------------------

This example sets up a simple network in NEST using the Connection Set
Algebra (CSA) instead of using the built-in connection routines.

Using the CSA requires NEST to be compiled with support for
libneurosim. For details, see [1]_.

See Also
~~~~~~~~~~

:doc:`csa_topology_example`

References
~~~~~~~~~~~~

.. [1] Djurfeldt M, Davison AP and Eppler JM (2014). Efficient generation of
       connectivity in neuronal networks from simulator-independent
       descriptions, Front. Neuroinform.
       http://dx.doi.org/10.3389/fninf.2014.00043

"""

###############################################################################
# First, we import all necessary modules for simulation and plotting.

import nest
from nest import voltage_trace
from nest import visualization

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
    sys.exit()

###############################################################################
# To set up the connectivity, We create a ``random`` connection set with a
# probability of 0.1 and two associated values (10000.0 and 1.0) used as
# weight and delay, respectively.

cs = csa.cset(csa.random(0.1), 10000.0, 1.0)

###############################################################################
# Using the ``Create`` command from PyNEST, we create the neurons of the pre-
# and postsynaptic populations, each of which containing 16 neurons.

pre = nest.Create("iaf_psc_alpha", 16)
post = nest.Create("iaf_psc_alpha", 16)

###############################################################################
# We can now connect the populations using the ``CGConnect`` function. It takes
# the IDs of pre- and postsynaptic neurons (``pre`` and ``post``),
# the connection set (``cs``) and a dictionary that maps the parameters
# weight and delay to positions in the value set associated with the
# connection set.

nest.CGConnect(pre, post, cs, {"weight": 0, "delay": 1})

###############################################################################
# To stimulate the network, we create a ``poisson_generator`` and set it up to
# fire with a rate of 100000 spikes per second. It is connected to the
# neurons of the pre-synaptic population.

pg = nest.Create("poisson_generator", params={"rate": 100000.0})
nest.Connect(pg, pre, "all_to_all")

###############################################################################
# To measure and record the membrane potentials of the neurons, we create a
# ``voltmeter`` and connect it to all post-synaptic nodes.

vm = nest.Create("voltmeter")
nest.Connect(vm, post, "all_to_all")

###############################################################################
# We save the whole connection graph of the network as a PNG image using the
# ``plot_network`` function of the ``visualization`` submodule of PyNEST.

allnodes = pg + pre + post + vm
visualization.plot_network(allnodes, "csa_example_graph.png")

###############################################################################
# Finally, we simulate the network for 50 ms. The voltage traces of the
# post-synaptic nodes are plotted.

nest.Simulate(50.0)
voltage_trace.from_device(vm)
