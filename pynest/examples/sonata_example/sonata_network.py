# -*- coding: utf-8 -*-
#
# sonata_network.py
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
NEST SONATA network
-------------------

This script builds and simulates a network of point neurons represented by
the SONATA format [1]_. The network model consists of 300 internal nodes
(explicitly simulated) and 100 external nodes (only provide inputs to the
simulated system). The SONATA files can be found in the
`pynest/examples/sonata_example/300_pointneurons
<https://github.com/nest/nest-simulator/tree/master/pynest/examples/sonata_example/300_pointneurons>`_
directory.

See the :ref:`nest_sonata` for details on the NEST support of the SONATA format.

References
~~~~~~~~~~

.. [1] Dai K, Hernando J, Billeh YN, Gratiy SL, Planas J, et al. (2020).
       The SONATA data format for efficient description of large-scale network models.
       PLOS Computational Biology 16(2): e1007696. https://doi.org/10.1371/journal.pcbi.1007696

"""

###############################################################################
# Import all necessary packages for simulation, analysis and plotting.

from pathlib import Path

import matplotlib.pyplot as plt
import nest

nest.set_verbosity("M_ERROR")
nest.ResetKernel()

###############################################################################
# Specify the path to the SONATA .json configuration file(s).
# The `300_pointneurons` model has two configuration files:
# One circuit and one simulation configuration file.
# We locate the configuration files relative to this example script.

base_path = Path(__file__).resolve().parent
sonata_path = base_path / "300_pointneurons"
net_config = sonata_path / "circuit_config.json"
sim_config = sonata_path / "simulation_config.json"

###############################################################################
# SONATA networks are built and simulated through the :py:class:`.SonataNetwork`
# class. SONATA config files are passed to the class constructor. Passing a
# second config is optional and only relevant if the network and simulation
# configurations are specified separately.
#
# First, we instantiate the class.

sonata_net = nest.SonataNetwork(net_config, sim_config=sim_config)

###############################################################################
# Next, we build the network. The network nodes are created by the membership
# function :py:meth:`~.SonataNetwork.Create` and their connections by the
# membership function :py:meth:`~.SonataNetwork.Connect`. For convenience,
# we only need to call the membership function
# :py:meth:`~.SonataNetwork.BuildNetwork` which internally calls
# :py:meth:`~.SonataNetwork.Create` and :py:meth:`~.SonataNetwork.Connect`
#
# For large networks, the edges HDF5 files might not fit into memory in their
# entirety. In the NEST kernel, the edges HDF5 datasets are therefore
# read sequentially in blocks of contiguous hyperslabs. The hyperslab size is
# modifiable through the keyword argument ``hdf5_hyperslab_size``. This allows
# the user to control the balance between the number of read operations and
# memory overhead.
#
# :py:meth:`~.SonataNetwork.BuildNetwork()` returns a dictionary containing
# the created :py:class:`.NodeCollection`\s. The population names are the
# dictionary keys.

node_collections = sonata_net.BuildNetwork(hdf5_hyperslab_size=2**20)

###############################################################################
# We can now verify whether the built network has the expected number of
# nodes and connections.

print(f'Network built from SONATA specifications in directory "{sonata_path.name}"')
print(f"  Number of nodes      : {nest.network_size}")
print(f"  Number of connections: {nest.num_connections}")

###############################################################################
# NEST does not currently support SONATA Spike Train Reports or utilize other
# ``output`` components in the SONATA config. This means that recording devices
# must be created and connected manually.
#
# Here, we create a ``spike_recorder`` to record the spiking events of neurons.
# We wish to connect the spike recorder to the internal population and only
# record from a subset of the neurons in the population. We extract the
# internal population's :py:class:`.NodeCollection` from the ``node_collections`` dictionary
# by using the internal population's name. Then we slice the :py:class:`.NodeCollection`
# with a list specifying the node ids of the neurons we wish to record from.

s_rec = nest.Create("spike_recorder")
pop_name = "internal"
record_node_ids = [1, 80, 160, 240, 270]
nest.Connect(node_collections[pop_name][record_node_ids], s_rec)

###############################################################################
# Finally, we call the membership function :py:meth:`~.SonataNetwork.Simulate`
# to simulate the network. Note that the simulation time and resolution are
# expected to be provided in the SONATA config.

sonata_net.Simulate()

###############################################################################
# After the simulation has finished, we can obtain the data recorded by the
# spike recorder for further analysis.

nest.raster_plot.from_device(s_rec)
plt.show()
