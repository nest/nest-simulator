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

SonataConnector::SonataConnector( const DictionaryDatum& sonata_dynamics, const long chunk_size )
  : sonata_dynamics_( sonata_dynamics )
  , chunk_size_( chunk_size )
  , weight_dataset_exist_( false )
  , delay_dataset_exist_( false )
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

  std::cerr << "chunk size: " << chunk_size_ << "\n";

  // TODO: remove debug file
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

      // Retrieve source and target attributes to find which node population to map to
      get_attributes_( source_attribute_value_, src_node_id_dset_, "node_population" );
      get_attributes_( target_attribute_value_, tgt_node_id_dset_, "node_population" );

      // Read datasets and connect sequentially in chunks
      create_connections_in_chunks_();

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
}


void
SonataConnector::create_connections_in_chunks_()
{

  // Retrieve number of connections described by datasets
  auto num_conn = get_num_connections_();

  //  Adjust if chunk_size is too large
  if ( num_conn < chunk_size_ )
  {
    chunk_size_ = num_conn;
  }

  // organize chunks; dv.quot = integral quotient, dv.rem = reaminder
  auto dv = std::div( static_cast< long long >( num_conn ), static_cast< long long >( chunk_size_ ) );

  // Iterate chunks
  hsize_t offset = 0; // start coordinates of data selection

  // TODO: should iterator also be hsize_t, and should then dv.quot & dv.rem be cast to hsize_t?

  for ( size_t i = 0; i < dv.quot; i++ )
  {
    // create connections
    connect_subset_( chunk_size_, offset );
    // increment offset
    offset += chunk_size_;
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
}

} // end namespace nest

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
