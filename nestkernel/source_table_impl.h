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
  // we stay in this loop either until we can return a valid
  // TargetData object or we have reached the end of the sources table
  while ( true )
  {
    if ( current_tid_[ tid ] == sources_.size() )
    {
      return false; // reached the end of the sources table
    }
    else
    {
      if ( current_syn_id_[ tid ] == sources_[ current_tid_[ tid ] ]->size() )
      {
        current_syn_id_[ tid ] = 0;
        ++current_tid_[ tid ];
        continue;
      }
      else
      {
        if ( current_lcid_[ tid ] == (*sources_[ current_tid_[ tid ] ])[ current_syn_id_[ tid ] ].size() )
        {
          current_lcid_[ tid ] = 0;
          ++current_syn_id_[ tid ];
          continue;
        }
        else
        {
          // the current position contains an entry, so we retrieve it
          Source& current_source = ( *sources_[ current_tid_[ tid ] ] )[ current_syn_id_[ tid ] ][ current_lcid_[ tid ] ];
          target_rank = kernel().node_manager.get_process_id_of_gid( current_source.gid );
          // now we need to determine whether this thread is
          // responsible for this part of the MPI buffer; if not we
          // just continue with the next iteration of the loop
          if ( rank_start <= target_rank && target_rank < rank_end )
          {
            if ( current_source.processed )
            {
              // looks like we've processed this already, let's
              // continue
              ++current_lcid_[ tid ];
              continue;
            }
            else
            {
              // we have found a valid entry, so mark it as processed,
              // update the values of next_target_data and return
              current_source.processed = true;
              next_target_data.lid = kernel().vp_manager.gid_to_lid( current_source.gid );
              next_target_data.tid = kernel().vp_manager.vp_to_thread( kernel().vp_manager.suggest_vp( current_source.gid ) );
              // we store the position in the sources table, not our own
              next_target_data.target.tid = current_tid_[ tid ];
              next_target_data.target.rank = kernel().mpi_manager.get_rank();
              next_target_data.target.processed = false;
              next_target_data.target.syn_index = current_syn_id_[ tid ];
              next_target_data.target.lcid = current_lcid_[ tid ];
              ++current_lcid_[ tid ];
              return true;
            }
          }
          else
          {
            ++current_lcid_[ tid ];
            continue;
          }
        }
      }
    }
  }
}

} // namespace nest

#endif /* SOURCE_TABLE_IMPL_H */
