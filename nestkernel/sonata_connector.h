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
 * The nodes have to first be created through the SonataConnector PyNEST class, and
 * provided through the `sonata_dynamics` dictionary, along with an array providing
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
  SonataConnector( const DictionaryDatum& sonata_dynamics );
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
   * Get number of elements in a dataset
   *
   * @param dataset dataset for which to get number of elements
   *
   * @return number of elements
   */
  hsize_t get_num_elements_( const H5::DataSet& dataset );

  /**
   * Read data in a dataset
   *
   * @param dataset dataset to read
   * @param num_elements number of elements in dataset
   *
   * @returns pointer object to data in dataset
   */
  int* read_data_( const H5::DataSet& dataset, hsize_t num_elements );

  // TODO: remove, only used once
  int* get_data_( const H5::Group& group, const std::string& name );

  /**
   * Get attribute of dataset
   *
   * @param attribute_value value of attribute
   * @param dataset dataset to get attribute from
   * @param attribute_name name of attribute
   */
  void get_attributes_( std::string& attribute_value, const H5::DataSet& dataset, const std::string& attribute_name );

  /**
   * Check if weight or delay are given as datasets
   *
   * If weight or delay are given as datasets, the respective weight_dataset_ and delay_dataset_ are updated.
   *
   * TODO: change name to weight_or_delay_from_dataset
   *
   * @param group H5 group for which to check if weight or delay exists
   */
  void is_weight_and_delay_from_dataset_( const H5::Group& group );

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
  void set_synapse_params( DictionaryDatum syn_dict, index synapse_model_id, int type_id );

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

  template < typename T >
  void read_subset_( const H5::DataSet& dataset,
    std::vector< T >& data_buf,
    H5::PredType datatype,
    hsize_t chunk_size,
    hsize_t offset );

  /*
  void read_subset_int_( const H5::DataSet& dataset, std::vector< int >& data_buf, hsize_t chunk_size, hsize_t offset );

  void read_subset_double_( const H5::DataSet& dataset,
    std::vector< double >& data_buf,
    hsize_t chunk_size,
    hsize_t offset );
  */

  double get_syn_property_( const DictionaryDatum& syn_spec,
    hsize_t index,
    const bool dataset,
    std::vector< double >& data,
    const Name& name );

  void create_connections_in_chunks_( hsize_t num_conn, hsize_t chunk_size );

  void create_connections_with_indices_();

  void create_connections_with_indices_deprecated_();

  void create_connections_with_indices_dev_();

  void connect_subset_( const hsize_t chunk_size, const hsize_t offset );

  void get_member_names_( hid_t loc_id, std::vector< std::string >& names );

  hsize_t find_edge_id_groups_( const H5::Group& population_group, std::vector< std::string >& edge_id_group_names );

  void try_to_load_tgt_indices_dsets_( H5::Group population_group );

  void open_dsets_( H5::Group population_group, H5::Group edge_id_group );

  void open_dsets_( H5::Group population_group, H5::Group edge_id_group, H5::Group indices_group );

  void close_dsets_();

  hsize_t get_num_connections_();

  hsize_t get_chunk_size_( hsize_t num_conn );

  hsize_t get_nrows_( H5::DataSet dataset, int ndim );

  void read_range_to_edge_id_dset_portion_( long data_buf[][ 2 ], hsize_t row_offset );


  /**
   * Reset all parameters
   */
  void reset_params();

  //! Dictionary containing SONATA dynamics
  DictionaryDatum sonata_dynamics_;

  //! Indicates whether weights are given as dataset in SONATA file
  bool weight_dataset_exist_;

  //! Indicates whether delays are given as dataset in SONATA file
  bool delay_dataset_exist_;

  //! Indicates whether indices group for target to source is present in SONATA file
  bool tgt_indices_exist_;

  //! Source node attribute
  std::string source_attribute_value_;

  //! Target node attribute
  std::string target_attribute_value_;

  //! Edge parameters
  DictionaryDatum edge_params_;

  //! Map from type id (in SONATA file) to synapse model
  std::map< int, index > type_id_2_syn_model_;

  //! Map from type id (in SONATA file) to synapse dictionary with ConnParameter's (one per thread)
  std::map< int, std::vector< std::map< Name, std::shared_ptr< ConnParameter > > > > type_id_2_syn_spec_;

  //! Map from type id (in SONATA file) to param dictionaries (one per thread) used when creating connections
  std::map< int, std::vector< DictionaryDatum > > type_id_2_param_dicts_;

  //! Datasets
  H5::DataSet src_node_id_dset_;
  H5::DataSet tgt_node_id_dset_;
  H5::DataSet edge_type_id_dset_;
  H5::DataSet syn_weight_dset_;
  H5::DataSet delay_dset_;
  H5::DataSet tgt_node_id_to_range_dset_;
  H5::DataSet tgt_range_to_edge_id_dset_;

  //! Profiling
  Stopwatch create_arrays_stopwatch_;
  Stopwatch read_subsets_stopwatch_;
  std::vector< Stopwatch > connect_stopwatches_;
  std::vector< double > connect_times_;
};


} // namespace nest

#endif // ifdef HAVE_HDF5

#endif /* ifdef SONATA_CONNECTOR_H */
