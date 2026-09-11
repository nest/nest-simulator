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
#include "stopwatch_impl.h"
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
  const size_t num_threads = kernel().vp_manager.get_num_threads();
  sources_.resize( num_threads );
  is_cleared_.initialize( num_threads, false );
  saved_entry_point_.initialize( num_threads, false );
  current_positions_.resize( num_threads );
  saved_positions_.resize( num_threads );
  compressible_sources_.resize( num_threads );

#pragma omp parallel
  {
    const size_t tid = kernel().vp_manager.get_thread_id();
    sources_.at( tid ).resize( 0 );
    resize_sources();
    compressible_sources_.at( tid ).resize( 0 );
  }  // of omp parallel
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
  for ( size_t tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
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
      if ( static_cast< synindex >( max_position.syn_id ) == syn_id )
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
        assert( max_position.syn_id < static_cast< long >( syn_id ) );
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
  if ( not kernel().connection_manager.get_keep_source_table() )
  {
    throw KernelException( "Cannot use SourceTable::get_node_id when get_keep_source_table is false" );
  }
  return sources_[ tid ][ syn_id ][ lcid ].get_node_id();
}

size_t
nest::SourceTable::find_first_source( const size_t tid, const synindex syn_id, const size_t snode_id ) const
{
  const auto source_begin = sources_[ tid ][ syn_id ].begin();
  const auto source_end = sources_[ tid ][ syn_id ].end();

  auto first_source_match = source_begin;
  if ( kernel().connection_manager.use_compressed_spikes() )
  {
    // Binary search for first entry matching snode_id; is_primary is ignored
    const Source requested_source { snode_id, /* is_primary */ true };
    first_source_match = std::lower_bound( source_begin, source_end, requested_source );
  }

  // Linear search for first non-disabled connection
  const auto first_enabled = std::find_if( first_source_match,
    source_end,
    [ &snode_id ]( const Source& src ) { return src.get_node_id() == snode_id and not src.is_disabled(); } );
  if ( first_enabled != source_end )
  {
    // lcid is iterator difference
    return first_enabled - source_begin;
  }
  else
  {
    // no enabled entry with this snode ID found
    return invalid_index;
  }
}

size_t
nest::SourceTable::remove_disabled_sources( const size_t tid, const synindex syn_id )
{
  assert( kernel().connection_manager.use_compressed_spikes() );

  if ( sources_[ tid ].size() <= syn_id )
  {
    return invalid_index;  // no source table entry for this synapse model
  }

  BlockVector< Source >& mysources = sources_[ tid ][ syn_id ];
  const size_t max_size = mysources.size();
  if ( max_size == 0 )
  {
    return invalid_index;  // no connections for this synapse model
  }

  // lcid needs to be signed, to allow lcid >= 0 check in while loop
  // to fail; afterwards we can be certain that it is non-negative and
  // we can static_cast it to index
  long lcid = max_size - 1;
  while ( lcid >= 0 and mysources[ lcid ].is_disabled() )
  {
    --lcid;
  }
  const size_t first_invalid_lcid = static_cast< size_t >( lcid + 1 );  // loop stopped on first valid entry or -1
  if ( first_invalid_lcid == max_size )
  {
    return invalid_index;  // all lcids are valid, nothing to remove
  }

  mysources.erase( mysources.begin() + first_invalid_lcid, mysources.end() );
  return first_invalid_lcid;
}

void
nest::SourceTable::compute_buffer_pos_for_unique_secondary_sources( const size_t tid,
  std::map< SecondarySourceId, size_t >& buffer_pos_of_secondary_source )
{
  // set of unique secondary sources, required to determine secondary-event
  // MPI buffer positions; keyed by (source node ID, synapse-type ID, source
  // port). Initialized and deleted by thread 0 in this method.
  static std::set< SecondarySourceId >* unique_secondary_source_id;
#pragma omp single
  {
    unique_secondary_source_id = new std::set< SecondarySourceId >();
  }

  // collect all unique (source node ID, synapse-type ID, source port) triples
  // corresponding to continuous-data connections on this MPI rank;
  // using a set makes sure secondary events are not duplicated for
  // targets on the same process, but different threads. Distinct source ports
  // of one node are distinct sources, so each obtains its own buffer slot.
  for ( size_t syn_id = 0; syn_id < sources_[ tid ].size(); ++syn_id )
  {
    const ConnectorModel& conn_model = kernel().model_manager.get_connection_model( syn_id, tid );
    const bool is_primary = conn_model.has_property( ConnectionModelProperties::IS_PRIMARY );

    if ( not is_primary )
    {
      const size_t num_connections = sources_[ tid ][ syn_id ].size();
      for ( size_t lcid = 0; lcid < num_connections; ++lcid )
      {
        const size_t source_node_id = sources_[ tid ][ syn_id ][ lcid ].get_node_id();
        const size_t source_port = kernel().connection_manager.get_source_port( tid, syn_id, lcid );
#pragma omp critical
        {
          ( *unique_secondary_source_id ).insert( SecondarySourceId { source_node_id, syn_id, source_port } );
        }
      }
    }
  }
  kernel().get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
  kernel().get_omp_synchronization_construction_stopwatch().stop();

#pragma omp single
  {
    // compute receive buffer positions for all unique secondary sources on
    // this MPI rank
    std::vector< int > recv_counts_secondary_events_in_int_per_rank( kernel().mpi_manager.get_num_processes(), 0 );

    for ( const auto& secondary_source_id : *unique_secondary_source_id )
    {
      const size_t source_node_id = std::get< 0 >( secondary_source_id );
      const synindex syn_id = std::get< 1 >( secondary_source_id );
      const size_t source_rank = kernel().mpi_manager.get_process_id_of_node_id( source_node_id );
      const size_t event_size = kernel().model_manager.get_secondary_event_prototype( syn_id, tid )->size();

      buffer_pos_of_secondary_source.insert(
        std::make_pair( secondary_source_id, recv_counts_secondary_events_in_int_per_rank[ source_rank ] ) );

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
    delete unique_secondary_source_id;
  }  // of omp single
}

void
nest::SourceTable::resize_sources()
{
  kernel().vp_manager.assert_thread_parallel();
  sources_.at( kernel().vp_manager.get_thread_id() ).resize( kernel().model_manager.get_num_connection_models() );
}

bool
nest::SourceTable::source_should_be_processed_( const size_t rank_start,
  const size_t rank_end,
  const Source& source ) const
{
  const size_t source_rank = kernel().mpi_manager.get_process_id_of_node_id( source.get_node_id() );

  // Check if thread is responsible for this part of the MPI buffer
  const bool responsible = source_rank < rank_start or rank_end <= source_rank;

  return not( source.is_processed() or source.is_disabled() or responsible );
}

bool
nest::SourceTable::next_entry_has_same_source_( const SourceTablePosition& current_position,
  const Source& current_source ) const
{
  assert( not current_position.is_invalid() );

  const auto& local_sources = sources_[ current_position.tid ][ current_position.syn_id ];
  const size_t next_lcid = current_position.lcid + 1;

  if ( next_lcid >= local_sources.size() or local_sources[ next_lcid ].get_node_id() != current_source.get_node_id() )
  {
    return false;
  }

  // For secondary connections a source is identified by (node, source port),
  // so a differing source port breaks the group even for the same node. This
  // keeps waveforms of distinct source ports in separate delivery groups.
  // Primary connections always use port zero, so this is a no-op for them.
  return current_source.is_primary() or same_source_port_( current_position, next_lcid );
}

bool
nest::SourceTable::previous_entry_has_same_source_( const SourceTablePosition& current_position,
  const Source& current_source ) const
{
  assert( not current_position.is_invalid() );

  const auto& local_sources = sources_[ current_position.tid ][ current_position.syn_id ];
  const long previous_lcid = current_position.lcid - 1;  // needs to be a signed type such that negative
                                                         // values can signal invalid indices

  if ( previous_lcid < 0 or local_sources[ previous_lcid ].is_processed()
    or local_sources[ previous_lcid ].get_node_id() != current_source.get_node_id() )
  {
    return false;
  }

  // See next_entry_has_same_source_: distinct source ports are distinct
  // secondary sources and must each be communicated once.
  return current_source.is_primary() or same_source_port_( current_position, previous_lcid );
}

bool
nest::SourceTable::same_source_port_( const SourceTablePosition& current_position, const size_t other_lcid ) const
{
  return kernel().connection_manager.get_source_port(
           current_position.tid, current_position.syn_id, current_position.lcid )
    == kernel().connection_manager.get_source_port( current_position.tid, current_position.syn_id, other_lcid );
}

bool
nest::SourceTable::populate_target_data_fields_( const SourceTablePosition& current_position,
  const Source& current_source,
  const size_t source_rank,
  TargetData& next_target_data ) const
{
  assert( not kernel().connection_manager.use_compressed_spikes() );  // handled elsewhere

  const auto node_id = current_source.get_node_id();

  // set values of next_target_data
  next_target_data.set_source_lid( kernel().vp_manager.node_id_to_lid( node_id ) );
  next_target_data.set_source_tid( kernel().vp_manager.vp_to_thread( kernel().vp_manager.node_id_to_vp( node_id ) ) );
  next_target_data.reset_marker();

  if ( current_source.is_primary() )  // primary connection, i.e., chemical synapses
  {
    next_target_data.set_is_primary( true );

    TargetDataFields& target_fields = next_target_data.target_data;
    target_fields.set_syn_id( current_position.syn_id );

    // we store the thread index of the source table, not our own tid!
    target_fields.set_tid( current_position.tid );
    target_fields.set_lcid( current_position.lcid );
  }
  else  // secondary connection, e.g., gap junctions
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
    secondary_fields.set_source_port( kernel().connection_manager.get_source_port(
      current_position.tid, current_position.syn_id, current_position.lcid ) );
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
    return false;  // nothing to do here
  }

  // we stay in this loop either until we can return a valid
  // TargetData object or we have reached the end of the sources table
  while ( true )
  {
    current_position.seek_to_next_valid_index( sources_ );
    if ( current_position.is_invalid() )
    {
      return false;  // reached the end of the sources table
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
      current_source.set_processed( true );  // no need to look at this entry again
      current_position.decrease();
      continue;
    }

    // reaching this means we found an entry that should be
    // communicated via MPI, so we prepare to return the relevant data

    // set the source rank
    source_rank = kernel().mpi_manager.get_process_id_of_node_id( current_source.get_node_id() );

    if ( not populate_target_data_fields_( current_position, current_source, source_rank, next_target_data ) )
    {
      current_position.decrease();
      continue;
    }

    // we are about to return a valid entry, so mark it as processed
    current_source.set_processed( true );

    current_position.decrease();
    return true;  // found a valid entry
  }
}

void
nest::SourceTable::resize_compressible_sources()
{
  for ( size_t tid = 0; tid < static_cast< size_t >( compressible_sources_.size() ); ++tid )
  {
    compressible_sources_[ tid ].clear();
    compressible_sources_[ tid ].resize(
      kernel().model_manager.get_num_connection_models(), std::map< size_t, SpikeData >() );
  }
}

void
nest::SourceTable::collect_compressible_sources( const size_t tid )
{
  for ( synindex syn_id = 0; syn_id < sources_[ tid ].size(); ++syn_id )
  {
    const ConnectorModel& conn_model = kernel().model_manager.get_connection_model( syn_id, tid );
    const bool is_primary = conn_model.has_property( ConnectionModelProperties::IS_PRIMARY );

    // For secondary connections a source is identified by (node, source port),
    // packed into the compression key so distinct ports of one node stay
    // separate sources. Primary connections use the node ID directly (port
    // zero packs to the node ID), so their keys and behavior are unchanged.
    auto compression_key = [ & ]( const size_t lcid ) -> size_t
    {
      const size_t node_id = sources_[ tid ][ syn_id ][ lcid ].get_node_id();
      if ( is_primary )
      {
        return node_id;
      }
      return pack_source_node_id_and_source_port(
        node_id, kernel().connection_manager.get_source_port( tid, syn_id, lcid ) );
    };

    size_t lcid = 0;
    auto& syn_sources = sources_[ tid ][ syn_id ];
    while ( lcid < syn_sources.size() )
    {
      const size_t old_key = compression_key( lcid );
      compressible_sources_[ tid ][ syn_id ].insert( std::make_pair( old_key, SpikeData( tid, syn_id, lcid, 0 ) ) );

      // For all subsequent connections with same source, set "has more targets" on preceding connection.
      // Requires sorted connections.
      ++lcid;
      while ( ( lcid < syn_sources.size() ) and ( compression_key( lcid ) == old_key ) )
      {
        kernel().connection_manager.set_source_has_more_targets( tid, syn_id, lcid - 1, true );
        ++lcid;
      }
      // Mark last connection in sequence as not having successor. This is essential if connections are
      // delete, e.g., by structural plasticity, because we do not globally reset the more_targets flag.
      assert( lcid - 1 < syn_sources.size() );
      kernel().connection_manager.set_source_has_more_targets( tid, syn_id, lcid - 1, false );
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
        kernel().write_to_dump( String::compose( "src  : r%1 t%2 s%3 tg%4 l%5 tt%6",
          kernel().mpi_manager.get_rank(),
          kernel().vp_manager.get_thread_id(),
          sources_[ tid ][ syn_id ][ lcid ].get_node_id(),
          kernel().connection_manager.get_target_node_id( tid, syn_id, lcid ),
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
        kernel().write_to_dump( String::compose( "csrc : r%1 t%2 s%3 l%4 tt%5",
          kernel().mpi_manager.get_rank(),
          kernel().vp_manager.get_thread_id(),
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
  const size_t num_synapse_models = kernel().model_manager.get_num_connection_models();
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
  for ( synindex syn_id = 0; syn_id < kernel().model_manager.get_num_connection_models(); ++syn_id )
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

          compressed_spike_data[ syn_id ].emplace_back( kernel().vp_manager.get_num_threads(),
            SpikeData( invalid_targetindex, invalid_synindex, invalid_lcid, 0 ) );

          compressed_spike_data_map_[ syn_id ].insert(
            std::make_pair( source_gid, CSDMapEntry( new_source_index, target_thread ) ) );
        }

        const auto source_index = compressed_spike_data_map_[ syn_id ].find( source_gid )->second.get_source_index();

        assert( compressed_spike_data[ syn_id ][ source_index ][ target_thread ].get_lcid() == invalid_lcid );

        compressed_spike_data[ syn_id ][ source_index ][ target_thread ] = connection.second;
      }  // for connection

      compressible_sources_[ target_thread ][ syn_id ].clear();

    }  // for target_thread
  }  // for syn_id
}

// Argument name only needed if full logging is activated. Macro-protect to avoid unused argument warning.
void
nest::SourceTable::dump_compressed_spike_data(
  const std::vector< std::vector< std::vector< SpikeData > > >& FULL_LOGGING_ONLY( compressed_spike_data ) ) const
{
  FULL_LOGGING_ONLY(
    for ( const auto& tab : compressed_spike_data_map_ ) {
      for ( const auto& entry : tab )
      {
        kernel().write_to_dump( String::compose( "csdm : r%1 t%2 s%3 sx%4 tt%5",
          kernel().mpi_manager.get_rank(),
          kernel().vp_manager.get_thread_id(),
          entry.first,
          entry.second.get_source_index(),
          entry.second.get_target_thread() ) );
      }
    }

    for ( const auto& tab : compressed_spike_data ) {
      for ( size_t six = 0; six < tab.size(); ++six )
      {
        for ( size_t tx = 0; tx < tab[ six ].size(); ++tx )
        {
          kernel().write_to_dump( String::compose( "csd  : r%1 t%2 six%3 tx%4 l%5 tt%6",
            kernel().mpi_manager.get_rank(),
            kernel().vp_manager.get_thread_id(),
            six,
            tx,
            tab[ six ][ tx ].get_lcid(),
            tab[ six ][ tx ].get_tid() ) );
        }
      }
    } )
}
