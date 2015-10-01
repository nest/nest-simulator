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

ModelManager::ModelManager()
  : model_defaults_modified_( false )
{
}

ModelManager::~ModelManager()
{
  clear_models_( true );

  clear_prototypes_();

  for ( std::vector< ConnectorModel* >::iterator i = pristine_prototypes_.begin();
        i != pristine_prototypes_.end();
        ++i )
    if ( *i != 0 )
      delete *i;
}

void ModelManager::init( Dictionary* synapsedict )
{
  // Re-create the model list from the clean prototypes
  for ( index i = 0; i < pristine_models_.size(); ++i )
    if ( pristine_models_[ i ].first != 0 )
    {
      std::string name = pristine_models_[ i ].first->get_name();
      models_.push_back( pristine_models_[ i ].first->clone( name ) );
      if ( !pristine_models_[ i ].second )
        modeldict_->insert( name, i );
    }

  int proxy_model_id = get_model_id( "proxynode" );
  assert( proxy_model_id > 0 );
  Model* proxy_model = models_[ proxy_model_id ];
  assert( proxy_model != 0 );

  // create proxy nodes, one for each thread and model
  proxy_nodes_.resize( kernel().vp_manager.get_num_threads() );
  for ( index t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
  {
    for ( index i = 0; i < pristine_models_.size(); ++i )
    {
      if ( pristine_models_[ i ].first != 0 )
      {
        Node* newnode = proxy_model->allocate( t );
        newnode->set_model_id( i );
        proxy_nodes_[ t ].push_back( newnode );
      }
    }
  }

  // TODO connection_manager stuff
  synapsedict_ = synapsedict;

  synapsedict_->clear();

  // one list of prototypes per thread
  std::vector< std::vector< ConnectorModel* > > tmp_proto( kernel().vp_manager.get_num_threads() );
  prototypes_.swap( tmp_proto );

  // (re-)append all synapse prototypes
  for ( std::vector< ConnectorModel* >::iterator i = pristine_prototypes_.begin();
        i != pristine_prototypes_.end();
        ++i )
    if ( *i != 0 )
    {
      std::string name = ( *i )->get_name();
      for ( thread t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
        prototypes_[ t ].push_back( ( *i )->clone( name ) );
      synapsedict_->insert( name, prototypes_[ 0 ].size() - 1 );
    }
}

void ModelManager::reset()
{
  clear_models_();
  clear_prototypes_();
}

void ModelManager::set_status( const DictionaryDatum& );
{
}

void ModelManager::get_status( DictionaryDatum& );
{
}

index
ModelManager::copy_model( Name old_name, Name new_name, DictionaryDatum params )
{
  if ( modeldict_.known( new_name ) || synapsedict_.known( new_name ) )
    throw NewModelNameExists( new_name );

  const Token oldnodemodel = modeldict.lookup( old_name );
  const Token oldsynmodel = synapsedict.lookup( old_name );

  index new_id;
  if ( !oldnodemodel.empty() )
  {
    index old_id = static_cast< index >( oldnodemodel );
    new_id = copy_node_model_( old_id, new_name);
    set_node_defaults_( old_id, params );
  }
  else if ( !oldsynmodel.empty() )
  {
    index old_id = static_cast< index >( oldsynmodel );
    new_id = copy_synapse_model_( old_id, new_name );
    set_synapse_defaults_( old_id, params );
  }
  else
    throw UnknownModelName( oldmodname );

  return new_id;
}

index
ModelManager::copy_node_model_( index old_id, Name new_name )
{
  Model* new_model = get_model( old_id )->clone( new_name );
  models_.push_back( new_model );

  index new_id = models_.size() - 1;
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

index
ModelManager::copy_synapse_model_( index old_id, Name new_name )
{
  size_t new_id = prototypes_[ 0 ].size();

  if ( new_id == invalid_synindex ) // we wrapped around (=255), maximal id of synapse_model = 254
  {
    LOG( M_ERROR, "ModelManager::copy_synapse_model_",
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


void
ModelManager::set_model_defaults( Name name, DictionaryDatum params )
{
  const Token nodemodel = modeldict.lookup( name );
  const Token synmodel = synapsedict.lookup( name );

  index id;
  if ( !nodemodel.empty() )
  {
    id = static_cast< index >( nodemodel );
    set_node_defaults_( id, params );
  }
  else if ( !synmodel.empty() )
  {
    id = static_cast< index >( synmodel );
    set_synapse_defaults_( id, params );
  }
  else
    throw UnknownModelName( name );

  model_defaults_modified_ = true;
}


void
ModelManager::set_node_defaults_(index model_id, const DictionaryDatum& params )
{
  params->clear_access_flags();

  get_model( model_id )->set_status( params );
  
  Name missed;
  if ( !params->all_accessed( missed ) )
  {
    if ( Network::get_network().dict_miss_is_error() )
      throw UnaccessedDictionaryEntry( missed );
    else
      LOG( M_WARNING, "ModelManager::set_node_defaults_",
	   ("Unread dictionary entries: " + missed ).c_str() );
  }
}

void
ModelManager::set_synapse_defaults_( index model_id, const DictionaryDatum& params )
{
  params->clear_access_flags();
  kernel().connection_manager.assert_valid_syn_id( model_id );

  for ( thread t = 0; t < Network::get_network().get_num_threads(); ++t )
  {
    try
    {
      prototypes_[ t ][ model_id ]->set_status( params );
    }
    catch ( BadProperty& e )
    {
      throw BadProperty( String::compose( "Setting status of prototype '%1': %2",
        prototypes_[ t ][ model_id ]->get_name(),
        e.message() ) );
    }
  }

  Name missed;
  if ( !params->all_accessed( missed ) )
  {
    if ( Network::get_network().dict_miss_is_error() )
      throw UnaccessedDictionaryEntry( missed );
    else
      LOG( M_WARNING, "ModelManager::set_synapse_defaults_",
	   ("Unread dictionary entries: " + missed ).c_str() );
  }
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


DictionaryDatum
ModelManager::get_connector_defaults( synindex syn_id ) const
{
  assert_valid_syn_id( syn_id );

  DictionaryDatum dict;

  for ( thread t = 0; t < Network::get_network().get_num_threads(); ++t )
    prototypes_[ t ][ syn_id ]->get_status( dict ); // each call adds to num_connections

  return dict;
}

void
ModelManager::clear_models_( bool called_from_destructor )
{
  // no message on destructor call, may come after MPI_Finalize()
  if ( not called_from_destructor )
    LOG( M_INFO, "ModelManager::clear_models_",
	 "Models will be cleared and parameters reset." );

  // We delete all models, which will also delete all nodes. The
  // built-in models will be recovered from the pristine_models_ in
  // init()
  for ( vector< Model* >::iterator m = models_.begin(); m != models_.end(); ++m )
    if ( *m != 0 )
      delete *m;

  models_.clear();
  proxy_nodes_.clear();

  modeldict_->clear();

  model_defaults_modified_ = false;
}

void
ModelManager::clear_prototypes_()
{
  for ( std::vector< std::vector< ConnectorModel* > >::iterator it = prototypes_.begin();
        it != prototypes_.end();
        ++it )
  {
    for ( std::vector< ConnectorModel* >::iterator pt = it->begin(); pt != it->end(); ++pt )
      if ( *pt != 0 )
        delete *pt;
    it->clear();
  }
  prototypes_.clear();
}

void
ModelManager::calibrate( const TimeConverter& tc )
{
  for ( thread t = 0; t < kernel().vp_manager.get_num_threads(); ++t )
    for ( std::vector< ConnectorModel* >::iterator pt = prototypes_[ t ].begin();
          pt != prototypes_[ t ].end();
          ++pt )
      if ( *pt != 0 )
        ( *pt )->calibrate( tc );
}

} // namespace nest
