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

#ifndef MODEL_MANAGER_IMPL_H
#define MODEL_MANAGER_IMPL_H

#include "model_manager.h"

// Includes from libnestutil:
#include "compose.hpp"
#include "string_utils.h"

// Includes from nestkernel:
#include "connection_label.h"
#include "kernel_manager.h"


namespace nest
{

template < class ModelT >
index
ModelManager::register_node_model( const Name& name, bool private_model, std::string deprecation_info )
{
  if ( not private_model and modeldict_->known( name ) )
  {
    std::string msg = String::compose(
      "A model called '%1' already exists.\n"
      "Please choose a different name!",
      name );
    throw NamingConflict( msg );
  }

  Model* model = new GenericModel< ModelT >( name.toString(), deprecation_info );
  return register_node_model_( model, private_model );
}

template < class ModelT >
index
ModelManager::register_preconf_node_model( const Name& name,
  DictionaryDatum& conf,
  bool private_model,
  std::string deprecation_info )
{
  if ( not private_model and modeldict_->known( name ) )
  {
    std::string msg = String::compose(
      "A model called '%1' already exists.\n"
      "Please choose a different name!",
      name );
    throw NamingConflict( msg );
  }

  Model* model = new GenericModel< ModelT >( name.toString(), deprecation_info );
  conf->clear_access_flags();
  model->set_status( conf );
  std::string missed;
  // we only get here from C++ code, no need for exception
  assert( conf->all_accessed( missed ) );
  return register_node_model_( model, private_model );
}

template < typename ConnectionT, template < typename > class ConnectorModelT >
void
ModelManager::register_connection_model( const std::string& name,
  const bool requires_symmetric,
  const bool requires_clopath_archiving )
{
  ConnectorModel* cf = new ConnectorModelT< ConnectionT >( name,
    /*is_primary=*/true,
    /*has_delay=*/true,
    requires_symmetric,
    /*supports_wfr*/ false,
    requires_clopath_archiving );
  register_connection_model_( cf );

  if ( not ends_with( name, "_hpc" ) )
  {
    cf = new ConnectorModelT< ConnectionLabel< ConnectionT > >( name + "_lbl",
      /*is_primary=*/true,
      /*has_delay=*/true,
      requires_symmetric,
      /*supports_wfr=*/false,
      requires_clopath_archiving );
    register_connection_model_( cf );
  }
}

template < typename ConnectionT >
void
ModelManager::register_connection_model( const std::string& name,
  const bool requires_symmetric,
  const bool requires_clopath_archiving )
{
  register_connection_model< ConnectionT, GenericConnectorModel >(
    name, requires_symmetric, requires_clopath_archiving );
}

/**
 * Register a synape with default Connector and without any common properties.
 */
template < typename ConnectionT >
void
ModelManager::register_secondary_connection_model( const std::string& name,
  const bool has_delay,
  const bool requires_symmetric,
  const bool supports_wfr )
{
  ConnectorModel* cm =
    new GenericSecondaryConnectorModel< ConnectionT >( name, has_delay, requires_symmetric, supports_wfr );

  synindex syn_id = register_connection_model_( cm );

  // idea: save *cm in data structure
  // otherwise when number of threads is increased no way to get further
  // elements
  if ( secondary_connector_models_.size() < syn_id + ( unsigned int ) 1 )
  {
    secondary_connector_models_.resize( syn_id + 1, NULL );
  }

  secondary_connector_models_[ syn_id ] = cm;

  ConnectionT::EventType::set_syn_id( syn_id );

  // create labeled secondary event connection model
  cm = new GenericSecondaryConnectorModel< ConnectionLabel< ConnectionT > >(
    name + "_lbl", has_delay, requires_symmetric, supports_wfr );

  syn_id = register_connection_model_( cm );

  // idea: save *cm in data structure
  // otherwise when number of threads is increased no way to get further
  // elements
  if ( secondary_connector_models_.size() < syn_id + ( unsigned int ) 1 )
  {
    secondary_connector_models_.resize( syn_id + 1, NULL );
  }

  secondary_connector_models_[ syn_id ] = cm;

  ConnectionT::EventType::set_syn_id( syn_id );
}

inline Node*
ModelManager::get_proxy_node( thread tid, index node_id )
{
  const int model_id = kernel().modelrange_manager.get_model_id( node_id );
  Node* proxy = proxy_nodes_[ tid ].at( model_id );
  proxy->set_node_id_( node_id );
  proxy->set_vp( kernel().vp_manager.suggest_vp_for_node_id( node_id ) );
  return proxy;
}


inline bool
ModelManager::is_model_in_use( index i )
{
  return kernel().modelrange_manager.model_in_use( i );
}


} // namespace nest

#endif // #ifndef MODEL_MANAGER_IMPL_H
