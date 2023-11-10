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
#include "nest.h"
#include "target_identifier.h"


namespace nest
{

template < class ModelT >
size_t
ModelManager::register_node_model( const Name& name, std::string deprecation_info )
{
  if ( modeldict_->known( name ) )
  {
    std::string msg = String::compose( "A model called '%1' already exists. Please choose a different name!", name );
    throw NamingConflict( msg );
  }

  Model* model = new GenericModel< ModelT >( name.toString(), deprecation_info );
  return register_node_model_( model );
}

template < template < typename targetidentifierT > class ConnectionT >
void
ModelManager::register_connection_model( const std::string& name )
{
  // We only check the main synapse name for conflicts.
  if ( synapsedict_->known( name ) )
  {
    std::string msg =
      String::compose( "A synapse type called '%1' already exists.\nPlease choose a different name!", name );
    throw NamingConflict( msg );
  }

  // Only required to check properties below
  ConnectorModel const* const dummy_connector =
    new GenericConnectorModel< ConnectionT< TargetIdentifierPtrRport > >( "dummy" );

  register_specific_connection_model_< ConnectionT< TargetIdentifierPtrRport > >( name );
  if ( dummy_connector->has_property( ConnectionModelProperties::SUPPORTS_HPC ) )
  {
    register_specific_connection_model_< ConnectionT< TargetIdentifierIndex > >( name + "_hpc" );
  }
  if ( dummy_connector->has_property( ConnectionModelProperties::SUPPORTS_LBL ) )
  {
    register_specific_connection_model_< ConnectionT< TargetIdentifierPtrRport > >( name + "_lbl" );
  }

  delete dummy_connector;
}

template < typename CompleteConnectionT >
void
ModelManager::register_specific_connection_model_( const std::string& name )
{
  kernel().vp_manager.assert_single_threaded();

  const auto new_syn_id = connection_models_[ 0 ].size();
  if ( new_syn_id >= invalid_synindex )
  {
    const std::string msg = String::compose(
      "CopyModel cannot generate another synapse. Maximal synapse model count of %1 exceeded.", MAX_SYN_ID );
    LOG( M_ERROR, "ModelManager::copy_connection_model_", msg );
    throw KernelException( "Synapse model count exceeded" );
  }

  synapsedict_->insert( name, new_syn_id );

#pragma omp parallel
  {
    ConnectorModel* conn_model = new GenericConnectorModel< CompleteConnectionT >( name );
    conn_model->set_syn_id( new_syn_id );
    connection_models_[ kernel().vp_manager.get_thread_id() ].push_back( conn_model );
    kernel().connection_manager.resize_connections();
  }
}

inline Node*
ModelManager::get_proxy_node( size_t tid, size_t node_id )
{
  const int model_id = kernel().modelrange_manager.get_model_id( node_id );
  Node* proxy = proxy_nodes_[ tid ].at( model_id );
  proxy->set_node_id_( node_id );
  proxy->set_vp( kernel().vp_manager.node_id_to_vp( node_id ) );
  return proxy;
}

} // namespace nest

#endif /* #ifndef MODEL_MANAGER_IMPL_H */
