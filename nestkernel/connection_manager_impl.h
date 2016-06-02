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
ConnectionManager::get_target_gid( const thread tid, const synindex syn_index, const index lcid ) const
{
  return connections_5g_[ tid ]->get_target_gid( tid, syn_index, lcid );
}

inline void
ConnectionManager::send_5g( const thread tid, const synindex syn_index, const index lcid, Event& e )
{
  const unsigned int target_count = source_table_.get_target_count( tid, syn_index, lcid );
  for ( index tmp_lcid = lcid; tmp_lcid < lcid + target_count; ++tmp_lcid )
  {
    connections_5g_[ tid ]->send( tid, syn_index, tmp_lcid, e, kernel().model_manager.get_synapse_prototypes( tid ) );
  }
}

inline void
ConnectionManager::send_to_devices( const thread tid, const index source_gid, Event& e )
{
  target_table_devices_.send_to_device( tid, source_gid, e, kernel().model_manager.get_synapse_prototypes( tid ) );
}

inline void
ConnectionManager::send_from_device( const thread tid, const index ldid, Event& e)
{
  target_table_devices_.send_from_device( tid, ldid, e, kernel().model_manager.get_synapse_prototypes( tid ) );
}

inline void
ConnectionManager::add_target( const thread tid, const TargetData& target_data)
{
  target_table_.add_target( tid, target_data );
}

inline bool
ConnectionManager::get_next_target_data( const thread tid, const thread rank_start, const thread rank_end, thread& target_rank, TargetData& next_target_data )
{
  return source_table_.get_next_target_data( tid, rank_start, rank_end, target_rank, next_target_data );
}

inline void
ConnectionManager::restructure_connection_tables()
{
#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    target_table_.clear( tid );
    source_table_.reset_processed_flags( tid );
  }
}

} // namespace nest

#endif /* CONNECTION_MANAGER_IMPL_H */
