.. _nest_sonata:

The NEST SONATA guide 
=====================

NEST supports building and simulating networks of point neurons described by the SONATA format [1]_. 
This guide provides the details about how a SONATA network must be specified to be supported natively by NEST. 


.. _sec:sonata_overview:

Overview of the SONATA format 
-----------------------------

The SONATA (Scalable Open Network Architecture TemplAte) format provides a framework for storage and exchange of 
network models and simulation configurations. A network is considered as a graph made of nodes and edges. Nodes of a 
network can be arranged into multiple populations. There are two categories of nodes: explicitly simulated nodes 
(neurons) and virtual nodes (devices) that only provide inputs to the simulated system. Nodes within and between 
populations are connected through edges (synapses). The SONATA format explicitly tabulates information about nodes 
and edges in the table-based file formats HDF5 and CSV. 

The cell and synapse properties of the nodes and edges, respectively, can be described either individually or for 
whole subsets. Nodes that share a global set of properties constitute a node type. Similarly, a subset of edges 
that share a global set of properties constitute an edge type. Whether a property is stored individually or on a 
per-type basis is up to the modeller. The number of node or edge types in a network model is typically small compared 
to the number of nodes or edges. Therefore, the node and edge type files are stored in the CSV format such that a 
particular node or edge type can be easily accessed by its node or edge type id.

Each node and edge in the network is explicitly tabulated in the binary format HDF5. Populations are hierarchically
organized by utilizing HDF5 groups and datasets. The SONATA format requires certain HDF5 datasets, for instance 
a dataset containing all the node ids and another containing all the corresponding node type ids. Properties that are 
stored on an individual-basis, for instance synaptic weights, are also stored in HDF5 datasets. 

Simulation parameters and the locations of the HDF5 and CSV files specifying the network are stored in JSON 
configuration files. 


.. _sec:sonata_nodes:

The NEST support of SONATA nodes 
--------------------------------

In the SONATA format, node populations are serialized in nodes HDF5 files. Each node in a population has a node type 
and each node population has a single associated node types CSV file that assigns properties to all nodes with a 
given node type. 

NEST assumes the following structure of the nodes HDF5 files: 

:: 

    <nodes_file.h5>                     Filename
    ├─ nodes                            Group - required
    │  ├─ <population_name>             Group - required - usually only one but can be more population groups per file
    │  │  ├─ node_type_id               Dataset {N_total_nodes} - required

Note that this structure means NEST assumes that the implicit row numbers in the `node_type_id` dataset correspond to the `node_id`s. 

NEST supports the following SONATA node `model_type`s:

* `point_neuron`
* `point_process`
* `virtual` 

Both `point_neuron` and `point_process` means that the node is a neuron model (explicitly simulated) whereas `virtual` 
means that the node only provide inputs to the simulated system. `virtual` nodes are modelled as `spike_generator`s 
(see :doc:`../models/spike_generator`). NEST requires that only one `model_type` is present per node types CSV file. 

The required headers for node types CSV files that describe neuron models are: 

* `node_type_id`
* `model_type`
* `model_template`
* `dynamics_params`

For a given `node_type_id`, the `model_template` is the name of the NEST neuron model with prefix `nest:`. NEST does 
not require the `model_template` entries to be the same, but the creation of the nodes described in the CSV file is 
faster if the neuron models are the same. 

For a given `node_type_id`, the `dynamics_params` entry is expected to be a reference to a JSON file that describes 
the parametrization of the neuron model. An example JSON file describing the parametrization of a given node type: 

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


NEST does not support node properties stored on an individual-basis in HDF5 datasets. This restriction can easily be 
circumvented by assigning a single node its own node type id. 

An example node types CSV file for neuron nodes: 

.. csv-table::
    :header: node_type_id, model_type, model_template, dynamics_params
    1, point_process, nest:iaf_psc_alpha, params_1.json
    2, point_process, nest:iaf_psc_alpha, params_2.json

The only required CSV header for `virtual` nodes is `model_type`. The `spike_generator`s spike-times arrays are expected
to be provided in HDF5 datasets with the configuration details specified in the JSON configuration file.  


.. _sec:sonata_edges:

Representation of network edges 
-------------------------------

individual connections 


Structure of SONATA HDF5 edge files:

<edge_file.h5>                      Filename
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
│  │  │  ├─ syn_weights             Dataset {M_edges} - optional
│  │  │  ├─ dynamics_params         Group - currently not supported
│  │  ├─ <edge_id2>                 Group - optional - currently no support for more than one edge id group
│  │  │  ├─ delay                   Dataset {K_edges} - optional
│  │  │  ├─ syn_weights             Dataset {K_edges} - optional
│  │  │  ├─ dynamics_params         Group

  For more details, see https://github.com/AllenInstitute/sonata/blob/master/docs/SONATA_DEVELOPER_GUIDE.md

  require numeric keys, i.e. 0, 1, 2, ..., for edge id groups


.. _sec:sonata_config:

Configuration file
------------------

Simulation parameters 

Target simulator 

.. _sec:sonata_refs:

More about SONATA 
-----------------

For full specification of the SONATA format, see [1]_ and the `SONATA GitHub page<https://github.com/AllenInstitute/sonata>`_.


.. _sec:sonata_examples:

NEST SONATA examples 
--------------------

link to example scripts 


References
~~~~~~~~~~

.. [1] Dai K, Hernando J, Billeh YN, Gratiy SL, Planas J, et al. (2020). 
       The SONATA data format for efficient description of large-scale network models. 
       PLOS Computational Biology 16(2): e1007696. https://doi.org/10.1371/journal.pcbi.1007696
