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

import os
import csv
import json
import numpy as np
import pandas as pd

from .. import pynestkernel as kernel
from ..ll_api import sps, sr, sli_func
from .hl_api_models import GetDefaults
from .hl_api_nodes import Create
from .hl_api_simulation import SetKernelStatus, Simulate
from .hl_api_types import NodeCollection

try:
    import h5py
    have_h5py = True
except ImportError:
    have_h5py = False

have_hdf5 = sli_func("statusdict/have_hdf5 ::")

__all__ = [
    'SonataConnector'
]


class SonataConnector(object):
    """ Class for creating networks from SONATA files.

    The nodes are created on Python level by iterating the SONATA node files and corresponding CSV parameter
    files and then calling :py:func:`Create()<nest.lib.hl_api_nodes.Create>`.
    The connections are created by first iterating the edge (synapse) CSV files on python level and creating
    synapse dictionaries. These are then sent to the NEST kernel along with the edge SONATA files to create
    the connections.

    Parameters
    ----------
    base_path: string
        absolute path to SONATA files (to the level of config.json)
    config: json file
        json file containing SONATA paths and parameters
    sim_config: json file (optional)
        json file containing simulation parameters
        Only needed if simulation parameters are not given in `config`.

    """

    def __init__(self, base_path, config, sim_config=None):
        self.base_path = base_path
        self.config = {}
        self.node_collections = {}
        self.edge_types = []

        if not have_hdf5:
            raise kernel.NESTError("Cannot use SonataConnector because NEST was compiled without HDF5 support")
        if not have_h5py:
            raise kernel.NESTError("Cannot use SonataConnector because h5py is not installed or could not be imported")

        self.convert_config_(config)
        if sim_config:
            self.convert_config_(sim_config)

    def convert_config_(self, json_config):
        """Convert SONATA config files to dictionary containing absolute paths and simulation parameters.

        Parameters
        ----------
        json_config: json file
            json file containing all SONATA paths and parameters.
        """

        with open(os.path.join(self.base_path, json_config)) as fp:
            config = json.load(fp)

        subs = {}
        for dir, path in config["manifest"].items():
            if dir.startswith('$'):
                dir = dir[1:]
            subs[dir] = os.path.join(self.base_path, path)
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

    def Create(self):
        """Create nodes from SONATA files.

        Iterates SONATA nodes files and creates nodes with parameters given in corresponding CSV files.

        Notes
        -----
        We currently do not iterate SONATA node groups, so node parameters can only be given
        through CSV files at the moment.

        Raises
        ------
        NESTError
            If setting node parameters fail.
        NotImplemented
            If the model type given in the CSV files do not match a NEST type, or if more than one NEST model is
            given per CSV file.
        """

        # Iterate node files in config file
        for nodes in self.config['networks']['nodes']:
            node_types_file_name = nodes['node_types_file']
            node_types = pd.read_csv(node_types_file_name, sep=r'\s+')  # CSV file containing parameters ++

            one_model = (node_types['model_type'] == 'virtual').all() or self.is_unique_(node_types['model_template'])

            if one_model:
                model_type = node_types.model_type.iloc[0]
                if model_type not in ['point_neuron', 'point_process', 'virtual']:
                    raise NotImplementedError(f'model of type {model_type} is not a NEST model, it cannot be used.')

                # Extract node parameters
                have_dynamics = 'dynamics_params' in node_types.keys()

                node_type_map = self.create_node_type_parameter_map_(node_types, have_dynamics)

                # Open sonata node files and create nodes
                with h5py.File(nodes["nodes_file"], 'r') as nodes_file:
                    # Iterate populations in nodes_file, there is usually only one population per file
                    for population_name in nodes_file['nodes']:

                        population = nodes_file['nodes'][population_name]
                        num_elements = population['node_id'].size

                        if model_type == 'virtual':
                            nodes = self.create_spike_generators_(population_name, num_elements)
                        else:
                            # Create non-device nodes
                            model = node_types.model_template.iloc[0].replace('nest:', '')
                            nodes = Create(model, num_elements)

                        # Set node parameters
                        if have_dynamics:
                            node_type_ids = population['node_type_id']
                            for node_type in np.unique(node_type_ids):
                                indx = np.where(node_type_ids[:] == node_type)[0]
                                nodes[indx].set(node_type_map[node_type])

                        self.node_collections[population_name] = nodes
            else:
                raise NotImplementedError("More than one NEST model per csv file currently not implemented")

    def is_unique_(self, col):
        """Check if all values in column are unique.

        Parameters
        ----------
        col: pandas dataframe series
            column for checking for uniqueness.

        Returns
        -------
        bool:
            Whether or not the values in the column are unique.
        """
        numpy_array = col.to_numpy()
        return (numpy_array[0] == numpy_array).all()

    def create_node_type_parameter_map_(self, node_types, have_dynamics):
        """Create map between node type id and node parameter.

        Node parameters are given as JSON files, and need to be mapped to the correct `node_type_id`. This is then
        used for setting the right node parameters when the nodes are created.

        Parameters
        ----------
        node_types : pandas dataframe
            Dataframe containing all node type information.
        have_dynamics : bool
            Whether or not node_types have a `dynamics` column. The `dynamics` column contains the name
            of the JSON file containing the node parameters for the respective `node_type_id`.

        Returns
        -------
        Dictionary:
            Dictionary containing the map between the node type id and the node parameter dictionary.
        """

        node_type_map = {}
        if have_dynamics:
            for ind in node_types.index:
                dynamics = {}
                with open(self.config['components']['point_neuron_models_dir'] + '/' +
                          node_types['dynamics_params'][ind]) as dynamics_file:
                    dynamics.update(json.load(dynamics_file))
                node_type_map[node_types['node_type_id'][ind]] = dynamics
        return node_type_map

    def create_spike_generators_(self, population_name, num_elements):
        """Create `num_elements` spike generators with `spike_times` given in SONATA files.

        Parameters
        ----------
        population_name: string
            Name of population
        num_elements: int
            Number of spike generators to be created

        Returns
        NodeCollection:
            Object representing the created spike generators
        """

        nodes = NodeCollection()  # Empty NC in case we don't need any spike generators
        model = 'spike_generator'
        # First need to iterate to the current spike population dictionary in config file
        for input_dict in self.config['inputs'].values():
            node_set = input_dict['node_set']
            if node_set == population_name:
                # Spiketrains are given in h5 files
                with h5py.File(input_dict['input_file'], 'r') as spiking_file:
                    # Convert data to NumPy arrays for performance
                    spikes = np.array(spiking_file['spikes']['timestamps'])
                    node_ids = np.array(spiking_file['spikes']['gids'])
                    timestamps = {i: spikes[node_ids == i] for i in range(num_elements)}  # Map node id's to spike times
                nodes = Create(model, num_elements)
                nodes.set([{'spike_times': timestamps[i], 'precise_times': True} for i in range(len(nodes))])
                break  # Once we have iterated to the correct node set, we can break and return the nodes
        return nodes

    def create_edge_dict_(self):
        """Create a list of edge dictionaries used when connecting with SONATA files.

        The function iterates edge (synapse) files in the config file and creates one dictionary per edge file.
        The dictionary contains two keys, `edges_file` and `edge_synapse`. `edges_file` is a path to the SONATA
        edge file used for creating the connections of the given edge. `edge_synapse` contains a dictionary
        that maps `edge_type_id` to a synapse dictionary.

        edge_types thus have the following structure:
        edge_types = [{'edges_file': string,
                       'edge_synapse': {'edge_type_id_1: syn_params, ..., 'edge_type_id_n: syn_params}},
                      ...,
                      {..}
                     ]
        """

        for edges in self.config['networks']['edges']:
            edge_dict = {}
            edge_dict['edges_file'] = edges['edges_file']  # File containing all source id's and target id's
            edge_types_file = edges['edge_types_file']  # CSV file containing synapse parameters

            with open(edge_types_file, 'r') as csv_file:
                reader = csv.DictReader(csv_file, delimiter=' ', quotechar='"')
                rows = list(reader)
                edge_params = {}

                for d in rows:
                    d['synapse_model'] = d.pop('model_template')
                    setable_params = GetDefaults(d['synapse_model'])
                    if 'syn_weight' in d:
                        d['weight'] = float(d.pop('syn_weight'))

                    synapse_dict = {key: d[key] for key in setable_params if key in d}
                    if 'delay' in synapse_dict:
                        # Convert to float
                        synapse_dict['delay'] = float(synapse_dict['delay'])

                    with open(self.config['components']['synaptic_models_dir'] + '/' +
                              d['dynamics_params']) as dynamics_file:
                        # Other synapse parameters not given in CSV files
                        dynamics = json.load(dynamics_file)
                    synapse_dict.update(dynamics)

                    # Map synapse_dict to edge_type_id so the correct synapse dictionary is used when connecting
                    edge_params[d['edge_type_id']] = synapse_dict
            edge_dict['edge_synapse'] = edge_params

            self.edge_types.append(edge_dict)

    def Connect(self):
        """Connect nodes in SONATA edge files.

        NEST kernel will iterate the list in `edge_types` containing dictionaries with the edge files and synapse
        dictionaries, and use the `NodeCollection`s given in `node_collections` to create the connections.
        """
        self.create_edge_dict_()

        sonata_dynamics = {'nodes': self.node_collections, 'edges': self.edge_types}
        # Check if HDF5 files exists and are not blocked.
        for d in self.edge_types:
            try:
                f = h5py.File(d['edges_file'], 'r')
                f.close()
            except BlockingIOError as err:
                raise BlockingIOError(f'{err.strerror} for {os.path.realpath(d["edges_file"])}') from None

        sps(sonata_dynamics)
        sr('Connect_sonata')

    def CreateSonataNetwork(self, simulate=False):
        """ Create network defined in SONATA files.

        Convenience function for creating networks in NEST from SONATA files.

        Parameters
        ----------
        simulate bool (optional)
            Whether or not to simulate
        """

        if self.config['target_simulator'] != 'NEST':
            raise NotImplementedError('Only `target_simulator` of type NEST is supported.')

        SetKernelStatus({'overwrite_files': True})

        if simulate:
            SetKernelStatus({'resolution': self.config['run']['dt']})

        # Create network
        self.Create()
        self.Connect()

        if simulate:
            simtime = 0
            if 'tstop' in self.config['run']:
                simtime = self.config['run']['tstop']
            else:
                simtime = self.config['run']['duration']

            Simulate(simtime)
