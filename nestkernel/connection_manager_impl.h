/*
 *  connection_manager_impl.h
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

#ifndef CONNECTION_MANAGER_IMPL_H
#define CONNECTION_MANAGER_IMPL_H

#include "connection_manager.h"

// C++ includes:
#include <string>

// Includes from nestkernel:
#include "conn_builder.h"
#include "conn_builder_factory.h"
#include "kernel_manager.h"
#include "connector_base.h"
#include "target_table_devices_impl.h"
#include "source_table_impl.h"

namespace nest
{

template < typename ConnBuilder >
void
ConnectionManager::register_conn_builder( const std::string& name )
{
  assert( !connruledict_->known( name ) );
  GenericConnBuilderFactory* cb = new ConnBuilderFactory< ConnBuilder >();
  assert( cb != 0 );
  const int id = connbuilder_factories_.size();
  connbuilder_factories_.push_back( cb );
  connruledict_->insert( name, id );
}

inline index
ConnectionManager::get_target_gid( const thread tid,
  const synindex syn_index,
  const index lcid ) const
{
  return ( *connections_5g_[ tid ] )[ syn_index ]->get_target_gid( tid, syn_index, lcid );
}

inline void
ConnectionManager::send_5g( const thread tid,
  const synindex syn_index,
  const index lcid,
  Event& e )
{
  index lcid_offset = 0;
  while ( ( *connections_5g_[ tid ] )[ syn_index ]->send( tid,
    syn_index,
    lcid + lcid_offset,
    e,
    kernel().model_manager.get_synapse_prototypes( tid ) ) )
  {
    ++lcid_offset;
  }
}

inline void
ConnectionManager::send_to_devices( const thread tid,
  const index source_gid,
  Event& e )
{
  target_table_devices_.send_to_device(
    tid, source_gid, e, kernel().model_manager.get_synapse_prototypes( tid ) );
}

inline void
ConnectionManager::send_from_device( const thread tid,
  const index ldid,
  Event& e )
{
  target_table_devices_.send_from_device(
    tid, ldid, e, kernel().model_manager.get_synapse_prototypes( tid ) );
}

inline void
ConnectionManager::restructure_connection_tables( const thread tid )
{
  if ( ( *syn_id_to_syn_index_[ tid ] ).size() != ( *connections_5g_[ tid ] ).size() )
  {
    update_syn_id_to_syn_index_( tid );
  }

  assert( not source_table_.is_cleared() );
  target_table_.clear( tid );
  source_table_.reset_processed_flags( tid );
  // TODO@5g: keep in order to only sort newly create connections?
  source_table_.reset_last_sorted_source( tid );
}

inline void
ConnectionManager::set_has_source_subsequent_targets( const thread tid,
  const synindex syn_index,
  const index lcid,
  const bool subsequent_targets )
{
  ( *connections_5g_[ tid ] )[ syn_index ]->set_has_source_subsequent_targets( lcid, subsequent_targets );
}

inline synindex
ConnectionManager::get_syn_id( const thread tid,
  const synindex syn_index ) const
{
  return ( *connections_5g_[ tid ] )[ syn_index ]->get_syn_id();
}

} // namespace nest

#endif /* CONNECTION_MANAGER_IMPL_H */
