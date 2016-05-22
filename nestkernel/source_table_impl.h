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
SourceTable::get_next_target_data( const thread tid, index& target_rank, TargetData& next_target_data, const unsigned int rank_start, const unsigned int rank_end )
{
  SourceTablePosition& current_position = *current_positions_[ tid ];
  // we stay in this loop either until we can return a valid
  // TargetData object or we have reached the end of the sources table
  while ( true )
  {
    if ( current_position.tid == sources_.size() )
    {
      return false; // reached the end of the sources table
    }
    else
    {
      if ( current_position.syn_id == sources_[ current_position.tid ]->size() )
      {
        current_position.syn_id = 0;
        ++current_position.tid;
        continue;
      }
      else
      {
        if ( current_position.lcid == (*sources_[ current_position.tid ])[ current_position.syn_id ].size() )
        {
          current_position.lcid = 0;
          ++current_position.syn_id;
          continue;
        }
        else
        {
          // the current position contains an entry, so we retrieve it
          Source& current_source = ( *sources_[ current_position.tid ] )[ current_position.syn_id ][ current_position.lcid ];
          if ( current_source.processed )
          {
            // looks like we've processed this already, let's
            // continue
            ++current_position.lcid;
            continue;
          }
          else
          {
            target_rank = kernel().node_manager.get_process_id_of_gid( current_source.gid );
            // now we need to determine whether this thread is
            // responsible for this part of the MPI buffer; if not we
            // just continue with the next iteration of the loop
            if ( rank_start <= target_rank && target_rank < rank_end )
            {
              // we have found a valid entry, so mark it as processed
              current_source.processed = true;

              // if current_first_source points to source with same
              // gid, we only increase target count, and continue
              if ( current_first_source_[ tid ] != 0 && (*current_first_source_[ tid ]).gid == current_source.gid )
              {
                ++(*current_first_source_[ tid ]).target_count;
                ++current_position.lcid;
                continue;
              }
              // only if gids differ, we update the values of
              // next_target_data and return
              else
              {
                current_first_source_[ tid ] = &current_source;
                ++(*current_first_source_[ tid ]).target_count;
                next_target_data.lid = kernel().vp_manager.gid_to_lid( current_source.gid );
                next_target_data.tid = kernel().vp_manager.vp_to_thread( kernel().vp_manager.suggest_vp( current_source.gid ) );
                // we store the thread index of the sources table, not our own tid
                next_target_data.target.tid = current_position.tid;
                next_target_data.target.rank = kernel().mpi_manager.get_rank();
                next_target_data.target.processed = false;
                next_target_data.target.syn_index = current_position.syn_id;
                next_target_data.target.lcid = current_position.lcid;
                ++current_position.lcid;
                return true;
              }
            }
            else
            {
              ++current_position.lcid;
              continue;
            }
          }
        }
      }
    }
  }
}

} // namespace nest

#endif /* SOURCE_TABLE_IMPL_H */
