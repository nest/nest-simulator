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

namespace nest
{

void ModelManager::init()
{
}

void ModelManager::reset()
{
}

void ModelManager::set_status( const Dictionary& );
{
}

void ModelManager::get_status( Dictionary& );
{
}

void ModelManager::register_basis_model( Model& m, bool private_model )
{
  Name name = m.get_name();

  if ( !private_model && modeldict_->known( name ) )
  {
    delete &m;
    throw NamingConflict("A model called '" + name + "' already exists. "
        "Please choose a different name!");
  }
  pristine_models_.push_back( std::pair< Model*, bool >( &m, private_model ) );
}

index ModelManager::register_model( Model& m, bool private_model )
{
  std::string name = m.get_name();

  if ( !private_model && modeldict_->known( name ) )
  {
    delete &m;
    throw NamingConflict("A model called '" + name + "' already exists.\n"
        "Please choose a different name!");
  }

  const index id = models_.size();
  m.set_model_id( id );
  m.set_type_id( id );

  pristine_models_.push_back( std::pair< Model*, bool >( &m, private_model ) );
  models_.push_back( m.clone( name ) );
  int proxy_model_id = get_model_id( "proxynode" );
  assert( proxy_model_id > 0 );
  Model* proxy_model = models_[ proxy_model_id ];
  assert( proxy_model != 0 );

  for ( thread t = 0; t < get_num_threads(); ++t )
  {
    Node* newnode = proxy_model->allocate( t );
    newnode->set_model_id( id );
    proxy_nodes_[ t ].push_back( newnode );
  }

  if ( !private_model )
    modeldict_->insert( name, id );

  return id;
}

index ModelManager::copy_model( index old_id, Name new_name )
{
  // we can assert here, as nestmodule checks this for us
  assert( !modeldict_->known( new_name ) );

  Model* new_model = get_model( old_id )->clone( new_name );
  models_.push_back( new_model );
  int new_id = models_.size() - 1;
  modeldict_->insert( new_name, new_id );
  int proxy_model_id = get_model_id( "proxynode" );
  assert( proxy_model_id > 0 );
  Model* proxy_model = models_[ proxy_model_id ];
  assert( proxy_model != 0 );
  for ( thread t = 0; t < get_num_threads(); ++t )
  {
    Node* newnode = proxy_model->allocate( t );
    newnode->set_model_id( new_id );
    proxy_nodes_[ t ].push_back( newnode );
  }
  return new_id;
}

synindex
ModelManager::register_synapse_prototype( ConnectorModel* cf )
{
  Name name = cf->get_name();

  if ( synapsedict_->known( name ) )
  {
    delete cf;
    throw NamingConflict("A synapse type called '" + name + "' already exists.\n"
                         "Please choose a different name!");
  }

  pristine_prototypes_.push_back( cf );

  const synindex id = prototypes_[ 0 ].size();
  pristine_prototypes_[ id ]->set_syn_id( id );

  for ( thread t = 0; t < Network::get_network().get_num_threads(); ++t )
  {
    prototypes_[ t ].push_back( cf->clone( name ) );
    prototypes_[ t ][ id ]->set_syn_id( id );
  }

  synapsedict_->insert( name, id );

  return id;
}

synindex
ModelManager::copy_synapse_prototype( synindex old_id, Name new_name )
{
  // we can assert here, as nestmodule checks this for us
  assert( !synapsedict_->known( new_name ) );

  int new_id = prototypes_[ 0 ].size();

  if ( new_id == invalid_synindex ) // we wrapped around (=255), maximal id of synapse_model = 254
  {
    Network::get_network().message( SLIInterpreter::M_ERROR,
      "ConnectionManager::copy_synapse_prototype",
      "CopyModel cannot generate another synapse. Maximal synapse model count of 255 exceeded." );
    throw KernelException( "Synapse model count exceeded" );
  }
  assert( new_id != invalid_synindex );

  for ( thread t = 0; t < Network::get_network().get_num_threads(); ++t )
  {
    prototypes_[ t ].push_back( get_synapse_prototype( old_id ).clone( new_name ) );
    prototypes_[ t ][ new_id ]->set_syn_id( new_id );
  }

  synapsedict_->insert( new_name, new_id );
  return new_id;
}

// TODO: replace int with index and return value -1 with invalid_index, also change all pertaining code
int
ModelManager::get_model_id( const Name name ) const
{
  const Name model_name( name );
  for ( int i = 0; i < ( int ) models_.size(); ++i )
  {
    assert( models_[ i ] != NULL );
    if ( model_name == models_[ i ]->get_name() )
      return i;
  }
  return -1;
}

void
ModelManager::set_connector_defaults( synindex syn_id, const Dictionary& d )
{
  assert_valid_syn_id( syn_id );
  for ( thread t = 0; t < Network::get_network().get_num_threads(); ++t )
  {
    try
    {
      prototypes_[ t ][ syn_id ]->set_status( d );
    }
    catch ( BadProperty& e )
    {
      throw BadProperty( String::compose( "Setting status of prototype '%1': %2",
        prototypes_[ t ][ syn_id ]->get_name(),
        e.message() ) );
    }
  }
}

Dictionary
ModelManager::get_connector_defaults( synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  Dictionary dict;

  for ( thread t = 0; t < Network::get_network().get_num_threads(); ++t )
    prototypes_[ t ][ syn_id ]->get_status( dict ); // each call adds to num_connections

  return dict;
}

void
ModelManager::clear_models_( bool called_from_destructor )
{
  // no message on destructor call, may come after MPI_Finalize()
  if ( not called_from_destructor )
    message( SLIInterpreter::M_INFO,
      "Network::clear_models",
      "Models will be cleared and parameters reset." );

  // We delete all models, which will also delete all nodes. The
  // built-in models will be recovered from the pristine_models_ in
  // init_()
  for ( vector< Model* >::iterator m = models_.begin(); m != models_.end(); ++m )
    if ( *m != 0 )
      delete *m;

  models_.clear();
  modeldict_->clear();
  model_defaults_modified_ = false;
}

} // namespace nest
