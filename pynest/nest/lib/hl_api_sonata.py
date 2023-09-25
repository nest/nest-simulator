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
Functions to build and simulate networks with the SONATA format
"""


import itertools
import json
import os
from pathlib import Path, PurePath

import numpy as np

from .. import nestkernel_api as nestkernel
from .hl_api_models import GetDefaults
from .hl_api_nodes import Create
from .hl_api_simulation import GetKernelStatus, SetKernelStatus, Simulate
from .hl_api_types import NodeCollection

try:
    import pandas as pd

    have_pandas = True
except ImportError:
    have_pandas = False

try:
    import h5py

    have_h5py = True
except ImportError:
    have_h5py = False

have_hdf5 = GetKernelStatus("build_info")["have_hdf5"]

__all__ = ["SonataNetwork"]


class SonataNetwork:
    """Class for building and simulating networks represented by the SONATA format.

    ``SonataNetwork`` provides native NEST support for building and simulating
    network models represented by the SONATA format. In the SONATA format,
    information about nodes, edges (synapses) and their respective properties
    are stored in the table-based file formats HDF5 and CSV. Model metadata,
    such as the path relation between files on disk and simulation parameters,
    are stored in JSON configuration files. See the :ref:`nest_sonata` for details
    on the NEST support of the SONATA format.

    The constructor takes the JSON configuration file specifying the paths to
    the HDF5 and CSV files describing the network. In case simulation
    parameters are stored in a separate JSON configuration file, the
    constructor also has the option to pass a second configuration file.

    Parameters
    ----------
    config : [str | pathlib.Path | pathlib.PurePath]
        String or pathlib object describing the path to the JSON
        configuration file.
    sim_config : [str | pathlib.Path | pathlib.PurePath], optional
        String or pathlib object describing the path to a JSON configuration
        file containing simulation parameters. This is only needed if simulation
        parameters are given in a separate configuration file.

    Example
    -------
        ::

            import nest

            nest.ResetKernel()

            # Instantiate SonataNetwork
            sonata_net = nest.SonataNetwork("path/to/config.json")

            # Create and connect nodes
            node_collections = sonata_net.BuildNetwork()

            # Connect spike recorder to a population
            s_rec = nest.Create("spike_recorder")
            nest.Connect(node_collections["name_of_population_to_record"], s_rec)

            # Simulate the network
            sonata_net.Simulate()
    """

    def __init__(self, config, sim_config=None):
        if not have_hdf5:
            raise ModuleNotFoundError("'SonataNetwork' unavailable because NEST was compiled without 'HDF5' support")
        if not have_h5py:
            raise ModuleNotFoundError("'SonataNetwork' unavailable because 'h5py' could not be loaded.")
        if not have_pandas:
            raise ModuleNotFoundError("'SonataNetwork' unavailable because 'pandas' could not be loaded.")

        self._node_collections = {}
        self._edges_maps = []
        self._hyperslab_size_default = 2**20

        self._are_nodes_created = False
        self._is_network_built = False

        self._conf = self._parse_config(config)
        if sim_config is not None:
            self._conf.update(self._parse_config(sim_config))

        if self._conf["target_simulator"] != "NEST":
            raise ValueError("'target_simulator' in configuration file must be 'NEST'.")

        if "dt" not in self._conf["run"]:
            raise ValueError("Time resolution 'dt' must be specified in configuration file.")

        SetKernelStatus({"resolution": self._conf["run"]["dt"]})

    def _parse_config(self, config):
        """Parse JSON configuration file.

        Parse JSON config file and convert to a dictionary containing
        absolute paths and simulation parameters.

        Parameters
        ----------
        config : [str | pathlib.Path | pathlib.PurePath]
            String or pathlib object describing the path to the JSON
            configuration file.

        Returns
        -------
        dict
            SONATA config as dictionary
        """

        if not isinstance(config, (str, PurePath, Path)):
            raise TypeError("Path to JSON configuration file must be passed as 'str' or 'pathlib.Path'.")

        # Get absolute path
        conf_path = Path(config).resolve(strict=True)
        base_path = conf_path.parent

        with open(conf_path) as fp:
            conf = json.load(fp)

        # Replace path variables (e.g. $MY_DIR) with absolute paths in manifest
        for k, v in conf["manifest"].copy().items():
            if "$BASE_DIR" in v:
                v = v.replace("$BASE_DIR", ".")
            conf["manifest"].update({k: base_path.joinpath(v).as_posix()})

            if k.startswith("$"):
                conf["manifest"][k[1:]] = conf["manifest"].pop(k)

        def recursive_substitutions(config_obj):
            # Recursive substitutions of path variables with entries from manifest
            if isinstance(config_obj, dict):
                return {k: recursive_substitutions(v) for k, v in config_obj.items()}
            elif isinstance(config_obj, list):
                return [recursive_substitutions(e) for e in config_obj]
            elif isinstance(config_obj, str) and config_obj.startswith("$"):
                for dir, path in conf["manifest"].items():
                    config_obj = config_obj.replace(dir, path)
                return config_obj[1:]
            return config_obj

        conf.update(recursive_substitutions(conf))

        return conf

    def Create(self):
        """Create the SONATA network nodes.

        Creates the network nodes. In the SONATA format, node populations are
        serialized in node HDF5 files. Each node in a population has a node
        type. Each node population has a single associated node types CSV file
        that assigns properties to all nodes with a given node type.

        Please note that it is assumed that all relevant node properties are
        stored in the node types CSV file. For neuron nodes, the relevant
        properties are model type, model template and reference to a JSON
        file describing the parametrization.

        Returns
        -------
        node_collections : dict
            A dictionary containing the created :py:class:`.NodeCollection`
            for each population. The population names are keys.
        """

        # Iterate node config files
        for nodes_conf in self._conf["networks"]["nodes"]:
            csv_fn = nodes_conf["node_types_file"]
            nodes_df = pd.read_csv(csv_fn, sep=r"\s+")

            # Require only one model type per CSV file
            model_types_arr = nodes_df["model_type"].to_numpy()
            is_one_model_type = (model_types_arr[0] == model_types_arr).all()

            if not is_one_model_type:
                raise ValueError(
                    f"Only one model type per node types CSV file is supported. {csv_fn} contains more than one."
                )

            model_type = model_types_arr[0]

            if model_type in ["point_neuron", "point_process"]:
                self._create_neurons(nodes_conf, nodes_df, csv_fn)
            elif model_type == "virtual":
                self._create_spike_train_injectors(nodes_conf)
            else:
                raise ValueError(f"Model type '{model_type}' in {csv_fn} is not supported by NEST.")

        self._are_nodes_created = True

        return self._node_collections

    def _create_neurons(self, nodes_conf, nodes_df, csv_fn):
        """Create neuron nodes.

        Parameters
        ----------
        nodes_conf : dict
            Config as dictionary specifying filenames
        nodes_df : pandas.DataFrame
            Associated node CSV table as dataframe
        csv_fn : str
            Name of current CSV file. Used for more informative error messages.
        """

        node_types_map = self._create_node_type_parameter_map(nodes_df, csv_fn)

        models_arr = nodes_df["model_template"].to_numpy()
        is_one_model = (models_arr[0] == models_arr).all()
        one_model_name = models_arr[0] if is_one_model else None

        with h5py.File(nodes_conf["nodes_file"], "r") as nodes_h5f:
            # Iterate node populations in current node.h5 file
            for pop_name in nodes_h5f["nodes"]:
                node_type_id_dset = nodes_h5f["nodes"][pop_name]["node_type_id"][:]

                if is_one_model:
                    nest_nodes = Create(one_model_name, n=node_type_id_dset.size)
                    node_type_ids, inv_ind = np.unique(node_type_id_dset, return_inverse=True)

                    # Extract node parameters
                    for i, node_type_id in enumerate(node_type_ids):
                        params_path = PurePath(
                            self._conf["components"]["point_neuron_models_dir"],
                            node_types_map[node_type_id]["dynamics_params"],
                        )

                        with open(params_path) as fp:
                            params = json.load(fp)

                        nest_nodes[inv_ind == i].set(params)
                else:
                    # More than one NEST neuron model in CSV file

                    # TODO: Utilizing np.unique(node_type_id_dset, return_<foo>=...)
                    # with the different return options might be more efficient

                    nest_nodes = NodeCollection()
                    for k, g in itertools.groupby(node_type_id_dset):
                        # k is a node_type_id key
                        # g is an itertools group object
                        # len(list(g)) gives the number of consecutive occurrences of the current k
                        model = node_types_map[k]["model_template"]
                        n_nrns = len(list(g))
                        params_path = PurePath(
                            self._conf["components"]["point_neuron_models_dir"],
                            node_types_map[k]["dynamics_params"],
                        )
                        with open(params_path) as fp:
                            params = json.load(fp)

                        nest_nodes += Create(model, n=n_nrns, params=params)

                self._node_collections[pop_name] = nest_nodes

    def _create_spike_train_injectors(self, nodes_conf):
        """Create spike train injector nodes.

        Parameters
        ----------
        nodes_conf : dict
            Config as dictionary specifying filenames
        """

        with h5py.File(nodes_conf["nodes_file"], "r") as nodes_h5f:
            for pop_name in nodes_h5f["nodes"]:
                node_type_id_dset = nodes_h5f["nodes"][pop_name]["node_type_id"]
                n_nodes = node_type_id_dset.size

                input_file = None
                for inputs_dict in self._conf["inputs"].values():
                    if inputs_dict["node_set"] == pop_name:
                        input_file = inputs_dict["input_file"]
                        break  # Break once we found the matching population

                if input_file is None:
                    raise ValueError(f"Could not find an input file for population {pop_name} in config file.")

                with h5py.File(input_file, "r") as input_h5f:
                    # Deduce the HDF5 file structure
                    all_groups = all([isinstance(g, h5py.Group) for g in input_h5f["spikes"].values()])
                    any_groups = any([isinstance(g, h5py.Group) for g in input_h5f["spikes"].values()])
                    if (all_groups or any_groups) and not (all_groups and any_groups):
                        raise ValueError(
                            "Unsupported HDF5 structure; groups and datasets cannot be on the same hierarchical "
                            f"level in input spikes file {input_file}."
                        )

                    if all_groups:
                        if pop_name in input_h5f["spikes"].keys():
                            spikes_grp = input_h5f["spikes"][pop_name]
                        else:
                            raise ValueError(
                                f"Did not find a matching HDF5 group name for population {pop_name} in {input_file}."
                            )
                    else:
                        spikes_grp = input_h5f["spikes"]

                    if "gids" in spikes_grp:
                        node_ids = spikes_grp["gids"][:]
                    elif "node_ids" in spikes_grp:
                        node_ids = spikes_grp["node_ids"][:]
                    else:
                        raise ValueError(f"No dataset called 'gids' or 'node_ids' in {input_file}.")

                    timestamps = spikes_grp["timestamps"][:]

                # Map node id's to spike times
                # TODO: Can this be done in a more efficient way?
                spikes_map = {node_id: timestamps[node_ids == node_id] for node_id in range(n_nodes)}
                params_lst = [
                    {"spike_times": spikes_map[node_id], "allow_offgrid_times": True} for node_id in range(n_nodes)
                ]

                # Create and store NC
                nest_nodes = Create("spike_train_injector", n=n_nodes, params=params_lst)
                self._node_collections[pop_name] = nest_nodes

    def _create_node_type_parameter_map(self, nodes_df, csv_fn):
        """Create map between node type id and node properties.

        For neuron models, each node type id in the node types CSV file has:
            * A model template which describes the name of the neuron model
            * A reference to a JSON file describing the neuron's parametrization

        This function creates a map of the above node properties with the
        node type id as key.

        Parameters
        ----------
        nodes_df : pandas.DataFrame
            Node type CSV table as dataframe.
        csv_fn : str
            Name of current CSV file. Used for more informative error messages.

        Returns
        -------
        dict :
            Map of node properties for the different node type ids.
        """

        if "model_template" not in nodes_df.columns:
            raise ValueError(f"Missing the required 'model_template' header specifying NEST neuron models in {csv_fn}.")

        if "dynamics_params" not in nodes_df.columns:
            raise ValueError(
                "Missing the required 'dynamics_params' header specifying .json "
                f"files with model parameters in {csv_fn}."
            )

        nodes_df["model_template"] = nodes_df["model_template"].str.replace("nest:", "")

        req_cols = ["model_template", "dynamics_params"]
        node_types_map = nodes_df.set_index("node_type_id")[req_cols].to_dict(orient="index")

        return node_types_map

    def Connect(self, hdf5_hyperslab_size=None):
        """Connect the SONATA network nodes.

        The connections are created by first parsing the edge (synapse) CSV
        files to create a map of synaptic properties on the Python level. This
        is then sent to the NEST kernel together with the edge HDF5 files to
        create the connections.

        For large networks, the edge HDF5 files might not fit into memory in
        their entirety. In the NEST kernel, the edge HDF5 datasets are therefore
        read sequentially as blocks of contiguous hyperslabs. The hyperslab size
        is modifiable so that the user is able to achieve a balance between
        the number of read operations and memory overhead.

        Parameters
        ----------
        hdf5_hyperslab_size : int, optional
            Size of the hyperslab to read in one read operation. The hyperslab
            size is applied to all HDF5 datasets that need to be read in order
            to create the connections. Default: ``2**20``.
        """

        if not self._are_nodes_created:
            raise RuntimeError("The SONATA network nodes must be created before any connections can be made.")

        if hdf5_hyperslab_size is None:
            hdf5_hyperslab_size = self._hyperslab_size_default

        self._verify_hyperslab_size(hdf5_hyperslab_size)

        graph_specs = self._create_graph_specs()

        # Check whether HDF5 files exist and are not blocked.
        for d in graph_specs["edges"]:
            try:
                f = h5py.File(d["edges_file"], "r")
                f.close()
            except BlockingIOError as err:
                raise BlockingIOError(f"{err.strerror} for {os.path.realpath(d['edges_file'])}") from None

        nestkernel.llapi_connect_sonata(graph_specs, hdf5_hyperslab_size)

        self._is_network_built = True

    def _verify_hyperslab_size(self, hyperslab_size):
        """Check if provided hyperslab size is valid."""

        if not isinstance(hyperslab_size, int):
            raise TypeError("hdf5_hyperslab_size must be passed as int")
        if hyperslab_size <= 0:
            raise ValueError("hdf5_hyperslab_size must be strictly positive")

    def _create_graph_specs(self):
        """Create graph specifications dictionary.

        The graph specifications (`graph_specs`) dictionary is passed to
        the kernel where the connections are created. `graph_specs` has the
        following structure:

        {
            "nodes":
            {
                "<pop_name_1>": NodeCollection,
                "<pop_name_2>": NodeCollection,
                ...
            },
            "edges":
            [
                {"edges_file": '<edges.h5>',
                 "syn_specs": {"<edge_type_id_1>": syn_spec,
                               "<edge_type_id_2>": syn_spec,
                               ...
                               }
                 },
                {"edges_file": '<edges.h5>',
                 "syn_specs": {"<edge_type_id_1>": syn_spec,
                               "<edge_type_id_2>": syn_spec,
                               ...
                               }
                 },
                ...
            ]
        }

        Returns
        -------
        dict :
            Map of SONATA graph specifications.
        """

        self._create_edges_maps()
        graph_specs = {"nodes": self._node_collections, "edges": self._edges_maps}
        return graph_specs

    def _create_edges_maps(self):
        """Create a collection of maps of edge properties.

        Creates a map between edge type id and edge (synapse) properties for
        each edge CSV file. The associated edge HDF5 filename is included in
        the map as well.
        """

        # Iterate edge config files
        for edges_conf in self._conf["networks"]["edges"]:
            edges_map = {}
            edges_csv_fn = edges_conf["edge_types_file"]
            edges_df = pd.read_csv(edges_csv_fn, sep=r"\s+")

            if "model_template" not in edges_df.columns:
                raise ValueError(
                    f"Missing the required 'model_template' header specifying NEST synapse models in {edges_csv_fn}."
                )

            # Rename column labels to names used by NEST. Note that rename
            # don't throw an error for extra labels (we want this behavior)
            edges_df.rename(
                columns={"model_template": "synapse_model", "syn_weight": "weight"},
                inplace=True,
            )

            # Cast edge type ids from int to str, needed for kernel dictionary
            edges_df["edge_type_id"] = edges_df["edge_type_id"].astype("string")

            edges_df_cols = set(edges_df.columns)

            # If 'dynamics_params' is specified, additional synapse
            # parameters may be given in a .json file
            have_dynamics = "dynamics_params" in edges_df.columns

            # Extract synapse models in the edge CSV file and check if
            # only one model is present; we can then use a more efficient
            # procedure for extracting the syn_specs.
            models_arr = edges_df["synapse_model"].to_numpy()
            is_one_model = (models_arr[0] == models_arr).all()

            if is_one_model:
                # Only one model in the edge CSV file

                synapse_model = models_arr[0]
                # Find set of settable parameters for synapse model
                settable_params = set([*GetDefaults(synapse_model)])
                # Parameters to extract (elements common to both sets)
                extract_cols = list(settable_params & edges_df_cols)
                if have_dynamics:
                    extract_cols.append("dynamics_params")

                # Extract syn_spec for each edge type
                syn_specs = edges_df.set_index("edge_type_id")[extract_cols].to_dict(orient="index")

                if have_dynamics:
                    # Include parameters from JSON file in the syn_spec
                    for edge_type_id, syn_spec in syn_specs.copy().items():
                        params_path = PurePath(
                            self._conf["components"]["synaptic_models_dir"],
                            syn_spec["dynamics_params"],
                        )
                        with open(params_path) as fp:
                            params = json.load(fp)

                        syn_specs[edge_type_id].update(params)
                        syn_specs[edge_type_id].pop("dynamics_params")
            else:
                # More than one synapse model in CSV file; in this case we
                # must iterate each row in the CSV table. For each row,
                # we extract the syn_spec associated with the specified model

                syn_specs = {}
                idx_map = {k: i for i, k in enumerate(list(edges_df), start=1)}

                for row in edges_df.itertuples(name=None):
                    # Set of settable parameters
                    settable_params = set([*GetDefaults(row[idx_map["synapse_model"]])])
                    # Parameters to extract (elements common to both sets)
                    extract_cols = list(settable_params & edges_df_cols)
                    syn_spec = {k: row[idx_map[k]] for k in extract_cols}

                    if have_dynamics:
                        # Include parameters from JSON file in the map
                        params_path = PurePath(
                            self._conf["components"]["synaptic_models_dir"],
                            row[idx_map["dynamics_params"]],
                        )

                        with open(params_path) as fp:
                            params = json.load(fp)

                        syn_spec.update(params)

                    syn_specs[row[idx_map["edge_type_id"]]] = syn_spec

            # Create edges map
            edges_map["syn_specs"] = syn_specs
            edges_map["edges_file"] = edges_conf["edges_file"]
            self._edges_maps.append(edges_map)

    def BuildNetwork(self, hdf5_hyperslab_size=None):
        """Build SONATA network.

        Convenience function for building the SONATA network. The function
        first calls the membership function :py:func:`Create()` to create the
        network nodes and then the membership function :py:func:`Connect()`
        to create their connections.

        For more details, see :py:func:`Create()` and :py:func:`Connect()`.

        Parameters
        ----------
        hdf5_hyperslab_size : int, optional
            Size of hyperslab that is read into memory in one read operation.
            Applies to all HDF5 datasets relevant for creating the connections.
            Default: ``2**20``.

        Returns
        -------
        node_collections : dict
            A dictionary containing the created :py:class:`.NodeCollection`
            for each population. The population names are keys.
        """

        if hdf5_hyperslab_size is not None:
            # hyperslab size is verfified in Connect, but we also verify here
            # to save computational resources in case of wrong input
            self._verify_hyperslab_size(hdf5_hyperslab_size)

        node_collections = self.Create()
        self.Connect(hdf5_hyperslab_size=hdf5_hyperslab_size)

        return node_collections

    def Simulate(self):
        """Simulate the SONATA network.

        The simulation time and resolution are expected to be provided in the
        JSON configuration file.
        """

        # Verify that network is built
        if not self._is_network_built:
            raise RuntimeError("The SONATA network must be built before a simulation can be done.")

        if "tstop" in self._conf["run"]:
            T_sim = self._conf["run"]["tstop"]
        elif "duration" in self._conf["run"]:
            T_sim = self._conf["run"]["duration"]
        else:
            raise ValueError("Simulation time 'tstop' or 'duration' must be specified in configuration file.")

        Simulate(T_sim)

    @property
    def node_collections(self):
        return self._node_collections

    @property
    def config(self):
        return self._conf
