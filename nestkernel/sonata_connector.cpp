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
#include "connection_manager.h"
#include "model_manager.h"

#ifdef HAVE_HDF5

// C++ includes:
#include <cstdlib> // for div()

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest.h"
#include "node_manager.h"

// Includes from sli:
#include "dictutils.h"

#include "H5Cpp.h"

extern "C" herr_t get_member_names_callback_( hid_t loc_id, const char* name, const H5L_info_t* linfo, void* opdata );

namespace nest
{

SonataConnector::SonataConnector( const DictionaryDatum& graph_specs, const long hyperslab_size )
  : graph_specs_( graph_specs )
  , hyperslab_size_( hyperslab_size )
  , weight_dataset_exist_( false )
  , delay_dataset_exist_( false )
{
}

SonataConnector::~SonataConnector()
{
  edge_type_id_2_syn_spec_.clear();
}

void
SonataConnector::connect()
{

  // clang-format off
  /*
  Structure of SONATA edge files:

  <edge_file.h5>                      Filename
  ├─ edges                            Group - required - top level group always named "edges"
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
  │  │  ├─ <edge_id1>                 Group - required - referred to as an edge group
  │  │  │  ├─ delay                   Dataset {M_edges} - optional
  │  │  │  ├─ syn_weight              Dataset {M_edges} - optional
  │  │  │  ├─ dynamics_params         Group - currently not supported
  │  │  ├─ <edge_id2>                 Group - optional - currently no support for more than one edge group
  │  │  │  ├─ delay                   Dataset {K_edges} - optional
  │  │  │  ├─ syn_weight              Dataset {K_edges} - optional
  │  │  │  ├─ dynamics_params         Group

  For more details, see https://github.com/AllenInstitute/sonata/blob/master/docs/SONATA_DEVELOPER_GUIDE.md
  */
  // clang-format on

  auto edges_container = getValue< ArrayDatum >( graph_specs_->lookup( "edges" ) );

  // synapse-specific parameters that should be skipped when we set default synapse parameters
  skip_syn_params_ = {
    names::weight, names::delay, names::min_delay, names::max_delay, names::num_connections, names::synapse_model
  };

  // Iterate edge files
  for ( auto edge_dict_datum : edges_container )
  {

    const auto edge_dict = getValue< DictionaryDatum >( edge_dict_datum );
    cur_fname_ = getValue< std::string >( edge_dict->lookup( "edges_file" ) );
    const auto file = open_file_( cur_fname_ );
    const auto edges_top_level_grp = open_group_( file, "edges" );

    // Create map of edge type ids to NEST synapse_model ids
    cur_edge_params_ = getValue< DictionaryDatum >( edge_dict->lookup( "syn_specs" ) );
    create_edge_type_id_2_syn_spec_( cur_edge_params_ );

    // Get names of population groups (usually just one population group)
    std::vector< std::string > pop_names;
    H5Literate(
      edges_top_level_grp->getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, get_member_names_callback_, &pop_names );

    // Iterate the population groups
    for ( const auto& pop_name : pop_names )
    {

      const auto pop_grp = open_group_( edges_top_level_grp, pop_name );

      // Find the number of edge groups and edge group names
      // NOTE: current find_edge_groups_() is only meant as a temporary helper function
      std::vector< std::string > edge_grp_names;
      const auto num_edge_groups = find_edge_groups_( pop_grp, edge_grp_names );

      // Currently only SONATA edge files with one edge group is supported
      // TODO: Handle more than one edge group. Check with Allen whether we
      // can require numeric keys, i.e. 0, 1, 2, ..., for edge groups
      if ( num_edge_groups != 1 )
      {
        throw NotImplemented(
          "Connecting with SONATA files with more than one edge group is currently not implemented" );
      }

      const auto edge_grp = open_group_( pop_grp, edge_grp_names[ 0 ] );

      open_required_dsets_( pop_grp );
      try_open_edge_group_dsets_( edge_grp );
      check_dsets_consistency();

      // Retrieve source and target attributes to find which node population to map to
      get_attribute_( source_attribute_value_, src_node_id_dset_, "node_population" );
      get_attribute_( target_attribute_value_, tgt_node_id_dset_, "node_population" );

      // Read datasets sequentially in chunks and connect
      sequential_chunkwise_connector_();

      close_dsets_();

    } // end iteration over population groups

    // Reset and close H5 objects in scope
    reset_params_();
    edges_top_level_grp->close();
    file->close();

  } // end iteration over edge files
}

H5::H5File*
SonataConnector::open_file_( std::string& fname )
{
  H5::H5File* file = nullptr;
  try
  {
    file = new H5::H5File( fname, H5F_ACC_RDONLY );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Could not open HDF5 file " + fname + ": " + e.getDetailMsg() );
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
    throw KernelException( "Could not open HDF5 group " + grp_name + " in " + cur_fname_ + ": " + e.getDetailMsg() );
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
    throw KernelException( "Could not open HDF5 group " + grp_name + " in " + cur_fname_ + ": " + e.getDetailMsg() );
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
    throw KernelException( "Could not open source_node_id dataset in " + cur_fname_ + ": " + e.getDetailMsg() );
  }

  try
  {
    tgt_node_id_dset_ = pop_grp->openDataSet( "target_node_id" );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Could not open target_node_id dataset in " + cur_fname_ + ": " + e.getDetailMsg() );
  }

  try
  {
    edge_type_id_dset_ = pop_grp->openDataSet( "edge_type_id" );
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Could not open edge_type_id dataset in " + cur_fname_ + ": " + e.getDetailMsg() );
  }
}

void
SonataConnector::try_open_edge_group_dsets_( const H5::Group* edge_grp )
{
  // NOTE: Currently only works if the edge file has a single edge group

  weight_dataset_exist_ = H5Lexists( edge_grp->getId(), "syn_weight", H5P_DEFAULT ) > 0;
  delay_dataset_exist_ = H5Lexists( edge_grp->getId(), "delay", H5P_DEFAULT ) > 0;

  if ( weight_dataset_exist_ )
  {
    try
    {
      syn_weight_dset_ = edge_grp->openDataSet( "syn_weight" );
    }
    catch ( const H5::Exception& e )
    {
      throw KernelException( "Could not open syn_weight dataset in " + cur_fname_ + ": " + e.getDetailMsg() );
    }
  }

  if ( delay_dataset_exist_ )
  {
    try
    {
      delay_dset_ = edge_grp->openDataSet( "delay" );
    }
    catch ( const H5::Exception& e )
    {
      throw KernelException( "Could not open delay dataset in " + cur_fname_ + ": " + e.getDetailMsg() );
    }
  }
}

void
SonataConnector::check_dsets_consistency()
{
  // Consistency checks
  const auto num_tgt_node_ids = get_nrows_( tgt_node_id_dset_ );

  // Ensure that target and source population have the same size
  if ( num_tgt_node_ids != get_nrows_( src_node_id_dset_ ) )
  {
    throw KernelException(
      "target_node_id and source_node_id datasets in " + cur_fname_ + " must be of the same size" );
  }

  // Ensure that edge_type_id dataset size is consistent with the number of target node ids
  if ( num_tgt_node_ids != get_nrows_( edge_type_id_dset_ ) )
  {
    throw KernelException( "target_node_id and edge_type_id datasets in " + cur_fname_ + " must be of the same size" );
  }

  // Ensure that, if provided, the syn_weight dataset size is consistent with the number of target node ids
  // Note that this check assumes SONATA edge files with one edge group
  if ( weight_dataset_exist_ )
  {
    if ( num_tgt_node_ids != get_nrows_( syn_weight_dset_ ) )
    {
      throw KernelException( "target_node_id and syn_weight datasets in " + cur_fname_ + " must be of the same size" );
    }
  }

  // Ensure that, if provided, the delay dataset size is consistent with the number of target node ids
  // Note that this check assumes SONATA edge files with one edge group
  if ( delay_dataset_exist_ )
  {
    if ( num_tgt_node_ids != get_nrows_( delay_dset_ ) )
    {
      throw KernelException( "target_node_id and delay datasets in " + cur_fname_ + " must be of the same size" );
    }
  }
}

void
SonataConnector::get_attribute_( std::string& attribute_value,
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
      "Unable to read attribute of source_node_id or target_node_id in " + cur_fname_ + ": " + e.getDetailMsg() );
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
SonataConnector::sequential_chunkwise_connector_()
{
  // Retrieve number of connections described by datasets
  const auto num_conn = get_nrows_( tgt_node_id_dset_ );

  //  Adjust if hyperslab (chunk) size is too large
  if ( num_conn < hyperslab_size_ )
  {
    hyperslab_size_ = num_conn;
  }

  // organize chunks; dv.quot = integral quotient, dv.rem = reaminder
  auto dv = std::div( static_cast< long long >( num_conn ), static_cast< long long >( hyperslab_size_ ) );

  // Iterate chunks
  hsize_t offset = 0; // start coordinates of data selection
  for ( long long i = 0; i < dv.quot; i++ )
  {
    connect_chunk_( hyperslab_size_, offset );
    offset += hyperslab_size_;
  }

  // Handle remainder
  if ( dv.rem > 0 )
  {
    connect_chunk_( dv.rem, offset );
  }
}

void
SonataConnector::connect_chunk_( const hsize_t hyperslab_size, const hsize_t offset )
{

  // Read subsets
  std::vector< unsigned long > src_node_id_data_subset( hyperslab_size );
  std::vector< unsigned long > tgt_node_id_data_subset( hyperslab_size );
  std::vector< unsigned long > edge_type_id_data_subset( hyperslab_size );
  std::vector< double > syn_weight_data_subset;
  std::vector< double > delay_data_subset;

  read_subset_( src_node_id_dset_, src_node_id_data_subset, H5::PredType::NATIVE_LONG, hyperslab_size, offset );
  read_subset_( tgt_node_id_dset_, tgt_node_id_data_subset, H5::PredType::NATIVE_LONG, hyperslab_size, offset );
  read_subset_( edge_type_id_dset_, edge_type_id_data_subset, H5::PredType::NATIVE_LONG, hyperslab_size, offset );

  if ( weight_dataset_exist_ )
  {
    syn_weight_data_subset.resize( hyperslab_size );
    read_subset_( syn_weight_dset_, syn_weight_data_subset, H5::PredType::NATIVE_DOUBLE, hyperslab_size, offset );
  }
  if ( delay_dataset_exist_ )
  {
    delay_data_subset.resize( hyperslab_size );
    read_subset_( delay_dset_, delay_data_subset, H5::PredType::NATIVE_DOUBLE, hyperslab_size, offset );
  }

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_(
    kernel::manager< VPManager >.get_num_threads() );

  // Retrieve the correct NodeCollections
  const auto nest_nodes = getValue< DictionaryDatum >( graph_specs_->lookup( "nodes" ) );

  NodeCollectionPTR src_nc;

  try
  {
    src_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( source_attribute_value_ ) );
  }
  catch ( const TypeMismatch& e )
  {
    throw KernelException(
      "Unable to find source node population '" + source_attribute_value_ + "' in node collection dictionary. Error caused "
      "by the population name specified by attribute of source_node_id dataset in "
      + cur_fname_ );
  }

  NodeCollectionPTR tgt_nc;
  try
  {
    tgt_nc = getValue< NodeCollectionPTR >( nest_nodes->lookup( target_attribute_value_ ) );
  }
  catch ( const TypeMismatch& e )
  {
    throw KernelException(
      "Unable to find target node population '" + target_attribute_value_ + "' in node collection dictionary. Error caused "
      "by the population name specified by attribute of target_node_id dataset in "
      + cur_fname_ );
  }

  const auto snode_begin = src_nc->begin();
  const auto tnode_begin = tgt_nc->begin();

#pragma omp parallel
  {
    const auto tid = kernel::manager< VPManager >.get_thread_id();
    RngPtr rng = get_vp_specific_rng( tid );

    try
    {
      // Iterate the datasets and create the connections
      for ( hsize_t i = 0; i < hyperslab_size; ++i )
      {

        const auto sonata_tgt_id = tgt_node_id_data_subset[ i ];
        const size_t tnode_id = ( *( tnode_begin + sonata_tgt_id ) ).node_id;

        if ( not kernel::manager< VPManager >.is_node_id_vp_local( tnode_id ) )
        {
          continue;
        }

        const auto sonata_src_id = src_node_id_data_subset[ i ];
        const size_t snode_id = ( *( snode_begin + sonata_src_id ) ).node_id;

        Node* target = kernel::manager< NodeManager >.get_node_or_proxy( tnode_id, tid );
        const size_t target_thread = target->get_thread();

        const auto edge_type_id = edge_type_id_data_subset[ i ];
        const auto syn_spec = getValue< DictionaryDatum >( cur_edge_params_->lookup( std::to_string( edge_type_id ) ) );
        const double weight =
          get_syn_property_( syn_spec, i, weight_dataset_exist_, syn_weight_data_subset, names::weight );
        const double delay = get_syn_property_( syn_spec, i, delay_dataset_exist_, delay_data_subset, names::delay );

        get_synapse_params_( snode_id, *target, target_thread, rng, edge_type_id );

        kernel::manager< ConnectionManager >.connect( snode_id,
          target,
          target_thread,
          edge_type_id_2_syn_model_.at( edge_type_id ),
          edge_type_id_2_param_dicts_.at( edge_type_id ).at( tid ),
          delay,
          weight );

      } // end for
    }   // end try

    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at the end of the catch block.
      exceptions_raised_.at( tid ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }

  } // end parallel region

  // Check if any exceptions have been raised
  for ( size_t thr = 0; thr < kernel::manager< VPManager >.get_num_threads(); ++thr )
  {
    if ( exceptions_raised_.at( thr ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( thr ) ) );
    }
  }

} // end create_connections_()

hsize_t
SonataConnector::get_nrows_( H5::DataSet dataset )
{

  H5::DataSpace dspace = dataset.getSpace();
  hsize_t dims_out[ 1 ];
  dspace.getSimpleExtentDims( dims_out, NULL );
  dspace.close();

  return dims_out[ 0 ];
}

hsize_t
SonataConnector::find_edge_groups_( H5::Group* pop_grp, std::vector< std::string >& edge_grp_names )
{

  // Retrieve names of all first level datasets and groups of the population group
  std::vector< std::string > member_names;
  H5Literate( pop_grp->getId(), H5_INDEX_NAME, H5_ITER_INC, NULL, get_member_names_callback_, &member_names );

  size_t num_edge_grps { 0 };
  bool is_edge_grp_name;

  for ( const auto& name : member_names )
  {
    // TODO: The below bool is convoluted, try to write it cleaner
    is_edge_grp_name = ( name.find_first_not_of( "0123456789" ) == std::string::npos );

    if ( is_edge_grp_name )
    {
      edge_grp_names.push_back( name );
      ++num_edge_grps;
    }
  }

  return num_edge_grps;
}

template < typename T >
void
SonataConnector::read_subset_( const H5::DataSet& dataset,
  std::vector< T >& data_buf,
  H5::PredType datatype,
  hsize_t hyperslab_size,
  hsize_t offset )
{
  try
  {
    H5::DataSpace mspace( 1, &hyperslab_size, NULL );
    H5::DataSpace dspace = dataset.getSpace();
    // Select hyperslab. H5S_SELECT_SET replaces any existing selection with this call
    dspace.selectHyperslab( H5S_SELECT_SET, &hyperslab_size, &offset );
    dataset.read( data_buf.data(), datatype, mspace, dspace );
    mspace.close();
    dspace.close();
  }
  catch ( const H5::Exception& e )
  {
    throw KernelException( "Unable to read datasets in " + cur_fname_ + ": " + e.getDetailMsg() );
  }
}

void
SonataConnector::create_edge_type_id_2_syn_spec_( DictionaryDatum edge_params )
{
  for ( auto it = edge_params->begin(); it != edge_params->end(); ++it )
  {
    const auto type_id = std::stoi( it->first.toString() );
    auto d = getValue< DictionaryDatum >( it->second );
    const auto syn_name = getValue< std::string >( ( *d )[ "synapse_model" ] );

    // The following call will throw "UnknownSynapseType" if syn_name is not naming a known model
    const size_t synapse_model_id = kernel::manager< ModelManager >.get_synapse_model_id( syn_name );

    set_synapse_params_( d, synapse_model_id, type_id );
    edge_type_id_2_syn_model_[ type_id ] = synapse_model_id;
  }
}

void
SonataConnector::set_synapse_params_( DictionaryDatum syn_dict, size_t synapse_model_id, int type_id )
{
  DictionaryDatum syn_defaults = kernel::manager< ModelManager >.get_connector_defaults( synapse_model_id );
  ConnParameterMap synapse_params;

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
        ConnParameter::create( ( *syn_dict )[ param_name ], kernel::manager< VPManager >.get_num_threads() ) );
    }
  }

  // Now create dictionary with dummy values that we will use to pass settings to the synapses created. We
  // create it here once to avoid re-creating the object over and over again.
  edge_type_id_2_param_dicts_[ type_id ].resize( kernel::manager< VPManager >.get_num_threads(), nullptr );
  edge_type_id_2_syn_spec_[ type_id ] = synapse_params;

  // TODO: Once NEST is SLIless, the below loop over threads should be parallelizable. In order to parallelize, the
  // change would be to replace the for loop with #pragma omp parallel and get the thread id (tid) inside the parallel
  // region. Currently, creation of NumericDatum objects is not thread-safe because sli::pool memory is a static
  // member variable; thus is also the new operator a static member function.
  // Note that this also applies to the equivalent loop in conn_builder.cpp
  for ( size_t tid = 0; tid < kernel::manager< VPManager >.get_num_threads(); ++tid )
  {
    edge_type_id_2_param_dicts_[ type_id ][ tid ] = new Dictionary;

    for ( auto param : synapse_params )
    {
      if ( param.second->provides_long() )
      {
        ( *edge_type_id_2_param_dicts_.at( type_id ).at( tid ) )[ param.first ] = Token( new IntegerDatum( 0 ) );
      }
      else
      {
        ( *edge_type_id_2_param_dicts_.at( type_id ).at( tid ) )[ param.first ] = Token( new DoubleDatum( 0.0 ) );
      }
    }
  }
}

void
SonataConnector::get_synapse_params_( size_t snode_id,
  Node& target,
  size_t target_thread,
  RngPtr rng,
  int edge_type_id )
{
  for ( auto const& syn_param : edge_type_id_2_syn_spec_.at( edge_type_id ) )
  {
    const Name param_name = syn_param.first;
    const auto param = syn_param.second;

    if ( param->provides_long() )
    {
      // change value of dictionary entry without allocating new datum
      IntegerDatum* dd = static_cast< IntegerDatum* >(
        ( ( *edge_type_id_2_param_dicts_.at( edge_type_id ).at( target_thread ) )[ param_name ] ).datum() );
      ( *dd ) = param->value_int( target_thread, rng, snode_id, &target );
    }
    else
    {
      // change value of dictionary entry without allocating new datum
      DoubleDatum* dd = static_cast< DoubleDatum* >(
        ( ( *edge_type_id_2_param_dicts_.at( edge_type_id ).at( target_thread ) )[ param_name ] ).datum() );
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
SonataConnector::reset_params_()
{
  edge_type_id_2_syn_model_.clear();
  for ( auto syn_params_vec_map : edge_type_id_2_syn_spec_ )
  {
    for ( auto syn_params : syn_params_vec_map.second )
    {
      syn_params.second->reset();
    }
  }
  edge_type_id_2_syn_spec_.clear();
  edge_type_id_2_param_dicts_.clear();
}

} // end namespace nest

herr_t
get_member_names_callback_( hid_t loc_id, const char* name, const H5L_info_t*, void* opdata )
{
  // Check that the group exists
  herr_t status = H5Gget_objinfo( loc_id, name, 0, NULL );
  if ( status != 0 )
  {
    throw nest::KernelException( "Could not get HDF5 object info" );
  }

  auto group_names = reinterpret_cast< std::vector< std::string >* >( opdata );
  group_names->push_back( name );

  return 0;
}

#endif // ifdef HAVE_HDF5
