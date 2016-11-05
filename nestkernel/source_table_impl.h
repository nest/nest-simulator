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

// C++ includes
#include <algorithm> // for std::max_element

// Includes from nestkernel:
#include "connection_manager_impl.h"
#include "event_impl.h"
#include "kernel_manager.h"
#include "mpi_manager.h"
#include "node_manager_impl.h"
#include "source_table.h"
#include "target_table.h"

namespace nest
{

inline bool
SourceTable::get_next_target_data( const thread tid, const thread rank_start, const thread rank_end, const size_t secondary_buffer_chunk_size, thread& target_rank, TargetData& next_target_data )
{
  SourceTablePosition& current_position = *current_positions_[ tid ];
  // we stay in this loop either until we can return a valid
  // TargetData object or we have reached the end of the sources table
  while( true )
  {
    // check for validity of indices and update if necessary
    if ( current_position.lcid < 0 )
    {
      --current_position.syn_index;
      if ( current_position.syn_index >= 0 )
      {
        current_position.lcid = (* sources_[ current_position.tid ] )[ current_position.syn_index ]->size() - 1;
        continue;
      }
      else
      {
        --current_position.tid;
        if ( current_position.tid >= 0 )
        {
          current_position.syn_index = ( *sources_[ current_position.tid ] ).size() - 1;
          if ( current_position.syn_index >= 0 )
          {
            current_position.lcid = (* sources_[ current_position.tid ] )[ current_position.syn_index ]->size() - 1;
          }
          continue;
        }
        else
        {
          assert( current_position.tid < 0 );
          assert( current_position.syn_index < 0 );
          assert( current_position.lcid < 0 );
          return false; // reached the end of the sources table
        }
      }
    }

    // the current position contains an entry, so we retrieve it
    Source& current_source = ( *( *sources_[ current_position.tid ] )[ current_position.syn_index ])[ current_position.lcid ];
    if ( current_source.processed )
    {
      // looks like we've processed this already, let's
      // continue
      --current_position.lcid;
      continue;
    }

    // TODO@5g: this really is the source rank, isn't it? rename?
    target_rank = kernel().node_manager.get_process_id_of_gid( current_source.gid );
    // now we need to determine whether this thread is
    // responsible for this part of the MPI buffer; if not we
    // just continue with the next iteration of the loop
    if ( target_rank < rank_start || target_rank >= rank_end )
    {
      --current_position.lcid;
      continue;
    }

    // we have found a valid entry, so mark it as processed
    current_source.processed = true;

    // we need to set the marker whether the entry following this
    // entry, if existent, has the same source
    if ( ( current_position.lcid + 1 < static_cast< long >( ( *sources_[ current_position.tid ] )[ current_position.syn_index  ]->size() )
           && ( *( *sources_[ current_position.tid ] )[ current_position.syn_index ])[ current_position.lcid + 1 ].gid == current_source.gid ) )
    {
      kernel().connection_manager.set_has_source_subsequent_targets( current_position.tid, current_position.syn_index, current_position.lcid, true );
    }

    // we decrease the counter without returning a TargetData if the
    // entry preceeding this entry has the same source
    if ( ( current_position.lcid - 1 > -1
           && (*( *sources_[ current_position.tid ] )[ current_position.syn_index  ])[ current_position.lcid - 1 ].gid == current_source.gid ) )
    {
      --current_position.lcid;
      continue;
    }
    // otherwise we return a valid TargetData
    else
    {
      // set values of next_target_data
      next_target_data.set_lid( kernel().vp_manager.gid_to_lid( current_source.gid ) );
      next_target_data.set_tid( kernel().vp_manager.vp_to_thread( kernel().vp_manager.suggest_vp( current_source.gid ) ) );
      if ( current_source.is_primary )
      {
        next_target_data.is_primary( true );
        // we store the thread index of the sources table, not our own tid
        next_target_data.get_target().set_tid( current_position.tid );
        next_target_data.get_target().set_rank( kernel().mpi_manager.get_rank() );
        next_target_data.get_target().set_processed( false );
        next_target_data.get_target().set_syn_index( current_position.syn_index );
        next_target_data.get_target().set_lcid( current_position.lcid );
      }
      else
      {
        next_target_data.is_primary( false );
        const size_t recv_buffer_pos = kernel().connection_manager.get_secondary_recv_buffer_position( current_position.tid, current_position.syn_index, current_position.lcid );
        const size_t send_buffer_pos = kernel().mpi_manager.get_rank() * secondary_buffer_chunk_size + ( recv_buffer_pos - target_rank * secondary_buffer_chunk_size );
        reinterpret_cast< SecondaryTargetData* >( &next_target_data )->set_send_buffer_pos( send_buffer_pos );
      }
      --current_position.lcid;
      return true; // found a valid entry
    }
  }
}

inline size_t
SourceTable::compute_send_recv_count_secondary_in_int_per_rank() const
{
  std::vector< size_t > count_per_rank( kernel().mpi_manager.get_num_processes() );

#pragma omp parallel shared( count_per_rank )
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    for ( size_t syn_index = 0; syn_index < sources_[ tid ]->size(); ++syn_index )
    {
      const synindex syn_id = kernel().connection_manager.get_syn_id( tid, syn_index );
      if ( not kernel().model_manager.get_synapse_prototype( syn_id, tid ).is_primary() )
      {
        const size_t event_size = kernel().model_manager.get_secondary_event_prototype( syn_id, tid ).prototype_size();

        index last_gid = invalid_index;
        for ( std::vector< Source >::const_iterator cit = ( *sources_[ tid ] )[ syn_index ]->begin(); cit != ( *sources_[ tid ] )[ syn_index ]->end(); ++cit )
        {
          const index gid = cit->gid;

          // during delivery all targets from the same source should
          // read the same MPI buffer entry, hence we only need to
          // count unique gids to determine the total number of
          // required entries in the MPI buffer
          if ( gid != last_gid )
          {
            const thread target_rank = kernel().node_manager.get_process_id_of_gid( gid );
#pragma omp atomic
            count_per_rank[ target_rank ] += event_size;

            last_gid = gid;
          }
        }
      }
    }
  }
  for ( std::vector< size_t >::const_iterator it = count_per_rank.begin(); it != count_per_rank.end(); ++it )
  {
    std::cout<<(*it)<<", ";
  }
  std::cout<<std::endl;
  std::vector< size_t > max_count( 1, *std::max_element( count_per_rank.begin(), count_per_rank.end() ) );
  kernel().mpi_manager.communicate_Allreduce_max_in_place( max_count );
  std::cout<<"max "<<max_count[ 0 ] << std::endl;
  return max_count[ 0 ];
}

} // namespace nest

#endif /* SOURCE_TABLE_IMPL_H */
