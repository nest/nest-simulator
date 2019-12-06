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
  : pristine_models_()
  , models_()
  , pristine_prototypes_()
  , prototypes_()
  , modeldict_( new Dictionary )
  , synapsedict_( new Dictionary )
  , proxynode_model_( 0 )
  , proxy_nodes_()
  , dummy_spike_sources_()
  , model_defaults_modified_( false )
{
}

ModelManager::~ModelManager()
{
  clear_models_( true );

  clear_prototypes_();

  // Now we can delete the clean model prototypes
  std::vector< ConnectorModel* >::iterator i;
  for ( i = pristine_prototypes_.begin(); i != pristine_prototypes_.end(); ++i )
  {
    if ( *i != 0 )
    {
      delete *i;
    }
  }

  std::vector< std::pair< Model*, bool > >::iterator j;
  for ( j = pristine_models_.begin(); j != pristine_models_.end(); ++j )
  {
    if ( ( *j ).first != 0 )
    {
      delete ( *j ).first;
    }
  }
}

void
ModelManager::initialize()
{
  if ( proxynode_model_ == 0 )
  {
    proxynode_model_ = new GenericModel< proxynode >( "proxynode", "" );
    proxynode_model_->set_type_id( 1 );
    pristine_models_.push_back( std::pair< Model*, bool >( proxynode_model_, true ) );
  }

  // Re-create the model list from the clean prototypes
  for ( index i = 0; i < pristine_models_.size(); ++i )
  {
    assert( pristine_models_[ i ].first != 0 );

    // set the number of threads for the number of sli pools
    pristine_models_[ i ].first->set_threads();
    std::string name = pristine_models_[ i ].first->get_name();
    models_.push_back( pristine_models_[ i ].first->clone( name ) );
    if ( not pristine_models_[ i ].second )
    {
      modeldict_->insert( name, i );
    }
  }

  // create proxy nodes, one for each thread and model and one dummy
  // spike source for each thread.

  proxy_nodes_.resize( kernel().vp_manager.get_num_threads() );
  dummy_spike_sources_.resize( kernel().vp_manager.get_num_threads() );

#pragma omp parallel
  {
    const thread t = kernel().vp_manager.get_thread_id();
    proxy_nodes_[ t ].clear();

    for ( index i = 0; i < pristine_models_.size(); ++i )
    {
      const int model_id = pristine_models_[ i ].first->get_model_id();
      proxy_nodes_[ t ].push_back( create_proxynode_( t, model_id ) );
    }

    const int model_id = get_model_id( "proxynode" );
    dummy_spike_sources_[ t ] = create_proxynode_( t, model_id );
  }

  synapsedict_->clear();

  // one list of prototypes per thread
  std::vector< std::vector< ConnectorModel* > > tmp_proto( kernel().vp_manager.get_num_threads() );
  prototypes_.swap( tmp_proto );

  // (re-)append all synapse prototypes
  for ( std::vector< ConnectorModel* >::iterator i = pristine_prototypes_.begin(); i != pristine_prototypes_.end();
        ++i )
  {
    if ( *i != 0 )
    {
      std::string name = ( *i )->get_name();
      for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
      {
        prototypes_[ t ].push_back( ( *i )->clone( name ) );
      }
      synapsedict_->insert( name, prototypes_[ 0 ].size() - 1 );
    }
  }
}

void
ModelManager::finalize()
{
  clear_models_();
  clear_prototypes_();
  delete_secondary_events_prototypes();

  // We free all Node memory
  std::vector< std::pair< Model*, bool > >::iterator m;
  for ( m = pristine_models_.begin(); m != pristine_models_.end(); ++m )
  {
    // delete all nodes, because cloning the model may have created instances.
    ( *m ).first->clear();
  }
}

void
ModelManager::set_status( const DictionaryDatum& )
{
}

void
ModelManager::get_status( DictionaryDatum& dict )
{
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
    new_id = copy_synapse_model_( old_id, new_name );
    set_synapse_defaults_( new_id, params );
  }
  else
  {
    throw UnknownModelName( old_name );
  }

  return new_id;
}

index
ModelManager::register_node_model_( Model* model, bool private_model )
{
  const index id = models_.size();
  model->set_model_id( id );
  model->set_type_id( id );

  std::string name = model->get_name();

  pristine_models_.push_back( std::pair< Model*, bool >( model, private_model ) );
  models_.push_back( model->clone( name ) );

#pragma omp parallel
  {
    const thread t = kernel().vp_manager.get_thread_id();
    const int model_id = model->get_model_id();
    proxy_nodes_[ t ].push_back( create_proxynode_( t, model_id ) );
  }

  if ( not private_model )
  {
    modeldict_->insert( name, id );
  }

  return id;
}

index
ModelManager::copy_node_model_( index old_id, Name new_name )
{
  Model* old_model = get_model( old_id );
  old_model->deprecation_warning( "CopyModel" );

  Model* new_model = old_model->clone( new_name.toString() );
  models_.push_back( new_model );

  index new_id = models_.size() - 1;
  modeldict_->insert( new_name, new_id );

#pragma omp parallel
  {
    const thread t = kernel().vp_manager.get_thread_id();
    const int model_id = new_model->get_model_id();
    proxy_nodes_[ t ].push_back( create_proxynode_( t, model_id ) );
  }

  return new_id;
}

index
ModelManager::copy_synapse_model_( index old_id, Name new_name )
{
  size_t new_id = prototypes_[ 0 ].size();

  if ( new_id == invalid_synindex ) // we wrapped around (=63), maximal id of
                                    // synapse_model = 62, see nest_types.h
  {
    const std::string msg =
      "CopyModel cannot generate another synapse. Maximal synapse model count "
      "of " + std::to_string( MAX_SYN_ID ) + " exceeded.";
    LOG( M_ERROR, "ModelManager::copy_synapse_model_", msg );
    throw KernelException( "Synapse model count exceeded" );
  }
  assert( new_id != invalid_synindex );

  // if the copied synapse is a secondary connector model the synid of the copy
  // has to be mapped to the corresponding secondary event type
  if ( not get_synapse_prototype( old_id ).is_primary() )
  {
    ( get_synapse_prototype( old_id ).get_event() )->add_syn_id( new_id );
  }

  for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    prototypes_[ t ].push_back( get_synapse_prototype( old_id ).clone( new_name.toString() ) );
    prototypes_[ t ][ new_id ]->set_syn_id( new_id );
  }

  synapsedict_->insert( new_name, new_id );

  kernel().connection_manager.resize_connections();
  return new_id;
}


void
ModelManager::set_model_defaults( Name name, DictionaryDatum params )
{
  const Token nodemodel = modeldict_->lookup( name );
  const Token synmodel = synapsedict_->lookup( name );

  index id;
  if ( not nodemodel.empty() )
  {
    id = static_cast< index >( nodemodel );
    set_node_defaults_( id, params );
  }
  else if ( not synmodel.empty() )
  {
    id = static_cast< index >( synmodel );
    set_synapse_defaults_( id, params );
  }
  else
  {
    throw UnknownModelName( name );
  }

  model_defaults_modified_ = true;
}


void
ModelManager::set_node_defaults_( index model_id, const DictionaryDatum& params )
{
  params->clear_access_flags();

  get_model( model_id )->set_status( params );

  ALL_ENTRIES_ACCESSED( *params, "ModelManager::set_node_defaults_", "Unread dictionary entries: " );
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
      prototypes_[ tid ][ model_id ]->set_status( params );
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
}

// TODO: replace int with index and return value -1 with invalid_index, also
// change all pertaining code
int
ModelManager::get_model_id( const Name name ) const
{
  const Name model_name( name );
  for ( int i = 0; i < ( int ) models_.size(); ++i )
  {
    assert( models_[ i ] != NULL );
    if ( model_name == models_[ i ]->get_name() )
    {
      return i;
    }
  }
  return -1;
}


DictionaryDatum
ModelManager::get_connector_defaults( synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  DictionaryDatum dict( new Dictionary() );

  for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    // each call adds to num_connections
    prototypes_[ t ][ syn_id ]->get_status( dict );
  }

  ( *dict )[ names::num_connections ] = kernel().connection_manager.get_num_connections( syn_id );

  return dict;
}

bool
ModelManager::connector_requires_symmetric( const synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  return prototypes_[ 0 ][ syn_id ]->requires_symmetric();
}

bool
ModelManager::connector_requires_clopath_archiving( const synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  return prototypes_[ 0 ][ syn_id ]->requires_clopath_archiving();
}

void
ModelManager::clear_models_( bool called_from_destructor )
{
  // no message on destructor call, may come after MPI_Finalize()
  if ( not called_from_destructor )
  {
    LOG( M_INFO, "ModelManager::clear_models_", "Models will be cleared and parameters reset." );
  }

  // We delete all models, which will also delete all nodes. The
  // built-in models will be recovered from the pristine_models_ in
  // init()
  for ( std::vector< Model* >::iterator m = models_.begin(); m != models_.end(); ++m )
  {
    if ( *m != 0 )
    {
      delete *m;
    }
  }

  models_.clear();
  proxy_nodes_.clear();
  dummy_spike_sources_.clear();

  modeldict_->clear();

  model_defaults_modified_ = false;
}

void
ModelManager::clear_prototypes_()
{
  for ( std::vector< std::vector< ConnectorModel* > >::iterator it = prototypes_.begin(); it != prototypes_.end();
        ++it )
  {
    for ( std::vector< ConnectorModel* >::iterator pt = it->begin(); pt != it->end(); ++pt )
    {
      if ( *pt != 0 )
      {
        delete *pt;
      }
    }
    it->clear();
  }
  prototypes_.clear();
}

void
ModelManager::calibrate( const TimeConverter& tc )
{
  for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    for ( std::vector< ConnectorModel* >::iterator pt = prototypes_[ t ].begin(); pt != prototypes_[ t ].end(); ++pt )
    {
      if ( *pt != 0 )
      {
        ( *pt )->calibrate( tc );
      }
    }
  }
}

//!< Functor to compare Models by their name.
bool
ModelManager::compare_model_by_id_( const int a, const int b )
{
  return kernel().model_manager.get_model( a )->get_name() < kernel().model_manager.get_model( b )->get_name();
}

void
ModelManager::memory_info() const
{

  std::cout.setf( std::ios::left );
  std::vector< index > idx( get_num_node_models() );

  for ( index i = 0; i < get_num_node_models(); ++i )
  {
    idx[ i ] = i;
  }

  std::sort( idx.begin(), idx.end(), compare_model_by_id_ );

  std::string sep( "--------------------------------------------------" );

  std::cout << sep << std::endl;
  std::cout << std::setw( 25 ) << "Name" << std::setw( 13 ) << "Capacity" << std::setw( 13 ) << "Available"
            << std::endl;
  std::cout << sep << std::endl;

  for ( index i = 0; i < get_num_node_models(); ++i )
  {
    Model* mod = models_[ idx[ i ] ];
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

  for ( thread tid = 0; tid < static_cast< thread >( secondary_events_prototypes_.size() ); ++tid )
  {
    secondary_events_prototypes_[ tid ].clear();
    for ( synindex syn_id = 0; syn_id < prototypes_[ tid ].size(); ++syn_id )
    {
      if ( not prototypes_[ tid ][ syn_id ]->is_primary() )
      {
        secondary_events_prototypes_[ tid ].insert(
          std::pair< synindex, SecondaryEvent* >( syn_id, prototypes_[ tid ][ syn_id ]->create_event( 1 )[ 0 ] ) );
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

  pristine_prototypes_.push_back( cf );

  const synindex syn_id = prototypes_[ 0 ].size();
  pristine_prototypes_[ syn_id ]->set_syn_id( syn_id );

  for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    prototypes_[ t ].push_back( cf->clone( cf->get_name() ) );
    prototypes_[ t ][ syn_id ]->set_syn_id( syn_id );
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
  Node* proxy = proxynode_model_->allocate( t );
  proxy->set_model_id( model_id );
  return proxy;
}

} // namespace nest
