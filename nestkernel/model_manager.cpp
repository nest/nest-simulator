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
#include "connection_manager.h"
#include "connector_model_impl.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "model_manager_impl.h"
#include "proxynode.h"

// Includes from models:
#include "models.h"

#include "modelrange_manager.h"

namespace nest
{

ModelManager::ModelManager()
  : node_models_()
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
  clear_node_models_();
}

void
ModelManager::initialize( const bool )
{
  assert( not proxynode_model_ ); // must be re-created on initialization
  proxynode_model_ = new GenericModel< proxynode >( "proxynode", "" );
  proxynode_model_->set_type_id( 1 );
  proxynode_model_->set_threads();

  const size_t num_threads = kernel::manager< VPManager >.get_num_threads();

  // Make space for one vector of connection models per thread
  connection_models_.resize( num_threads );

  // Make space for one vector of proxynodes for each thread
  proxy_nodes_.resize( num_threads );

  // We must re-register all models even if only changing the number of threads because
  // the model-managing data structures depend on the number of threads.
  // Models provided by extension modules will be re-registered by the ModulesManager.
  register_models();
}

void
ModelManager::finalize( const bool )
{
  // We must clear all models even if only changing the number of threads because
  // the model-managing data structures depend on the number of threads
  clear_node_models_();
  clear_connection_models_();
}

size_t
ModelManager::get_num_connection_models() const
{
  // For the case when the ModelManager is not yet fully initialized
  if ( connection_models_.empty() )
  {
    return 0;
  }

  return connection_models_.at( kernel::manager< VPManager >.get_thread_id() ).size();
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
  // the last ID is however used as "invalid_synindex", so the final array
  // position will always be empty in the `connections` and `source_table`.
  def< int >( dict, names::max_num_syn_models, MAX_SYN_ID );
}

void
ModelManager::copy_model( Name old_name, Name new_name, DictionaryDatum params )
{
  if ( modeldict_->known( new_name ) or synapsedict_->known( new_name ) )
  {
    throw NewModelNameExists( new_name );
  }

  const Token oldnodemodel = modeldict_->lookup( old_name );
  const Token oldsynmodel = synapsedict_->lookup( old_name );

  if ( not oldnodemodel.empty() )
  {
    const size_t old_id = static_cast< size_t >( oldnodemodel );
    copy_node_model_( old_id, new_name, params );
  }
  else if ( not oldsynmodel.empty() )
  {
    const size_t old_id = static_cast< size_t >( oldsynmodel );
    copy_connection_model_( old_id, new_name, params );
  }
  else
  {
    throw UnknownModelName( old_name );
  }
}

size_t
ModelManager::register_node_model_( Model* model )
{
  assert( model );

  const size_t id = node_models_.size();
  const std::string name = model->get_name();

  model->set_model_id( id );
  model->set_type_id( id );
  model->set_threads();

  node_models_.push_back( model );
  modeldict_->insert( name, id );

#pragma omp parallel
  {
    const size_t t = kernel::manager< VPManager >.get_thread_id();
    proxy_nodes_.at( t ).push_back( create_proxynode_( t, id ) );
  }

  return id;
}

void
ModelManager::copy_node_model_( const size_t old_id, Name new_name, DictionaryDatum params )
{
  Model* old_model = get_node_model( old_id );
  old_model->deprecation_warning( "CopyModel" );

  Model* new_model = old_model->clone( new_name.toString() );
  const size_t new_id = node_models_.size();
  new_model->set_model_id( new_id );

  node_models_.push_back( new_model );
  modeldict_->insert( new_name, new_id );

  set_node_defaults_( new_id, params );

#pragma omp parallel
  {
    const size_t t = kernel::manager< VPManager >.get_thread_id();
    proxy_nodes_.at( t ).push_back( create_proxynode_( t, new_id ) );
  }
}

void
ModelManager::copy_connection_model_( const size_t old_id, Name new_name, DictionaryDatum params )
{
  kernel::manager< VPManager >.assert_single_threaded();

  const size_t new_id = connection_models_.at( kernel::manager< VPManager >.get_thread_id() ).size();

  if ( new_id == invalid_synindex )
  {
    const std::string msg = String::compose(
      "CopyModel cannot generate another synapse. Maximal synapse model count of %1 exceeded.", MAX_SYN_ID );
    LOG( M_ERROR, "ModelManager::copy_connection_model_", msg );
    throw KernelException( "Synapse model count exceeded" );
  }
  synapsedict_->insert( new_name, new_id );


#pragma omp parallel
  {
    const size_t thread_id = kernel::manager< VPManager >.get_thread_id();
    connection_models_.at( thread_id )
      .push_back( get_connection_model( old_id, thread_id ).clone( new_name.toString(), new_id ) );

    kernel::manager< ConnectionManager >.resize_connections();
  }

  set_synapse_defaults_( new_id, params ); // handles parallelism internally
}


bool
ModelManager::set_model_defaults( Name name, DictionaryDatum params )
{
  const Token nodemodel = modeldict_->lookup( name );
  const Token synmodel = synapsedict_->lookup( name );

  size_t id;
  if ( not nodemodel.empty() )
  {
    id = static_cast< size_t >( nodemodel );
    set_node_defaults_( id, params );
    return true;
  }
  else if ( not synmodel.empty() )
  {
    id = static_cast< size_t >( synmodel );
    set_synapse_defaults_( id, params );
    return true;
  }
  else
  {
    return false;
  }
}


void
ModelManager::set_node_defaults_( size_t model_id, const DictionaryDatum& params )
{
  params->clear_access_flags();

  get_node_model( model_id )->set_status( params );

  ALL_ENTRIES_ACCESSED( *params, "ModelManager::set_node_defaults_", "Unread dictionary entries: " );
  model_defaults_modified_ = true;
}

void
ModelManager::set_synapse_defaults_( size_t model_id, const DictionaryDatum& params )
{
  params->clear_access_flags();
  assert_valid_syn_id( model_id, kernel::manager< VPManager >.get_thread_id() );

  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_(
    kernel::manager< VPManager >.get_num_threads() );

// We have to run this in parallel to set the status on nodes that exist on each
// thread, such as volume_transmitter.
#pragma omp parallel
  {
    size_t tid = kernel::manager< VPManager >.get_thread_id();

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

  for ( size_t tid = 0; tid < kernel::manager< VPManager >.get_num_threads(); ++tid )
  {
    if ( exceptions_raised_.at( tid ).get() )
    {
      throw WrappedThreadException( *( exceptions_raised_.at( tid ) ) );
    }
  }

  ALL_ENTRIES_ACCESSED( *params, "ModelManager::set_synapse_defaults_", "Unread dictionary entries: " );
  model_defaults_modified_ = true;
}

size_t
ModelManager::get_node_model_id( const Name name ) const
{
  const Name model_name( name );
  for ( int i = 0; i < static_cast< int >( node_models_.size() ); ++i )
  {
    assert( node_models_[ i ] );
    if ( model_name == node_models_[ i ]->get_name() )
    {
      return i;
    }
  }

  throw UnknownModelName( model_name );
  return 0; // supress missing return value warning; never reached
}

size_t
ModelManager::get_synapse_model_id( std::string model_name )
{
  const Token synmodel = synapsedict_->lookup( model_name );
  if ( synmodel.empty() )
  {
    throw UnknownSynapseType( model_name );
  }
  return static_cast< size_t >( synmodel );
}

DictionaryDatum
ModelManager::get_connector_defaults( synindex syn_id ) const
{
  assert_valid_syn_id( syn_id, kernel::manager< VPManager >.get_thread_id() );

  DictionaryDatum dict( new Dictionary() );

  for ( size_t t = 0; t < static_cast< size_t >( kernel::manager< VPManager >.get_num_threads() ); ++t )
  {
    // each call adds to num_connections
    connection_models_[ t ][ syn_id ]->get_status( dict );
  }

  ( *dict )[ names::num_connections ] = kernel::manager< ConnectionManager >.get_num_connections( syn_id );
  ( *dict )[ names::element_type ] = "synapse";

  return dict;
}

void
ModelManager::clear_node_models_()
{
  for ( const auto& node_model : node_models_ )
  {
    if ( node_model )
    {
      node_model->clear(); // Make sure all node memory is gone
      delete node_model;
    }
  }
  node_models_.clear();

  for ( const auto& proxy_nodes_per_thread : proxy_nodes_ )
  {
    for ( const auto& proxy_node : proxy_nodes_per_thread )
    {
      delete proxy_node;
    }
  }
  proxy_nodes_.clear();

  delete proxynode_model_;
  proxynode_model_ = nullptr;


  modeldict_->clear();

  model_defaults_modified_ = false;
}

void
ModelManager::clear_connection_models_()
{
  for ( size_t t = 0; t < connection_models_.size(); ++t )
  {
    for ( const auto& connection_model : connection_models_[ t ] )
    {
      if ( connection_model )
      {
        const bool is_primary = connection_model->has_property( ConnectionModelProperties::IS_PRIMARY );

        if ( not is_primary )
        {
          connection_model->get_secondary_event()->reset_supported_syn_ids();
        }
        delete connection_model;
      }
    }
    connection_models_[ t ].clear();
  }
  connection_models_.clear();
  synapsedict_->clear();
}

void
ModelManager::calibrate( const TimeConverter& tc )
{
  for ( auto&& model : node_models_ )
  {
    model->calibrate_time( tc );
  }
  for ( size_t t = 0; t < static_cast< size_t >( kernel::manager< VPManager >.get_num_threads() ); ++t )
  {
    for ( auto&& connection_model : connection_models_[ t ] )
    {
      if ( connection_model )
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
  return kernel::manager< ModelManager >.get_node_model( a )->get_name()
    < kernel::manager< ModelManager >.get_node_model( b )->get_name();
}

void
ModelManager::memory_info() const
{

  std::cout.setf( std::ios::left );
  std::vector< size_t > idx( node_models_.size() );

  for ( size_t i = 0; i < node_models_.size(); ++i )
  {
    idx[ i ] = i;
  }

  std::sort( idx.begin(), idx.end(), compare_model_by_id_ );

  std::string sep( "--------------------------------------------------" );

  std::cout << sep << std::endl;
  std::cout << std::setw( 25 ) << "Name" << std::setw( 13 ) << "Capacity" << std::setw( 13 ) << "Available"
            << std::endl;
  std::cout << sep << std::endl;

  for ( size_t i = 0; i < node_models_.size(); ++i )
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

Node*
ModelManager::create_proxynode_( size_t t, int model_id )
{
  Node* proxy = proxynode_model_->create( t );
  proxy->set_model_id( model_id );
  return proxy;
}

Node*
ModelManager::get_proxy_node( size_t tid, size_t node_id )
{

  const int model_id = kernel::manager< ModelRangeManager >.get_model_id( node_id );
  Node* proxy = proxy_nodes_[ tid ].at( model_id );
  proxy->set_node_id_( node_id );
  proxy->set_vp( kernel::manager< VPManager >.node_id_to_vp( node_id ) );
  return proxy;
}

std::unique_ptr< SecondaryEvent >
ModelManager::get_secondary_event_prototype( const synindex syn_id, const size_t tid )
{
  assert_valid_syn_id( syn_id, tid );
  return get_connection_model( syn_id, tid ).get_secondary_event();
}

void
ModelManager::assert_valid_syn_id( synindex syn_id, size_t t ) const
{

  if ( syn_id >= connection_models_[ t ].size() or not connection_models_[ t ][ syn_id ] )
  {
    throw UnknownSynapseType( syn_id );
  }
}

const std::vector< ConnectorModel* >&
ModelManager::get_connection_models( size_t tid )
{

  return connection_models_[ tid ];
}

ConnectorModel&
ModelManager::get_connection_model( synindex syn_id, size_t thread_id )
{

  assert_valid_syn_id( syn_id, thread_id );
  return *( connection_models_[ thread_id ][ syn_id ] );
}

bool
ModelManager::are_model_defaults_modified() const
{

  return model_defaults_modified_;
}

Model*
ModelManager::get_node_model( size_t m ) const
{

  assert( m < node_models_.size() );
  return node_models_[ m ];
}

} // namespace nest
