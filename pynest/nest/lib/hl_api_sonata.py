# -*- coding: utf-8 -*-
#
# hl_api_sonata.py
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
...
"""

import h5py   # TODO this need to be a try except thing
import json
import pandas as pd
import numpy as np
import warnings
import csv
from string import Template

from .hl_api_info import GetStatus
from .hl_api_types import NodeCollection
from .hl_api_nodes import Create
from .hl_api_models import GetDefaults
from .hl_api_connections import GetConnections
from .hl_api_simulation import GetKernelStatus


__all__ = [
    'SonataConnector'
]

class SonataConnector(object):
    """
    """

    def __init__(self, base_path, config, sim_config=None):
        self.base_path = base_path
        self.config = {}
        self.node_collections = {}
        self.edge_types = []

        self.convert_config_(config)
        if sim_config:
            self.convert_config_(sim_config)

    def convert_config_(self, json_config):
        with open(self.base_path + json_config) as fp:
            config = json.load(fp)

        subs = {}
        for dir, path in config["manifest"].items():
            if dir.startswith('$'):
                dir = dir[1:]
            subs[dir] = self.base_path + path
        for dir, path in subs.items():
            if '$BASE_DIR' in path:
                subs[dir] = path.replace('$BASE_DIR', '.')

        # Traverse config and do substitutions
        def do_substitutions(obj):
            if isinstance(obj, dict):
                return {k: do_substitutions(v) for k, v in obj.items()}
            elif isinstance(obj, list):
                return [do_substitutions(elem) for elem in obj]
            else:
                if isinstance(obj, str) and obj.startswith('$'):
                    for dir, path in subs.items():
                        obj = obj.replace(dir, path)
                    return obj[1:]
                else:
                    return obj
        self.config.update(do_substitutions(config))
        print(self.config)
        print("")
        
    def is_unique_(self, col):
        numpy_array = col.to_numpy()
        return (numpy_array[0] == numpy_array).all()
    
    def save_nodes_to_file(self):
        with open('node_information.txt', 'w') as file:
            for name, nc in self.node_collections.items():
                file.write(name + ': ')
                file.write(str(nc))
                file.write('\n')
                params = nc.get()
                file.write(str(params))
                file.write('\n\n')

    def check_node_params(self):
        for nodes in self.config['networks']['nodes']:
            node_types_file = nodes['node_types_file']
            node_types = pd.read_csv(node_types_file, sep='\s+')

            with h5py.File(nodes["nodes_file"], 'r') as nodes_file:
                population_name = list(nodes_file['nodes'].keys())[0]
                population = nodes_file['nodes'][population_name]

                nc = self.node_collections[population_name]

                node_type_ids = population['node_type_id']
                if 'dynamics_params' in node_types.keys():
                    for count, node_type in enumerate(node_type_ids):
                        dynamics = {}
                        json_file = node_types.dynamics_params[node_types['node_type_id'] == node_type].iloc[0]

                        with open(self.config['components']['point_neuron_models_dir'] + '/' + json_file) as dynamics_file:
                            dynamics.update(json.load(dynamics_file))
                        node_params = nc[count].get(dynamics.keys())
                        for k, v in node_params.items():
                            if isinstance(v, tuple):
                                node_params[k] = list(v)
                        if node_params != dynamics:
                            raise ValueError(f'NodeCollection {nc} with name {population_name} has the wrong parameters! Expected \n{dynamics}, \n got \n{node_params}')

            print(f'All node parameters for nc {population_name} ok!')

    def create_nodes(self):
        # Create nodes

        for nodes in self.config['networks']['nodes']:
            node_types_file = nodes['node_types_file']
            node_types = pd.read_csv(node_types_file, sep='\s+')

            one_model = node_types['model_type'].all() == 'virtual' or self.is_unique_(node_types['model_template'])

            if one_model:
                model_type = node_types.model_type.iloc[0]
                if model_type not in ['point_neuron', 'point_process', 'virtual']:
                    warnings.warn(f'model of type {model_type} is not a NEST model, it will not be used.')

                have_dynamics = 'dynamics_params' in node_types.keys()

                node_type_map = {}
                for ind in node_types.index:
                    dynamics = {}
                    if have_dynamics:
                        with open(self.config['components']['point_neuron_models_dir'] + '/' + node_types['dynamics_params'][ind]) as dynamics_file:
                            dynamics.update(json.load(dynamics_file))
                    node_type_map[node_types['node_type_id'][ind]] = dynamics

                with h5py.File(nodes["nodes_file"], 'r') as nodes_file:
                    population_name = list(nodes_file['nodes'].keys())[0]  # What if we have more than one?? can iterate over .items()

                    population = nodes_file['nodes'][population_name]
                    num_elements =  population['node_id'].size

                    if model_type == 'virtual':
                        model = 'spike_generator'
                        # Spiketrains are given in h5 files
                        for input_name, input_dict in self.config['inputs'].items():
                            node_set = input_dict['node_set']
                            if node_set == population_name:
                                with h5py.File(input_dict['input_file'], 'r') as spiking_file:
                                    spikes = spiking_file['spikes']['timestamps']
                                    node_ids = spiking_file['spikes']['gids']
                                    timestamps = {i: [] for i in range(num_elements)}
                                    dt = self.config['run']['dt']
                                    for indx, node_id in enumerate(node_ids):
                                        rate = round(spikes[indx] / dt) * dt
                                        if rate == 0.:
                                            continue
                                        timestamps[node_id].append(rate)#np.round(spikes[indx], 1))

                                    #nodes = NodeCollection([])
                                    #for node_id in np.unique(node_ids):
                                    #    nc = Create(model, params={'spike_times': timestamps[node_id]})
                                        #if dynamics:
                                        #   nc.set(**dynamics)  #this needs to be re-added.
                                    #    nodes += nc
                                nodes = Create(model, num_elements)
                                nodes.set([{'spike_times': timestamps[i]} for i in range(len(nodes))])
                    else:
                        model = node_types.model_template.iloc[0].replace('nest:','')
                        print(model)
                        nodes = Create(model, num_elements)
                        
                        node_type_ids = population['node_type_id']
                        for node_type in np.unique(node_type_ids):  # might have to iterate over node_group_id as well
                            indx = np.where(node_type_ids[:] == node_type)[0]
                            nodes[indx].set(node_type_map[node_type])
    
                    self.node_collections[population_name] = nodes
            else:
                raise NotImplemented("TODO: More than one NEST model currently not implemented")

    def create_edge_dict(self):

        for edges in self.config['networks']['edges']:
            edge_dict = {}
            edge_dict['edges_file'] = edges['edges_file']
            edge_types_file = edges['edge_types_file']  # Synapses

            with open(edge_types_file, 'r') as csv_file:
                reader = csv.DictReader(csv_file, delimiter=' ', quotechar='"')
                rows = list(reader)
                skip_params = ['edge_type_id', 'target_query', 'source_query', 'dynamics_params']
                edge_params = {}

                for d in rows:
                    d['synapse_model'] = d.pop('model_template')
                    setable_params = GetDefaults(d['synapse_model'])
                    if 'syn_weight' in d:
                        d['weight'] = d.pop('syn_weight')

                    synapse_dict = {key: d[key] for key in setable_params if key in d}

                    with open(self.config['components']['synaptic_models_dir'] + '/' + d['dynamics_params']) as dynamics_file:
                        dynamics = json.load(dynamics_file)
                    synapse_dict.update(dynamics)

                    edge_params[d['edge_type_id']] = synapse_dict
            edge_dict['edge_synapse'] = edge_params

            self.edge_types.append(edge_dict)

    def dump_connections(self, outname):
        net_size = GetKernelStatus('network_size')
        print(f'network size: {net_size}')
        if True:
            with open(outname, 'w') as connections_file:
                net_size = GetKernelStatus('network_size')
                print(f'network size: {net_size}')
                counter = 0
                ncs = [self.node_collections['v1'],]#list(self.node_collections.values())
                for nc in ncs:
                    prev = 0
                    step = 10000
                    for i in range(step, len(nc), step):
                        conns = GetConnections(source=nc[prev:i])
                        if len(conns) == 0:
                            print("no connections!")
                            conns = GetConnections(target=nc[prev:i])
                        for l in GetStatus(conns):  # for comparison with bmtk built on NEST 2.20
                            s = f'{l["source"]} {l["target"]} {l["delay"]} {l["weight"]} {l["synapse_model"]} {l["receptor"]}\n'
                            connections_file.write(s)
                        #connections_file.write(str(GetStatus(conns)))  # for comparison with bmtk built on NEST 2.20
                        counter += len(conns)
                        if counter >= 40000:
                            break
                        prev = i
                    if prev < len(nc) and counter < 400000:
                        conns = GetConnections(source=nc[prev:])
                        if len(conns) == 0:
                            print("no connections!")
                            conns = GetConnections(target=nc[prev:])
                        for l in GetStatus(conns):  # for comparison with bmtk built on NEST 2.20
                            s = f'{l["source"]} {l["target"]} {l["delay"]} {l["weight"]} {l["synapse_model"]} {l["receptor"]}\n'
                            connections_file.write(s)
                        #connections_file.write(str(GetStatus(conns)))  # for comparison with bmtk built on NEST 2.20
                        counter += len(conns)
                print(f'number of counted connections: {counter}')
        print(f'num connections: {GetKernelStatus("num_connections")}')
