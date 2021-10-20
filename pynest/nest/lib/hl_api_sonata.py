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

from .hl_api_types import NodeCollection
from .hl_api_nodes import Create


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
    
    def create_nodes(self):
        # Create nodes

        for nodes in self.config['networks']['nodes']:
            node_types_file = nodes['node_types_file']
            node_types = pd.read_csv(node_types_file, sep='\s+')

            nodes_file = h5py.File(nodes["nodes_file"], 'r')
            population_name = list(nodes_file['nodes'].keys())[0]  # What if we have more than one?? can iterate over .items()

            population = nodes_file['nodes'][population_name]

            node_type_ids = population['node_type_id']
            nodes = NodeCollection([])
            for node_type in np.unique(node_type_ids):  # might have to iterate over node_group_id as well
                ntw = np.where(node_type_ids[:] == node_type)[0]

                n = len(ntw) - 1
                if sum(np.diff(ntw) == 1) == n:  # We check if node_types are continous
                    node_type_df = node_types[node_types.node_type_id == node_type]
                    if node_type_df['model_type'].iloc[0] not in ['point_neuron', 'point_process', 'virtual']:
                        model_type = node_type_df["model_type"].iloc[0]
                        warnings.warn(f'model of type {model_type} is not a NEST model, it will not be used.')

                    dynamics = {}
                    if 'dynamics_params' in node_type_df.keys():
                        model_dynamics = node_type_df.dynamics_params.iloc[0]
                        with open(self.config['components']['point_neuron_models_dir'] + '/' + model_dynamics) as dynamics_file:
                            dynamics.update(json.load(dynamics_file))

                    if node_type_df['model_type'].iloc[0] == 'virtual':
                        model = 'spike_generator'
                        # Spiketrains are given in h5 files
                        for input_name, input_dict in self.config['inputs'].items():
                            node_set = input_dict['node_set']
                            if node_set == population_name:
                                spiking_file = h5py.File(input_dict['input_file'])
                                spikes = spiking_file['spikes']['timestamps']
                                node_ids = spiking_file['spikes']['gids']
                                timestamps = {}
                                for indx, node_id in enumerate(node_ids):
                                    if node_id in timestamps:
                                        timestamps[node_id].append(np.round(spikes[indx], 2))
                                    else:
                                        timestamps[node_id] = [np.round(spikes[indx], 2)]
                                for node_id in np.unique(node_ids):
                                    nc = Create(model, params={'spike_times': timestamps[node_id]})
                                    if dynamics:
                                        nc.set(**dynamics)
                                    nodes += nc
                    else:
                        model = node_type_df.model_template.iloc[0].replace('nest:','')
                        nodes += Create(model, n+1, params=dynamics)
                        

            self.node_collections[population_name] = nodes

    def create_edge_dict(self):

        for edges in self.config['networks']['edges']:
            edge_dict = {}
            edge_dict['edges_file'] = edges['edges_file']
            edge_types_file = edges['edge_types_file']  # Synapses

            edge_file = h5py.File(edges["edges_file"], 'r')
            file_name = list(edge_file['edges'].keys())[0]  # What if we have more than one?? can iterate over .items()
            source = edge_file['edges'][file_name]['source_node_id'].attrs['node_population'].decode('UTF-8')

            with open(edge_types_file, 'r') as csv_file:
                reader = csv.DictReader(csv_file, delimiter=' ', quotechar='"')
                rows = list(reader)
                skip_params = ['edge_type_id', 'target_query', 'source_query', 'dynamics_params']
                edge_params = {}

                for d in rows:
                    synapse_dict = {key: d[key] for key in d if key not in skip_params}
                    synapse_dict['synapse_model'] = synapse_dict.pop('model_template')

                    with open(self.config['components']['synaptic_models_dir'] + '/' + d['dynamics_params']) as dynamics_file:
                        dynamics = json.load(dynamics_file)
                    synapse_dict.update(dynamics)

                    edge_params[d['edge_type_id']] = synapse_dict
            edge_dict['edge_synapse'] = edge_params

            self.edge_types.append(edge_dict)

