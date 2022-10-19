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

  auto edges = getValue< ArrayDatum >( sonata_dynamics_->lookup( "edges" ) );

  const auto this_rank = kernel().mpi_manager.get_rank(); // for debugging

  for ( auto edge_dictionary_datum : edges )
  {

    const auto edge_dict = getValue< DictionaryDatum >( edge_dictionary_datum );
    const auto edge_filename = getValue< std::string >( edge_dict->lookup( "edges_file" ) );

    // std::cerr << "[Rank " << this_rank << "] Edge file: " << edge_filename << "\n";

    // Create map of edge type ids to NEST synapse_model ids
    edge_params_ = getValue< DictionaryDatum >( edge_dict->lookup( "edge_synapse" ) );
    create_type_id_2_syn_spec_( edge_params_ );

    try
    {

      // Open specified HDF5 edge file with read only access
      H5::H5File edge_file( edge_filename, H5F_ACC_RDONLY );

      // Open top-level group (always one group named 'edges')
      H5::Group edges_group( edge_file.openGroup( "edges" ) );

      // Get names of population groups (usually just one population group)
      std::vector< std::string > population_group_names;
      get_member_names_( edges_group.getId(), population_group_names );

      // Iterate the population groups
      for ( const auto& population_group_name : population_group_names )
      {

        // Open population group
        const H5::Group population_group( edges_group.openGroup( population_group_name ) );

        // Find the number of edge id groups and edge id group names
        std::vector< std::string > edge_id_group_names;
        const auto num_edge_id_groups = find_edge_id_groups_( population_group, edge_id_group_names );

        // Currently only SONATA edge files with one edge id group is supported
        if ( num_edge_id_groups != 1 )
        {
          throw NotImplemented(
            "Connecting with Sonata files with more than one edge id group is currently not implemented" );
        }

        // Open edge id group
        const H5::Group edge_id_group( population_group.openGroup( edge_id_group_names[ 0 ] ) );

        // select read method; either by indices or sequentially in chunks
        open_dsets_( population_group, edge_id_group );
        try_to_load_tgt_indices_dsets_( population_group );

        // tgt_indices_exist_ = false;
        if ( tgt_indices_exist_ )
        {
          // create_connections_with_indices_();
          create_connections_with_indices_dev_();
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
      edges_group.close();
      edge_file.close();

    } // end try block

    catch ( const H5::Exception& e )
    {
      throw KernelException( "H5 exception caught: " + e.getDetailMsg() );
    }

    catch ( ... )
    {
      std::cerr << __FILE__ << ":" << __LINE__ << " : "
                << "Unknown exception caught\n";
    }

  } // end iteration over edge files
}

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
  H5::DataSpace dataspace = tgt_node_id_to_range_dset_.getSpace();
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
  tgt_node_id_to_range_dset_.read( data, H5::PredType::NATIVE_INT, memspace, dataspace );

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
  H5::DataSpace dataspace = tgt_range_to_edge_id_dset_.getSpace();
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
    tgt_range_to_edge_id_dset_.read( data, H5::PredType::NATIVE_INT, memspace, dataspace );

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

/*
void
SonataConnector::read_connection_dset_int_( const H5::DataSet& dataset,
  std::vector< int >& data_buf,
  hsize_t start_edge_id,
  hsize_t end_edge_id )
{


  auto range_start_idx = tgt_node_id_to_range_data[ valid_node_id ][ 0 ];
  auto range_end_idx = tgt_node_id_to_range_data[ valid_node_id ][ 1 ];
  auto n_rows_tmp = range_end_idx - range_start_idx;

}
*/

void
SonataConnector::create_connections_with_indices_dev_()
{

  const auto this_rank = kernel().mpi_manager.get_rank(); // for debugging

  // Retrieve the correct NodeCollections
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
  // std::cerr << "[Rank " << this_rank << "] tgt_nc_local_size " << tgt_nc_size << "\n";
  // std::cerr << "[Rank " << this_rank << "] sonata_first_node_id " << sonata_first_node_id << "\n";

  // const auto n_sonata_node_ids = get_nrows_( tgt_node_id_to_range_dset_, 2 );
  // auto sonata_first_node_id = nest_node_id_to_sonata_node_id( tgt_nc_first_node_id, n_sonata_node_ids );

  // Read node_id_to_range dset
  // std::cerr << "[Rank " << this_rank << "] read node_id_to_range\n";
  auto tgt_node_id_to_range_data = read_node_id_to_range_dset_( tgt_nc_size, tgt_nc_step, sonata_first_node_id );

  // Read range_to_edge_id dset
  // std::cerr << "[Rank " << this_rank << "] read range_to_edge_id\n";
  auto tgt_range_to_edge_id_data = read_range_to_edge_id_dset_( tgt_nc_size, tgt_node_id_to_range_data );

  /*
if ( this_rank == 0 )
{
  for ( size_t i = 0; i < 8; i++ )
  {
    for ( size_t j = 0; j < 2; j++ )
    {
      // std::cerr << tgt_node_id_to_range_data[ i ][ j ] << " ";
      std::cerr << tgt_range_to_edge_id_data[ i ][ j ] << " ";
    }
    std::cerr << "\n";
  }
}
*/


  // Read connection dsets
  // std::cerr << "[Rank " << this_rank << "] read connection dsets\n";
  auto tgt_range_to_edge_id_data_size = tgt_range_to_edge_id_data.size();
  // std::cerr << "Rank " << this_rank << " size " << tgt_range_to_edge_id_data.size() << "\n";

  // Allocate storage for connection info
  size_t n_conns_local = 0;
  for ( size_t i = 0; i < tgt_range_to_edge_id_data_size; i++ )
  {
    n_conns_local += tgt_range_to_edge_id_data[ i ][ 1 ] - tgt_range_to_edge_id_data[ i ][ 0 ];
  }

  // std::cerr << "Rank " << this_rank << " n_conns_local " << n_conns_local << "\n";

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
  // std::cerr << "[Rank " << this_rank << "] enter parallel region\n";
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


  /*
  // const size_t n_cols = 2;
  const auto n_sonata_node_ids = get_nrows_( tgt_node_id_to_range_dset_, 2 );
  // const auto n_range_edge_ids = get_nrows_( tgt_range_to_edge_id_dset_, 2 );
  const auto num_conn = get_num_connections_();

  // read datasets
  auto tgt_node_id_to_range_data = read_indices_dset_( tgt_node_id_to_range_dset_ );
  auto tgt_range_to_edge_id_data = read_indices_dset_( tgt_range_to_edge_id_dset_ );

  // Retrieve the correct NodeCollections
  const auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "local_nodes" ) );
  const auto current_target_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value_ ) );

  const auto this_rank = kernel().mpi_manager.get_rank(); // for debugging

  auto target_it = current_target_nc->begin(); // can tgt nc be empty? if so, how to handle?

  auto tgt_nc_size = current_target_nc->size();
  // Somewhat hacky way of retrieving the NC step - perhaps create PR that expose NC step (only getter)
  auto tgt_nc_first_node = ( *current_target_nc->begin() ).node_id;
  auto tgt_nc_second_node = ( *++current_target_nc->begin() ).node_id;
  auto tgt_nc_step = tgt_nc_second_node - tgt_nc_first_node;

  if ( this_rank == 0 )
  {
    std::cerr << "target_nc_first_node: " << tgt_nc_first_node << "\n";
    std::cerr << "target_nc_second_node: " << tgt_nc_second_node << "\n";
    std::cerr << "target_nc_step: " << tgt_nc_step << "\n";
    std::cerr << "target_nc_size: " << tgt_nc_size << "\n";
  }
  */

  /*
 // for ( ; target_it < current_target_nc->end(); ++target_it, ++source_it )
 for ( ; target_it < current_target_nc->end(); ++target_it )
 {
   const index tnode_id = ( *target_it ).node_id;

   if ( this_rank == 0 )
   {
     std::cerr << "Rank: " << this_rank << ", tgt id: " << tnode_id << "\n";
   }
 }
*/

  // auto step = getValue< NodeCollectionPTR >( current_target_nc->lookup( "step" ) );
  // std::cerr << "Rank: " << this_rank << ", first " << current_target_nc.<< "\n";

  /*
  #pragma omp parallel
    {
      const auto tid = kernel().vp_manager.get_thread_id();
      RngPtr rng = get_vp_specific_rng( tid );
      auto local_nodes = kernel().node_manager.get_local_nodes( tid );
    }
  */
  /*
    // Create the parallel region
  #pragma omp parallel
    {


      printf( "[Thread %d] Every thread executes this printf.\n", omp_get_thread_num() );

  #pragma omp barrier

  #pragma omp master
      {
        printf( "[Thread %d] Only the master thread executes this printf, which is me.\n", omp_get_thread_num() );
      }
    }
    */
}


void
SonataConnector::create_connections_with_indices_()
{

  // const size_t n_cols = 2;
  const auto n_sonata_node_ids = get_nrows_( tgt_node_id_to_range_dset_, 2 );
  // const auto n_range_edge_ids = get_nrows_( tgt_range_to_edge_id_dset_, 2 );
  const auto num_conn = get_num_connections_();

  // read datasets
  auto tgt_node_id_to_range_data = read_indices_dset_( tgt_node_id_to_range_dset_ );
  auto tgt_range_to_edge_id_data = read_indices_dset_( tgt_range_to_edge_id_dset_ );

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


void
SonataConnector::create_connections_with_indices_deprecated_()
{
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
SonataConnector::find_edge_id_groups_( const H5::Group& population_group,
  std::vector< std::string >& edge_id_group_names )
{
  // Find the number of edge id groups, i.e. ones with label "0", "1", ..., by finding
  // the names of the population's datasets and subgroups
  // Note we assume edge ids are contiguous starting from zero, which is the
  // SONATA default. Edge id keys can also be custom (not handled here)

  std::vector< std::string > population_group_dset_and_subgroup_names;
  get_member_names_( population_group.getId(), population_group_dset_and_subgroup_names );

  size_t num_edge_id_groups { 0 };
  bool is_edge_id_name;

  for ( const auto& name : population_group_dset_and_subgroup_names )
  {
    is_edge_id_name = ( name.find_first_not_of( "0123456789" ) == std::string::npos );

    if ( is_edge_id_name == 1 )
    {
      edge_id_group_names.push_back( name );
    }

    num_edge_id_groups += is_edge_id_name;
  }

  return num_edge_id_groups;
}

void
SonataConnector::try_to_load_tgt_indices_dsets_( H5::Group population_group )
{
  bool indices_group_exist = H5Lexists( population_group.getId(), "indices", H5P_DEFAULT ) > 0;
  if ( indices_group_exist )
  {
    const H5::Group indices_group( population_group.openGroup( "indices" ) );

    bool tgt_to_src_group_exist = H5Lexists( indices_group.getId(), "target_to_source", H5P_DEFAULT ) > 0;

    if ( tgt_to_src_group_exist )
    {
      const H5::Group tgt_to_src_indices_group( indices_group.openGroup( "target_to_source" ) );

      bool tgt_node_id_to_range_dset_exist =
        H5Lexists( tgt_to_src_indices_group.getId(), "node_id_to_range", H5P_DEFAULT ) > 0;
      bool tgt_range_to_edge_id_dset_exist =
        H5Lexists( tgt_to_src_indices_group.getId(), "range_to_edge_id", H5P_DEFAULT ) > 0;

      if ( tgt_node_id_to_range_dset_exist && tgt_range_to_edge_id_dset_exist )
      {
        tgt_node_id_to_range_dset_ = tgt_to_src_indices_group.openDataSet( "node_id_to_range" );
        tgt_range_to_edge_id_dset_ = tgt_to_src_indices_group.openDataSet( "range_to_edge_id" );
        tgt_indices_exist_ = true;
      }
    }
  }
}


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
    tgt_node_id_to_range_dset_.close();
    tgt_range_to_edge_id_dset_.close();
  }
}

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
  H5::DataSpace dataspace = tgt_range_to_edge_id_dset_.getSpace();
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

  tgt_range_to_edge_id_dset_.read( data_buf, H5::PredType::NATIVE_LONG, memspace, dataspace );

  // Close dataspaces
  dataspace.close();
  memspace.close();
}

void
SonataConnector::get_attributes_( std::string& attribute_value,
  const H5::DataSet& dataset,
  const std::string& attribute_name )
{
  H5::Attribute attr = dataset.openAttribute( attribute_name );
  H5::DataType type = attr.getDataType();
  attr.read( type, attribute_value );
}

void
SonataConnector::is_weight_and_delay_from_dataset_( const H5::Group& group )
{
  weight_dataset_exist_ = H5Lexists( group.getId(), "syn_weight", H5P_DEFAULT ) > 0;
  delay_dataset_exist_ = H5Lexists( group.getId(), "delay", H5P_DEFAULT ) > 0;
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
