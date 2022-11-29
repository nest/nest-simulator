/*
 *  sonata_connector.cpp
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

#include "sonata_connector.h"

#include "config.h"

#ifdef HAVE_HDF5

#include <cstdlib>

// Includes from nestkernel:
#include "conn_parameter.h"
#include "kernel_manager.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "dictutils.h"

#include <chrono>   // for debugging
#include <fstream>  // for debugging
#include <iostream> // for debugging
#include <stdio.h>  // for debugging

//#define PROFILE_ENABLED false // for profiling

#include "H5Cpp.h" // HDF5 C++ API header file

extern "C" herr_t get_member_names_callback_( hid_t loc_id, const char* name, const H5L_info_t* linfo, void* opdata );

namespace nest
{

// constexpr hsize_t CHUNK_SIZE = 10000;      // 1e4
// constexpr hsize_t CHUNK_SIZE = 100000;     // 1e5
// constexpr hsize_t CHUNK_SIZE = 1000000; // 1e6
// constexpr hsize_t CHUNK_SIZE = 10000000; // 1e7
// constexpr hsize_t CHUNK_SIZE = 100000000; // 1e8
constexpr hsize_t CHUNK_SIZE = 1000000000; // 1e9

SonataConnector::SonataConnector( const DictionaryDatum& sonata_dynamics )
  : sonata_dynamics_( sonata_dynamics )
  , weight_dataset_exist_( false )
  , delay_dataset_exist_( false )
  , tgt_indices_exist_( false )
{
}

SonataConnector::~SonataConnector()
{
  type_id_2_syn_spec_.clear();
}

void
SonataConnector::connect()
{

  // clang-format off
  /*
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
  */
  // clang-format on

  // TODO: Consistency checks For indices approach we can ensure that:
  // - range_to_edge_id has the expected shape after tallying the ranges from node_id_to_range
  // - target_node_id has the expected shape after tallying the number of connections from range_to_edge_id;
  // the remaining datasets then have to be of correct shape because of the above checks

  const auto this_rank = kernel().mpi_manager.get_rank(); // for debugging
  const std::string dbg_filename = "dbg_" + std::to_string( this_rank ) + ".txt";
  std::ofstream dbg_file( dbg_filename );


  auto edges = getValue< ArrayDatum >( sonata_dynamics_->lookup( "edges" ) );


  // Iterate edge files
  for ( auto edge_dictionary_datum : edges )
  {

    const auto edge_dict = getValue< DictionaryDatum >( edge_dictionary_datum );
    fname_ = getValue< std::string >( edge_dict->lookup( "edges_file" ) );

    dbg_file << "Edge file: " << fname_ << "\n";
    std::cerr << "[Rank " << this_rank << "] Edge file: " << fname_ << "\n";

    // Create map of edge type ids to NEST synapse_model ids
    edge_params_ = getValue< DictionaryDatum >( edge_dict->lookup( "edge_synapse" ) );
    create_type_id_2_syn_spec_( edge_params_ );

    // Open edge file
    const auto file = open_file_();

    // Open top-level group (always one group named 'edges')
    const auto edges_grp = open_group_( file, "edges" );

    // Get names of population groups (usually just one population group)
    std::vector< std::string > pop_names;
    get_member_names_( edges_grp->getId(), pop_names );

    // Iterate the population groups
    for ( const auto& pop_name : pop_names )
    {
      // Open population group
      const auto pop_grp = open_group_( edges_grp, pop_name );

      // Find the number of edge id groups and edge id group names
      // NOTE: current find_edge_id_groups_() is only meant as a temporary helper function
      std::vector< std::string > edge_id_grp_names;
      const auto num_edge_id_groups = find_edge_id_groups_( pop_grp, edge_id_grp_names );

      // std::cerr << "num_edge_id_groups: " << num_edge_id_groups << "\n";

      // Currently only SONATA edge files with one edge id group is supported
      // TODO: Handle more than one edge id group. Check with Allen whether we
      // can require numeric keys, i.e. 0, 1, 2, ..., for edge id groups
      if ( num_edge_id_groups != 1 )
      {
        throw NotImplemented(
          "Connecting with SONATA files with more than one edge id group is currently not implemented" );
      }

      // Open edge id group
      const auto edge_id_grp = open_group_( pop_grp, edge_id_grp_names[ 0 ] );

      // Open datasets
      open_required_dsets_( pop_grp );
      try_open_edge_group_id_dsets_( edge_id_grp );
      try_open_indices_dsets_( pop_grp );

      // Retrieve source and target attributes to find which node population to map to
      get_attributes_( source_attribute_value_, src_node_id_dset_, "node_population" );
      get_attributes_( target_attribute_value_, tgt_node_id_dset_, "node_population" );

      // TODO: Remove below flag; only present for debugging
      // tgt_indices_exist_ = false;

      // Ensure datasets have expected shape
      check_dsets_consistency_();

      // select read method; either by indices or sequentially in chunks
      if ( tgt_indices_exist_ )
      {
        read_indices_dev_( dbg_file );
        // create_connections_with_indices_();
        // create_connections_with_indices_dev_();
      }
      else
      {
        const auto num_conn = get_num_connections_();
        const auto chunk_size = get_chunk_size_( num_conn );
        create_connections_in_chunks_( num_conn, chunk_size );
      }

      // Close datasets
      close_dsets_();

      // Reset all parameters
      reset_params();

    } // end iteration over population groups

    // Close H5 objects in scope
    edges_grp->close();
    file->close();

  } // end iteration over edge files
}


H5::H5File*
SonataConnector::open_file_()
{
  H5::H5File* file = nullptr;
  try
  {
    file = new H5::H5File( fname_, H5F_ACC_RDONLY );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Could not open HDF5 file " + fname_ + ": " + e.getDetailMsg() );
  }
  return file;
}

H5::Group*
SonataConnector::open_group_( const H5::H5File* file, const std::string& grp_name )
{
  H5::Group* group = nullptr;
  try
  {
    group = new H5::Group( file->openGroup( grp_name ) );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Could not open HDF5 group " + grp_name + " in " + fname_ + ": " + e.getDetailMsg() );
  }
  return group;
}

H5::Group*
SonataConnector::open_group_( const H5::Group* group, const std::string& grp_name )
{
  H5::Group* subgroup = nullptr;
  try
  {
    subgroup = new H5::Group( group->openGroup( grp_name ) );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Could not open HDF5 group " + grp_name + " in " + fname_ + ": " + e.getDetailMsg() );
  }
  return subgroup;
}


void
SonataConnector::open_required_dsets_( const H5::Group* pop_grp )
{
  try
  {
    src_node_id_dset_ = pop_grp->openDataSet( "source_node_id" );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Could not open source_node_id dataset in " + fname_ + ": " + e.getDetailMsg() );
  }

  try
  {
    tgt_node_id_dset_ = pop_grp->openDataSet( "target_node_id" );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Could not open target_node_id dataset in " + fname_ + ": " + e.getDetailMsg() );
  }

  try
  {
    edge_type_id_dset_ = pop_grp->openDataSet( "edge_type_id" );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Could not open edge_type_id dataset in " + fname_ + ": " + e.getDetailMsg() );
  }

  // Consistency checks
  const auto num_tgt_node_ids = get_nrows_( tgt_node_id_dset_, 1 );

  // Ensure that target and source population have the same size
  if ( num_tgt_node_ids != get_nrows_( src_node_id_dset_, 1 ) )
  {
    throw KernelException( "target_node_id and source_node_id datasets in " + fname_ + " must be of the same size" );
  }

  // Ensure that edge_type_id dataset size is consistent with the number of target node ids
  if ( num_tgt_node_ids != get_nrows_( edge_type_id_dset_, 1 ) )
  {
    throw KernelException( "target_node_id and edge_type_id datasets in " + fname_ + " must be of the same size" );
  }
}

void
SonataConnector::try_open_edge_group_id_dsets_( const H5::Group* edge_id_grp )
{
  // TODO: Currently only works if the edge file has a single edge id group

  // Try to open synaptic weight and delay datasets
  weight_dataset_exist_ = H5Lexists( edge_id_grp->getId(), "syn_weight", H5P_DEFAULT ) > 0;
  delay_dataset_exist_ = H5Lexists( edge_id_grp->getId(), "delay", H5P_DEFAULT ) > 0;

  if ( weight_dataset_exist_ )
  {
    try
    {
      syn_weight_dset_ = edge_id_grp->openDataSet( "syn_weight" );
    }
    catch ( const H5::Exception& e )
    {
      throw KernelException( "Could not open syn_weight dataset in " + fname_ + ": " + e.getDetailMsg() );
    }
  }

  if ( delay_dataset_exist_ )
  {
    try
    {
      delay_dset_ = edge_id_grp->openDataSet( "delay" );
    }
    catch ( const H5::Exception& e )
    {
      throw KernelException( "Could not open delay dataset in " + fname_ + ": " + e.getDetailMsg() );
    }
  }

  // TODO: If present, ensure correct size of syn_weight and delay dsets. This might not be straightforward if there
  // are multiple edge id groups
}


void
SonataConnector::try_open_indices_dsets_( const H5::Group* pop_grp )
{
  // TODO: Can groups be closed after dsets are opened?
  bool indices_grp_exist = H5Lexists( pop_grp->getId(), "indices", H5P_DEFAULT ) > 0;
  if ( indices_grp_exist )
  {
    const H5::Group indices_grp( pop_grp->openGroup( "indices" ) );

    bool tgt_to_src_grp_exist = H5Lexists( indices_grp.getId(), "target_to_source", H5P_DEFAULT ) > 0;

    if ( tgt_to_src_grp_exist )
    {
      const H5::Group tgt_to_src_indices_grp( indices_grp.openGroup( "target_to_source" ) );

      bool node_id_to_range_dset_exist =
        H5Lexists( tgt_to_src_indices_grp.getId(), "node_id_to_range", H5P_DEFAULT ) > 0;
      bool range_to_edge_id_dset_exist =
        H5Lexists( tgt_to_src_indices_grp.getId(), "range_to_edge_id", H5P_DEFAULT ) > 0;

      if ( node_id_to_range_dset_exist and range_to_edge_id_dset_exist )
      {
        try
        {
          node_id_to_range_dset_ = tgt_to_src_indices_grp.openDataSet( "node_id_to_range" );
        }
        catch ( const H5::Exception& e )
        {
          throw KernelException( "Could not open node_id_to_range dataset in " + fname_ + ": " + e.getDetailMsg() );
        }

        try
        {
          range_to_edge_id_dset_ = tgt_to_src_indices_grp.openDataSet( "range_to_edge_id" );
        }
        catch ( const H5::Exception& e )
        {
          throw KernelException( "Could not open range_to_edge_id dataset in " + fname_ + ": " + e.getDetailMsg() );
        }

        tgt_indices_exist_ = true;
      }
    }
  }
}

void
SonataConnector::is_weight_and_delay_from_dataset_( const H5::Group* group )
{
  // TODO: Can be removed as checks have been moved to try_open_edge_group_id_dsets_()
  weight_dataset_exist_ = H5Lexists( group->getId(), "syn_weight", H5P_DEFAULT ) > 0;
  delay_dataset_exist_ = H5Lexists( group->getId(), "delay", H5P_DEFAULT ) > 0;
}

void
SonataConnector::get_attributes_( std::string& attribute_value,
  const H5::DataSet& dataset,
  const std::string& attribute_name )
{
  try
  {
    H5::Attribute attr = dataset.openAttribute( attribute_name );
    H5::DataType type = attr.getDataType();
    attr.read( type, attribute_value );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException(
      "Unable to read attribute of source_node_id or target_node_id in " + fname_ + ": " + e.getDetailMsg() );
  }
}

void
SonataConnector::close_dsets_()
{
  src_node_id_dset_.close();
  src_node_id_dset_.close();
  edge_type_id_dset_.close();

  if ( weight_dataset_exist_ )
  {
    syn_weight_dset_.close();
  }

  if ( delay_dataset_exist_ )
  {
    delay_dset_.close();
  }

  if ( tgt_indices_exist_ )
  {
    node_id_to_range_dset_.close();
    range_to_edge_id_dset_.close();
  }
}

void
SonataConnector::check_dsets_consistency_()
{

  const auto num_tgt_node_ids = get_nrows_( tgt_node_id_dset_, 1 );

  // Ensure that target and source population have the same size
  if ( num_tgt_node_ids != get_nrows_( src_node_id_dset_, 1 ) )
  {
    throw KernelException( "target_node_id and source_node_id datasets in " + fname_ + " must be of the same size" );
  }

  // Ensure that edge_type_id dataset size is consistent with the number of target node ids
  if ( num_tgt_node_ids != get_nrows_( edge_type_id_dset_, 1 ) )
  {
    throw KernelException( "target_node_id and edge_type_id datasets in " + fname_ + " must be of the same size" );
  }


  if ( tgt_indices_exist_ )
  {
    // Retrieve target NodeCollection
    const auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
    const auto tgt_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value_ ) );
    const auto tgt_nc_size = tgt_nc->size();

    // Ensure that the size of target NodeCollection is consistent with number of entries in node_id_to_range dataset
    if ( tgt_nc_size != get_nrows_( node_id_to_range_dset_, 2 ) )
    {
      throw KernelException(
        "The number of target nodes in NodeCollection does not match the number of node_id_to_range rows in "
        + fname_ );
    }
  }
}

void
SonataConnector::read_indices_dev_( std::ofstream& dbg_file )
{
  const auto this_rank = kernel().mpi_manager.get_rank(); // for debugging

  // Retrieve global and local NodeCollections for target
  const auto nest_global_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
  const auto nc_global = getValue< NodeCollectionPTR >( nest_global_nodes->lookup( target_attribute_value_ ) );
  const auto nest_local_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "local_nodes" ) );
  const auto nc_local = getValue< NodeCollectionPTR >( nest_local_nodes->lookup( target_attribute_value_ ) );

  const auto src_nc_global = getValue< NodeCollectionPTR >( nest_global_nodes->lookup( source_attribute_value_ ) );
  const auto snode_begin = src_nc_global->begin();

  // Ensure that the size of global NC is consistent with number of entries in node_id_to_range dataset
  if ( nc_global->size() != get_nrows_( node_id_to_range_dset_, 2 ) )
  {
    throw KernelException(
      "The number of target nodes in NodeCollection does not match the number of node_id_to_range rows in " + fname_ );
  }

  const auto nc_size = nc_local->size();

  const auto first_node_gid = ( *nc_global->begin() ).node_id;
  const auto first_node_lid = ( *nc_local->begin() ).node_id;

  // First SONATA node id
  const auto first_node_sid = first_node_lid - first_node_gid;

  // Somewhat hacky way of retrieving the NC step, will be fixed by #2518
  const auto second_node_lid = ( *++nc_local->begin() ).node_id;
  const auto nc_step = second_node_lid - first_node_lid;


  // Read node_id_to_range dset
  // TODO: perhaps rename node_id_to_range_* variables to something like nid2rge_*

  /*
  auto node_id_to_range_data = new unsigned long[ nc_size ][ 2 ];
  hsize_t node_id_to_range_offset[ 2 ] = { first_node_sid, 0 }; // origin of the hyperslab in dataspace
  hsize_t node_id_to_range_stride[ 2 ] = { nc_step, 1 }; // number of elements to increment between selected elements
  hsize_t node_id_to_range_count[ 2 ] = { nc_size, 2 };  // number of elements in the hyperslab selection
  const hsize_t node_id_to_range_dimsm[ 2 ] = { nc_size, 2 }; // memory space dimension
  H5::DataSpace node_id_to_range_dspace = node_id_to_range_dset_.getSpace();
  H5::DataSpace node_id_to_range_memspace( 2, node_id_to_range_dimsm );
  */
  const auto node_id_to_range_nrows = get_nrows_( node_id_to_range_dset_, 2 );
  auto node_id_to_range_data = new unsigned long[ node_id_to_range_nrows ][ 2 ];

  try
  {
    /*
    node_id_to_range_dspace.selectHyperslab(
      H5S_SELECT_SET, node_id_to_range_count, node_id_to_range_offset, node_id_to_range_stride );
    node_id_to_range_dset_.read(
      node_id_to_range_data, H5::PredType::NATIVE_LONG, node_id_to_range_memspace, node_id_to_range_dspace );
      */

    node_id_to_range_dset_.read( node_id_to_range_data, H5::PredType::NATIVE_LONG );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Unable to read node_id_to_range dataset in " + fname_ + ": " + e.getDetailMsg() );
  }
  // node_id_to_range_dspace.close();
  // node_id_to_range_memspace.close();

  // Read range_to_edge_id
  const auto range_to_edge_id_nrows = get_nrows_( range_to_edge_id_dset_, 2 );
  auto range_to_edge_id_data = new unsigned long[ range_to_edge_id_nrows ][ 2 ];
  try
  {
    range_to_edge_id_dset_.read( range_to_edge_id_data, H5::PredType::NATIVE_LONG );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Unable to read range_to_edge_id dataset in " + fname_ + ": " + e.getDetailMsg() );
  }

  // CONNECT
  // Get dataspaces
  H5::DataSpace src_node_id_dspace = src_node_id_dset_.getSpace();
  H5::DataSpace tgt_node_id_dspace = tgt_node_id_dset_.getSpace();
  H5::DataSpace edge_type_id_dspace = edge_type_id_dset_.getSpace();
  H5::DataSpace syn_weight_dspace;
  H5::DataSpace delay_dspace;
  if ( weight_dataset_exist_ )
  {
    syn_weight_dspace = syn_weight_dset_.getSpace();
  }
  if ( delay_dataset_exist_ )
  {
    delay_dspace = delay_dset_.getSpace();
  }

  hsize_t offset_data[ 1 ]; // origin of the hyperslab in dataspace
  hsize_t count_data[ 1 ];  // number of elements in the hyperslab selection
  hsize_t count_mem[ 1 ];

  size_t n_conns_local = 0;
  size_t n_conns_cur;
  size_t n_conns_tnode;
  auto tgt_it = nc_local->begin();

  for ( ; tgt_it < nc_local->end(); ++tgt_it )
  {
    const index tnode_id = ( *tgt_it ).node_id;
    auto tnode_sid = tnode_id - first_node_gid;
    // dbg_file << "tnode_id: " << tnode_id << ", tnode_sid: " << tnode_sid << "\n";


    auto range_start_idx = node_id_to_range_data[ tnode_sid ][ 0 ];
    auto range_end_idx = node_id_to_range_data[ tnode_sid ][ 1 ];

    // dbg_file << "range_start_idx: " << range_start_idx << ", range_end_idx: " << range_end_idx << "\n";


    if ( range_start_idx < 0 )
    {
      continue;
    }

    if ( ( range_start_idx == 0 ) and ( range_end_idx == 0 ) )
    {
      continue;
    }

    if ( range_start_idx >= range_end_idx )
    {
      continue;
    }


    // dbg_file << "range_start_idx: " << range_start_idx << ", range_end_idx: " << range_end_idx << "\n";

    n_conns_tnode = 0;
    src_node_id_dspace.selectNone(); // resets the selection region to include no elements
    tgt_node_id_dspace.selectNone();
    edge_type_id_dspace.selectNone();
    if ( weight_dataset_exist_ )
    {
      syn_weight_dspace.selectNone();
    }
    if ( delay_dataset_exist_ )
    {
      delay_dspace.selectNone();
    }

    for ( auto i = range_start_idx; i < range_end_idx; ++i )
    {


      auto start_edge_id = range_to_edge_id_data[ i ][ 0 ];
      auto end_edge_id = range_to_edge_id_data[ i ][ 1 ];


      n_conns_cur = end_edge_id - start_edge_id;
      n_conns_tnode += n_conns_cur;
      n_conns_local += n_conns_cur; // debugging


      // std::vector< long > src_node_id_data( n_conns_cur );
      // std::vector< long > tgt_node_id_data( n_conns_cur );
      // std::vector< long > edge_type_id_data( n_conns_cur );

      // Select dataspace hyperslab
      offset_data[ 0 ] = start_edge_id;
      count_data[ 0 ] = n_conns_cur;

      // H5::DataSpace memspace( 1, count_data );
      // memspace.selectAll();

      auto coord = new hsize_t[ n_conns_cur ];

      // Setting values
      hsize_t counter = 0;
      for ( auto j = 0; j < n_conns_cur; ++j )
      {
        coord[ j ] = start_edge_id + counter;
        counter++;
      }

      try
      {
        src_node_id_dspace.selectElements( H5S_SELECT_APPEND, n_conns_cur, coord );
        tgt_node_id_dspace.selectElements( H5S_SELECT_APPEND, n_conns_cur, coord );
        edge_type_id_dspace.selectElements( H5S_SELECT_APPEND, n_conns_cur, coord );
        if ( weight_dataset_exist_ )
        {
          syn_weight_dspace.selectElements( H5S_SELECT_APPEND, n_conns_cur, coord );
        }
        if ( delay_dataset_exist_ )
        {
          delay_dspace.selectElements( H5S_SELECT_APPEND, n_conns_cur, coord );
        }
      }
      catch ( const H5::Exception& e )
      {
        throw KernelException( "Unable to select elements in connection dsets in" + fname_ + ": " + e.getDetailMsg() );
      }
      delete[] coord;

      // src_node_id_dspace.selectHyperslab( H5S_SELECT_OR, count_data, offset_data );


      // Read dataset


      /*
      try
      {
        src_node_id_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
        src_node_id_dset_.read( src_node_id_data.data(), H5::PredType::NATIVE_LONG, memspace, src_node_id_dspace );
      }
      catch ( const H5::Exception& e )
      {
        throw KernelException( "Unable to read source_node_id dataset in " + fname_ + ": " + e.getDetailMsg() );
      }

      try
      {
        tgt_node_id_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
        tgt_node_id_dset_.read( tgt_node_id_data.data(), H5::PredType::NATIVE_LONG, memspace, tgt_node_id_dspace );
      }
      catch ( const H5::Exception& e )
      {
        throw KernelException( "Unable to read target_node_id dataset in " + fname_ + ": " + e.getDetailMsg() );
      }

      try
      {
        edge_type_id_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
        edge_type_id_dset_.read( edge_type_id_data.data(), H5::PredType::NATIVE_LONG, memspace, edge_type_id_dspace );
      }
      catch ( const H5::Exception& e )
      {
        throw KernelException( "Unable to read edge_type_id dataset in " + fname_ + ": " + e.getDetailMsg() );
      }
      */


    } // end range-loop

    std::vector< long > src_node_id_data( n_conns_tnode );
    std::vector< long > tgt_node_id_data( n_conns_tnode );
    std::vector< long > edge_type_id_data( n_conns_tnode );
    std::vector< double > syn_weight_data( n_conns_tnode );
    std::vector< double > delay_data( n_conns_tnode );

    count_mem[ 0 ] = n_conns_tnode;
    H5::DataSpace memspace( 1, count_mem );
    memspace.selectAll();

    try
    {
      src_node_id_dset_.read( src_node_id_data.data(), H5::PredType::NATIVE_LONG, memspace, src_node_id_dspace );
    }
    catch ( const H5::Exception& e )
    {
      throw KernelException( "Unable to read source_node_id dataset in " + fname_ + ": " + e.getDetailMsg() );
    }

    try
    {
      tgt_node_id_dset_.read( tgt_node_id_data.data(), H5::PredType::NATIVE_LONG, memspace, tgt_node_id_dspace );
    }
    catch ( const H5::Exception& e )
    {
      throw KernelException( "Unable to read target_node_id dataset in " + fname_ + ": " + e.getDetailMsg() );
    }

    try
    {

      edge_type_id_dset_.read( edge_type_id_data.data(), H5::PredType::NATIVE_LONG, memspace, edge_type_id_dspace );
    }
    catch ( const H5::Exception& e )
    {
      throw KernelException( "Unable to read edge_type_id dataset in " + fname_ + ": " + e.getDetailMsg() );
    }

    if ( weight_dataset_exist_ )
    {
      try
      {
        syn_weight_dset_.read( syn_weight_data.data(), H5::PredType::NATIVE_DOUBLE, memspace, syn_weight_dspace );
      }
      catch ( const H5::Exception& e )
      {
        throw KernelException( "Unable to read syn_weight dataset in " + fname_ + ": " + e.getDetailMsg() );
      }
    }
    if ( delay_dataset_exist_ )
    {
      try
      {
        delay_dset_.read( delay_data.data(), H5::PredType::NATIVE_DOUBLE, memspace, delay_dspace );
      }
      catch ( const H5::Exception& e )
      {
        throw KernelException( "Unable to read delay dataset in " + fname_ + ": " + e.getDetailMsg() );
      }
    }

    memspace.close();


#pragma omp parallel
    {
      // check if target node is vp local first
      if ( kernel().vp_manager.is_node_id_vp_local( tnode_id ) )
      {
        const auto tid = kernel().vp_manager.get_thread_id();
        RngPtr rng = get_vp_specific_rng( tid );

        for ( auto k = 0; k < n_conns_tnode; ++k )
        {
          const auto sonata_src_node_id = src_node_id_data[ k ];
          const index snode_id = ( *( snode_begin + sonata_src_node_id ) ).node_id;

          Node* target = kernel().node_manager.get_node_or_proxy( tnode_id, tid );
          const thread target_thread = target->get_thread();

          const auto edge_type_id = edge_type_id_data[ k ];
          const auto syn_spec = getValue< DictionaryDatum >( edge_params_->lookup( std::to_string( edge_type_id ) ) );
          const double weight = get_syn_property_( syn_spec, k, weight_dataset_exist_, syn_weight_data, names::weight );
          const double delay = get_syn_property_( syn_spec, k, delay_dataset_exist_, delay_data, names::delay );

          get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

          kernel().connection_manager.connect( snode_id,
            target,
            target_thread,
            type_id_2_syn_model_.at( edge_type_id ),
            type_id_2_param_dicts_.at( edge_type_id ).at( tid ),
            delay,
            weight );
        }
      }
    }

  } // end loop over target nodes

  src_node_id_dspace.close();
  tgt_node_id_dspace.close();
  edge_type_id_dspace.close();
  if ( weight_dataset_exist_ )
  {
    syn_weight_dspace.close();
  }
  if ( delay_dataset_exist_ )
  {
    delay_dspace.close();
  }


  /*
  auto start_edge_id = tgt_range_to_edge_id_data[ i ][ 0 ];
    auto end_edge_id = tgt_range_to_edge_id_data[ i ][ 1 ];
    auto n_rows_tmp = end_edge_id - start_edge_id;

    // Select dataspace hyperslab
    hsize_t offset_data[ 1 ]; // origin of the hyperslab in dataspace
    hsize_t count_data[ 1 ];  // number of elements in the hyperslab selection
    //'block' is implicitly given as NULL.
    offset_data[ 0 ] = start_edge_id;
    count_data[ 0 ] = n_rows_tmp;

    src_node_id_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
    tgt_node_id_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
    edge_type_id_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );

    // Select memoryspace hyperslab
    hsize_t offset_mem[ 1 ]; // hyperslab offset in memory
    hsize_t count_mem[ 1 ];  // size of the hyperslab in memory
    offset_mem[ 0 ] = offset_mem_counter;
    count_mem[ 0 ] = n_rows_tmp;
    memspace.selectHyperslab( H5S_SELECT_SET, count_mem, offset_mem );

    // Read dataset
    src_node_id_dset_.read( src_node_id_data.data(), H5::PredType::NATIVE_INT, memspace, src_node_id_dspace );
    tgt_node_id_dset_.read( tgt_node_id_data.data(), H5::PredType::NATIVE_INT, memspace, tgt_node_id_dspace );
    edge_type_id_dset_.read( edge_type_id_data.data(), H5::PredType::NATIVE_INT, memspace, edge_type_id_dspace );

    if ( weight_dataset_exist_ )
    {
      syn_weight_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
      syn_weight_dset_.read( syn_weight_data.data(), H5::PredType::NATIVE_DOUBLE, memspace, syn_weight_dspace );
    }
    if ( delay_dataset_exist_ )
    {
      delay_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
      delay_dset_.read( delay_data.data(), H5::PredType::NATIVE_DOUBLE, memspace, delay_dspace );
    }
  */

  std::cerr << "n_conns_local: " << n_conns_local << "\n";

  delete[] node_id_to_range_data;
  delete[] range_to_edge_id_data;
}

void
SonataConnector::read_indices_dev_depr_()
{
  // DEPRECATED

  const auto this_rank = kernel().mpi_manager.get_rank(); // for debugging

  // Retrieve global and local NodeCollections for target
  const auto nest_global_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
  const auto nc_global = getValue< NodeCollectionPTR >( nest_global_nodes->lookup( target_attribute_value_ ) );
  const auto nest_local_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "local_nodes" ) );
  const auto nc_local = getValue< NodeCollectionPTR >( nest_local_nodes->lookup( target_attribute_value_ ) );

  // Ensure that the size of global NC is consistent with number of entries in node_id_to_range dataset
  if ( nc_global->size() != get_nrows_( node_id_to_range_dset_, 2 ) )
  {
    throw KernelException(
      "The number of target nodes in NodeCollection does not match the number of node_id_to_range rows in " + fname_ );
  }

  const auto nc_size = nc_local->size();

  const auto first_node_gid = ( *nc_global->begin() ).node_id;
  const auto first_node_lid = ( *nc_local->begin() ).node_id;

  // First SONATA node id
  const auto first_node_sid = first_node_lid - first_node_gid;

  // Somewhat hacky way of retrieving the NC step, will be fixed by #2518
  const auto second_node_lid = ( *++nc_local->begin() ).node_id;
  const auto nc_step = second_node_lid - first_node_lid;
  std::cerr << "[Rank " << this_rank << "] nc_size: " << nc_size << "\n";
  std::cerr << "[Rank " << this_rank << "] nc_step: " << nc_step << "\n";
  std::cerr << "[Rank " << this_rank << "] first_node_sid: " << first_node_sid << "\n";


  // Read node_id_to_range dset
  // TODO: perhaps rename node_id_to_range_* variables to something like nid2rge_*
  const hsize_t node_id_to_range_dim = nc_size * 2;
  std::vector< long > node_id_to_range_data( node_id_to_range_dim ); // data buffer
  hsize_t node_id_to_range_offset[ 2 ] = { first_node_sid, 0 };      // origin of the hyperslab in dataspace
  hsize_t node_id_to_range_stride[ 2 ] = { nc_step, 1 }; // number of elements to increment between selected elements
  hsize_t node_id_to_range_count[ 2 ] = { nc_size, 2 };  // number of elements in the hyperslab selection
  const hsize_t node_id_to_range_dimsm[ 1 ] = { node_id_to_range_dim }; // memory space dimension
  H5::DataSpace node_id_to_range_dspace = node_id_to_range_dset_.getSpace();
  H5::DataSpace node_id_to_range_memspace( 1, node_id_to_range_dimsm );

  try
  {
    node_id_to_range_dspace.selectHyperslab(
      H5S_SELECT_SET, node_id_to_range_count, node_id_to_range_offset, node_id_to_range_stride );
    node_id_to_range_dset_.read(
      node_id_to_range_data.data(), H5::PredType::NATIVE_LONG, node_id_to_range_memspace, node_id_to_range_dspace );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Unable to read node_id_to_range dataset in " + fname_ + ": " + e.getDetailMsg() );
  }
  node_id_to_range_dspace.close();
  node_id_to_range_memspace.close();


  // Read range_to_edge_id dset (portion belonging to MPI process only)
  H5::DataSpace range_to_edge_id_dspace = range_to_edge_id_dset_.getSpace();
  range_to_edge_id_dspace.selectNone();            // resets the selection region to include no elements
  hsize_t range_to_edge_id_offset[ 2 ] = { 0, 0 }; // origin of the hyperslab in dataspace
  hsize_t range_to_edge_id_count[ 2 ] = { 0, 2 };  // number of elements in the hyperslab selection
  size_t range_to_edge_id_nrows = 0;               // to determine the size of data buffer

  // Create union of hyperslab selections
  for ( size_t i = 0; i < nc_size; ++i )
  {
    auto range_start_idx = node_id_to_range_data[ i * 2 ];   // first column
    auto range_end_idx = node_id_to_range_data[ i * 2 + 1 ]; // second column

    // We need to skip if there are no edges specified by the current edge file
    // for the associated node id. According to documentation, the start index
    // should then be a negative value, however some files also indicate non-existence
    // by assigning both start and end index value zero
    if ( range_start_idx < 0 )
    {
      continue;
    }

    if ( ( range_start_idx == 0 ) and ( range_end_idx == 0 ) )
    {
      continue;
    }

    if ( range_start_idx >= range_end_idx )
    {
      continue;
    }

    auto cur_nrows = range_end_idx - range_start_idx;
    range_to_edge_id_nrows += cur_nrows; // increment data buffer size counter

    range_to_edge_id_offset[ 0 ] = range_start_idx;
    range_to_edge_id_count[ 0 ] = cur_nrows;

    try
    {
      // H5S_SELECT_OR adds new selection to existing selection
      range_to_edge_id_dspace.selectHyperslab( H5S_SELECT_OR, range_to_edge_id_count, range_to_edge_id_offset );
    }
    catch ( const H5::Exception& e )
    {
      throw KernelException(
        "range_to_edge_id dataset hyperslab selection failed for " + fname_ + ": " + e.getDetailMsg() );
    }
  }

  const hsize_t range_to_edge_id_dim = range_to_edge_id_nrows * 2;
  std::vector< long > range_to_edge_id_data( range_to_edge_id_nrows * 2 ); // data buffer
  const hsize_t range_to_edge_id_dimsm[ 1 ] = { range_to_edge_id_dim };    // memory space dimension
  H5::DataSpace range_to_edge_id_memspace( 1, range_to_edge_id_dimsm );

  try
  {
    range_to_edge_id_dset_.read(
      range_to_edge_id_data.data(), H5::PredType::NATIVE_LONG, range_to_edge_id_memspace, range_to_edge_id_dspace );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Unable to read range_to_edge_id dataset in " + fname_ + ": " + e.getDetailMsg() );
  }
  range_to_edge_id_dspace.close();
  range_to_edge_id_memspace.close();


  // Read connection dsets
  // Get dataspaces
  H5::DataSpace src_node_id_dspace = src_node_id_dset_.getSpace();
  H5::DataSpace tgt_node_id_dspace = tgt_node_id_dset_.getSpace();
  H5::DataSpace edge_type_id_dspace = edge_type_id_dset_.getSpace();
  H5::DataSpace syn_weight_dspace;
  H5::DataSpace delay_dspace;
  if ( weight_dataset_exist_ )
  {
    syn_weight_dspace = syn_weight_dset_.getSpace();
  }
  if ( delay_dataset_exist_ )
  {
    delay_dspace = delay_dset_.getSpace();
  }

  hsize_t offset_data[ 1 ]; // origin of the hyperslab in dataspace
  hsize_t count_data[ 1 ];  // number of elements in the hyperslab selection
  const hsize_t tgt_count[ 1 ] = { 1 };


  // Allocate storage for connection info
  size_t n_conns_local = 0;
  for ( size_t i = 0; i < range_to_edge_id_nrows; ++i )
  {
    auto start_edge_id = range_to_edge_id_data[ i * 2 ];   // first column
    auto end_edge_id = range_to_edge_id_data[ i * 2 + 1 ]; // second column
    auto n_conns_cur = end_edge_id - start_edge_id;
    n_conns_local += n_conns_cur;


    offset_data[ 0 ] = start_edge_id;
    count_data[ 0 ] = n_conns_cur;
    src_node_id_dspace.selectHyperslab( H5S_SELECT_OR, count_data, offset_data );
    edge_type_id_dspace.selectHyperslab( H5S_SELECT_OR, count_data, offset_data );
    // tgt_node_id_dspace.selectHyperslab( H5S_SELECT_OR, tgt_count, offset_data );
  }

  std::cerr << "n_conns_local: " << n_conns_local << "\n";
  std::vector< long > src_node_id_data( n_conns_local );
  std::vector< long > tgt_node_id_data( n_conns_local );
  std::vector< long > edge_type_id_data( n_conns_local );
  std::vector< double > syn_weight_data( n_conns_local );
  std::vector< double > delay_data( n_conns_local );
  const hsize_t conn_data_dimsm[ 1 ] = { n_conns_local }; // memory space dimension
  H5::DataSpace conn_data_memspace( 1, conn_data_dimsm );


  /*
  try
  {
    src_node_id_dset_.read(
      src_node_id_data.data(), H5::PredType::NATIVE_LONG, conn_data_memspace, src_node_id_dspace );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Unable to read source_node_id dataset in " + fname_ + ": " + e.getDetailMsg() );
  }
  src_node_id_dspace.close();
  */
  /*
      try
      {
        edge_type_id_dset_.read( edge_type_id_data.data(), H5::PredType::NATIVE_LONG, edge_type_id_dspace );
      }
      catch ( const H5::Exception& e )
      {
        throw KernelException( "Unable to read edge_type_id dataset in " + fname_ + ": " + e.getDetailMsg() );
      }
      edge_type_id_dspace.close();
      */

  // conn_data_memspace.close();
}


void
SonataConnector::create_connections_with_indices_()
{
  // TODO: Refactor this monstrosity

  const auto this_rank = kernel().mpi_manager.get_rank(); // for debugging

  // Retrieve the correct NodeCollections
  // std::cerr << "[Rank " << this_rank << "] Retrieve NodeColletions\n";
  const auto nest_global_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
  const auto tgt_nc_global = getValue< NodeCollectionPTR >( nest_global_nodes->lookup( target_attribute_value_ ) );
  const auto src_nc_global = getValue< NodeCollectionPTR >( nest_global_nodes->lookup( source_attribute_value_ ) );

  const auto nest_local_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "local_nodes" ) );
  const auto tgt_nc = getValue< NodeCollectionPTR >( nest_local_nodes->lookup( target_attribute_value_ ) );
  const auto src_nc = getValue< NodeCollectionPTR >( nest_local_nodes->lookup( source_attribute_value_ ) );

  // Retrieve NC information
  const auto tgt_nc_first_node_gid = ( *tgt_nc_global->begin() ).node_id;

  auto target_it = tgt_nc->begin();          // can tgt nc be empty? if so, how to handle? for parallel approach 1
  auto tnode_begin = tgt_nc_global->begin(); // for parallel approach 2
  const auto snode_begin = src_nc_global->begin();


  const auto tgt_nc_size = tgt_nc->size();
  // Somewhat hacky way of retrieving the NC step - perhaps create PR that exposes NC step (only getter)
  const auto tgt_nc_first_node_lid = ( *tgt_nc->begin() ).node_id;
  const auto tgt_nc_second_node_lid = ( *++tgt_nc->begin() ).node_id;
  const auto tgt_nc_step = tgt_nc_second_node_lid - tgt_nc_first_node_lid;

  // Translate NEST node id to SONATA node id
  auto sonata_first_node_id = tgt_nc_first_node_lid - tgt_nc_first_node_gid;
  std::cerr << "[Rank " << this_rank << "] tgt_nc_local_size " << tgt_nc_size << "\n";
  std::cerr << "[Rank " << this_rank << "] sonata_first_node_id " << sonata_first_node_id << "\n";

  // const auto n_sonata_node_ids = get_nrows_( tgt_node_id_to_range_dset_, 2 );
  // auto sonata_first_node_id = nest_node_id_to_sonata_node_id( tgt_nc_first_node_id, n_sonata_node_ids );


  // Read node_id_to_range dset
  std::cerr << "[Rank " << this_rank << "] read node_id_to_range\n";
  auto tgt_node_id_to_range_data = read_node_id_to_range_dset_( tgt_nc_size, tgt_nc_step, sonata_first_node_id );


  // Read range_to_edge_id dset
  std::cerr << "[Rank " << this_rank << "] read range_to_edge_id\n";
  auto tgt_range_to_edge_id_data = read_range_to_edge_id_dset_( tgt_nc_size, tgt_node_id_to_range_data );


  // Read connection dsets
  std::cerr << "[Rank " << this_rank << "] read connection dsets\n";
  auto tgt_range_to_edge_id_data_size = tgt_range_to_edge_id_data.size();
  std::cerr << "Rank " << this_rank << " size " << tgt_range_to_edge_id_data.size() << "\n";

  // Allocate storage for connection info
  size_t n_conns_local = 0;
  for ( size_t i = 0; i < tgt_range_to_edge_id_data_size; i++ )
  {
    n_conns_local += tgt_range_to_edge_id_data[ i ][ 1 ] - tgt_range_to_edge_id_data[ i ][ 0 ];
  }

  std::cerr << "Rank " << this_rank << " n_conns_local " << n_conns_local << "\n";

  std::vector< int > src_node_id_data( n_conns_local );
  std::vector< int > tgt_node_id_data( n_conns_local );
  std::vector< int > edge_type_id_data( n_conns_local );
  std::vector< double > syn_weight_data( n_conns_local );
  std::vector< double > delay_data( n_conns_local );


  // Get dataspaces
  H5::DataSpace src_node_id_dspace = src_node_id_dset_.getSpace();
  H5::DataSpace tgt_node_id_dspace = tgt_node_id_dset_.getSpace();
  H5::DataSpace edge_type_id_dspace = edge_type_id_dset_.getSpace();
  H5::DataSpace syn_weight_dspace;
  H5::DataSpace delay_dspace;
  if ( weight_dataset_exist_ )
  {
    syn_weight_dspace = syn_weight_dset_.getSpace();
  }
  if ( delay_dataset_exist_ )
  {
    delay_dspace = delay_dset_.getSpace();
  }

  // Define memoryspace
  hsize_t mem_rank = 1;
  hsize_t dimsm[ 1 ]; // = { n_conns_local }; // memory space dimension
  dimsm[ 0 ] = n_conns_local;
  H5::DataSpace memspace( mem_rank, dimsm, NULL );


  // Read connection dsets
  hsize_t offset_mem_counter = 0;
  for ( size_t i = 0; i < tgt_range_to_edge_id_data_size; i++ )
  {
    auto start_edge_id = tgt_range_to_edge_id_data[ i ][ 0 ];
    auto end_edge_id = tgt_range_to_edge_id_data[ i ][ 1 ];
    auto n_rows_tmp = end_edge_id - start_edge_id;

    // Select dataspace hyperslab
    hsize_t offset_data[ 1 ]; // origin of the hyperslab in dataspace
    hsize_t count_data[ 1 ];  // number of elements in the hyperslab selection
    //'block' is implicitly given as NULL.
    offset_data[ 0 ] = start_edge_id;
    count_data[ 0 ] = n_rows_tmp;

    src_node_id_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
    tgt_node_id_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
    edge_type_id_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );

    // Select memoryspace hyperslab
    hsize_t offset_mem[ 1 ]; // hyperslab offset in memory
    hsize_t count_mem[ 1 ];  // size of the hyperslab in memory
    offset_mem[ 0 ] = offset_mem_counter;
    count_mem[ 0 ] = n_rows_tmp;
    memspace.selectHyperslab( H5S_SELECT_SET, count_mem, offset_mem );

    // Read dataset
    src_node_id_dset_.read( src_node_id_data.data(), H5::PredType::NATIVE_INT, memspace, src_node_id_dspace );
    tgt_node_id_dset_.read( tgt_node_id_data.data(), H5::PredType::NATIVE_INT, memspace, tgt_node_id_dspace );
    edge_type_id_dset_.read( edge_type_id_data.data(), H5::PredType::NATIVE_INT, memspace, edge_type_id_dspace );

    if ( weight_dataset_exist_ )
    {
      syn_weight_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
      syn_weight_dset_.read( syn_weight_data.data(), H5::PredType::NATIVE_DOUBLE, memspace, syn_weight_dspace );
    }
    if ( delay_dataset_exist_ )
    {
      delay_dspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );
      delay_dset_.read( delay_data.data(), H5::PredType::NATIVE_DOUBLE, memspace, delay_dspace );
    }

    offset_mem_counter += n_rows_tmp;
  }

  // connect
  std::cerr << "[Rank " << this_rank << "] enter parallel region\n";
#pragma omp parallel
  {

    // two possible approaches
    // 1.
    // for ( ; target_it < tgt_nc->end(); ++target_it )
    //{
    //  const index tnode_id = ( *target_it ).node_id;
    // check if tid = tgt_thread
    //}
    // 2.
    // also read target_node_id dset and just iterate the connection info dsets


    const auto tid = kernel().vp_manager.get_thread_id();
    RngPtr rng = get_vp_specific_rng( tid );


    // approach 2
    for ( size_t i = 0; i < n_conns_local; i++ )
    {

      const auto sonata_tgt_node_id = tgt_node_id_data[ i ];
      const index tnode_id = ( *( tnode_begin + sonata_tgt_node_id ) ).node_id;

      // check if target node is vp local first
      if ( not kernel().vp_manager.is_node_id_vp_local( tnode_id ) )
      {
        continue;
      }

      const auto sonata_src_node_id = src_node_id_data[ i ];
      const index snode_id = ( *( snode_begin + sonata_src_node_id ) ).node_id;

      Node* target = kernel().node_manager.get_node_or_proxy( tnode_id, tid );
      const thread target_thread = target->get_thread();

      const auto edge_type_id = edge_type_id_data[ i ];
      const auto syn_spec = getValue< DictionaryDatum >( edge_params_->lookup( std::to_string( edge_type_id ) ) );
      const double weight = get_syn_property_( syn_spec, i, weight_dataset_exist_, syn_weight_data, names::weight );
      const double delay = get_syn_property_( syn_spec, i, delay_dataset_exist_, delay_data, names::delay );

      get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

      kernel().connection_manager.connect( snode_id,
        target,
        target_thread,
        type_id_2_syn_model_.at( edge_type_id ),
        type_id_2_param_dicts_.at( edge_type_id ).at( tid ),
        delay,
        weight );
    }
  }
}


void
SonataConnector::create_connections_in_chunks_( hsize_t num_conn, hsize_t chunk_size )
{

  // organize chunks; dv.quot = integral quotient, dv.rem = reaminder
  // https://learn.microsoft.com/en-us/cpp/c-language/cpp-integer-limits?view=msvc-170
  auto dv = std::div( static_cast< long long >( num_conn ), static_cast< long long >( chunk_size ) );

  // Iterate chunks
  hsize_t offset = 0; // start coordinates of data selection

  // TODO: should iterator also be hsize_t, and should then dv.quot & dv.rem be cast to hsize_t?

  for ( size_t i = 0; i < dv.quot; i++ )
  {
    // create connections
    connect_subset_( chunk_size, offset );
    // increment offset
    offset += chunk_size;
  } // end chunk loop

  // Handle remainder
  if ( dv.rem > 0 )
  {
    connect_subset_( dv.rem, offset );
  }
}

void
SonataConnector::connect_subset_( const hsize_t chunk_size, const hsize_t offset )
{

  // Read subsets
  std::vector< int > src_node_id_data_subset( chunk_size );
  std::vector< int > tgt_node_id_data_subset( chunk_size );
  std::vector< int > edge_type_id_data_subset( chunk_size );
  std::vector< double > syn_weight_data_subset( chunk_size );
  std::vector< double > delay_data_subset( chunk_size );

  read_subset_( src_node_id_dset_, src_node_id_data_subset, H5::PredType::NATIVE_INT, chunk_size, offset );
  read_subset_( tgt_node_id_dset_, tgt_node_id_data_subset, H5::PredType::NATIVE_INT, chunk_size, offset );
  read_subset_( edge_type_id_dset_, edge_type_id_data_subset, H5::PredType::NATIVE_INT, chunk_size, offset );

  if ( weight_dataset_exist_ )
  {
    // syn_weight_data_subset.reserve( chunk_size );
    read_subset_( syn_weight_dset_, syn_weight_data_subset, H5::PredType::NATIVE_DOUBLE, chunk_size, offset );
  }
  if ( delay_dataset_exist_ )
  {
    // delay_data_subset.reserve( chunk_size );
    read_subset_( delay_dset_, delay_data_subset, H5::PredType::NATIVE_DOUBLE, chunk_size, offset );
  }

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_( kernel().vp_manager.get_num_threads() );

  // Retrieve the correct NodeCollections
  const auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
  const auto current_source_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value_ ) );
  const auto current_target_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value_ ) );
  const auto snode_begin = current_source_nc->begin();
  const auto tnode_begin = current_target_nc->begin();

#pragma omp parallel
  {
    const auto tid = kernel().vp_manager.get_thread_id();
    const auto this_vp = kernel().vp_manager.thread_to_vp( tid );
    RngPtr rng = get_vp_specific_rng( tid );

    try
    {

      // Iterate the datasets and create the connections
      for ( hsize_t i = 0; i < chunk_size; ++i )
      {

        const auto sonata_target_id = tgt_node_id_data_subset[ i ];
        const index tnode_id = ( *( tnode_begin + sonata_target_id ) ).node_id;

        thread tgt_vp = kernel().vp_manager.node_id_to_vp( tnode_id );

        if ( tgt_vp != this_vp )
        {
          continue;
        }

        const auto sonata_source_id = src_node_id_data_subset[ i ];
        const index snode_id = ( *( snode_begin + sonata_source_id ) ).node_id;

        Node* target = kernel().node_manager.get_node_or_proxy( tnode_id, tid );
        const thread target_thread = target->get_thread();

        const auto edge_type_id = edge_type_id_data_subset[ i ];
        const auto syn_spec = getValue< DictionaryDatum >( edge_params_->lookup( std::to_string( edge_type_id ) ) );
        const double weight =
          get_syn_property_( syn_spec, i, weight_dataset_exist_, syn_weight_data_subset, names::weight );
        const double delay = get_syn_property_( syn_spec, i, delay_dataset_exist_, delay_data_subset, names::delay );

        get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

        kernel().connection_manager.connect( snode_id,
          target,
          target_thread,
          type_id_2_syn_model_.at( edge_type_id ),             // static synapse
          type_id_2_param_dicts_.at( edge_type_id ).at( tid ), // send empty param dict
          delay,                                               // fixed as 1
          weight );

      } // end for
    }   // end try

    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }

  } // end parallel region

  // check if any exceptions have been raised
  for ( thread thr = 0; thr < kernel().vp_manager.get_num_threads(); ++thr )
  {
    if ( exceptions_raised_.at( thr ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
    }
  }
} // end create_connections_()


std::vector< std::vector< int > >
SonataConnector::read_indices_dset_( H5::DataSet dataset )
{
  const auto n_rows = get_nrows_( dataset, 2 );
  const size_t n_cols = 2;

  // from https://github.com/stevenwalton/H5Easy/blob/master/H5Easy.h#L840
  auto data = new int[ n_rows * n_cols ];
  auto md = new int*[ n_rows ];
  for ( size_t i = 0; i < n_rows; ++i )
  {
    md[ i ] = data + i * n_cols;
  }

  dataset.read( data, H5::PredType::NATIVE_INT );

  std::vector< std::vector< int > > v( n_rows, std::vector< int >( n_cols, 0 ) ); // data, data + npts);
  // Assign 2D vector
  for ( size_t i = 0; i < n_rows; ++i )
    for ( size_t j = 0; j < n_cols; ++j )
      v[ i ][ j ] = md[ i ][ j ];
  delete[] md;
  delete[] data;

  return v;
}


std::vector< std::vector< int > >
SonataConnector::read_node_id_to_range_dset_( const size_t tgt_nc_size,
  const size_t tgt_nc_step,
  const hsize_t sonata_first_node_id )
{

  // Create data buffer, borrowed from https://github.com/stevenwalton/H5Easy/blob/master/H5Easy.h#L840
  const size_t n_cols = 2;
  const size_t data_dim = tgt_nc_size * n_cols; // nrows * ncols
  auto data = new int[ data_dim ];
  auto md = new int*[ tgt_nc_size ];
  for ( size_t i = 0; i < tgt_nc_size; ++i )
  {
    md[ i ] = data + i * n_cols;
  }

  // Select hyperslab
  H5::DataSpace dataspace = node_id_to_range_dset_.getSpace();
  hsize_t offset[ 2 ]; // origin of the hyperslab in dataspace
  hsize_t stride[ 2 ]; // number of elements to increment between selected elements
  hsize_t count[ 2 ];  // number of elements in the hyperslab selection
  //'block' is implicitly given as NULL.
  offset[ 0 ] = sonata_first_node_id;
  offset[ 1 ] = 0;
  stride[ 0 ] = tgt_nc_step;
  stride[ 1 ] = 1;
  count[ 0 ] = tgt_nc_size;
  count[ 1 ] = 2;
  dataspace.selectHyperslab( H5S_SELECT_SET, count, offset, stride );

  // Define Memory Dataspace. H5S_SELECT_SET replaces any existing selection
  // with the parameters from this call
  hsize_t mem_rank = 1;
  hsize_t dimsm[ 1 ]; // memory space dimension
  dimsm[ 0 ] = data_dim;
  H5::DataSpace memspace( mem_rank, dimsm, NULL );

  // Read dataset
  node_id_to_range_dset_.read( data, H5::PredType::NATIVE_INT, memspace, dataspace );

  // Assign 2D vector
  std::vector< std::vector< int > > v( tgt_nc_size, std::vector< int >( n_cols, 0 ) );
  for ( size_t i = 0; i < tgt_nc_size; ++i )
    for ( size_t j = 0; j < n_cols; ++j )
      v[ i ][ j ] = md[ i ][ j ];

  // Close dataspaces and free dynamic allocations
  dataspace.close();
  memspace.close();
  delete[] md;
  delete[] data;

  return v;
}


std::vector< std::vector< int > >
SonataConnector::read_range_to_edge_id_dset_( const size_t tgt_nc_size,
  std::vector< std::vector< int > > tgt_node_id_to_range_data )
{
  const auto this_rank = kernel().mpi_manager.get_rank(); // for debugging

  // Count the number of valid rows to be read from the range_to_edge_id dset
  std::vector< int > valid_node_ids;    // Store valid indices
  std::vector< int > n_rows_collection; // Store n_rows corresponding to valid idx
  size_t n_rows = 0;
  for ( size_t i = 0; i < tgt_nc_size; i++ )
  {
    // Need to skip if there are no edges for the associated node id.
    // According to documentation, the start index should then be a
    // negative value, however some files also indicate non-existence
    // by assigning both start and end index value zero
    auto range_start_idx = tgt_node_id_to_range_data[ i ][ 0 ];

    if ( range_start_idx < 0 )
    {
      continue;
    }

    auto range_end_idx = tgt_node_id_to_range_data[ i ][ 1 ];

    if ( ( range_start_idx == 0 ) and ( range_end_idx == 0 ) )
    {
      continue;
    }

    if ( range_start_idx >= range_end_idx )
    {
      continue;
    }

    auto n_rows_cur = range_end_idx - range_start_idx;
    valid_node_ids.push_back( i );
    n_rows_collection.push_back( n_rows_cur );
    n_rows += n_rows_cur;
  }

  // std::cerr << "Rank " << this_rank << " nrows " << n_rows << "\n";
  //  const auto n_rows2 = get_nrows_( tgt_range_to_edge_id_dset_, 2 );
  //  std::cerr << "Rank " << this_rank << " nrows " << n_rows << " nrows2 " << n_rows2 << "\n";

  // Create data buffer
  const size_t n_cols = 2;
  const size_t data_dim = n_rows * n_cols;
  auto data = new int[ data_dim ];
  auto md = new int*[ n_rows ];
  for ( size_t i = 0; i < n_rows; ++i )
  {
    md[ i ] = data + i * n_cols;
  }

  // define data- and memoryspace
  H5::DataSpace dataspace = range_to_edge_id_dset_.getSpace();
  hsize_t mem_rank = 1;
  hsize_t dimsm[ 1 ]; // memory space dimension
  dimsm[ 0 ] = data_dim;
  H5::DataSpace memspace( mem_rank, dimsm, NULL );


  // Read loop
  hsize_t offset_mem_counter = 0;
  for ( auto valid_node_id : valid_node_ids )
  {
    auto range_start_idx = tgt_node_id_to_range_data[ valid_node_id ][ 0 ];
    auto range_end_idx = tgt_node_id_to_range_data[ valid_node_id ][ 1 ];

    auto n_rows_tmp = range_end_idx - range_start_idx;

    // Select dataspace hyperslab
    hsize_t offset_data[ 2 ]; // origin of the hyperslab in dataspace
    hsize_t count_data[ 2 ];  // number of elements in the hyperslab selection
    //'block' is implicitly given as NULL.
    offset_data[ 0 ] = range_start_idx;
    offset_data[ 1 ] = 0;
    count_data[ 0 ] = n_rows_tmp;
    count_data[ 1 ] = 2;
    dataspace.selectHyperslab( H5S_SELECT_SET, count_data, offset_data );

    // Select memoryspace hyperslab
    hsize_t offset_mem[ 1 ]; // hyperslab offset in memory
    hsize_t count_mem[ 1 ];  // size of the hyperslab in memory
    offset_mem[ 0 ] = offset_mem_counter;
    count_mem[ 0 ] = n_rows_tmp * n_cols;
    memspace.selectHyperslab( H5S_SELECT_SET, count_mem, offset_mem );

    // Read dataset
    range_to_edge_id_dset_.read( data, H5::PredType::NATIVE_INT, memspace, dataspace );

    /*
    std::cerr << "[Rank " << this_rank << "] node_id: " << valid_node_id << ", range: [" << range_start_idx << ", "
              << range_end_idx << "] n_rows_tmp: " << n_rows_tmp << ", n_rows_tmp*n_cols: " << n_rows_tmp * n_cols
              << ", offset_mem_counter: " << offset_mem_counter << ", tot_rows: " << n_rows
              << ", data_dim: " << data_dim << "\n";
    */
    offset_mem_counter += ( n_rows_tmp * n_cols );
  }
  // std::cerr << "Read loop done\n";

  // Assign 2D vector
  std::vector< std::vector< int > > v( n_rows, std::vector< int >( n_cols, 0 ) );
  for ( size_t i = 0; i < n_rows; ++i )
    for ( size_t j = 0; j < n_cols; ++j )
      v[ i ][ j ] = md[ i ][ j ];

  // Close dataspaces and free dynamic allocations
  dataspace.close();
  memspace.close();
  delete[] md;
  delete[] data;

  return v;
}

hsize_t
SonataConnector::get_nrows_( H5::DataSet dataset, int ndim )
{

  H5::DataSpace dataspace = dataset.getSpace();
  hsize_t dims_out[ ndim ];
  dataspace.getSimpleExtentDims( dims_out, NULL );
  dataspace.close();

  return dims_out[ 0 ];
}


hsize_t
SonataConnector::get_num_elements_( const H5::DataSet& dataset )
{

  // TODO: Remove this function

  // std::cerr << "get_num_elements_...\n";
  H5::DataSpace dataspace = dataset.getSpace();
  hsize_t dims_out[ 1 ];
  dataspace.getSimpleExtentDims( dims_out, NULL );
  // std::cerr << "dims_out: " << *dims_out << "\n";
  dataspace.close();
  return *dims_out;
}

void
SonataConnector::get_member_names_( hid_t loc_id, std::vector< std::string >& names )
{
  H5Literate( loc_id, H5_INDEX_NAME, H5_ITER_INC, NULL, get_member_names_callback_, &names );
}


hsize_t
SonataConnector::find_edge_id_groups_( H5::Group* pop_grp, std::vector< std::string >& edge_id_grp_names )
{
  // Find the number of edge id groups, i.e. ones with label "0", "1", ..., by finding
  // the names of the population's datasets and subgroups
  // Note we assume edge ids are contiguous starting from zero, which is the
  // SONATA default. Edge id keys can also be custom (not handled here)

  std::vector< std::string > population_group_dset_and_subgroup_names;
  get_member_names_( pop_grp->getId(), population_group_dset_and_subgroup_names );

  size_t num_edge_id_groups { 0 };
  bool is_edge_id_name;

  for ( const auto& name : population_group_dset_and_subgroup_names )
  {
    is_edge_id_name = ( name.find_first_not_of( "0123456789" ) == std::string::npos );

    if ( is_edge_id_name )
    {
      edge_id_grp_names.push_back( name );
      ++num_edge_id_groups;
    }
  }

  return num_edge_id_groups;
}


/*OLD
void
SonataConnector::open_dsets_( H5::Group population_group, H5::Group edge_id_group )
{

  // Check if weight and delay are given as h5 files
  is_weight_and_delay_from_dataset_( edge_id_group );

  // Open src, tgt and edge type id datasets
  src_node_id_dset_ = population_group.openDataSet( "source_node_id" );
  tgt_node_id_dset_ = population_group.openDataSet( "target_node_id" );
  edge_type_id_dset_ = population_group.openDataSet( "edge_type_id" );

  // Open weight and/or delay dsets if exist
  if ( weight_dataset_exist_ )
  {
    syn_weight_dset_ = edge_id_group.openDataSet( "syn_weight" );
  }

  if ( delay_dataset_exist_ )
  {
    delay_dset_ = edge_id_group.openDataSet( "delay" );
  }

  // Retrieve source and target attributes to find which node population to map to
  get_attributes_( source_attribute_value_, src_node_id_dset_, "node_population" );
  get_attributes_( target_attribute_value_, tgt_node_id_dset_, "node_population" );
}


void
SonataConnector::open_dsets_( H5::Group population_group, H5::Group edge_id_group, H5::Group indices_group )
{

  // Open datasets in population and edge id group
  open_dsets_( population_group, edge_id_group );

  // Open target_to_source indices dsets
  const H5::Group tgt_to_src_indices_group( indices_group.openGroup( "target_to_source" ) );
  tgt_node_id_to_range_dset_ = tgt_to_src_indices_group.openDataSet( "node_id_to_range" );
  tgt_range_to_edge_id_dset_ = tgt_to_src_indices_group.openDataSet( "range_to_edge_id" );
}
*/


hsize_t
SonataConnector::get_num_connections_()
{

  // const auto src_array_size = get_num_elements_( src_node_id_dset_ );
  // const auto tgt_array_size = get_num_elements_( tgt_node_id_dset_ );

  const auto src_array_size = get_nrows_( src_node_id_dset_, 1 );
  const auto tgt_array_size = get_nrows_( tgt_node_id_dset_, 1 );

  // make sure that target and source population have the same size
  if ( src_array_size != tgt_array_size )
  {
    throw DimensionMismatch( "Source and Target population must be of the same size." );
  }

  return tgt_array_size;
}

hsize_t
SonataConnector::get_chunk_size_( hsize_t num_conn )
{
  // Set chunk size
  hsize_t chunk_size = CHUNK_SIZE;

  // adjust if chunk_size is too large
  if ( num_conn < chunk_size )
  {
    chunk_size = num_conn;
  }

  return chunk_size;
}


hsize_t
SonataConnector::nest_node_id_to_sonata_node_id( index nest_node_id, hsize_t num_sonata_node_ids )
{
  hsize_t sonata_node_id;

  if ( nest_node_id < num_sonata_node_ids )
  {
    sonata_node_id = nest_node_id - 1;
  }
  else
  {
    sonata_node_id = nest_node_id - num_sonata_node_ids - 1;
  }

  return sonata_node_id;
}

template < typename T >
void
SonataConnector::read_subset_( const H5::DataSet& dataset,
  std::vector< T >& data_buf,
  H5::PredType datatype,
  hsize_t chunk_size,
  hsize_t offset )
{

  // Define Memory Dataspace. Get file dataspace and select
  // hyperslab from the file dataspace. In call for selecting
  // hyperslab 'stride' and 'block' are implicitly given as NULL.
  // H5S_SELECT_SET replaces any existing selection with the parameters
  // from this call
  H5::DataSpace dataspace = dataset.getSpace();
  // Get the number of dimensions in the dataspace.
  int array_dim = dataspace.getSimpleExtentNdims(); // should maybe assert if array_dim == 1
  dataspace.selectHyperslab( H5S_SELECT_SET, &chunk_size, &offset );
  H5::DataSpace memspace( array_dim, &chunk_size, NULL );

  // Read subset
  dataset.read( data_buf.data(), datatype, memspace, dataspace );

  // Close dataspaces
  dataspace.close();
  memspace.close();
}

void
SonataConnector::read_range_to_edge_id_dset_portion_( long data_buf[][ 2 ], hsize_t row_offset )
{
  H5::DataSpace dataspace = range_to_edge_id_dset_.getSpace();
  hsize_t offset[ 2 ]; // hyperslab offset in the file
  hsize_t count[ 2 ];  // size of the hyperslab in the file
  offset[ 0 ] = row_offset;
  offset[ 1 ] = 0;
  count[ 0 ] = 1;
  count[ 1 ] = 2;
  dataspace.selectHyperslab( H5S_SELECT_SET, count, offset );

  // memory space dimensions
  hsize_t dimsm[ 2 ];
  dimsm[ 0 ] = 1;
  dimsm[ 1 ] = 2;
  hsize_t mem_rank = 2;
  H5::DataSpace memspace( mem_rank, dimsm, NULL );

  range_to_edge_id_dset_.read( data_buf, H5::PredType::NATIVE_LONG, memspace, dataspace );

  // Close dataspaces
  dataspace.close();
  memspace.close();
}


void
SonataConnector::create_type_id_2_syn_spec_( DictionaryDatum edge_params )
{
  for ( auto it = edge_params->begin(); it != edge_params->end(); ++it )
  {
    const auto type_id = std::stoi( it->first.toString() );
    auto d = getValue< DictionaryDatum >( it->second );
    const auto syn_name = getValue< std::string >( ( *d )[ "synapse_model" ] );
    // std::cerr << "type_id=" << type_id << " syn_name=" << syn_name << "\n";

    // The following call will throw "UnknownSynapseType" if syn_name is not naming a known model
    const index synapse_model_id = kernel().model_manager.get_synapse_model_id( syn_name );
    // std::cerr << "synapse_model_id=" << synapse_model_id << "\n";

    set_synapse_params( d, synapse_model_id, type_id );
    // std::cerr << "synapse_model_id=" << synapse_model_id << "\n";
    type_id_2_syn_model_[ type_id ] = synapse_model_id;
  }
}

void
SonataConnector::set_synapse_params( DictionaryDatum syn_dict, index synapse_model_id, int type_id )
{
  // std::cerr << "set_synapse_params...\n";
  DictionaryDatum syn_defaults = kernel().model_manager.get_connector_defaults( synapse_model_id );
  std::set< Name > skip_syn_params_ = {
    names::weight, names::delay, names::min_delay, names::max_delay, names::num_connections, names::synapse_model
  };

  std::map< Name, std::shared_ptr< ConnParameter > > synapse_params; // TODO: Use unique_ptr/shared_ptr

  for ( Dictionary::const_iterator default_it = syn_defaults->begin(); default_it != syn_defaults->end(); ++default_it )
  {
    const Name param_name = default_it->first;
    if ( skip_syn_params_.find( param_name ) != skip_syn_params_.end() )
    {
      continue; // weight, delay or other not-settable parameter
    }

    if ( syn_dict->known( param_name ) )
    {
      synapse_params[ param_name ] = std::shared_ptr< ConnParameter >(
        ConnParameter::create( ( *syn_dict )[ param_name ], kernel().vp_manager.get_num_threads() ) );
    }
  }

  // Now create dictionary with dummy values that we will use to pass settings to the synapses created. We
  // create it here once to avoid re-creating the object over and over again.
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    // std::cerr << "type_id_2_syn_spec_.at...\n";
    type_id_2_syn_spec_[ type_id ].push_back( synapse_params ); // DO WE NEED TO DEFINE THIS PER THREAD???
    // std::cerr << "type_id_2_param_dicts_.at...\n";
    type_id_2_param_dicts_[ type_id ].push_back( new Dictionary );

    for ( auto param : synapse_params )
    {
      if ( param.second->provides_long() )
      {
        // std::cerr << "int type_id_2_param_dicts_.at...\n";
        ( *type_id_2_param_dicts_.at( type_id ).at( tid ) )[ param.first ] = Token( new IntegerDatum( 0 ) );
      }
      else
      {
        // std::cerr << "double type_id_2_param_dicts_.at...\n";
        ( *type_id_2_param_dicts_.at( type_id ).at( tid ) )[ param.first ] = Token( new DoubleDatum( 0.0 ) );
      }
    }
  }
}

void
SonataConnector::get_synapse_params_( index snode_id, Node& target, thread target_thread, RngPtr rng, int edge_type_id )
{
  for ( auto const& syn_param : type_id_2_syn_spec_.at( edge_type_id ).at( target_thread ) )
  {
    const Name param_name = syn_param.first;
    const auto param = syn_param.second;

    if ( param->provides_long() )
    {
      // change value of dictionary entry without allocating new datum
      IntegerDatum* dd = static_cast< IntegerDatum* >(
        ( ( *type_id_2_param_dicts_.at( edge_type_id ).at( target_thread ) )[ param_name ] ).datum() );
      ( *dd ) = param->value_int( target_thread, rng, snode_id, &target );
    }
    else
    {
      // change value of dictionary entry without allocating new datum
      DoubleDatum* dd = static_cast< DoubleDatum* >(
        ( ( *type_id_2_param_dicts_.at( edge_type_id ).at( target_thread ) )[ param_name ] ).datum() );
      ( *dd ) = param->value_double( target_thread, rng, snode_id, &target );
    }
  }
}

double
SonataConnector::get_syn_property_( const DictionaryDatum& syn_spec,
  hsize_t index,
  const bool dataset_exists,
  std::vector< double >& data,
  const Name& name )
{
  if ( dataset_exists )
  {
    return data[ index ];
  }
  else if ( syn_spec->known( name ) )
  {
    return static_cast< double >( ( *syn_spec )[ name ] );
  }
  // default value is NaN
  return numerics::nan;
}

void
SonataConnector::reset_params()
{
  type_id_2_syn_model_.clear();
  for ( auto params_vec_map : type_id_2_syn_spec_ )
  {
    for ( auto params : params_vec_map.second )
    {
      for ( auto synapse_parameters : params )
      {
        synapse_parameters.second->reset();
      }
    }
  }
  type_id_2_syn_spec_.clear();
  type_id_2_param_dicts_.clear();
  tgt_indices_exist_ = false;
}


void
SonataConnector::create_connections_with_indices_dev_()
{

  // const size_t n_cols = 2;
  const auto n_sonata_node_ids = get_nrows_( node_id_to_range_dset_, 2 );
  // const auto n_range_edge_ids = get_nrows_( tgt_range_to_edge_id_dset_, 2 );
  const auto num_conn = get_num_connections_();

  // read datasets
  auto tgt_node_id_to_range_data = read_indices_dset_( node_id_to_range_dset_ );
  auto tgt_range_to_edge_id_data = read_indices_dset_( range_to_edge_id_dset_ );

  std::vector< int > src_node_id_data( num_conn );
  src_node_id_dset_.read( src_node_id_data.data(), H5::PredType::NATIVE_INT );

  std::vector< int > edge_type_id_data( num_conn );
  edge_type_id_dset_.read( edge_type_id_data.data(), H5::PredType::NATIVE_INT );

  std::vector< double > syn_weight_data( num_conn );
  std::vector< double > delay_data( num_conn );

  if ( weight_dataset_exist_ )
  {
    syn_weight_dset_.read( syn_weight_data.data(), H5::PredType::NATIVE_DOUBLE );
  }
  if ( delay_dataset_exist_ )
  {
    delay_dset_.read( delay_data.data(), H5::PredType::NATIVE_DOUBLE );
  }

  // Retrieve the correct NodeCollections
  const auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
  const auto current_source_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value_ ) );
  const auto current_target_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value_ ) );
  const auto tnode_begin = current_target_nc->begin();
  const auto snode_begin = current_source_nc->begin();

#pragma omp parallel
  {
    const auto tid = kernel().vp_manager.get_thread_id();
    RngPtr rng = get_vp_specific_rng( tid );

    // iterate node ids
    for ( hsize_t sonata_tgt_node_id = 0; sonata_tgt_node_id < n_sonata_node_ids; sonata_tgt_node_id++ )
    {
      // check if target node is vp local first
      const index tnode_id = ( *( tnode_begin + sonata_tgt_node_id ) ).node_id;
      if ( not kernel().vp_manager.is_node_id_vp_local( tnode_id ) )
      {
        continue;
      }

      Node* target = kernel().node_manager.get_node_or_proxy( tnode_id, tid );
      const thread target_thread = target->get_thread();

      const auto start_node_id_range = tgt_node_id_to_range_data[ sonata_tgt_node_id ][ 0 ];
      const auto end_node_id_range = tgt_node_id_to_range_data[ sonata_tgt_node_id ][ 1 ];

      // iterate node id range
      for ( auto node_id_range_it = start_node_id_range; node_id_range_it < end_node_id_range; node_id_range_it++ )
      {

        const auto start_edge_id = tgt_range_to_edge_id_data[ node_id_range_it ][ 0 ];
        const auto end_edge_id = tgt_range_to_edge_id_data[ node_id_range_it ][ 1 ];

        // iterate subsets and connect
        for ( auto edge_id_it = start_edge_id; edge_id_it < end_edge_id; edge_id_it++ )
        {
          const auto sonata_source_id = src_node_id_data[ edge_id_it ];
          const index snode_id = ( *( snode_begin + sonata_source_id ) ).node_id;


          const auto edge_type_id = edge_type_id_data[ edge_id_it ];
          const auto syn_spec = getValue< DictionaryDatum >( edge_params_->lookup( std::to_string( edge_type_id ) ) );
          const double weight =
            get_syn_property_( syn_spec, edge_id_it, weight_dataset_exist_, syn_weight_data, names::weight );
          const double delay =
            get_syn_property_( syn_spec, edge_id_it, delay_dataset_exist_, delay_data, names::delay );

          get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

          kernel().connection_manager.connect( snode_id,
            target,
            target_thread,
            type_id_2_syn_model_.at( edge_type_id ),             // static synapse
            type_id_2_param_dicts_.at( edge_type_id ).at( tid ), // send empty param dict
            delay,                                               // fixed as 1
            weight );
        }
      }
    } // end iterate node ids
  }   // end parallel region
}


} // namespace nest

herr_t
get_member_names_callback_( hid_t loc_id, const char* name, const H5L_info_t*, void* opdata )
{
  // Check that the group exists
  herr_t status = H5Gget_objinfo( loc_id, name, 0, NULL );
  if ( status != 0 )
  {
    // Group doesn't exist, or some error occurred.
    return 0; // TODO: is this a dataset?
  }

  auto group_names = reinterpret_cast< std::vector< std::string >* >( opdata );
  group_names->push_back( name );

  return 0;
}

#endif // ifdef HAVE_HDF5
