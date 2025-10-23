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
#include "kernel_manager.h"
#include "model_manager.h"
#include "source_table.h"

#include "connector_model.h"

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
  const size_t num_threads = kernel::manager< VPManager >.get_num_threads();
  sources_.resize( num_threads );
  is_cleared_.initialize( num_threads, false );
  saved_entry_point_.initialize( num_threads, false );
  current_positions_.resize( num_threads );
  saved_positions_.resize( num_threads );
  compressible_sources_.resize( num_threads );

#pragma omp parallel
  {
    const size_t tid = kernel::manager< VPManager >.get_thread_id();
    sources_.at( tid ).resize( 0 );
    resize_sources();
    compressible_sources_.at( tid ).resize( 0 );
  } // of omp parallel
}

void
nest::SourceTable::finalize()
{
  for ( size_t tid = 0; tid < static_cast< size_t >( sources_.size() ); ++tid )
  {
    if ( is_cleared_[ tid ].is_false() )
    {
      clear( tid );
      compressible_sources_[ tid ].clear();
    }
  }

  sources_.clear();
  current_positions_.clear();
  saved_positions_.clear();
  compressible_sources_.clear();
  compressed_spike_data_map_.clear();
}

bool
nest::SourceTable::is_cleared() const
{
  return is_cleared_.all_true();
}

std::vector< BlockVector< nest::Source > >&
nest::SourceTable::get_thread_local_sources( const size_t tid )
{
  return sources_[ tid ];
}

nest::SourceTablePosition
nest::SourceTable::find_maximal_position() const
{
  SourceTablePosition max_position( -1, -1, -1 );
  for ( size_t tid = 0; tid < kernel::manager< VPManager >.get_num_threads(); ++tid )
  {
    if ( max_position < saved_positions_[ tid ] )
    {
      max_position = saved_positions_[ tid ];
    }
  }
  return max_position;
}

void
nest::SourceTable::clean( const size_t tid )
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
  if ( max_position.tid == static_cast< long >( tid ) )
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
  else if ( max_position.tid < static_cast< long >( tid ) )
  {
    sources_[ tid ].clear();
  }
  else
  {
    // do nothing
    assert( static_cast< long >( tid ) < max_position.tid );
  }
}

size_t
nest::SourceTable::get_node_id( const size_t tid, const synindex syn_id, const size_t lcid ) const
{
  if ( not kernel::manager< ConnectionManager >.get_keep_source_table() )
  {
    throw KernelException( "Cannot use SourceTable::get_node_id when get_keep_source_table is false" );
  }
  return sources_[ tid ][ syn_id ][ lcid ].get_node_id();
}

size_t
nest::SourceTable::remove_disabled_sources( const size_t tid, const synindex syn_id )
{
  if ( sources_[ tid ].size() <= syn_id )
  {
    return invalid_index; // no source table entry for this synapse model
  }

  BlockVector< Source >& mysources = sources_[ tid ][ syn_id ];
  const size_t max_size = mysources.size();
  if ( max_size == 0 )
  {
    return invalid_index; // no connections for this synapse model
  }

  // lcid needs to be signed, to allow lcid >= 0 check in while loop
  // to fail; afterwards we can be certain that it is non-negative and
  // we can static_cast it to index
  long lcid = max_size - 1;
  while ( lcid >= 0 and mysources[ lcid ].is_disabled() )
  {
    --lcid;
  }
  const size_t first_invalid_lcid = static_cast< size_t >( lcid + 1 ); // loop stopped on first valid entry or -1
  if ( first_invalid_lcid == max_size )
  {
    return invalid_index; // all lcids are valid, nothing to remove
  }

  mysources.erase( mysources.begin() + first_invalid_lcid, mysources.end() );
  return first_invalid_lcid;
}

void
nest::SourceTable::compute_buffer_pos_for_unique_secondary_sources( const size_t tid,
  std::map< size_t, size_t >& buffer_pos_of_source_node_id_syn_id )
{
  // set of unique sources & synapse types, required to determine
  // secondary events MPI buffer positions
  // initialized and deleted by thread 0 in this method
  static std::set< std::pair< size_t, size_t > >* unique_secondary_source_node_id_syn_id;
#pragma omp single
  {
    unique_secondary_source_node_id_syn_id = new std::set< std::pair< size_t, size_t > >();
  }

  // collect all unique pairs of source node ID and synapse-type id
  // corresponding to continuous-data connections on this MPI rank;
  // using a set makes sure secondary events are not duplicated for
  // targets on the same process, but different threads
  for ( size_t syn_id = 0; syn_id < sources_[ tid ].size(); ++syn_id )
  {
    const ConnectorModel& conn_model = kernel::manager< ModelManager >.get_connection_model( syn_id, tid );
    const bool is_primary = conn_model.has_property( ConnectionModelProperties::IS_PRIMARY );

    if ( not is_primary )
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
  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();

#pragma omp single
  {
    // compute receive buffer positions for all unique pairs of source
    // node ID and synapse-type id on this MPI rank
    std::vector< int > recv_counts_secondary_events_in_int_per_rank(
      kernel::manager< MPIManager >.get_num_processes(), 0 );

    for ( std::set< std::pair< size_t, size_t > >::const_iterator cit =
            ( *unique_secondary_source_node_id_syn_id ).begin();
          cit != ( *unique_secondary_source_node_id_syn_id ).end();
          ++cit )
    {
      const size_t source_rank = kernel::manager< MPIManager >.get_process_id_of_node_id( cit->first );
      const size_t event_size =
        kernel::manager< ModelManager >.get_secondary_event_prototype( cit->second, tid )->size();

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

    kernel::manager< MPIManager >.set_recv_counts_secondary_events_in_int_per_rank(
      recv_counts_secondary_events_in_int_per_rank );
    delete unique_secondary_source_node_id_syn_id;
  } // of omp single
}

void
nest::SourceTable::resize_sources()
{
  kernel::manager< VPManager >.assert_thread_parallel();
  sources_.at( kernel::manager< VPManager >.get_thread_id() )
    .resize( kernel::manager< ModelManager >.get_num_connection_models() );
}

bool
nest::SourceTable::source_should_be_processed_( const size_t rank_start,
  const size_t rank_end,
  const Source& source ) const
{
  const size_t source_rank = kernel::manager< MPIManager >.get_process_id_of_node_id( source.get_node_id() );

  return not( source.is_processed()
    or source.is_disabled()
    // is this thread responsible for this part of the MPI buffer?
    or source_rank < rank_start or rank_end <= source_rank );
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

bool
nest::SourceTable::populate_target_data_fields_( const SourceTablePosition& current_position,
  const Source& current_source,
  const size_t source_rank,
  TargetData& next_target_data ) const
{
  assert( not kernel::manager< ConnectionManager >.use_compressed_spikes() ); // handled elsewhere

  const auto node_id = current_source.get_node_id();

  // set values of next_target_data
  next_target_data.set_source_lid( kernel::manager< VPManager >.node_id_to_lid( node_id ) );
  next_target_data.set_source_tid(
    kernel::manager< VPManager >.vp_to_thread( kernel::manager< VPManager >.node_id_to_vp( node_id ) ) );
  next_target_data.reset_marker();

  if ( current_source.is_primary() ) // primary connection, i.e., chemical synapses
  {
    next_target_data.set_is_primary( true );

    TargetDataFields& target_fields = next_target_data.target_data;
    target_fields.set_syn_id( current_position.syn_id );

    // we store the thread index of the source table, not our own tid!
    target_fields.set_tid( current_position.tid );
    target_fields.set_lcid( current_position.lcid );
  }
  else // secondary connection, e.g., gap junctions
  {
    next_target_data.set_is_primary( false );

    // the source rank will write to the buffer position relative to
    // the first position from the absolute position in the receive
    // buffer
    const size_t relative_recv_buffer_pos = kernel::manager< ConnectionManager >.get_secondary_recv_buffer_position(
                                              current_position.tid, current_position.syn_id, current_position.lcid )
      - kernel::manager< MPIManager >.get_recv_displacement_secondary_events_in_int( source_rank );

    SecondaryTargetDataFields& secondary_fields = next_target_data.secondary_data;
    secondary_fields.set_recv_buffer_pos( relative_recv_buffer_pos );
    secondary_fields.set_syn_id( current_position.syn_id );
  }

  return true;
}

bool
nest::SourceTable::get_next_target_data( const size_t tid,
  const size_t rank_start,
  const size_t rank_end,
  size_t& source_rank,
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
    kernel::manager< ConnectionManager >.set_source_has_more_targets( current_position.tid,
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
    source_rank = kernel::manager< MPIManager >.get_process_id_of_node_id( current_source.get_node_id() );

    if ( not populate_target_data_fields_( current_position, current_source, source_rank, next_target_data ) )
    {
      current_position.decrease();
      continue;
    }

    // we are about to return a valid entry, so mark it as processed
    current_source.set_processed( true );

    current_position.decrease();
    return true; // found a valid entry
  }
}

void
nest::SourceTable::resize_compressible_sources()
{
  for ( size_t tid = 0; tid < static_cast< size_t >( compressible_sources_.size() ); ++tid )
  {
    compressible_sources_[ tid ].clear();
    compressible_sources_[ tid ].resize(
      kernel::manager< ModelManager >.get_num_connection_models(), std::map< size_t, SpikeData >() );
  }
}

void
nest::SourceTable::collect_compressible_sources( const size_t tid )
{
  for ( synindex syn_id = 0; syn_id < sources_[ tid ].size(); ++syn_id )
  {
    size_t lcid = 0;
    auto& syn_sources = sources_[ tid ][ syn_id ];
    while ( lcid < syn_sources.size() )
    {
      const size_t old_source_node_id = syn_sources[ lcid ].get_node_id();
      const std::pair< size_t, SpikeData > source_node_id_to_spike_data =
        std::make_pair( old_source_node_id, SpikeData( tid, syn_id, lcid, 0 ) );
      compressible_sources_[ tid ][ syn_id ].insert( source_node_id_to_spike_data );

      // For all subsequent connections with same source, set "has more targets" on preceding connection.
      // Requires sorted connections.
      ++lcid;
      while ( ( lcid < syn_sources.size() ) and ( syn_sources[ lcid ].get_node_id() == old_source_node_id ) )
      {
        kernel::manager< ConnectionManager >.set_source_has_more_targets( tid, syn_id, lcid - 1, true );
        ++lcid;
      }
      // Mark last connection in sequence as not having successor. This is essential if connections are
      // delete, e.g., by structural plasticity, because we do not globally reset the more_targets flag.
      assert( lcid - 1 < syn_sources.size() );
      kernel::manager< ConnectionManager >.set_source_has_more_targets( tid, syn_id, lcid - 1, false );
    }
  }
}

void
nest::SourceTable::dump_sources() const
{
  FULL_LOGGING_ONLY( for ( size_t tid = 0; tid < sources_.size(); ++tid ) {
    for ( size_t syn_id = 0; syn_id < sources_[ tid ].size(); ++syn_id )
    {
      for ( size_t lcid = 0; lcid < sources_[ tid ][ syn_id ].size(); ++lcid )
      {
        kernel::manager< KernelManager >.write_to_dump( String::compose( "src  : r%1 t%2 s%3 tg%4 l%5 tt%6",
          kernel::manager< MPIManager >.get_rank(),
          kernel::manager< VPManager >.get_thread_id(),
          sources_[ tid ][ syn_id ][ lcid ].get_node_id(),
          kernel::manager< ConnectionManager >.get_target_node_id( tid, syn_id, lcid ),
          lcid,
          tid ) );
      }
    }
  } )
}


void
nest::SourceTable::dump_compressible_sources() const
{
  FULL_LOGGING_ONLY( for ( size_t tid = 0; tid < compressible_sources_.size(); ++tid ) {
    for ( size_t syn_id = 0; syn_id < compressible_sources_[ tid ].size(); ++syn_id )
    {
      for ( const auto& entry : compressible_sources_[ tid ][ syn_id ] )
      {
        kernel::manager< KernelManager >.write_to_dump( String::compose( "csrc : r%1 t%2 s%3 l%4 tt%5",
          kernel::manager< MPIManager >.get_rank(),
          kernel::manager< VPManager >.get_thread_id(),
          entry.first,
          entry.second.get_lcid(),
          entry.second.get_tid() ) );
      }
    }
  } )
}

void
nest::SourceTable::fill_compressed_spike_data(
  std::vector< std::vector< std::vector< SpikeData > > >& compressed_spike_data )
{
  const size_t num_synapse_models = kernel::manager< ModelManager >.get_num_connection_models();
  compressed_spike_data.clear();
  compressed_spike_data.resize( num_synapse_models );
  compressed_spike_data_map_.clear();
  compressed_spike_data_map_.resize( num_synapse_models, std::map< size_t, CSDMapEntry >() );

  // For each synapse type, and for each source neuron with at least one local target,
  // store in compressed_spike_data one SpikeData entry for each local thread that
  // owns a local target. In compressed_spike_data_map_ store index into compressed_spike_data[syn_id]
  // where data for a given source is stored.

  // TODO: I believe that at this point compressible_sources_ is ordered by source gid.
  //       Maybe one can exploit that to avoid searching with find() below.
  for ( synindex syn_id = 0; syn_id < kernel::manager< ModelManager >.get_num_connection_models(); ++syn_id )
  {
    for ( size_t target_thread = 0; target_thread < static_cast< size_t >( compressible_sources_.size() );
          ++target_thread )
    {
      for ( const auto& connection : compressible_sources_[ target_thread ][ syn_id ] )
      {
        const auto source_gid = connection.first;

        if ( compressed_spike_data_map_[ syn_id ].find( source_gid ) == compressed_spike_data_map_[ syn_id ].end() )
        {
          // Set up entry for new source
          const auto new_source_index = compressed_spike_data[ syn_id ].size();

          compressed_spike_data[ syn_id ].emplace_back( kernel::manager< VPManager >.get_num_threads(),
            SpikeData( invalid_targetindex, invalid_synindex, invalid_lcid, 0 ) );

          compressed_spike_data_map_[ syn_id ].insert(
            std::make_pair( source_gid, CSDMapEntry( new_source_index, target_thread ) ) );
        }

        const auto source_index = compressed_spike_data_map_[ syn_id ].find( source_gid )->second.get_source_index();

        assert( compressed_spike_data[ syn_id ][ source_index ][ target_thread ].get_lcid() == invalid_lcid );

        compressed_spike_data[ syn_id ][ source_index ][ target_thread ] = connection.second;
      } // for connection

      compressible_sources_[ target_thread ][ syn_id ].clear();

    } // for target_thread
  }   // for syn_id
}

// Argument name only needed if full logging is activated. Macro-protect to avoid unused argument warning.
void
nest::SourceTable::dump_compressed_spike_data(
  const std::vector< std::vector< std::vector< SpikeData > > >& FULL_LOGGING_ONLY( compressed_spike_data ) ) const
{
  FULL_LOGGING_ONLY(
    for ( const auto& tab
          : compressed_spike_data_map_ ) {
      for ( const auto& entry : tab )
      {
        kernel::manager< KernelManager >.write_to_dump( String::compose( "csdm : r%1 t%2 s%3 sx%4 tt%5",
          kernel::manager< MPIManager >.get_rank(),
          kernel::manager< VPManager >.get_thread_id(),
          entry.first,
          entry.second.get_source_index(),
          entry.second.get_target_thread() ) );
      }
    }

    for ( const auto& tab
          : compressed_spike_data ) {
      for ( size_t six = 0; six < tab.size(); ++six )
      {
        for ( size_t tx = 0; tx < tab[ six ].size(); ++tx )
        {
          kernel::manager< KernelManager >.write_to_dump( String::compose( "csd  : r%1 t%2 six%3 tx%4 l%5 tt%6",
            kernel::manager< MPIManager >.get_rank(),
            kernel::manager< VPManager >.get_thread_id(),
            six,
            tx,
            tab[ six ][ tx ].get_lcid(),
            tab[ six ][ tx ].get_tid() ) );
        }
      }
    } )
}
void
nest::SourceTable::clear_compressed_spike_data_map()
{

  for ( auto& source_index_map : compressed_spike_data_map_ )
  {
    source_index_map.clear();
  }
}

size_t
nest::SourceTable::pack_source_node_id_and_syn_id( const size_t source_node_id, const synindex syn_id ) const
{

  assert( source_node_id < 72057594037927936 );
  assert( syn_id < invalid_synindex );
  // syn_id is maximally 256, so shifting node ID by 8 bits and storing
  // syn_id in the lowest 8 leads to a unique number
  return ( source_node_id << 8 ) + syn_id;
}

size_t
nest::SourceTable::num_unique_sources( const size_t tid, const synindex syn_id ) const
{

  size_t n = 0;
  size_t last_source = 0;
  for ( BlockVector< Source >::const_iterator cit = sources_[ tid ][ syn_id ].begin();
        cit != sources_[ tid ][ syn_id ].end();
        ++cit )
  {
    if ( last_source != ( *cit ).get_node_id() )
    {
      last_source = ( *cit ).get_node_id();
      ++n;
    }
  }
  return n;
}

void
nest::SourceTable::get_source_node_ids( const size_t tid,
  const synindex syn_id,
  const std::vector< size_t >& source_lcids,
  std::vector< size_t >& sources )
{

  for ( std::vector< size_t >::const_iterator cit = source_lcids.begin(); cit != source_lcids.end(); ++cit )
  {
    sources.push_back( sources_[ tid ][ syn_id ][ *cit ].get_node_id() );
  }
}

void
nest::SourceTable::disable_connection( const size_t tid, const synindex syn_id, const size_t lcid )
{

  // disabling a source changes its node ID to 2^62 -1
  // source here
  assert( not sources_[ tid ][ syn_id ][ lcid ].is_disabled() );
  sources_[ tid ][ syn_id ][ lcid ].disable();
}

size_t
nest::SourceTable::find_first_source( const size_t tid, const synindex syn_id, const size_t snode_id ) const
{

  // binary search in sorted sources
  const BlockVector< Source >::const_iterator begin = sources_[ tid ][ syn_id ].begin();
  const BlockVector< Source >::const_iterator end = sources_[ tid ][ syn_id ].end();
  BlockVector< Source >::const_iterator it = std::lower_bound( begin, end, Source( snode_id, true ) );

  // source found by binary search could be disabled, iterate through
  // sources until a valid one is found
  while ( it != end )
  {
    if ( it->get_node_id() == snode_id and not it->is_disabled() )
    {
      const size_t lcid = it - begin;
      return lcid;
    }
    ++it;
  }

  // no enabled entry with this snode ID found
  return invalid_index;
}

void
nest::SourceTable::no_targets_to_process( const size_t tid )
{

  current_positions_[ tid ].tid = -1;
  current_positions_[ tid ].syn_id = -1;
  current_positions_[ tid ].lcid = -1;
}

void
nest::SourceTable::reset_processed_flags( const size_t tid )
{

  for ( std::vector< BlockVector< Source > >::iterator it = sources_[ tid ].begin(); it != sources_[ tid ].end(); ++it )
  {
    for ( BlockVector< Source >::iterator iit = it->begin(); iit != it->end(); ++iit )
    {
      iit->set_processed( false );
    }
  }
}

void
nest::SourceTable::reset_entry_point( const size_t tid )
{

  // Since we read the source table backwards, we need to set saved
  // values to the biggest possible value. These will be used to
  // initialize current_positions_ correctly upon calling
  // restore_entry_point. However, this can only be done if other
  // values have valid values.
  saved_positions_[ tid ].tid = sources_.size() - 1;
  if ( saved_positions_[ tid ].tid > -1 )
  {
    saved_positions_[ tid ].syn_id = sources_[ saved_positions_[ tid ].tid ].size() - 1;
  }
  else
  {
    saved_positions_[ tid ].syn_id = -1;
  }
  if ( saved_positions_[ tid ].syn_id > -1 )
  {
    saved_positions_[ tid ].lcid = sources_[ saved_positions_[ tid ].tid ][ saved_positions_[ tid ].syn_id ].size() - 1;
  }
  else
  {
    saved_positions_[ tid ].lcid = -1;
  }
}

void
nest::SourceTable::restore_entry_point( const size_t tid )
{

  current_positions_[ tid ] = saved_positions_[ tid ];
  saved_entry_point_.set_false( tid );
}

void
nest::SourceTable::save_entry_point( const size_t tid )
{

  if ( saved_entry_point_[ tid ].is_false() )
  {
    saved_positions_[ tid ].tid = current_positions_[ tid ].tid;
    saved_positions_[ tid ].syn_id = current_positions_[ tid ].syn_id;

    // if tid and syn_id are valid entries, also store valid entry for lcid
    if ( current_positions_[ tid ].tid > -1 and current_positions_[ tid ].syn_id > -1 )
    {
      // either store current_position.lcid + 1, since this can
      // contain non-processed entry (see reject_last_target_data()) or
      // store maximal value for lcid.
      saved_positions_[ tid ].lcid = std::min( current_positions_[ tid ].lcid + 1,
        static_cast< long >(
          sources_[ current_positions_[ tid ].tid ][ current_positions_[ tid ].syn_id ].size() - 1 ) );
    }
    else
    {
      assert( current_positions_[ tid ].lcid == -1 );
      saved_positions_[ tid ].lcid = -1;
    }
    saved_entry_point_.set_true( tid );
  }
}

void
nest::SourceTable::reject_last_target_data( const size_t tid )
{

  // The last target data returned by get_next_target_data() could not
  // be inserted into MPI buffer due to overflow. We hence need to
  // correct the processed flag of the last entry (see
  // source_table.cpp)
  assert( current_positions_[ tid ].lcid + 1
    < static_cast< long >( sources_[ current_positions_[ tid ].tid ][ current_positions_[ tid ].syn_id ].size() ) );

  sources_[ current_positions_[ tid ].tid ][ current_positions_[ tid ].syn_id ][ current_positions_[ tid ].lcid + 1 ]
    .set_processed( false );
}

void
nest::SourceTable::clear( const size_t tid )
{

  for ( std::vector< BlockVector< Source > >::iterator it = sources_[ tid ].begin(); it != sources_[ tid ].end(); ++it )
  {
    it->clear();
  }
  sources_[ tid ].clear();
  is_cleared_.set_true( tid );
}

void
nest::SourceTable::add_source( const size_t tid, const synindex syn_id, const size_t node_id, const bool is_primary )
{

  const Source src( node_id, is_primary );
  sources_[ tid ][ syn_id ].push_back( src );
}

size_t
nest::CSDMapEntry::get_target_thread() const
{

  return target_thread_;
}

size_t
nest::CSDMapEntry::get_source_index() const
{

  return source_index_;
}
