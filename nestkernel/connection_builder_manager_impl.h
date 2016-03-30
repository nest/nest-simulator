/*
 *  connection_builder_manager_impl.h
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

#ifndef CONNECTION_BUILDER_MANAGER_IMPL_H
#define CONNECTION_BUILDER_MANAGER_IMPL_H

#include "connection_builder_manager.h"

// C++ includes:
#include <string>

// Includes from nestkernel:
#include "conn_builder.h"
#include "conn_builder_factory.h"
#include "kernel_manager.h"
#include "connector_base.h"
#include "target_table_impl.h"
#include "target_table_devices_impl.h"
#include "source_table_impl.h"

namespace nest
{

template < typename ConnBuilder >
void
ConnectionBuilderManager::register_conn_builder( const std::string& name )
{
  assert( !connruledict_->known( name ) );
  GenericConnBuilderFactory* cb = new ConnBuilderFactory< ConnBuilder >();
  assert( cb != 0 );
  const int id = connbuilder_factories_.size();
  connbuilder_factories_.push_back( cb );
  connruledict_->insert( name, id );
}

inline void
ConnectionBuilderManager::send_5g( thread tid, synindex syn_index, unsigned int lcid, Event& e )
{
  connections_5g_[ tid ]->send( tid, syn_index, lcid, e, kernel().model_manager.get_synapse_prototypes( tid ) );
}

inline void
ConnectionBuilderManager::send_to_devices( thread tid, const index s_gid, Event& e )
{
  target_table_devices_.send_to_device( tid, s_gid, e, kernel().model_manager.get_synapse_prototypes( tid ) );
}

inline void
ConnectionBuilderManager::send_from_device( thread tid, const index ldid, Event& e)
{
  target_table_devices_.send_from_device( tid, ldid, e, kernel().model_manager.get_synapse_prototypes( tid ) );
}

inline void
ConnectionBuilderManager::add_target( const thread tid, const TargetData& target_data)
{
  target_table_.add_target( tid, target_data );
}

inline bool
ConnectionBuilderManager::get_next_spike_data( const thread tid, const thread current_tid, const index lid, index& rank, SpikeData& next_spike_data, const unsigned int rank_start, const unsigned int rank_end)
{
  return target_table_.get_next_spike_data( tid, current_tid, lid, rank, next_spike_data, rank_start, rank_end );
}

inline bool
ConnectionBuilderManager::get_next_target_data( const thread tid, index& target_rank, TargetData& next_target_data, const unsigned int rank_start, const unsigned int rank_end )
{
  return source_table_.get_next_target_data( tid, target_rank, next_target_data, rank_start, rank_end );
}

} // namespace nest

#endif // CONNECTION_BUILDER_MANAGER_IMPL_H
