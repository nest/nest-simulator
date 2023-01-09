.. _nest_sonata:

The NEST SONATA guide 
=====================

Native support for networks specified by the SONATA format [REF] was introduced in NEST 3.x. The SONATA format is 
meant to support a variety of neural simulator software, and hence has soft restrictions on how a SONATA network is 
specified.


is hence lenient in . The NEST SONATA reader therefore .

If you have a SONATA network specification that is not yet supported by NEST but you think should be, please contact the developers through the [mailing list](link).


large that it can't be imported into memory

HDF5 data is read  

http://www.graphviz.org/

Directory Structure
Before running a simulation, we will need to create the runtime environment, including parameter files, run-script and configuration files.
https://alleninstitute.github.io/bmtk/tutorial_pointnet_modeling.html


The simulation time and resolution are expected to be provided in the
JSON configuration file. Additional kernel attributes can be passed as
as arbitrary keyword arguments (`kwargs`). See the documentation of
:ref:`sec:kernel_attributes` for a valid list of kernel attributes.

Note that the number of threads and MPI processes should be set in
advance of *building* the network.

For convenience and for compliance with SONATA format


The network nodes are created on the Python level. In the SONATA format,
node populations are serialized in node HD5 files. Each node in a
population has a node type. Each node population has a single associated
node types CSV file that assigns properties to all nodes with a given node
type. Please note that it is assumed that all relevant node properties are
stored in the node types CSV file. For neuron nodes relevant properties
are model type, model name and reference to a JSON file describing the
parametrization.

Each node population is NodeCollection

Iterates SONATA nodes files and creates nodes with parameters given in corresponding CSV files.


.. csv-table::
    :header: node_type_id, model_type, model_template, dynamics_params
    1, point_process, nest:iaf_psc_alpha, params_1.json
    2, point_process, nest:iaf_psc_alpha, params_2.json


node_type_id model_type pop_name ei location population
0 virtual sON_TF8 e LGN lgn
1 virtual sON_TF4 e LGN lgn


  Structure of SONATA edge files:

  <edge_file.h5>                      Filename
  ├─ edges                            Group - required
  │  ├─ <population_name>             Group - required - usually only one but can be more population groups per file
  │  │  ├─ source_node_id             Dataset {N_total_edges} - required - with attribute specifying source population name
  │  │  ├─ edge_group_id              Dataset {N_total_edges} - required
  │  │  ├─ edge_group_index           Dataset {N_total_edges} - required
  │  │  ├─ target_node_id             Dataset {N_total_edges} - required - with attribute specifying target population name
  │  │  ├─ edge_type_id               Dataset {N_total_edges} - required
  │  │  ├─ indices                    Group - optional
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