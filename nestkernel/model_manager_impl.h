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

#include "model_manager.h"

#include "compose.hpp"
#include "kernel_manager.h"


namespace nest
{

template < class ModelT >
index
ModelManager::register_node_model( const Name& name, bool private_model)
{
  if ( !private_model && modeldict_.known( name ) )
  {
    std::string msg = String::compose("A model called '%1' already exists.\n"
      "Please choose a different name!", name);
    throw NamingConflict(msg);
  }

  Model* model = new GenericModel< ModelT >( name.toString() );
  return register_node_model_( model, private_model );
}

template < class ModelT >
index
ModelManager::register_preconf_node_model( const Name& name, DictionaryDatum& conf, bool private_model )
{
  if ( !private_model && modeldict_.known( name ) )
  {
    std::string msg = String::compose("A model called '%1' already exists.\n"
      "Please choose a different name!", name);
    throw NamingConflict(msg);
  }

  Model* model = new GenericModel< ModelT >( name.toString() );
  conf->clear_access_flags();
  model->set_status( conf );
  std::string missed;
  assert( conf->all_accessed( missed ) ); // we only get here from C++ code, no need for exception
  return register_node_model_( model, private_model );
}

template < class ConnectionT >
synindex
ModelManager::register_connection_model( const std::string& name )
{
  ConnectorModel* cf = new GenericConnectorModel< ConnectionT >( name );
  
  if ( synapsedict_.known( name ) )
  {
    delete cf;
    std::string msg = String::compose("A synapse type called '%1' already exists.\n"
                                      "Please choose a different name!", name);
    throw NamingConflict(msg);
  }
  
  pristine_prototypes_.push_back( cf );
  
  const synindex syn_id = prototypes_[ 0 ].size();
  pristine_prototypes_[ syn_id ]->set_syn_id( syn_id );
  
  for ( thread t = 0; t < static_cast < thread >( kernel().vp_manager.get_num_threads() ); ++t )
  {
    prototypes_[ t ].push_back( cf->clone( name ) );
    prototypes_[ t ][ syn_id ]->set_syn_id( syn_id );
  }
  
  synapsedict_.insert( name, syn_id );
  
  return syn_id;
}


inline
Node*
ModelManager::get_proxy_node( thread tid, index gid )
{
  return proxy_nodes_[ tid ].at( kernel().modelrange_manager.get_model_id( gid ) );
}


inline
bool
ModelManager::is_model_in_use( index i )
{
  return kernel().modelrange_manager.model_in_use( i );
}


} // namespace nest
