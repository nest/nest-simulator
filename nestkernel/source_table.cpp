/*
 *  source_table.cpp
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

// C++ includes:
#include <iostream>

// Includes from nestkernel:
#include "connection_manager.h"
#include "connection_manager_impl.h"
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
#include "source_table.h"
#include "vp_manager_impl.h"

nest::SourceTable::SourceTable()
{
}

nest::SourceTable::~SourceTable()
{
}

void
nest::SourceTable::initialize()
{
  assert( sizeof( Source ) == 8 );
  const thread num_threads = kernel().vp_manager.get_num_threads();
  sources_.resize( num_threads );
  is_cleared_.initialize( num_threads, false );
  saved_entry_point_.initialize( num_threads, false );
  current_positions_.resize( num_threads );
  saved_positions_.resize( num_threads );

#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    sources_[ tid ].resize( 0 );
    resize_sources( tid );
  } // of omp parallel
}

void
nest::SourceTable::finalize()
{
  for ( thread tid = 0; tid < static_cast< thread >( sources_.size() ); ++tid )
  {
    if ( is_cleared_[ tid ].is_false() )
    {
      clear( tid );
    }
  }

  sources_.clear();
  current_positions_.clear();
  saved_positions_.clear();
}

bool
nest::SourceTable::is_cleared() const
{
  return is_cleared_.all_true();
}

std::vector< BlockVector< nest::Source > >&
nest::SourceTable::get_thread_local_sources( const thread tid )
{
  return sources_[ tid ];
}

nest::SourceTablePosition
nest::SourceTable::find_maximal_position() const
{
  SourceTablePosition max_position( -1, -1, -1 );
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( max_position < saved_positions_[ tid ] )
    {
      max_position = saved_positions_[ tid ];
    }
  }
  return max_position;
}

void
nest::SourceTable::clean( const thread tid )
{
  // Find maximal position in source table among threads to make sure
  // unprocessed entries are not removed. Given this maximal position,
  // we can safely delete all larger entries since they will not be
  // touched any more.
  const SourceTablePosition max_position = find_maximal_position();

  // If this thread corresponds to max_position's thread, we can only
  // delete part of the sources table, with indices larger than those
  // in max_position; if this thread is larger than max_positions's
  // thread, we can delete all sources; otherwise we do nothing.
  if ( max_position.tid == tid )
  {
    for ( synindex syn_id = max_position.syn_id; syn_id < sources_[ tid ].size(); ++syn_id )
    {
      BlockVector< Source >& sources = sources_[ tid ][ syn_id ];
      if ( max_position.syn_id == syn_id )
      {
        // we need to add 2 to max_position.lcid since
        // max_position.lcid + 1 can contain a valid entry which we
        // do not want to delete.
        if ( max_position.lcid + 2 < static_cast< long >( sources.size() ) )
        {
          sources.erase( sources.begin() + max_position.lcid + 2, sources.end() );
        }
      }
      else
      {
        assert( max_position.syn_id < syn_id );
        sources.clear();
      }
    }
  }
  else if ( max_position.tid < tid )
  {
    sources_[ tid ].clear();
  }
  else
  {
    // do nothing
    assert( tid < max_position.tid );
  }
}

nest::index
nest::SourceTable::get_node_id( const thread tid, const synindex syn_id, const index lcid ) const
{
  if ( not kernel().connection_manager.get_keep_source_table() )
  {
    throw KernelException( "Cannot use SourceTable::get_node_id when get_keep_source_table is false" );
  }
  return sources_[ tid ][ syn_id ][ lcid ].get_node_id();
}

nest::index
nest::SourceTable::remove_disabled_sources( const thread tid, const synindex syn_id )
{
  if ( sources_[ tid ].size() <= syn_id )
  {
    return invalid_index;
  }

  BlockVector< Source >& mysources = sources_[ tid ][ syn_id ];
  const index max_size = mysources.size();
  if ( max_size == 0 )
  {
    return invalid_index;
  }

  // lcid needs to be signed, to allow lcid >= 0 check in while loop
  // to fail; afterwards we can be certain that it is non-negative and
  // we can static_cast it to index
  long lcid = max_size - 1;
  while ( lcid >= 0 and mysources[ lcid ].is_disabled() )
  {
    --lcid;
  }
  ++lcid; // lcid marks first disabled source, but the while loop only
          // exits if lcid points at a not disabled element, hence we
          // need to increase it by one again
  mysources.erase( mysources.begin() + lcid, mysources.end() );
  if ( static_cast< index >( lcid ) == max_size )
  {
    return invalid_index;
  }
  return static_cast< index >( lcid );
}

void
nest::SourceTable::compute_buffer_pos_for_unique_secondary_sources( const thread tid,
  std::map< index, size_t >& buffer_pos_of_source_node_id_syn_id )
{
  // set of unique sources & synapse types, required to determine
  // secondary events MPI buffer positions
  // initialized and deleted by thread 0 in this method
  static std::set< std::pair< index, size_t > >* unique_secondary_source_node_id_syn_id;
#pragma omp single
  {
    unique_secondary_source_node_id_syn_id = new std::set< std::pair< index, size_t > >();
  }

  // collect all unique pairs of source node ID and synapse-type id
  // corresponding to continuous-data connections on this MPI rank;
  // using a set makes sure secondary events are not duplicated for
  // targets on the same process, but different threads
  for ( size_t syn_id = 0; syn_id < sources_[ tid ].size(); ++syn_id )
  {
    if ( not kernel().model_manager.get_synapse_prototype( syn_id, tid ).is_primary() )
    {
      for ( BlockVector< Source >::const_iterator source_cit = sources_[ tid ][ syn_id ].begin();
            source_cit != sources_[ tid ][ syn_id ].end();
            ++source_cit )
      {
#pragma omp critical
        {
          ( *unique_secondary_source_node_id_syn_id ).insert( std::make_pair( source_cit->get_node_id(), syn_id ) );
        }
      }
    }
  }
#pragma omp barrier

#pragma omp single
  {
    // compute receive buffer positions for all unique pairs of source
    // node ID and synapse-type id on this MPI rank
    std::vector< int > recv_counts_secondary_events_in_int_per_rank( kernel().mpi_manager.get_num_processes(), 0 );

    for (
      std::set< std::pair< index, size_t > >::const_iterator cit = ( *unique_secondary_source_node_id_syn_id ).begin();
      cit != ( *unique_secondary_source_node_id_syn_id ).end();
      ++cit )
    {
      const thread source_rank = kernel().mpi_manager.get_process_id_of_node_id( cit->first );
      const size_t event_size = kernel().model_manager.get_secondary_event_prototype( cit->second, tid ).size();

      buffer_pos_of_source_node_id_syn_id.insert(
        std::make_pair( pack_source_node_id_and_syn_id( cit->first, cit->second ),
          recv_counts_secondary_events_in_int_per_rank[ source_rank ] ) );

      recv_counts_secondary_events_in_int_per_rank[ source_rank ] += event_size;
    }

    // each chunk needs to contain one additional int that can be used
    // to communicate whether waveform relaxation has converged
    for ( auto& recv_count : recv_counts_secondary_events_in_int_per_rank )
    {
      ++recv_count;
    }

    kernel().mpi_manager.set_recv_counts_secondary_events_in_int_per_rank(
      recv_counts_secondary_events_in_int_per_rank );
    delete unique_secondary_source_node_id_syn_id;
  } // of omp single
}

void
nest::SourceTable::resize_sources( const thread tid )
{
  sources_[ tid ].resize( kernel().model_manager.get_num_synapse_prototypes() );
}

bool
nest::SourceTable::source_should_be_processed_( const thread rank_start,
  const thread rank_end,
  const Source& source ) const
{
  const thread source_rank = kernel().mpi_manager.get_process_id_of_node_id( source.get_node_id() );

  return not( source.is_processed() or source.is_disabled()
           // is this thread responsible for this part of the MPI
           // buffer?
           or source_rank < rank_start
           or rank_end <= source_rank );
}

bool
nest::SourceTable::next_entry_has_same_source_( const SourceTablePosition& current_position,
  const Source& current_source ) const
{
  assert( not current_position.is_invalid() );

  const auto& local_sources = sources_[ current_position.tid ][ current_position.syn_id ];
  const size_t next_lcid = current_position.lcid + 1;

  return (
    next_lcid < local_sources.size() and local_sources[ next_lcid ].get_node_id() == current_source.get_node_id() );
}

bool
nest::SourceTable::previous_entry_has_same_source_( const SourceTablePosition& current_position,
  const Source& current_source ) const
{
  assert( not current_position.is_invalid() );

  const auto& local_sources = sources_[ current_position.tid ][ current_position.syn_id ];
  const long previous_lcid = current_position.lcid - 1; // needs to be a signed type such that negative
                                                        // values can signal invalid indices

  return ( previous_lcid >= 0 and not local_sources[ previous_lcid ].is_processed()
    and local_sources[ previous_lcid ].get_node_id() == current_source.get_node_id() );
}

void
nest::SourceTable::populate_target_data_fields_( const SourceTablePosition& current_position,
  const Source& current_source,
  const thread source_rank,
  TargetData& next_target_data ) const
{
  const auto node_id = current_source.get_node_id();

  // set values of next_target_data
  next_target_data.set_source_lid( kernel().vp_manager.node_id_to_lid( node_id ) );
  next_target_data.set_source_tid( kernel().vp_manager.vp_to_thread( kernel().vp_manager.node_id_to_vp( node_id ) ) );
  next_target_data.reset_marker();

  if ( current_source.is_primary() ) // primary connection, i.e., chemical synapses
  {
    next_target_data.set_is_primary( true );

    TargetDataFields& target_fields = next_target_data.target_data;
    // we store the thread index of the source table, not our own tid!
    target_fields.set_tid( current_position.tid );
    target_fields.set_syn_id( current_position.syn_id );
    target_fields.set_lcid( current_position.lcid );
  }
  else // secondary connection, e.g., gap junctions
  {
    next_target_data.set_is_primary( false );

    // the source rank will write to the buffer position relative to
    // the first position from the absolute position in the receive
    // buffer
    const size_t relative_recv_buffer_pos = kernel().connection_manager.get_secondary_recv_buffer_position(
                                              current_position.tid, current_position.syn_id, current_position.lcid )
      - kernel().mpi_manager.get_recv_displacement_secondary_events_in_int( source_rank );

    SecondaryTargetDataFields& secondary_fields = next_target_data.secondary_data;
    secondary_fields.set_recv_buffer_pos( relative_recv_buffer_pos );
    secondary_fields.set_syn_id( current_position.syn_id );
  }
}

bool
nest::SourceTable::get_next_target_data( const thread tid,
  const thread rank_start,
  const thread rank_end,
  thread& source_rank,
  TargetData& next_target_data )
{
  SourceTablePosition& current_position = current_positions_[ tid ];

  if ( current_position.is_invalid() )
  {
    return false; // nothing to do here
  }

  // we stay in this loop either until we can return a valid
  // TargetData object or we have reached the end of the sources table
  while ( true )
  {
    current_position.seek_to_next_valid_index( sources_ );
    if ( current_position.is_invalid() )
    {
      return false; // reached the end of the sources table
    }

    // the current position contains an entry, so we retrieve it
    Source& current_source = sources_[ current_position.tid ][ current_position.syn_id ][ current_position.lcid ];

    if ( not source_should_be_processed_( rank_start, rank_end, current_source ) )
    {
      current_position.decrease();
      continue;
    }

    // we need to set a marker stating whether the entry following this
    // entry, if existent, has the same source
    kernel().connection_manager.set_source_has_more_targets( current_position.tid,
      current_position.syn_id,
      current_position.lcid,
      next_entry_has_same_source_( current_position, current_source ) );

    // no need to communicate this entry if the previous entry has the same source
    if ( previous_entry_has_same_source_( current_position, current_source ) )
    {
      current_source.set_processed( true ); // no need to look at this entry again
      current_position.decrease();
      continue;
    }

    // reaching this means we found an entry that should be
    // communicated via MPI, so we prepare to return the relevant data

    // set the source rank
    source_rank = kernel().mpi_manager.get_process_id_of_node_id( current_source.get_node_id() );

    populate_target_data_fields_( current_position, current_source, source_rank, next_target_data );

    // we are about to return a valid entry, so mark it as processed
    current_source.set_processed( true );

    current_position.decrease();
    return true; // found a valid entry
  }
}
