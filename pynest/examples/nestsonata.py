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

import warnings
import nest
import nest.raster_plot
import time
from pprint import pprint

import matplotlib.pyplot as plt

NUM_PROCESSES = 4
NUM_THREADS = 2

SYS_LINUX_SCALE = 1e6
SYS_DARWIN_SCALE = 1e9

# Set scaling of memory print
#sys_scale = SYS_LINUX_SCALE
sys_scale = SYS_DARWIN_SCALE

# Set network config
#example = '300_pointneurons'
example = 'GLIF'
simulate = False
plot = False
verbose_print = True
create_sonata_network = False  # For testing of convenience function
pre_sim_time = 10


def memory_thisjob():
    """Wrapper to obtain current memory usage"""
    nest.ll_api.sr('memory_thisjob')
    return nest.ll_api.spp() / sys_scale


start_time = time.time()

nest.ResetKernel()

if example == '300_pointneurons':
    base_path = '/Users/nicolai/github/sonata/examples/300_pointneurons/'
    config = 'circuit_config.json'
    sim_config = 'simulation_config.json'
    population_to_plot = 'internal'
elif example == 'GLIF':
    base_path = '/Users/nicolai/github/nest_dev/nest_sonata/glif_nest_220/'
    config = 'config.json'
    sim_config = None
    population_to_plot = 'v1'


sonata_connector = nest.SonataConnector(base_path, config, sim_config)

if create_sonata_network:
    nest.set(total_num_virtual_procs=NUM_PROCESSES * NUM_THREADS)
    sonata_connector.CreateSonataNetwork(simulate=True)

    if plot:
        s_rec = nest.Create('spike_recorder')
        s_rec.record_to = 'ascii'
        nest.Connect(sonata_connector.node_collections[population_to_plot], s_rec)
else:
    if not sonata_connector.config['target_simulator'] == 'NEST':
        raise NotImplementedError('Only `target_simulator` of type NEST is supported.')

    nest.set(resolution=sonata_connector.config['run']['dt'],
             overwrite_files=True,
             total_num_virtual_procs=NUM_PROCESSES * NUM_THREADS)

    print("kernel number of processes:", nest.GetKernelStatus('num_processes'))
    print("kernel number of threads:", nest.GetKernelStatus('local_num_threads'), '\n')

    mem_ini = memory_thisjob()
    start_time_create = time.time()

    # Create nodes
    sonata_connector.Create()

    end_time_create = time.time() - start_time_create
    mem_create = memory_thisjob()

    #print(f"Sonata NC (rank {nest.Rank()}):", sonata_connector.node_collections)

    print(f"Sonata Local NC (rank {nest.Rank()}):", sonata_connector.local_node_collections)

    # Connect
    start_time_connect = time.time()

    sonata_connector.Connect()
    #print("done connecting")

    end_time_connect = time.time() - start_time_connect
    mem_connect = memory_thisjob()

    print(f"number of connections: {nest.GetKernelStatus('num_connections'):,}")
    print(f"number of neurons: {nest.GetKernelStatus('network_size'):,}")

    # Simulate
    if simulate:
        print("pre-simulation")
        start_time_presim = time.time()
        nest.Simulate(pre_sim_time)
        end_time_presim = time.time() - start_time_presim

    if plot:
        s_rec = nest.Create('spike_recorder')
        s_rec.record_to = 'memory'
        nest.Connect(sonata_connector.node_collections[population_to_plot], s_rec)

    if simulate:
        print('simulating')

        start_time_sim = time.time()

        simtime = 0
        if 'tstop' in sonata_connector.config['run']:
            simtime = sonata_connector.config['run']['tstop']
        else:
            simtime = sonata_connector.config['run']['duration']

        nest.Simulate(simtime)

        end_time_sim = time.time() - start_time_sim

    end_time = time.time() - start_time

    if verbose_print:
        print(f"creation took: {end_time_create} s")
        print(f"connection took: {end_time_connect} s")
        if simulate:
            print(f"Pre-simulation (10 ms) took: {end_time_presim} s")
            print(f"simulation took: {end_time_sim} s")
        print(f"all took: {end_time} s")
        print(f'initial memory: {mem_ini} GB')
        print(f'memory create: {mem_create} GB')
        print(f'memory connect: {mem_connect} GB')
        if simulate:
            print(f'number of spikes: {nest.GetKernelStatus("local_spike_counter"):,}')
        # pprint(nest.GetKernelStatus())
        # print(nest.GetConnections())


if plot:
    if not simulate:
        print("simulate must be True in order to plot")
    else:
        nest.raster_plot.from_device(s_rec)
        plt.show()


""" 
net_size = nest.GetKernelStatus('network_size')
print(nest.NodeCollection([net_size - 1]).get())
print(nest.NodeCollection([net_size]).get())
print("number of neurons: ", nest.GetKernelStatus('network_size'))
print(nest.NodeCollection(list(range(1, net_size+1))))
"""
