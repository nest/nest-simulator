/*
 *  model_manager.cpp
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

#include "model_manager.h"

// C++ includes:
#include <algorithm>
#include <iostream>
#include <vector>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from nestkernel:
#include "connector_model_impl.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "model_manager_impl.h"
#include "proxynode.h"
#include "vp_manager_impl.h"


namespace nest
{

ModelManager::ModelManager()
  : builtin_node_models_()
  , node_models_()
  , builtin_connection_models_()
  , connection_models_()
  , modeldict_( new Dictionary )
  , synapsedict_( new Dictionary )
  , proxynode_model_( nullptr )
  , proxy_nodes_()
  , model_defaults_modified_( false )
{
}

ModelManager::~ModelManager()
{
  clear_connection_models_();
  for ( auto&& connection_model : builtin_connection_models_ )
  {
    if ( connection_model != nullptr )
    {
      delete connection_model;
    }
  }

  clear_node_models_();
  for ( auto&& node_model : builtin_node_models_ )
  {
    if ( node_model != nullptr )
    {
      delete node_model;
    }
  }
}

void
ModelManager::initialize()
{
  if ( proxynode_model_ == nullptr )
  {
    proxynode_model_ = new GenericModel< proxynode >( "proxynode", "" );
    proxynode_model_->set_type_id( 1 );
    proxynode_model_->set_threads();
  }

  // Re-create the node model list from the clean prototypes
  for ( index i = 0; i < builtin_node_models_.size(); ++i )
  {
    // set the number of threads for the number of sli pools
    builtin_node_models_[ i ]->set_threads();
    std::string name = builtin_node_models_[ i ]->get_name();
    node_models_.push_back( builtin_node_models_[ i ]->clone( name ) );
    modeldict_->insert( name, i );
  }

  // Create proxy nodes, one for each thread and model
  proxy_nodes_.resize( kernel().vp_manager.get_num_threads() );

#pragma omp parallel
  {
    const thread t = kernel().vp_manager.get_thread_id();
    proxy_nodes_[ t ].clear();

    for ( auto&& builtin_node_model : builtin_node_models_ )
    {
      const int model_id = builtin_node_model->get_model_id();
      proxy_nodes_[ t ].push_back( create_proxynode_( t, model_id ) );
    }
  }

  synapsedict_->clear();

  // one list of prototypes per thread
  std::vector< std::vector< ConnectorModel* > > tmp_proto( kernel().vp_manager.get_num_threads() );
  connection_models_.swap( tmp_proto );

  // (re-)append all synapse prototypes
  for ( auto&& connection_model : builtin_connection_models_ )
  {
    if ( connection_model != nullptr )
    {
      std::string name = connection_model->get_name();
      for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
      {
        connection_models_[ t ].push_back( connection_model->clone( name ) );
      }
      synapsedict_->insert( name, connection_models_[ 0 ].size() - 1 );
    }
  }
}

void
ModelManager::finalize()
{
  clear_node_models_();
  clear_connection_models_();
  delete_secondary_events_prototypes();

  // We free all Node memory
  for ( auto& node_model : builtin_node_models_ )
  {
    // delete all nodes, because cloning the model may have created instances.
    node_model->clear();
  }
}

void
ModelManager::change_number_of_threads()
{
  finalize();
  initialize();
}

void
ModelManager::set_status( const DictionaryDatum& )
{
}

void
ModelManager::get_status( DictionaryDatum& dict )
{
  ArrayDatum node_models;
  for ( auto const& element : *modeldict_ )
  {
    node_models.push_back( new LiteralDatum( element.first ) );
  }
  def< ArrayDatum >( dict, names::node_models, node_models );

  ArrayDatum synapse_models;
  for ( auto const& element : *synapsedict_ )
  {
    synapse_models.push_back( new LiteralDatum( element.first ) );
  }
  def< ArrayDatum >( dict, names::synapse_models, synapse_models );

  // syn_ids start at 0, so the maximal number of syn models is MAX_SYN_ID + 1
  def< int >( dict, names::max_num_syn_models, MAX_SYN_ID + 1 );
}

index
ModelManager::copy_model( Name old_name, Name new_name, DictionaryDatum params )
{
  if ( modeldict_->known( new_name ) or synapsedict_->known( new_name ) )
  {
    throw NewModelNameExists( new_name );
  }

  const Token oldnodemodel = modeldict_->lookup( old_name );
  const Token oldsynmodel = synapsedict_->lookup( old_name );

  index new_id;
  if ( not oldnodemodel.empty() )
  {
    index old_id = static_cast< index >( oldnodemodel );
    new_id = copy_node_model_( old_id, new_name );
    set_node_defaults_( new_id, params );
  }
  else if ( not oldsynmodel.empty() )
  {
    index old_id = static_cast< index >( oldsynmodel );
    new_id = copy_connection_model_( old_id, new_name );
    set_synapse_defaults_( new_id, params );
  }
  else
  {
    throw UnknownModelName( old_name );
  }

  return new_id;
}

index
ModelManager::register_node_model_( Model* model )
{
  const index id = node_models_.size();
  const std::string name = model->get_name();

  model->set_model_id( id );
  model->set_type_id( id );
  builtin_node_models_.push_back( model );

  Model* cloned_model = model->clone( name );
  cloned_model->set_model_id( id );
  node_models_.push_back( cloned_model );

  modeldict_->insert( name, id );

#pragma omp parallel
  {
    const thread t = kernel().vp_manager.get_thread_id();
    proxy_nodes_[ t ].push_back( create_proxynode_( t, id ) );
  }

  return id;
}

index
ModelManager::copy_node_model_( index old_id, Name new_name )
{
  Model* old_model = get_node_model( old_id );
  old_model->deprecation_warning( "CopyModel" );

  Model* new_model = old_model->clone( new_name.toString() );
  const index new_id = node_models_.size();
  new_model->set_model_id( new_id );

  node_models_.push_back( new_model );
  modeldict_->insert( new_name, new_id );

#pragma omp parallel
  {
    const thread t = kernel().vp_manager.get_thread_id();
    proxy_nodes_[ t ].push_back( create_proxynode_( t, new_id ) );
  }

  return new_id;
}

index
ModelManager::copy_connection_model_( index old_id, Name new_name )
{
  size_t new_id = connection_models_[ 0 ].size();

  if ( new_id == invalid_synindex ) // we wrapped around (=63), maximal id of
                                    // connection_model = 62, see nest_types.h
  {
    const std::string msg =
      "CopyModel cannot generate another synapse. Maximal synapse model count "
      "of "
      + std::to_string( MAX_SYN_ID ) + " exceeded.";
    LOG( M_ERROR, "ModelManager::copy_connection_model_", msg );
    throw KernelException( "Synapse model count exceeded" );
  }
  assert( new_id != invalid_synindex );

  // if the copied synapse is a secondary connector model the synid of the copy
  // has to be mapped to the corresponding secondary event type
  if ( not get_connection_model( old_id ).is_primary() )
  {
    ( get_connection_model( old_id ).get_event() )->add_syn_id( new_id );
  }

  for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    connection_models_[ t ].push_back( get_connection_model( old_id ).clone( new_name.toString() ) );
    connection_models_[ t ][ new_id ]->set_syn_id( new_id );
  }

  synapsedict_->insert( new_name, new_id );

  kernel().connection_manager.resize_connections();
  return new_id;
}


bool
ModelManager::set_model_defaults( Name name, DictionaryDatum params )
{
  const Token nodemodel = modeldict_->lookup( name );
  const Token synmodel = synapsedict_->lookup( name );

  index id;
  if ( not nodemodel.empty() )
  {
    id = static_cast< index >( nodemodel );
    set_node_defaults_( id, params );
    return true;
  }
  else if ( not synmodel.empty() )
  {
    id = static_cast< index >( synmodel );
    set_synapse_defaults_( id, params );
    return true;
  }
  else
  {
    return false;
  }
}


void
ModelManager::set_node_defaults_( index model_id, const DictionaryDatum& params )
{
  params->clear_access_flags();

  get_node_model( model_id )->set_status( params );

  ALL_ENTRIES_ACCESSED( *params, "ModelManager::set_node_defaults_", "Unread dictionary entries: " );
  model_defaults_modified_ = true;
}

void
ModelManager::set_synapse_defaults_( index model_id, const DictionaryDatum& params )
{
  params->clear_access_flags();
  assert_valid_syn_id( model_id );

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_( kernel().vp_manager.get_num_threads() );

// We have to run this in parallel to set the status on nodes that exist on each
// thread, such as volume_transmitter.
#pragma omp parallel
  {
    thread tid = kernel().vp_manager.get_thread_id();

    try
    {
      connection_models_[ tid ][ model_id ]->set_status( params );
    }
    catch ( std::exception& err )
    {
      // We must create a new exception here, err's lifetime ends at
      // the end of the catch block.
      exceptions_raised_.at( tid ) = std::shared_ptr< WrappedThreadException >( new WrappedThreadException( err ) );
    }
  }

  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( exceptions_raised_.at( tid ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( tid ) ) );
    }
  }

  ALL_ENTRIES_ACCESSED( *params, "ModelManager::set_synapse_defaults_", "Unread dictionary entries: " );
  model_defaults_modified_ = true;
}

index
ModelManager::get_node_model_id( const Name name ) const
{
  const Name model_name( name );
  for ( int i = 0; i < ( int ) node_models_.size(); ++i )
  {
    assert( node_models_[ i ] != nullptr );
    if ( model_name == node_models_[ i ]->get_name() )
    {
      return i;
    }
  }

  throw UnknownModelName( model_name );
  return 0; // supress missing return value warning; never reached
}

index
ModelManager::get_synapse_model_id( std::string model_name )
{
  const Token synmodel = synapsedict_->lookup( model_name );
  if ( synmodel.empty() )
  {
    throw UnknownSynapseType( model_name );
  }
  return static_cast< index >( synmodel );
}

DictionaryDatum
ModelManager::get_connector_defaults( synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  DictionaryDatum dict( new Dictionary() );

  for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    // each call adds to num_connections
    connection_models_[ t ][ syn_id ]->get_status( dict );
  }

  ( *dict )[ names::num_connections ] = kernel().connection_manager.get_num_connections( syn_id );
  ( *dict )[ names::element_type ] = "synapse";

  return dict;
}

bool
ModelManager::connector_requires_symmetric( const synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  return connection_models_[ 0 ][ syn_id ]->requires_symmetric();
}

bool
ModelManager::connector_requires_clopath_archiving( const synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  return connection_models_[ 0 ][ syn_id ]->requires_clopath_archiving();
}

bool
ModelManager::connector_requires_urbanczik_archiving( const synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  return connection_models_[ 0 ][ syn_id ]->requires_urbanczik_archiving();
}

void
ModelManager::clear_node_models_()
{
  // We delete all models, which will also delete all nodes. The
  // built-in models will be recovered from the builtin_node_models_ in
  // init()
  for ( auto&& node_model : node_models_ )
  {
    if ( node_model != nullptr )
    {
      delete node_model;
    }
  }

  delete proxynode_model_;
  proxynode_model_ = nullptr;

  node_models_.clear();
  proxy_nodes_.clear();

  modeldict_->clear();

  model_defaults_modified_ = false;
}

void
ModelManager::clear_connection_models_()
{
  for ( size_t t = 0; t < connection_models_.size(); ++t )
  {
    for ( auto&& connection_model : connection_models_[ t ] )
    {
      if ( connection_model != nullptr )
      {
        delete connection_model;
      }
    }
    connection_models_[ t ].clear();
  }
  connection_models_.clear();
}

void
ModelManager::calibrate( const TimeConverter& tc )
{
  for ( auto&& model : node_models_ )
  {
    model->calibrate_time( tc );
  }
  for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    for ( auto&& connection_model : connection_models_[ t ] )
    {
      if ( connection_model != nullptr )
      {
        connection_model->calibrate( tc );
      }
    }
  }
}

//!< Functor to compare Models by their name.
bool
ModelManager::compare_model_by_id_( const int a, const int b )
{
  return kernel().model_manager.get_node_model( a )->get_name()
    < kernel().model_manager.get_node_model( b )->get_name();
}

void
ModelManager::memory_info() const
{

  std::cout.setf( std::ios::left );
  std::vector< index > idx( node_models_.size() );

  for ( index i = 0; i < node_models_.size(); ++i )
  {
    idx[ i ] = i;
  }

  std::sort( idx.begin(), idx.end(), compare_model_by_id_ );

  std::string sep( "--------------------------------------------------" );

  std::cout << sep << std::endl;
  std::cout << std::setw( 25 ) << "Name" << std::setw( 13 ) << "Capacity" << std::setw( 13 ) << "Available"
            << std::endl;
  std::cout << sep << std::endl;

  for ( index i = 0; i < node_models_.size(); ++i )
  {
    Model* mod = node_models_[ idx[ i ] ];
    if ( mod->mem_capacity() != 0 )
    {
      std::cout << std::setw( 25 ) << mod->get_name() << std::setw( 13 )
                << mod->mem_capacity() * mod->get_element_size() << std::setw( 13 )
                << mod->mem_available() * mod->get_element_size() << std::endl;
    }
  }

  std::cout << sep << std::endl;
  std::cout.unsetf( std::ios::left );
}

void
ModelManager::create_secondary_events_prototypes()
{
  delete_secondary_events_prototypes();
  secondary_events_prototypes_.resize( kernel().vp_manager.get_num_threads() );

  for ( thread tid = 0; tid < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++tid )
  {
    secondary_events_prototypes_[ tid ].clear();
    for ( synindex syn_id = 0; syn_id < connection_models_[ tid ].size(); ++syn_id )
    {
      if ( not connection_models_[ tid ][ syn_id ]->is_primary() )
      {
        secondary_events_prototypes_[ tid ].insert(
          std::pair< synindex, SecondaryEvent* >( syn_id, connection_models_[ tid ][ syn_id ]->create_event() ) );
      }
    }
  }
}

synindex
ModelManager::register_connection_model_( ConnectorModel* cf )
{
  if ( synapsedict_->known( cf->get_name() ) )
  {
    delete cf;
    std::string msg = String::compose(
      "A synapse type called '%1' already exists.\n"
      "Please choose a different name!",
      cf->get_name() );
    throw NamingConflict( msg );
  }

  builtin_connection_models_.push_back( cf );

  const synindex syn_id = connection_models_[ 0 ].size();
  builtin_connection_models_[ syn_id ]->set_syn_id( syn_id );

  for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    connection_models_[ t ].push_back( cf->clone( cf->get_name() ) );
    connection_models_[ t ][ syn_id ]->set_syn_id( syn_id );
  }

  synapsedict_->insert( cf->get_name(), syn_id );

  // Need to resize Connector vectors in case connection model is added after
  // ConnectionManager is initialised.
  kernel().connection_manager.resize_connections();

  return syn_id;
}

Node*
ModelManager::create_proxynode_( thread t, int model_id )
{
  Node* proxy = proxynode_model_->create( t );
  proxy->set_model_id( model_id );
  return proxy;
}

} // namespace nest
