/*
 *  source_table_impl.h
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

#ifndef SOURCE_TABLE_IMPL_H
#define SOURCE_TABLE_IMPL_H

// Includes from nestkernel:
#include "kernel_manager.h"
#include "source_table.h"
#include "target_table.h"
#include "node_manager.h"
#include "mpi_manager.h"

namespace nest
{

inline bool
SourceTable::get_next_target_data( const thread tid, const thread rank_start, const thread rank_end, const bool keep_source_table, thread& target_rank, TargetData& next_target_data )
{
  SourceTablePosition& current_position = *current_positions_[ tid ];
  // we stay in this loop either until we can return a valid
  // TargetData object or we have reached the end of the sources table
  while( current_position.tid < static_cast< thread >( sources_.size() ) )
  {
    if ( current_position.syn_index == sources_[ current_position.tid ]->size() )
    {
      current_position.syn_index = 0;
      ++current_position.tid;
      continue;
    }
    if ( current_position.lcid == (*sources_[ current_position.tid ])[ current_position.syn_index ].size() )
    {
      current_position.lcid = 0;
      ++current_position.syn_index;
      last_source_[ tid ] = 0;
      continue;
    }

    // the current position contains an entry, so we retrieve it
    Source& current_source = ( *sources_[ current_position.tid ] )[ current_position.syn_index ][ current_position.lcid ];
    if ( current_source.processed )
    {
      // looks like we've processed this already, let's
      // continue
      ++current_position.lcid;
      continue;
    }

    target_rank = kernel().node_manager.get_process_id_of_gid( current_source.gid );
    // now we need to determine whether this thread is
    // responsible for this part of the MPI buffer; if not we
    // just continue with the next iteration of the loop
    if ( target_rank < rank_start || target_rank >= rank_end )
    {
      ++current_position.lcid;
      continue;
    }

    // we have found a valid entry, so mark it as processed
    current_source.processed = true;
    // if we keep source table and if current_first_source points to
    // source with same gid, we only increase target count, and
    // continue (this effectively compresses the amount of data needed
    // to communicate a spike, but only works if full source table is
    // stored.)
    if ( last_source_[ tid ] != 0 && (*last_source_[ tid ]).gid == current_source.gid )
    {
      kernel().connection_manager.set_has_source_subsequent_targets( current_position.tid, current_position.syn_index, current_position.lcid - 1, true );
      last_source_[ tid ] = &current_source;
      ++current_position.lcid;
      continue;
    }

    // set values of next_target_data
    next_target_data.lid = kernel().vp_manager.gid_to_lid( current_source.gid );
    next_target_data.tid = kernel().vp_manager.vp_to_thread( kernel().vp_manager.suggest_vp( current_source.gid ) );
    // we store the thread index of the sources table, not our own tid
    next_target_data.target.set_tid( current_position.tid );
    next_target_data.target.set_rank( kernel().mpi_manager.get_rank() );
    next_target_data.target.set_processed( false );
    next_target_data.target.set_syn_index( current_position.syn_index );
    next_target_data.target.set_lcid( current_position.lcid );
    last_source_[ tid ] = &current_source;
    ++current_position.lcid;
    return true; // found a valid entry
  }
  return false; // reached the end of the sources table
}

} // namespace nest

#endif /* SOURCE_TABLE_IMPL_H */
