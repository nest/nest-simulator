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

example = '300_pointneurons'
#example = 'GLIF'
plot = True

if example == '300_pointneurons':
    base_path = '/home/stine/Work/sonata/examples/300_pointneurons/'
    config = 'circuit_config.json'
    sim_config = 'simulation_config.json'
    population_to_plot = 'internal'
elif example == 'GLIF':
    base_path = '/home/stine/Work/sonata/examples/GLIF_NEST/'
    config = 'config.json'
    sim_config = None
    population_to_plot = 'v1'


sonata_connector = nest.SonataConnector(base_path, config, sim_config)

if not sonata_connector.config['target_simulator'] == 'NEST':
    raise NotImplementedError('Only `target_simulator` of type NEST is supported.')

nest.set(resolution=sonata_connector.config['run']['dt'])#, tics_per_ms=sonata_connector.config['run']['nsteps_block'])

mem_ini = memory_thisjob()

# Create nodes
sonata_connector.create_nodes()
#sonata_connector.check_node_params()
mem_create = memory_thisjob()

sonata_connector.create_edge_dict()


sonata_dynamics = {'nodes': sonata_connector.node_collections, 'edges': sonata_connector.edge_types}
print(sonata_connector.node_collections)

sonata_connector.dump_connections('check_connections.h5')

#print()
#print('sonata_dynamics', sonata_dynamics)
print()

connect_start_time = time.time()

nest.Connect(sonata_dynamics=sonata_dynamics)
print("done connecting")

connect_end_time = time.time() - connect_start_time
mem_connect = memory_thisjob()

# conns = nest.GetConnections()
# print(conns)
# print("")
print("number of connections: ", nest.GetKernelStatus('num_connections'))
# #print("num_connections with alpha: ", len(conns.alpha))

if plot:
    s_rec = nest.Create('spike_recorder')
    nest.Connect(sonata_connector.node_collections[population_to_plot], s_rec)

print('simulating')

simtime = 0
if 'tstop' in sonata_connector.config['run']:
    simtime = sonata_connector.config['run']['tstop']
else:
    #simtime = 1000.
    simtime = sonata_connector.config['run']['duration']

nest.Simulate(simtime)

end_time = time.time() - start_time

print(f"\nconnection took: {connect_end_time} s")
print(f"all took: {end_time} s")
print(f'initial memory: {mem_ini}')
print(f'memory create: {mem_create}')
print(f'memory connect: {mem_connect}')

if plot:
    print(s_rec.events)
    nest.raster_plot.from_device(s_rec)
    plt.show()











