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
import csv

from sonata.circuit import File

nest.ResetKernel()

base_path = '/home/stine/Work/sonata/examples/300_pointneurons/'

with open(base_path + '/circuit_config.json') as config_file:
    config = json.load(config_file)
    config_file.close()

NETWORK_DIR = base_path + config['manifest']['$NETWORK_DIR']
COMPONENT_DIR = base_path + config['manifest']['$COMPONENT_DIR']

if not config['target_simulator'] == 'NEST':
    raise NotImplementedError('Only `target_simulator` of type NEST is supported.')

# Substitute so we have full path (can be recursive)
for network_type, network_files in config['networks'].items():
    for file_dict in network_files:
        for key, value in file_dict.items():
            if '$NETWORK_DIR' in str(value):
                file_dict[key] = value.replace('$NETWORK_DIR', NETWORK_DIR)
for component_type, component_files in config['components'].items():
    if '$COMPONENT_DIR' in component_files:
        config['components'][component_type] = component_files.replace('$COMPONENT_DIR', COMPONENT_DIR)

print(config)
print("")

# Create nodes
node_collections = {}

for nodes in config['networks']['nodes']:
    node_types_file = nodes['node_types_file']
    node_types = pd.read_csv(node_types_file, sep='\s+')

    nodes_file = h5py.File(nodes["nodes_file"], 'r')
    population_name = list(nodes_file['nodes'].keys())[0]  # What if we have more than one?? can iterate over .items()

    population = nodes_file['nodes'][population_name]

    node_type_ids = population['node_type_id']
    nodes = nest.NodeCollection([])
    for node_type in np.unique(node_type_ids):  # might have to iterate over node_group_id as well
        ntw = np.where(node_type_ids[:] == node_type)[0]

        n = len(ntw) - 1
        if sum(np.diff(ntw) == 1) == n:  # We check if node_types are continous
            node_type_df = node_types[node_types.node_type_id == node_type]
            if node_type_df['model_type'].iloc[0] == 'point_process':  #can there be other names?
                model = node_type_df.model_template.iloc[0].replace('nest:','')
                model_name = node_type_df.model_name.iloc[0]
                model_dynamics = node_type_df.dynamics_params.iloc[0]

                with open(config['components']['point_neuron_models_dir'] + '/' + model_dynamics) as dymanics_file:
                    dynamics = json.load(dymanics_file)
                nodes += nest.Create(model, n+1, params=dynamics)
            else:
                model_type = node_type_df["model_type"].iloc[0]
                warnings.warn(f'model of type {model_type} is not a NEST model, it will not be used.')
    node_collections[population_name] = nodes

print(node_collections)

edge_types = {}
for edges in config['networks']['edges']:
    edge_types_file = edges['edge_types_file']
    
    edge_file = h5py.File(edges["edges_file"], 'r')
    file_name = list(edge_file['edges'].keys())[0]  # What if we have more than one?? can iterate over .items()
    print(file_name)
    source = edge_file['edges'][file_name]['source_node_id'].attrs['node_population'].decode('UTF-8')
    print(source)
    
    with open(edge_types_file, 'r') as csv_file:
        reader = csv.DictReader(csv_file, delimiter=' ', quotechar='"')
        rows = list(reader)
        edge_params = {d['edge_type_id']: {key: d[key] for key in d if key not in ['edge_type_id', 'target_query', 'source_query', ]} for d in rows}
        for type_id, type_d in edge_params.items():
            edge_params[type_id]['synapse_model'] = edge_params[type_id]['model_template']

            with open(config['components']['synaptic_models_dir'] + '/' + edge_params[type_id]['dynamics_params']) as dymanics_file:
                dynamics = json.load(dymanics_file) #fix
            edge_params.update(dynamics)
            edge_params[type_id].pop('model_template', None)
            edge_params[type_id].pop('dynamics_params', None)
    edge_types[source] = edge_params
    
    print(edge_types)

#push {'internal': nc, 'external': nc}
#push csv og dynamics
sonata_dynamics = {'nodes': node_collections, 'edges': edge_types}

print()
print("sonata_dynamics", sonata_dynamics)
print()

nest.Connect(sonata_config=config['networks'], sonata_dynamics=sonata_dynamics)

print(nest.GetKernelStatus('num_connections'))










