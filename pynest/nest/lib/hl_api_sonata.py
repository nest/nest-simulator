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
Class for creating networks from SONATA files
"""

import csv
import h5py   # TODO this need to be a try except thing
import json
import numpy as np
import pandas as pd
import warnings

from .hl_api_connections import GetConnections
from .hl_api_info import GetStatus
from .hl_api_models import GetDefaults
from .hl_api_nodes import Create
from .hl_api_simulation import GetKernelStatus
from .hl_api_types import NodeCollection


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
        """Convert SONATA config files to dictionary containing absolute paths and simulation parameters."""

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

    def is_unique_(self, col):
        """Check if all values in column are unique."""
        numpy_array = col.to_numpy()
        return (numpy_array[0] == numpy_array).all()

    def create_nodes(self):
        """Create nodes from SONATA files"""

        for nodes in self.config['networks']['nodes']:
            node_types_file = nodes['node_types_file']
            node_types = pd.read_csv(node_types_file, sep='\s+')

            one_model = (node_types['model_type'] == 'virtual').all() or self.is_unique_(node_types['model_template'])

            if one_model:
                model_type = node_types.model_type.iloc[0]
                if model_type not in ['point_neuron', 'point_process', 'virtual']:
                    warnings.warn(f'model of type {model_type} is not a NEST model, it will not be used.')

                # Extract node parameters
                have_dynamics = 'dynamics_params' in node_types.keys()

                node_type_map = {}
                for ind in node_types.index:
                    dynamics = {}
                    if have_dynamics:
                        with open(self.config['components']['point_neuron_models_dir'] + '/' +
                                  node_types['dynamics_params'][ind]) as dynamics_file:
                            dynamics.update(json.load(dynamics_file))
                    node_type_map[node_types['node_type_id'][ind]] = dynamics

                # Open sonata node files and create nodes
                with h5py.File(nodes["nodes_file"], 'r') as nodes_file:
                    # Iterate populations in nodes_file, there is usually only one population per file
                    for population_name in nodes_file['nodes']:

                        population = nodes_file['nodes'][population_name]
                        num_elements =  population['node_id'].size

                        if model_type == 'virtual':
                            model = 'spike_generator'
                            # First need to iterate to the current spike population dictionary in config file
                            for input_dict in self.config['inputs'].values():
                                node_set = input_dict['node_set']
                                if node_set == population_name:
                                    # Spiketrains are given in h5 files
                                    with h5py.File(input_dict['input_file'], 'r') as spiking_file:
                                        spikes = spiking_file['spikes']['timestamps']
                                        node_ids = spiking_file['spikes']['gids']
                                        timestamps = {i: [] for i in range(num_elements)}
                                        for indx, node_id in enumerate(node_ids):
                                            timestamps[node_id].append(spikes[indx])

                                    nodes = Create(model, num_elements)
                                    nodes.set([{'spike_times': timestamps[i], 'precise_times': True}
                                               for i in range(len(nodes))])
                        else:
                            # Create non-device nodes
                            model = node_types.model_template.iloc[0].replace('nest:','')
                            nodes = Create(model, num_elements)

                        # Set node parameters
                        if have_dynamics:
                            node_type_ids = population['node_type_id']
                            for node_type in np.unique(node_type_ids):  # might have to iterate over node_group_id as well
                                indx = np.where(node_type_ids[:] == node_type)[0]
                                nodes[indx].set(node_type_map[node_type])

                        self.node_collections[population_name] = nodes
            else:
                raise NotImplemented("More than one NEST model per csv file currently not implemented")

    def create_edge_dict(self):
        """Create edge dictionary used when connecting with SONATA files"""

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
                        d['weight'] = float(d.pop('syn_weight'))

                    synapse_dict = {key: d[key] for key in setable_params if key in d}
                    if 'delay' in synapse_dict:
                        synapse_dict['delay'] = float(synapse_dict['delay'])

                    with open(self.config['components']['synaptic_models_dir'] + '/' +
                              d['dynamics_params']) as dynamics_file:
                        dynamics = json.load(dynamics_file)
                    synapse_dict.update(dynamics)

                    edge_params[d['edge_type_id']] = synapse_dict
            edge_dict['edge_synapse'] = edge_params

            self.edge_types.append(edge_dict)
