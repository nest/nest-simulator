/*
 *  target_table_impl.h
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

#ifndef TARGET_TABLE_IMPL_H
#define TARGET_TABLE_IMPL_H

// Includes from nestkernel:
#include "kernel_manager.h"
#include "target_table.h"
#include "vp_manager_impl.h"
#include "spike_register_table.h"

namespace nest
{

inline void
TargetTable::add_target( thread tid, const TargetData& target_data )
{
  const index lid = kernel().vp_manager.gid_to_lid( target_data.gid );
  assert( tid < targets_.size() );
  assert( lid < targets_[ tid ]->size() );
  (*targets_[ tid ])[ lid ].push_back( target_data.target );
}

inline bool
TargetTable::get_next_spike_data( const thread tid, const thread current_tid, const index lid, index& rank, SpikeData& next_spike_data, const unsigned int rank_start, const unsigned int rank_end )
{
  // we stay in this loop either until we can return a valid SpikeData
  // object or we have reached the end of the target vector for this
  // node
  while ( true )
  {
    assert( current_target_index_[ tid ] <= (*targets_[ current_tid ])[ lid ].size() );
    if ( current_target_index_[ tid ] == (*targets_[ current_tid ])[ lid ].size() )
    {
      // reached the end of the target vector for this node, so we
      // reset the current_target_index and return false.
      current_target_index_[ tid ] = 0;
      return false;
    }
    else
    {
      // the current position contains an entry, so we retrieve it
      Target& current_target = (*targets_[ current_tid ])[ lid ][ current_target_index_[ tid ] ];
      // we determine whether this thread is responsible for this part
      // of the MPI buffer; if not, we continue with the loop
      if ( rank_start <= current_target.rank && current_target.rank < rank_end )
      {
        if ( current_target.processed == (*target_processed_flag_[ current_tid ])[ lid ] )
        {
          // looks like we've processed this already, let's
          // continue
          ++current_target_index_[ tid ];
          continue;
        }
        else
        {
          // we have found a valid entry, so mark it as processed and
          // set appropiate values for rank and next_spike_data
          current_target.processed = (*target_processed_flag_[ current_tid ])[ lid ];
          rank = current_target.rank;
          next_spike_data.tid = current_target.tid;
          next_spike_data.syn_index = current_target.syn_index;
          next_spike_data.lcid = current_target.lcid;
          ++current_target_index_[ tid ];
          return true;
        }
      }
      else
      {
        ++current_target_index_[ tid ];
        continue;
      }
    }
  }
}

} // namespace nest

#endif /* TARGET_TABLE_IMPL_H */
