.. _nest_sonata:

NEST SONATA guide
=================

NEST supports building and simulating networks of point neurons described by the
`SONATA <https://github.com/AllenInstitute/sonata>`_ format [1]_.
This guide provides the details about how a SONATA network must be specified to be supported natively by NEST.

.. _sec:sonata_configure:

Configure NEST for SONATA
-------------------------

To use SONATA with NEST, both `HDF5 <https://hdfgroup.org/>`_ and `h5py <https://www.h5py.org/>`_ must be installed on
the system and NEST must be configured properly.

If you install NEST from a pre-built package, NEST will automatically be configured for SONATA support.

If you install NEST from source, the following configuration option must be added to
your CMake invocation:

.. code-block:: sh

    -Dwith-hdf5=ON

For further details, see :ref:`cmake_options`.

.. _sec:sonata_overview:

Overview of the SONATA format
-----------------------------

The SONATA (Scalable Open Network Architecture TemplAte) format provides a framework for storage and exchange of
network models and simulation configurations. A network is considered as a graph made of nodes and edges. Nodes of a
network can be arranged into multiple populations. There are two categories of nodes: explicitly simulated nodes
and virtual nodes that only provide inputs to the simulated system. Nodes within and between populations are connected
through edges (synapses). The SONATA format explicitly tabulates information about nodes and edges in the table-based
file formats HDF5 and CSV.

The cell and synapse properties of the nodes and edges, respectively, can be described either individually or for
whole subsets. Nodes that share a global set of properties constitute a node type. Similarly, a subset of edges
that share a global set of properties constitute an edge type. Whether a property is stored individually or on a
per-type basis is up to the modeler. The number of node or edge types in a network model is typically small compared
to the number of nodes or edges. Therefore, the node and edge type files are stored in the CSV format such that a
particular node or edge type can be easily accessed by its node or edge type id.

Each node and edge in the network is explicitly tabulated in the binary format HDF5. Populations are hierarchically
organized by utilizing HDF5 groups and datasets. The SONATA format requires certain HDF5 datasets, for instance,
a dataset containing all the node type ids. Properties that are
stored on an individual basis, for instance, synaptic weights, are also stored in HDF5 datasets.

Simulation parameters and the locations of the HDF5 and CSV files specifying the network are stored in JSON
configuration files.

.. _sec:sonata_examples:

NEST SONATA Example
-------------------

Here is a minimal example of how to build and simulate from SONATA specifications:

.. code-block:: python

    # Instantiate SonataNetwork
    sonata_net = nest.SonataNetwork("path/to/config.json")

    # Create and connect nodes
    node_collections = sonata_net.BuildNetwork()

    # Connect spike recorder to a population
    s_rec = nest.Create("spike_recorder")
    nest.Connect(node_collections["name_of_population_to_record"], s_rec)

    # Simulate the network
    sonata_net.Simulate()

For more detailed examples, see

* :doc:`../auto_examples/sonata_example/sonata_network`

.. _sec:sonata_nodes:

NEST support of SONATA nodes
----------------------------

In the SONATA format, node populations are serialized in node HDF5 files and have a single associated node type
CSV file that assigns properties to all nodes with a given node type id. A node type CSV file may be shared by
multiple node population HDF5 files.

NEST assumes the following structure of the node HDF5 files:

:: 

    <nodes_file.h5>                     Filename
    ├─ nodes                            Group - required
    │  ├─ <population_name>             Group - required - usually only one but can be more population groups per file
    │  │  ├─ node_type_id               Dataset {N_total_nodes} - required


.. note::

    NEST assumes that the implicit row numbers in the ``node_type_id`` dataset correspond to the ``node_id``\s.

NEST supports the following SONATA node ``model_type``\s:

* ``point_neuron``
* ``point_process``
* ``virtual``

Both ``point_neuron`` and ``point_process`` mean that the node is a neuron model (explicitly simulated; the two terms
can be used interchangeably) whereas ``virtual`` means that the node only provide inputs to the simulated system.
``virtual`` nodes are modeled as ``spike_train_injector``\s (see :doc:`the model documentation for spike_train_injector <../models/spike_train_injector>`\). NEST requires
that only one ``model_type`` is present per node type CSV file.

The required headers for node type CSV files that describe neuron models are:

* ``node_type_id``
* ``model_type``
* ``model_template``
* ``dynamics_params``

For a given ``node_type_id``, the ``model_template`` entry is the name of the NEST neuron model with prefix ``nest:``.
NEST does not require the ``model_template`` entries to be the same, but the creation of the nodes described in a
single node type CSV file is faster if the neuron models are the same.

For a given ``node_type_id``, the ``dynamics_params`` entry is expected to be a reference to a JSON file that describes
the parametrization of the neuron model. Below is an example of a JSON file describing the parametrization of a given
node type:

.. code-block:: json

    {
        "I_e": 0.0,
        "tau_m": 44.9,
        "C_m": 239.0,
        "t_ref": 3.0,
        "E_L": -78.0,
        "V_th": -43.0,
        "V_reset": -55.0
    }


NEST does not support node properties stored on an individual basis in HDF5 datasets. This restriction can be
circumvented by assigning a single node its own node type id.

Below is an example of a node type CSV file with the required headers for neuron nodes:

+--------------+---------------+--------------------+-----------------+
| node_type_id | model_type    | model_template     | dynamics_params | 
+==============+===============+====================+=================+
| 1            | point_process | nest:iaf_psc_alpha | params_1.json   |
+--------------+---------------+--------------------+-----------------+
| 2            | point_process | nest:iaf_psc_alpha | params_2.json   |
+--------------+---------------+--------------------+-----------------+

The only required CSV header for ``virtual`` nodes is ``model_type``. The ``spike_train_injector``\s spike-time arrays
are expected to be provided in HDF5 datasets with the configuration details specified in the JSON configuration file. 


.. _sec:sonata_edges:

The NEST support of SONATA edges
--------------------------------

Analogous to nodes, edge populations are serialized in edge HDF5 files and have a single associated edge types
CSV file that assigns properties to all edges with a given edge type id.

NEST assumes the following structure of the edge HDF5 files:

:: 

    <edges_file.h5>                     Filename
    ├─ edges                            Group - required
    │  ├─ <population_name>             Group - required - usually only one but can be more population groups per file
    │  │  ├─ source_node_id             Dataset {N_total_edges} - required - with attribute specifying source population name
    │  │  ├─ edge_group_id              Dataset {N_total_edges} - required
    │  │  ├─ edge_group_index           Dataset {N_total_edges} - required
    │  │  ├─ target_node_id             Dataset {N_total_edges} - required - with attribute specifying target population name
    │  │  ├─ edge_type_id               Dataset {N_total_edges} - required
    │  │  ├─ indices                    Group - optional - currently not utilized
    │  │  │  ├─ source_to_target        Group
    │  │  │  │  ├─ node_id_to_range     Dataset {N_source_nodes x 2}
    │  │  │  │  ├─ range_to_edge_id     Dataset {N_source_nodes x 2}
    │  │  │  ├─ target_to_source        Group
    │  │  │  │  ├─ node_id_to_range     Dataset {N_target_nodes x 2}
    │  │  │  │  ├─ range_to_edge_id     Dataset {N_target_nodes x 2}
    │  │  ├─ <edge_id1>                 Group - required
    │  │  │  ├─ delay                   Dataset {M_edges} - optional
    │  │  │  ├─ syn_weight              Dataset {M_edges} - optional
    │  │  │  ├─ dynamics_params         Group - currently not supported
    │  │  ├─ <edge_id2>                 Group - optional - currently no support for more than one edge group
    │  │  │  ├─ delay                   Dataset {K_edges} - optional
    │  │  │  ├─ syn_weight              Dataset {K_edges} - optional
    │  │  │  ├─ dynamics_params         Group


Together the ``source_node_id`` and ``target_node_id`` datasets explicitly tabulate all individual connections.
The ``edge_type_id`` dataset attributes each edge its edge type id, which is used to assign synaptic properties from the
edge types CSV file.

In the SONATA format, edges within a population can be organized into one or more edge groups. Synaptic properties that
are specified on an individual basis are stored in these edge groups. The groups are identified by an ``edge_id`` key.
NEST assumes the ``edge_id``\s are contiguous numeric keys starting from zero, that is, 0, 1, 2, ...

.. note::

    NEST currently only supports one edge group per edge population. Furthermore, NEST only reads the ``delay``
    and ``syn_weight`` datasets, given that they are provided. This means that only connection delays and synaptic weights
    can be stored on an individual basis in the HDF5 format. Other synaptic properties must be given in the edge type
    CSV file(s).

Below is an example of a edge type CSV file:

+--------------+----------------+-------+-----------------+
| edge_type_id | model_template | delay | dynamics_params | 
+==============+================+=======+=================+
| 1            | static_synapse | 2.0   | params_1.json   |
+--------------+----------------+-------+-----------------+
| 2            | static_synapse | 2.5   | params_2.json   |
+--------------+----------------+-------+-----------------+

.. note::

    Only the synaptic properties ``delay`` and ``syn_weight`` can be provided as headers in the edge types CSV file.
    Other synaptic properties must be given in the JSON file under ``dynamics_params``.


.. _sec:sonata_config:

The SONATA configuration files
------------------------------

Model metadata, such as the relative location of the network files and simulation parameters, are stored in the
SONATA configuration ("config") file(s) in the JSON format. Below is an example SONATA config with the components NEST
expects to be included:

.. code-block:: json

    {
      "target_simulator": "NEST",
      "manifest": {
        "$BASE_DIR": "${configdir}",
        "$NETWORK_DIR": "$BASE_DIR/network",
        "$COMPONENTS_DIR": "$BASE_DIR/components",
        "$INPUT_DIR": "$BASE_DIR/inputs"
      },
      "components": {
        "point_neuron_models_dir": "$COMPONENTS_DIR/cell_models",
        "synaptic_models_dir": "$COMPONENTS_DIR/synaptic_models"
      },
      "networks": {
        "nodes": [
          {
            "nodes_file": "$NETWORK_DIR/internal_nodes.h5",
            "node_types_file": "$NETWORK_DIR/internal_node_types.csv"
          },
          {
            "nodes_file": "$NETWORK_DIR/external_nodes.h5",
            "node_types_file": "$NETWORK_DIR/external_node_types.csv"
          }
        ],
        "edges": [
          {
            "edges_file": "$NETWORK_DIR/internal_internal_edges.h5",
            "edge_types_file": "$NETWORK_DIR/internal_internal_edge_types.csv",
          },
          {
            "edges_file": "$NETWORK_DIR/external_internal_edges.h5",
            "edge_types_file": "$NETWORK_DIR/external_internal_edge_types.csv"
          }
        ]
      },
      "inputs": {
        "external_spike_trains": {
          "input_file": "$INPUT_DIR/external_spike_trains.h5",
          "node_set": "external"
        }
      },
      "run": {
        "tstop": 1500,
        "dt": 0.01
      }
    }

.. note::

    NEST supports the use of two config files, that is, one network and one simulation config. NEST does not currently
    support SONATA Spike Train Reports or utilize other ``output`` components in the SONATA config.

.. _sec:sonata_refs:

More about SONATA
-----------------

For a full specification of the SONATA format, see [1]_ and the `SONATA GitHub page <https://github.com/AllenInstitute/sonata>`_.


References
~~~~~~~~~~

.. [1] Dai K, Hernando J, Billeh YN, Gratiy SL, Planas J, et al. (2020).
       The SONATA data format for efficient description of large-scale network models.
       PLOS Computational Biology 16(2): e1007696. https://doi.org/10.1371/journal.pcbi.1007696
