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
#include <set>
#include <vector>

// Includes from nestkernel:
#include "conn_parameter.h"
#include "nest_datums.h"

#include "H5Cpp.h"

namespace nest
{

/**
 * @brief Manage connection creation from SONATA specifications.
 *
 * This class provides an interface for creating connections between the
 * nodes in a network specified by the SONATA format. The interface is
 * used by `SonataNetwork` PyNEST class. The nodes must first be created
 * by `SonataNetwork` before any connections can be created. Connections
 * between sources and targets are explicitly tabulated in HDF5 files
 * specified in the `graph_specs` dictionary. The connections are created
 * with the `SonataConnector::connect()` function, which iterates the SONATA
 * edge files, extracts the connection data and creates the specified
 * one-to-one connections.
 *
 * @note Files representing large-scale networks need to be read in chunks due
 * to memory constraints; we implement this using HDF5 hyperslabs of
 * configurable size. HDF5 files can only read with concurrency in an MPI
 * parallel context. Although the HDF5 library can be compiled with
 * thread-safety, it is not thread-efficient as the usage of locks effectively
 * serialize function calls. Since HDF5 does not provide support for
 * thread-parallel reading, only one thread per MPI process reads connectivity
 * data, before all threads create connections in parallel.
 */
class SonataConnector
{
public:
  /**
   * @brief Constructor
   *
   * @param graph_specs Specification dictionary, see PyNEST `SonataNetwork._create_graph_specs` for details.
   * @param hyperslab_size Size of the hyperslab to read in one read operation, applies to all HDF5 datasets.
   */
  SonataConnector( const DictionaryDatum& graph_specs, const long hyperslab_size );

  ~SonataConnector();

  /**
   * @brief Connect sources to targets according to SONATA specifications.
   */
  void connect();

private:
  /**
   * @brief Open an HDF5 edge file.
   *
   * Open an HDF5 edge file with read access only.
   *
   * @param fname Name of the edge file.
   * @return H5File pointer.
   */
  H5::H5File* open_file_( std::string& fname );

  /**
   * @brief Open an HDF5 edge file group.
   * @param file H5File pointer.
   * @param grp_name Name of the group.
   * @return H5 group pointer.
   */
  H5::Group* open_group_( const H5::H5File* file, const std::string& grp_name );

  /**
   * @brief Open an HDF5 edge file subgroup.
   * @param group H5 group.
   * @param grp_name Name of the group.
   * @return H5 subgroup pointer.
   */
  H5::Group* open_group_( const H5::Group* group, const std::string& grp_name );

  /**
   * @brief Open required datasets.
   *
   * Open HDF5 datasets required by the SONATA format.
   *
   * @param pop_grp Population group pointer.
   */
  void open_required_dsets_( const H5::Group* pop_grp );

  /**
   * @brief Open optional datasets.
   *
   * Open the optional edge group datasets if they exist.
   *
   * @param edge_grp Edge group pointer.
   */
  void try_open_edge_group_dsets_( const H5::Group* edge_grp );

  /**
   * @brief Check consistency of datasets.
   */
  void check_dsets_consistency();

  /**
   * @brief Close all open datasets.
   */
  void close_dsets_();

  /**
   * @brief Get dataset attribute.
   * @param attribute_value Address to assign value of attribute.
   * @param dataset Dataset to get attribute from.
   * @param attribute_name Name of attribute.
   */
  void get_attribute_( std::string& attribute_value, const H5::DataSet& dataset, const std::string& attribute_name );


  /**
   * @brief Create map between edge type id and syn_spec.
   *
   * Iterates edge_dict, extracts synapse parameters and creates map between
   * synapse parameters and edge type id. Checks that all synapse parameters
   * are valid parameters for the synapse.
   *
   * @param edge_dict Dictionary containing edge type ids and synapse parameters.
   */
  void create_edge_type_id_2_syn_spec_( DictionaryDatum edge_dict );


  /**
   * @brief Set synapse parameters.
   *
   * Set synapse parameters in edge_type_id_2_syn_spec_ and edge_type_id_2_param_dicts_.
   *
   * @param syn_dict Synapse dictionary from which to set synapse params.
   * @param synapse_model_id Model id of synapse
   * @param type_id SONATA edge type id for mapping synapse parameters.
   */
  void set_synapse_params_( DictionaryDatum syn_dict, size_t synapse_model_id, int type_id );

  /**
   * @brief Get synapse parameters.
   *
   * Get synapse parameters and convert them to correct datums for connecting.
   *
   * @param snode_id id of source node
   * @param target target node
   * @param target_thread thread of target
   * @param rng rng pointer of target thread
   * @param edge_type_id type id of current edge to be connected
   */
  void get_synapse_params_( size_t snode_id, Node& target, size_t target_thread, RngPtr rng, int edge_type_id );

  /**
   * @brief Get synapse property.
   *
   * The synapse property, i.e., synaptic weight or delay, is either set
   * from a HDF5 dataset or CSV entry. Default value is NaN.
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
   * @brief Manage the sequential chunkwise connections to be created.
   */
  void sequential_chunkwise_connector_();

  /**
   * @brief Create connections in chunks.
   * @param hyperslab_size Size of hyperslab (chunk) to be read from datasets
   * @param offset Offset from start coordinate of data selection
   */
  void connect_chunk_( const hsize_t hyperslab_size, const hsize_t offset );

  /**
   * @brief Read subset of dataset into memory.
   * @tparam T
   * @param dataset HDF5 dataset to read.
   * @param data_buf Buffer to store data in memory.
   * @param datatype Type of data in dataset.
   * @param hyperslab_size Size of hyperslab to be read from dataset.
   * @param offset Offset from start coordinate of data selection.
   */
  template < typename T >
  void read_subset_( const H5::DataSet& dataset,
    std::vector< T >& data_buf,
    H5::PredType datatype,
    hsize_t hyperslab_size,
    hsize_t offset );

  /**
   * @brief Find the number and names of edge groups.
   *
   * Finds the number of edge groups, i.e. ones with label "0", "1", ..., by
   * iterating the names of the population's datasets and subgroups. We
   * assume edge group labels are contiguous starting from zero, which is the
   * SONATA default.
   *
   * @note Edge group labels can also be custom and are currently not handled
   * by this function.
   *
   * @param pop_grp Population group pointer.
   * @param edge_grp_names Buffer to store edge group names.
   * @return Number of edge groups.
   */
  hsize_t find_edge_groups_( H5::Group* pop_grp, std::vector< std::string >& edge_grp_names );

  /**
   * @brief Get the number of rows (elements) in dataset.
   *
   * It is assumed that the connectivity datasets are one dimensional, which is
   * the standard of the SONATA format.
   *
   * @param dataset
   * @return hsize_t
   */
  hsize_t get_nrows_( H5::DataSet dataset );

  /**
   * @brief Reset all parameters
   */
  void reset_params_();

  typedef std::map< Name, std::shared_ptr< ConnParameter > > ConnParameterMap;

  //! synapse-specific parameters that should be skipped when we set default synapse parameters
  std::set< Name > skip_syn_params_;

  //! Dictionary containing SONATA graph specifications
  DictionaryDatum graph_specs_;

  //! Size of hyperslab that is read into memory in one read operation. Applies to all relevant HDF5 datasets.
  hsize_t hyperslab_size_;

  //! Indicates whether weights are given as HDF5 dataset
  bool weight_dataset_exist_;

  //! Indicates whether delays are given as HDF5 dataset
  bool delay_dataset_exist_;

  //! Source node attribute
  std::string source_attribute_value_;

  //! Target node attribute
  std::string target_attribute_value_;

  //! Current edge parameters
  DictionaryDatum cur_edge_params_;

  //! Map from edge type id (SONATA specification) to synapse model
  std::map< int, size_t > edge_type_id_2_syn_model_;

  //! Map from edge type id (SONATA specification) to synapse dictionary with ConnParameter's
  std::map< int, ConnParameterMap > edge_type_id_2_syn_spec_;

  //! Map from edge type id (SONATA specification) to param dictionaries (one per thread) used when creating connections
  std::map< int, std::vector< DictionaryDatum > > edge_type_id_2_param_dicts_;

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
