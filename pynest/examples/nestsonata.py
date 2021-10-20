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

nest.ResetKernel()
nest.set(resolution=0.001, tics_per_ms=5000)

base_path = '/home/stine/Work/sonata/examples/300_pointneurons/'
#base_path = '/home/stine/Work/sonata/examples/GLIF_network/'


sonata_connector = nest.SonataConnector(base_path, 'circuit_config.json')

if not sonata_connector.config['target_simulator'] == 'NEST':
    raise NotImplementedError('Only `target_simulator` of type NEST is supported.')

# Create nodes
sonata_connector.create_nodes()
sonata_connector.create_edge_dict()


#
# with open(base_path + '/circuit_config.json') as config_file:
    # config = json.load(config_file)
    #
# NETWORK_DIR = base_path + config['manifest']['$NETWORK_DIR']
# COMPONENT_DIR = base_path + config['manifest']['$COMPONENT_DIR']
#
# if not config['target_simulator'] == 'NEST':
    # raise NotImplementedError('Only `target_simulator` of type NEST is supported.')
    #
# # Substitute so we have full path (can be recursive)
# for network_type, network_files in config['networks'].items():
    # for file_dict in network_files:
        # for key, value in file_dict.items():
            # if '$NETWORK_DIR' in str(value):
                # file_dict[key] = value.replace('$NETWORK_DIR', NETWORK_DIR)
# for component_type, component_files in config['components'].items():
    # if '$COMPONENT_DIR' in component_files:
        # config['components'][component_type] = component_files.replace('$COMPONENT_DIR', COMPONENT_DIR)
        #
# print(config)
# print("")
#
# # Create nodes
# node_collections = {}
#
# for nodes in config['networks']['nodes']:
    # node_types_file = nodes['node_types_file']
    # node_types = pd.read_csv(node_types_file, sep='\s+')
    #
    # nodes_file = h5py.File(nodes["nodes_file"], 'r')
    # population_name = list(nodes_file['nodes'].keys())[0]  # What if we have more than one?? can iterate over .items()
    #
    # population = nodes_file['nodes'][population_name]
    #
    # node_type_ids = population['node_type_id']
    # nodes = nest.NodeCollection([])
    # for node_type in np.unique(node_type_ids):  # might have to iterate over node_group_id as well
        # ntw = np.where(node_type_ids[:] == node_type)[0]
        #
        # n = len(ntw) - 1
        # if sum(np.diff(ntw) == 1) == n:  # We check if node_types are continous
            # node_type_df = node_types[node_types.node_type_id == node_type]
            # if node_type_df['model_type'].iloc[0] not in ['point_neuron', 'point_process', 'virtual']:
                # model_type = node_type_df["model_type"].iloc[0]
                # warnings.warn(f'model of type {model_type} is not a NEST model, it will not be used.')
                #
            # dynamics = {}
            # if 'dynamics' in node_type_df.keys():
                # model_dynamics = node_type_df.dynamics_params.iloc[0]
                #
                # with open(config['components']['point_neuron_models_dir'] + '/' + model_dynamics) as dynamics_file:
                    # dynamics = json.load(dynamics_file)
                    #
            # if node_type_df['model_type'].iloc[0] == 'virtual':
                # model = 'spike_generator'  # TODO: Need to add spike-data
                # spiking_file = h5py.File('/home/stine/Work/sonata/examples/300_pointneurons/inputs/external_spike_trains.h5')
                # spikes = spiking_file['spikes']['timestamps']
                # node_ids = spiking_file['spikes']['gids']
                # timestamps = {}
                # for indx, node_id in enumerate(node_ids):
                    # if node_id in timestamps:
                        # timestamps[node_id].append(np.round(spikes[indx], 3))
                    # else:
                        # timestamps[node_id] = [np.round(spikes[indx], 3)]
                # for node_id in np.unique(node_ids):
                    # nc = nest.Create(model, params={'spike_times': timestamps[node_id]})
                    # if dynamics:
                        # nc.set(**dynamics)
                    # nodes += nc
            # else:
                # model = node_type_df.model_template.iloc[0].replace('nest:','')
                # nodes += nest.Create(model, n+1, params=dynamics)
                #
    # node_collections[population_name] = nodes
    #
# edge_types = {}
# for edges in config['networks']['edges']:
    # edge_types_file = edges['edge_types_file']
    #
    # edge_file = h5py.File(edges["edges_file"], 'r')
    # file_name = list(edge_file['edges'].keys())[0]  # What if we have more than one?? can iterate over .items()
    # source = edge_file['edges'][file_name]['source_node_id'].attrs['node_population'].decode('UTF-8')
    #
    # with open(edge_types_file, 'r') as csv_file:
        # reader = csv.DictReader(csv_file, delimiter=' ', quotechar='"')
        # rows = list(reader)
        # skip_params = ['edge_type_id', 'target_query', 'source_query', 'dynamics_params']
        # edge_params = {}
        #
        # for d in rows:
            # synapse_dict = {key: d[key] for key in d if key not in skip_params}
            # synapse_dict['synapse_model'] = synapse_dict.pop('model_template')
            #
            # with open(config['components']['synaptic_models_dir'] + '/' + d['dynamics_params']) as dynamics_file:
                # dynamics = json.load(dynamics_file)
            # synapse_dict.update(dynamics)
            #
            # edge_params[d['edge_type_id']] = synapse_dict
            #
    # edge_types[source] = edge_params


sonata_dynamics = {'nodes': sonata_connector.node_collections, 'edges': sonata_connector.edge_types}

print()
print("sonata_dynamics", sonata_dynamics)
print()

start_time = time.time()

nest.Connect(sonata_config=sonata_connector.config['networks'], sonata_dynamics=sonata_dynamics)

end_time = time.time() - start_time

conns = nest.GetConnections(synapse_model = 'stdp_synapse')
print(conns)
print("")
print("number of connections: ", nest.GetKernelStatus('num_connections'))
print("num_connections with alpha: ", len(conns.alpha))

print(f"\nconnection took: {end_time} s")

s_rec = nest.Create('spike_recorder')

nest.Connect(sonata_connector.node_collections['internal'], s_rec)

print('simulating')

nest.Simulate(1500.)

print(s_rec.events)

nest.raster_plot.from_device(s_rec)
plt.show()











