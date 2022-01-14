# -*- coding: utf-8 -*-
#
# nestsonata.py
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


import h5py   # TODO this need to be a try except thing
import json
import pandas as pd
import numpy as np
import warnings
import nest
import nest.raster_plot
import csv
import time

import matplotlib.pyplot as plt

def memory_thisjob():
    """Wrapper to obtain current memory usage"""
    nest.ll_api.sr('memory_thisjob')
    return nest.ll_api.spp()

start_time = time.time()

nest.ResetKernel()

#example = '300_pointneurons'
example = 'GLIF'
plot = False

if example == '300_pointneurons':
    base_path = '/home/stine/Work/sonata/examples/300_pointneurons/'
    config = 'circuit_config.json'
    sim_config = 'simulation_config.json'
    population_to_plot = 'internal'
elif example == 'GLIF':
    base_path = '/home/stine/Work/sonata/examples/GLIF_NEST/'
    config = 'config_small.json'
    sim_config = None
    population_to_plot = 'v1'


sonata_connector = nest.SonataConnector(base_path, config, sim_config)

if not sonata_connector.config['target_simulator'] == 'NEST':
    raise NotImplementedError('Only `target_simulator` of type NEST is supported.')

nest.set(resolution=sonata_connector.config['run']['dt'])#, tics_per_ms=sonata_connector.config['run']['nsteps_block'])

mem_ini = memory_thisjob()

# Create nodes
sonata_connector.create_nodes()
# sonata_connector.check_node_params()
mem_create = memory_thisjob()

sonata_connector.create_edge_dict()


sonata_dynamics = {'nodes': sonata_connector.node_collections, 'edges': sonata_connector.edge_types}
print(sonata_connector.node_collections)
print()

connect_start_time = time.time()

nest.Connect(sonata_dynamics=sonata_dynamics)
print("done connecting")

connect_end_time = time.time() - connect_start_time
mem_connect = memory_thisjob()

print("number of connections: ", nest.GetKernelStatus('num_connections'))

if plot:
    #mm = nest.Create('multimeter')
    #mm.record_from = ['V_m']

    s_rec = nest.Create('spike_recorder')
    s_rec.record_to = 'ascii'
    nest.Connect(sonata_connector.node_collections[population_to_plot], s_rec)

print('simulating')

simtime = 0
if 'tstop' in sonata_connector.config['run']:
    simtime = sonata_connector.config['run']['tstop']
else:
    simtime = sonata_connector.config['run']['duration']

if plot:
    nest.Simulate(simtime)

end_time = time.time() - start_time

print(f"\nconnection took: {connect_end_time} s")
print(f"all took: {end_time} s")
print(f'initial memory: {mem_ini}')
print(f'memory create: {mem_create}')
print(f'memory connect: {mem_connect}')

#sonata_connector.dump_connections('check_connections_glif')

#with open('NEST_neurons_GLIF.txt', 'w') as neurons_file:
#    for model in nest.GetStatus(nest.NodeCollection(list(range(1, nest.GetKernelStatus('network_size') + 1))), 'model'):
#        neurons_file.write(str(model) + "\n")
#with open('NEST_neuron_params_GLIF.txt', 'w') as neurons_file:
#    for dd in nest.GetStatus(nest.NodeCollection(list(range(1, nest.GetKernelStatus('network_size') + 1)))):
#        neurons_file.write(str(dd) + "\n")

if plot:
    print(s_rec.events)
    nest.raster_plot.from_device(s_rec)
    plt.show()

net_size = nest.GetKernelStatus('network_size')
print(nest.NodeCollection([net_size - 1]).get())
print(nest.NodeCollection([net_size]).get())
#print(s_rec.get())

# Check against h5 files
if False:
    with h5py.File('/home/stine/Work/sonata/examples/GLIF_NEST/./network/v1_v1_edges.h5', 'r') as edges_file:
        edge = edges_file['edges']['v1_to_v1']
        source_node_id = edge['target_node_id']
    
        with open('check_conns.txt', 'r') as check_conns_file:
            count = 0
            for line in check_conns_file:
                src = line[:]
                s_src = source_node_id[count]
                count += 1
                if int(src) - 1 != s_src:
                    raise ValueError(f'sonata target {s_src} do not match NEST target {src} for connection number {count}')











