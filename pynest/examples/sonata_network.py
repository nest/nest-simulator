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

import nest
import matplotlib.pyplot as plt


nest.ResetKernel()

n_procs = 1
n_threads = 1
nest.set(total_num_virtual_procs=n_procs * n_threads,
         print_time=True)

# Specify path to the SONATA .json configuration file
net_config = "./300_pointneurons/circuit_config.json"
sim_config = "./300_pointneurons/simulation_config.json"
# Instantiate SonataNetwork
sonata_net = nest.SonataNetwork(net_config, sim_config=sim_config)

# Create and connect nodes
node_collections = sonata_net.BuildNetwork(chunk_size=2**20)

print(f"number of connections: {nest.GetKernelStatus('num_connections'):,}")
print(f"number of neurons: {nest.GetKernelStatus('network_size'):,}")

# Connect spike recorder to a population
s_rec = nest.Create("spike_recorder")
pop_name = "internal"
nest.Connect(node_collections[pop_name], s_rec)

# Simulate the network
sonata_net.Simulate()

nest.raster_plot.from_device(s_rec)
plt.show()
