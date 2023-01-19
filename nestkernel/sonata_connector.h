/*
 *  sonata_connector.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SONATA_CONNECTOR_H
#define SONATA_CONNECTOR_H

#include "config.h"

#ifdef HAVE_HDF5

// C++ includes:
#include <map>
#include <vector>

// Includes from nestkernel:
#include "conn_parameter.h"
#include "kernel_manager.h"
#include "nest_datums.h"

#include "H5Cpp.h"

namespace nest
{

/**
 * Create connections from SONATA files.
 *
 * This class provides an interface for connecting nodes provided in SONTATA files.
 * The nodes have to first be created through the `SonataNetwork` PyNEST class, and
 * provided through the `graph_specs` dictionary, along with an array providing
 * connection parameters for the different SONATA files.
 *
 * The connections are created through the `SonataConnector::connect()` function,
 * which iterates the SONATA files, extracting the connection parameters and create
 * the individual connections.
 *
 * Note that HDF5 do NOT support MPI parallel and thread parallel runs simultaneously, see
 * https://portal.hdfgroup.org/display/knowledge/Questions+about+thread-safety+and+concurrent+access
 */

class SonataConnector
{
public:
  SonataConnector( const DictionaryDatum& graph_specs, const long chunk_size );
  ~SonataConnector();

  /**
   * Connect all sources and targets given in SONATA files.
   *
   * Goes through all SONATA edge files, iterates the source, target and parameter datasets
   * in the files and create all the connections. To create the connections, the nodes first
   * have to be created on PyNEST level.
   */
  void connect();

private:
  /**
   * Open an HDF5 edge file with read access only
   *
   * @param fname name of the edge file
   *
   * @return H5File pointer
   */
  H5::H5File* open_file_( std::string& fname );

  /**
   * Open an HDF5 edge file group
   *
   * @param file H5File pointer
   * @param grp_name name of the group
   *
   * @return H5 group pointer
   */
  H5::Group* open_group_( const H5::H5File* file, const std::string& grp_name );

  /**
   * Open an HDF5 edge file subgroup
   *
   * @param group H5 group
   * @param grp_name name of the group
   *
   * @return H5 subgroup pointer
   */
  H5::Group* open_group_( const H5::Group* group, const std::string& grp_name );

  /**
   * Open datasets required by the SONATA format
   *
   * @param pop_grp population group pointer
   */
  void open_required_dsets_( const H5::Group* pop_grp );

  /**
   * Try to open optional edge group id datasets
   *
   * @param edge_id_grp edge id group pointer
   */
  void try_open_edge_group_id_dsets_( const H5::Group* edge_id_grp );

  /**
   * Close all open datasets
   *
   */
  void close_dsets_();

  /**
   * Get attribute of dataset
   *
   * @param attribute_value value of attribute
   * @param dataset dataset to get attribute from
   * @param attribute_name name of attribute
   */
  void get_attribute_( std::string& attribute_value, const H5::DataSet& dataset, const std::string& attribute_name );


  /**
   * Create map between type id and syn_spec given in edge_dict
   *
   * Iterates edge_dict, extract synapse parameters and creates map between connection
   * parameters and type id. Checks that all synapse parameters are valid parameters
   * for the synapse.
   *
   * @param edge_dict dictionary containing edge type id's and synapse parameters
   */
  void create_type_id_2_syn_spec_( DictionaryDatum edge_dict );


  /**
   * Set synapse parameters in type_id_2_syn_spec_ and type_id_2_param_dicts_
   *
   * @param syn_dict synapse dictionary from which to set synapse params
   * @param synapse_model_id model id of synapse
   * @param type_id SONATA edge type id for mapping synapse parameters
   */
  void set_synapse_params_( DictionaryDatum syn_dict, index synapse_model_id, int type_id );

  /**
   * Get synapse parameters and convert them to correct datums for connecting
   *
   * @param snode_id id of source node
   * @param target target node
   * @param target_thread thread of target
   * @param rng rng pointer of target thread
   * @param edge_type_id type id of current edge to be connected
   */
  void get_synapse_params_( index snode_id, Node& target, thread target_thread, RngPtr rng, int edge_type_id );

  /**
   * Get synapse property
   *
   * The synapse property, i.e., synaptic weight or delay, is either set from a HDF5 dataset or CSV entry.
   * Default value is NaN
   *
   * @param syn_spec synapse specification dictionary
   * @param index the index to access in data
   * @param dataset_exists bool indicating whether dataset exists
   * @param data data from synaptic property HDF5 dataset
   * @param name name of the synaptic property
   * @return double
   */
  double get_syn_property_( const DictionaryDatum& syn_spec,
    hsize_t index,
    const bool dataset_exists,
    std::vector< double >& data,
    const Name& name );

  /**
   * Manage the sequential chunkwise connections to be made
   *
   */
  void sequential_chunkwise_connector_();

  /**
   * Create connections in chunks
   *
   * @param chunk_size size of chunk to be read from datasets
   * @param offset offset from start coordinate of data selection
   */
  void connect_chunk_( const hsize_t chunk_size, const hsize_t offset );

  /**
   * Read subset of dataset into memory
   *
   * @tparam T
   * @param dataset HDF5 dataset to read
   * @param data_buf buffer to store data in memory
   * @param datatype type of data in dataset
   * @param chunk_size size of chunk to be read from dataset
   * @param offset offset from start coordinate of data selection
   */
  template < typename T >
  void read_subset_( const H5::DataSet& dataset,
    std::vector< T >& data_buf,
    H5::PredType datatype,
    hsize_t chunk_size,
    hsize_t offset );


  /**
   * Find the number of edge id groups
   *
   * @param pop_grp population group pointer
   * @param edge_id_grp_names buffer to store edge id group names
   * @return hsize_t
   */
  hsize_t find_edge_id_groups_( H5::Group* pop_grp, std::vector< std::string >& edge_id_grp_names );

  /**
   * Get the number of rows (elements) in dataset
   *
   * It is assumed that the dataset is one dimensional, which is the standard
   * SONATA format
   *
   * @param dataset
   * @return hsize_t
   */
  hsize_t get_nrows_( H5::DataSet dataset );

  /**
   * Reset all parameters
   */
  void reset_params_();

  //! Dictionary containing SONATA dynamics
  DictionaryDatum graph_specs_;

  //! Size of chunk
  hsize_t chunk_size_;

  //! Indicates whether weights are given as dataset in SONATA file
  bool weight_dataset_exist_;

  //! Indicates whether delays are given as dataset in SONATA file
  bool delay_dataset_exist_;

  //! Source node attribute
  std::string source_attribute_value_;

  //! Target node attribute
  std::string target_attribute_value_;

  //! Edge parameters
  DictionaryDatum cur_edge_params_;

  //! Map from type id (in SONATA file) to synapse model
  std::map< int, index > type_id_2_syn_model_;

  //! Map from type id (in SONATA file) to synapse dictionary with ConnParameter's (one per thread)
  std::map< int, std::vector< std::map< Name, std::shared_ptr< ConnParameter > > > > type_id_2_syn_spec_;

  //! Map from type id (in SONATA file) to param dictionaries (one per thread) used when creating connections
  std::map< int, std::vector< DictionaryDatum > > type_id_2_param_dicts_;

  //! Datasets
  std::string cur_fname_;
  H5::DataSet src_node_id_dset_;
  H5::DataSet tgt_node_id_dset_;
  H5::DataSet edge_type_id_dset_;
  H5::DataSet syn_weight_dset_;
  H5::DataSet delay_dset_;
};


} // namespace nest

#endif // ifdef HAVE_HDF5

#endif /* ifdef SONATA_CONNECTOR_H */
