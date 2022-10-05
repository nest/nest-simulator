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

//#define PROFILE_ENABLED false // for profiling

#include "H5Cpp.h" // HDF5 C++ API header file

extern "C" herr_t get_member_names_callback_( hid_t loc_id, const char* name, const H5L_info_t* linfo, void* opdata );

namespace nest
{

// constexpr hsize_t CHUNK_SIZE = 10000;      // 1e4
// constexpr hsize_t CHUNK_SIZE = 100000;     // 1e5
// constexpr hsize_t CHUNK_SIZE = 1000000;    // 1e6
// constexpr hsize_t CHUNK_SIZE = 10000000;   // 1e7
// constexpr hsize_t CHUNK_SIZE = 100000000;  // 1e8
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

  for ( auto edge_dictionary_datum : edges )
  {

    const auto edge_dict = getValue< DictionaryDatum >( edge_dictionary_datum );
    const auto edge_filename = getValue< std::string >( edge_dict->lookup( "edges_file" ) );

    std::cerr << "Edge file: " << edge_filename << "\n";

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
          create_connections_with_indices_();
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

void
SonataConnector::create_connections_with_indices_dev_()
{
  // read datasets
  // read node id ranges

  const auto n_sonata_node_ids = get_nrows_( tgt_node_id_to_range_dset_, 2 );
  std::cerr << "n_sonata_node_ids: " << n_sonata_node_ids << "\n";
  int tgt_node_id_to_range_data[ n_sonata_node_ids ][ 2 ];
  tgt_node_id_to_range_dset_.read( tgt_node_id_to_range_data, H5::PredType::NATIVE_INT );

  // connection info
  const auto num_conn = get_num_connections_();
  std::cerr << "num_conn: " << num_conn << "\n";
  std::vector< int > src_node_id_data_subset( num_conn );
  src_node_id_dset_.read( src_node_id_data_subset.data(), H5::PredType::NATIVE_INT );

  // read range to edge id
  const auto n_range_edge_ids = get_nrows_( tgt_range_to_edge_id_dset_, 2 );
  std::cerr << "n_range_edge_ids: " << n_range_edge_ids << "\n";
  int tgt_range_to_edge_id_data[ n_range_edge_ids ][ 2 ];
  tgt_range_to_edge_id_dset_.read( tgt_range_to_edge_id_data, H5::PredType::NATIVE_INT );


  std::cerr << "Dev zone done\n";
}

void
SonataConnector::create_connections_with_indices_()
{

  const auto n_sonata_node_ids = get_nrows_( tgt_node_id_to_range_dset_, 2 );
  std::cerr << "n_sonata_node_ids: " << n_sonata_node_ids << "\n";

  // read node id ranges
  int tgt_node_id_to_range_data[ n_sonata_node_ids ][ 2 ];
  tgt_node_id_to_range_dset_.read( tgt_node_id_to_range_data, H5::PredType::NATIVE_INT );

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_( kernel().vp_manager.get_num_threads() );

  // Retrieve the correct NodeCollections
  const auto nest_nodes = getValue< DictionaryDatum >( sonata_dynamics_->lookup( "nodes" ) );
  const auto current_source_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value_ ) );
  const auto current_target_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value_ ) );
  const auto tnode_begin = current_target_nc->begin();
  const auto snode_begin = current_source_nc->begin();

#pragma omp parallel
  {

    const auto tid = kernel().vp_manager.get_thread_id();
    // const auto this_vp = kernel().vp_manager.thread_to_vp( tid );
    RngPtr rng = get_vp_specific_rng( tid );


    // iterate node ids
    for ( hsize_t sonata_tgt_node_id = 0; sonata_tgt_node_id < n_sonata_node_ids; sonata_tgt_node_id++ )
    {
      // check if target node is vp local first
      const index tnode_id = ( *( tnode_begin + sonata_tgt_node_id ) ).node_id;

      // std::cerr << "sonata_tgt_node_id: " << sonata_tgt_node_id << "\n";

      // if ( not kernel().node_manager.is_local_node_id( tnode_id ) )
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

        long tgt_range_to_edge_id_data[ 1 ][ 2 ] = { { 0, 0 } };
        read_range_to_edge_id_dset_portion_( tgt_range_to_edge_id_data, node_id_range_it );

        const auto start_edge_id = tgt_range_to_edge_id_data[ 0 ][ 0 ];
        const auto end_edge_id = tgt_range_to_edge_id_data[ 0 ][ 1 ];

        auto count = end_edge_id - start_edge_id;

        std::vector< int > src_node_id_data_subset( count );
        // std::vector< int > tgt_node_id_data_subset( count );
        std::vector< int > edge_type_id_data_subset( count );
        std::vector< double > syn_weight_data_subset( count );
        std::vector< double > delay_data_subset( count );

        read_subset_( src_node_id_dset_, src_node_id_data_subset, H5::PredType::NATIVE_INT, count, start_edge_id );
        // read_subset_( tgt_node_id_dset_, tgt_node_id_data_subset, H5::PredType::NATIVE_INT, count, start_edge_id );
        read_subset_( edge_type_id_dset_, edge_type_id_data_subset, H5::PredType::NATIVE_INT, count, start_edge_id );

        if ( weight_dataset_exist_ )
        {
          // syn_weight_data_subset.reserve( chunk_size );
          read_subset_( syn_weight_dset_, syn_weight_data_subset, H5::PredType::NATIVE_DOUBLE, count, start_edge_id );
        }
        if ( delay_dataset_exist_ )
        {
          // delay_data_subset.reserve( chunk_size );
          read_subset_( delay_dset_, delay_data_subset, H5::PredType::NATIVE_DOUBLE, count, start_edge_id );
        }


        // iterate subsets and connect
        for ( size_t i = 0; i < count; i++ )
        {
          const auto sonata_source_id = src_node_id_data_subset[ i ];
          const index snode_id = ( *( snode_begin + sonata_source_id ) ).node_id;


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
        }
      }


    } // end iterate node ids


  } // end parallel region
}


void
SonataConnector::create_connections_with_indices_deprecated_()
{

  const auto num_conn = get_num_connections_();
  std::cerr << "num_conn: " << num_conn << "\n";

  // target_to_source/node_id_to_range
  const auto tgt_node_id_to_range_nrows = get_nrows_( tgt_node_id_to_range_dset_, 2 );
  int tgt_node_id_to_range_data[ tgt_node_id_to_range_nrows ][ 2 ];
  tgt_node_id_to_range_dset_.read( tgt_node_id_to_range_data, H5::PredType::NATIVE_INT );
  std::cerr << "tgt_node_id_to_range_nrows: " << tgt_node_id_to_range_nrows << "\n";

  // target_to_source/range_to_edge_id
  const auto tgt_range_to_edge_id_nrows = get_nrows_( tgt_range_to_edge_id_dset_, 2 );
  std::cerr << "tgt_range_to_edge_id_nrows: " << tgt_range_to_edge_id_nrows << "\n";


  /*
    // DEV ZONE
    H5::DataSpace tgt_node_id_to_range_dspace = tgt_node_id_to_range_dset_.getSpace();
    const int rank = tgt_node_id_to_range_dspace.getSimpleExtentNdims(); // should be 2, i.e. no. columns
    hsize_t dim[ rank ];
    tgt_node_id_to_range_dspace.getSimpleExtentDims( dim, NULL );
    tgt_node_id_to_range_dspace.close();

    std::cerr << "Rank: " << rank << ", Dim: " << *dim << "\n";

    std::vector< std::vector< int > > tgt_node_id_to_range_data( *dim, std::vector< int >( rank ) );
    // tgt_node_id_to_range_dspace.selectHyperslab( H5S_ALL );
    // tgt_node_id_to_range_dset_.read(
    //   tgt_node_id_to_range_data.data(), H5::PredType::NATIVE_INT, tgt_node_id_to_range_dspace );

    // int tgt_node_id_to_range_data[ *dim ][ rank ];
    int tgt_node_id_to_range_data[ 300 ][ 2 ];
    */

  /*
  for ( int i = 0; i < *dim; i++ )
  {
    for ( int j = 0; j < rank; j++ )
    {
      tgt_node_id_to_range_data[ i ][ j ] = 0;
    }
  }
  */

  /*

   tgt_node_id_to_range_dset_.read( tgt_node_id_to_range_data, H5::PredType::NATIVE_INT );

   // tgt_node_id_to_range_dspace.close();

   for ( int i = 0; i < *dim; i++ )
   {
     for ( int j = 0; j < rank; j++ )
     {
       std::cout << tgt_node_id_to_range_data[ i ][ j ] << " ";
     }
     std::cout << "\n";
   }
   */


  /*
   const auto sonata_target_id = tgt_node_id_data_subset[ i ];
       const index tnode_id = ( *( tnode_begin + sonata_target_id ) ).node_id;

       thread tgt_vp = kernel().vp_manager.node_id_to_vp( tnode_id );

       if ( tgt_vp != this_vp )
       {
         continue;
       }
     */

  // const bool local_only = true;
  // DictionaryDatum dd_empty = DictionaryDatum( new Dictionary );
  // const auto local_nodes = kernel().node_manager.get_nodes( dd_empty, local_only );
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
