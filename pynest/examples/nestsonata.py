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
plot = True
pre_sim_time = 10

if example == '300_pointneurons':
    base_path = '/home/stine/Work/sonata/examples/300_pointneurons/'
    config = 'circuit_config.json'
    sim_config = 'simulation_config.json'
    population_to_plot = 'internal'
elif example == 'GLIF':
    base_path = '/home/stine/Work/sonata/examples/glif_nest_220/'
    config = 'config.json'
    sim_config = None
    population_to_plot = 'v1'


sonata_connector = nest.SonataConnector(base_path, config, sim_config)

if not sonata_connector.config['target_simulator'] == 'NEST':
    raise NotImplementedError('Only `target_simulator` of type NEST is supported.')

nest.set(resolution=sonata_connector.config['run']['dt'], overwrite_files=True, total_num_virtual_procs=4)#, tics_per_ms=sonata_connector.config['run']['nsteps_block'])

mem_ini = memory_thisjob()
start_time_create = time.time()

# Create nodes
sonata_connector.Create()

end_time_create = time.time() - start_time_create
mem_create = memory_thisjob()

# Create edge dict
start_time_dict = time.time()
sonata_connector.create_edge_dict()

end_time_dict = time.time() - start_time_dict
print(sonata_connector.node_collections)
print()

start_time_connect = time.time()

# Connect
sonata_connector.Connect()
print("done connecting")

end_time_connect = time.time() - start_time_connect
mem_connect = memory_thisjob()

print("number of connections: ", nest.GetKernelStatus('num_connections'))
print("number of neurons: ", nest.GetKernelStatus('network_size'))


# Simulate
start_time_presim = time.time()
#nest.Simulate(pre_sim_time)
end_time_presim = time.time() - start_time_presim

if plot:
    #mm = nest.Create('multimeter')
    #mm.record_from = ['V_m']

    s_rec = nest.Create('spike_recorder')
    s_rec.record_to = 'ascii'
    nest.Connect(sonata_connector.node_collections[population_to_plot], s_rec)

print('simulating')


start_time_sim = time.time()

simtime = 0
if 'tstop' in sonata_connector.config['run']:
    simtime = sonata_connector.config['run']['tstop']
else:
    simtime = sonata_connector.config['run']['duration']

if plot:
    nest.Simulate(simtime)

end_time_sim = time.time() - start_time_sim

end_time = time.time() - start_time

print(f"\ncreation took: {end_time_create} s")
print(f"edge dict creation took: {end_time_dict} s")
print(f"connection took: {end_time_connect} s")
print(f"Pre-simulation (10 ms) took: {end_time_presim} s")
print(f"simulation took: {end_time_sim} s")
print(f"all took: {end_time} s")
print(f'initial memory: {mem_ini}')
print(f'memory create: {mem_create}')
print(f'memory connect: {mem_connect}\n')
print(f'number of spikes: {nest.GetKernelStatus("local_spike_counter")}')

if plot:
    nest.raster_plot.from_device(s_rec)
    plt.show()

net_size = nest.GetKernelStatus('network_size')
print(nest.NodeCollection([net_size - 1]).get())
print(nest.NodeCollection([net_size]).get())
print("number of neurons: ", nest.GetKernelStatus('network_size'))
print(nest.NodeCollection(list(range(1, net_size+1))))




