/*
 *  model_manager_impl.h
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

#include "kernel_manager.h"

namespace nest
{

template < class ModelT >
index
ModelManager::register_node_model( const Name& name, bool private_model)
{
  if ( !private_model && modeldict_->known( name ) )
  {
    std::string msg = String::compose("A model called '%1' already exists.\n"
      "Please choose a different name!", name);
    throw NamingConflict(msg);
  }

  Model* model = new GenericModel< ModelT >( name );
  return register_node_model_( model, private_model );
}

template < class ModelT >
index
ModelManager::register_preconf_node_model( const Name& name, DictionaryDatum& conf, bool private_model )
{
  if ( !private_model && modeldict_->known( name ) )
  {
    std::string msg = String::compose("A model called '%1' already exists.\n"
      "Please choose a different name!", name);
    throw NamingConflict(msg);
  }

  Model* model = new GenericModel< ModelT >( name );
  conf->clear_access_flags();
  model->set_status( conf );
  std::string missed;
  assert( conf->all_accessed( missed ) ); // we only get here from C++ code, no need for exception
  return register_node_model_( model, private_model );
}

index
ModelManager::register_node_model_( Model* model, bool private_model)
{
  const index id = models_.size();
  model->set_model_id( id );
  model->set_type_id( id );

  std::string name = model->get_name();

  pristine_models_.push_back( std::pair< Model*, bool >( model, private_model ) );
  models_.push_back( model->clone( name ) );
  int proxy_model_id = get_model_id( "proxynode" );
  assert( proxy_model_id > 0 );
  Model* proxy_model = models_[ proxy_model_id ];
  assert( proxy_model != 0 );

  for ( thread t = 0; t < static_cast< thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    Node* newnode = proxy_model->allocate( t );
    newnode->set_model_id( id );
    proxy_nodes_[ t ].push_back( newnode );
  }

  if ( !private_model )
    modeldict_->insert( name, id );

  return id;
}

template < class ConnectionT >
synindex
register_connection_model( const std::string& name )
{
  return register_synapse_prototype(new GenericConnectorModel< ConnectionT >( name ) );
}

inline
Model* 
ModelManager::get_model( index m ) const
{
  if ( m >= models_.size() || models_[ m ] == 0 )
    throw UnknownModelID( m );

  return models_[ m ];
}

inline
Model*
ModelManager::get_subnet_model()
{
  return subnet_model_;
}

inline
Model*
ModelManager::get_siblingcontainer_model()
{
  return siblingcontainer_model_;
}

inline
Node*
ModelManager::get_proxy_node( thread tid, index gid )
{
  return proxy_nodes_[ tid ].at( kernel().modelrange_manager.get_model_id( gid ) );
}

inline
bool 
ModelManager::are_model_defaults_modified() const
{
  return model_defaults_modified_;
}

bool
ModelManager::is_model_in_use( index i )
{
  return kernel().modelrange_manager.model_in_use( i );
}

inline
const DictionaryDatum&
ModelManager::get_modeldict()
{
  return modeldict_;
}

inline
const DictionaryDatum&
ModelManager::get_synapsedict() const
{
  return synapsedict_;
}

inline
bool
ModelManager::has_user_models() const
{
  return models_.size() > pristine_models_.size();
}

inline
ConnectorModel&
ModelManager::get_synapse_prototype( synindex syn_id, thread t )
{
  assert_valid_syn_id( syn_id );
  return *( prototypes_[ t ][ syn_id ] );
}

inline
size_t
ModelManager::get_num_node_models() const
{
  return models_.size();
}

inline
size_t
ModelManager::get_num_synapse_prototypes() const
{
  return prototypes_[ 0 ].size();
}

inline
void
ModelManager::assert_valid_syn_id( synindex syn_id, thread t ) const
{
  if ( syn_id >= prototypes_[ t ].size() || prototypes_[ t ][ syn_id ] == 0 )
    throw UnknownSynapseType( syn_id );
}

inline
bool
ModelManager::has_user_prototypes() const
{
  return prototypes_[ 0 ].size() > pristine_prototypes_.size();
}

} // namespace nest
