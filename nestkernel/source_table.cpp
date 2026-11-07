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
#include <algorithm>
#include <functional>
#include <iostream>

// Includes from nestkernel:
#include "connection_manager.h"
#include "connection_manager_impl.h"
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
#include "source_table.h"
#include "stopwatch_impl.h"
#include "vp_manager_impl.h"


namespace nest
{
SourceTable::SourceTable()
{
}

SourceTable::~SourceTable()
{
}

void
SourceTable::initialize()
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
SourceTable::finalize()
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
SourceTable::is_cleared() const
{
  return is_cleared_.all_true();
}

std::vector< BlockVector< Source > >&
SourceTable::get_thread_local_sources( const size_t tid )
{
  return sources_[ tid ];
}

SourceTablePosition
SourceTable::find_maximal_position() const
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
SourceTable::clean( const size_t tid )
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
SourceTable::get_node_id( const size_t tid, const synindex syn_id, const size_t lcid ) const
{
  if ( not kernel().connection_manager.get_keep_source_table() )
  {
    throw KernelException( "Cannot use SourceTable::get_node_id when get_keep_source_table is false" );
  }
  return sources_[ tid ][ syn_id ][ lcid ].get_node_id();
}

size_t
SourceTable::find_first_source( const size_t tid, const synindex syn_id, const size_t snode_id ) const
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
SourceTable::remove_disabled_sources( const size_t tid, const synindex syn_id )
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
SourceTable::compute_buffer_pos_for_unique_secondary_sources( const size_t tid,
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
    const ConnectorModel& conn_model = kernel().model_manager.get_connection_model( syn_id, tid );
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
  kernel().get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
  kernel().get_omp_synchronization_construction_stopwatch().stop();

#pragma omp single
  {
    // compute receive buffer positions for all unique pairs of source
    // node ID and synapse-type id on this MPI rank
    std::vector< int > recv_counts_secondary_events_in_int_per_rank( kernel().mpi_manager.get_num_processes(), 0 );

    for ( std::set< std::pair< size_t, size_t > >::const_iterator cit =
            ( *unique_secondary_source_node_id_syn_id ).begin();
      cit != ( *unique_secondary_source_node_id_syn_id ).end();
      ++cit )
    {
      const size_t source_rank = kernel().mpi_manager.get_process_id_of_node_id( cit->first );
      const size_t event_size = kernel().model_manager.get_secondary_event_prototype( cit->second, tid )->size();

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
  }  // of omp single
}

void
SourceTable::resize_sources()
{
  kernel().vp_manager.assert_thread_parallel();
  sources_.at( kernel().vp_manager.get_thread_id() ).resize( kernel().model_manager.get_num_connection_models() );
}

bool
SourceTable::source_should_be_processed_( const size_t rank_start, const size_t rank_end, const Source& source ) const
{
  const size_t source_rank = kernel().mpi_manager.get_process_id_of_node_id( source.get_node_id() );

  // Check if thread is responsible for this part of the MPI buffer
  const bool responsible = source_rank < rank_start or rank_end <= source_rank;

  return not( source.is_processed() or source.is_disabled() or responsible );
}

bool
SourceTable::next_entry_has_same_source_( const SourceTablePosition& current_position,
  const Source& current_source ) const
{
  assert( not current_position.is_invalid() );

  const auto& local_sources = sources_[ current_position.tid ][ current_position.syn_id ];
  const size_t next_lcid = current_position.lcid + 1;

  return (
    next_lcid < local_sources.size() and local_sources[ next_lcid ].get_node_id() == current_source.get_node_id() );
}

bool
SourceTable::previous_entry_has_same_source_( const SourceTablePosition& current_position,
  const Source& current_source ) const
{
  assert( not current_position.is_invalid() );

  const auto& local_sources = sources_[ current_position.tid ][ current_position.syn_id ];
  const long previous_lcid = current_position.lcid - 1;  // needs to be a signed type such that negative
                                                         // values can signal invalid indices

  return ( previous_lcid >= 0 and not local_sources[ previous_lcid ].is_processed()
    and local_sources[ previous_lcid ].get_node_id() == current_source.get_node_id() );
}

bool
SourceTable::populate_target_data_fields_( const SourceTablePosition& current_position,
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
  }

  return true;
}

bool
SourceTable::get_next_target_data( const size_t tid,
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
SourceTable::resize_compressible_sources()
{
  for ( size_t tid = 0; tid < static_cast< size_t >( compressible_sources_.size() ); ++tid )
  {
    compressible_sources_[ tid ].clear();
    compressible_sources_[ tid ].resize( kernel().model_manager.get_num_connection_models(), CompressibleSources() );
  }
}

void
SourceTable::collect_compressible_sources( const size_t tid )
{
  for ( synindex syn_id = 0; syn_id < sources_[ tid ].size(); ++syn_id )
  {
    size_t lcid = 0;
    auto& syn_sources = sources_[ tid ][ syn_id ];
    auto& syn_compressible_sources = compressible_sources_[ tid ][ syn_id ];

    // Reserve one entry per run of equal sources. Keep this counting pass: the spare capacity of a growing vector in
    // every thread raised the peak memory of Prepare by 0.3 GB per process (PD14, 1 x 256 threads).
    size_t num_runs = 0;
    for ( size_t i = 0; i < syn_sources.size(); ++i )
    {
      num_runs += ( i == 0 or syn_sources[ i ].get_node_id() != syn_sources[ i - 1 ].get_node_id() ) ? 1 : 0;
    }
    syn_compressible_sources.reserve( num_runs );

    while ( lcid < syn_sources.size() )
    {
      const size_t old_source_node_id = syn_sources[ lcid ].get_node_id();
      syn_compressible_sources.emplace_back( old_source_node_id, SpikeData( tid, syn_id, lcid, 0 ) );

      // For all subsequent connections with same source, set "has more targets" on preceding connection.
      // Requires sorted connections.
      ++lcid;
      while ( ( lcid < syn_sources.size() ) and ( syn_sources[ lcid ].get_node_id() == old_source_node_id ) )
      {
        kernel().connection_manager.set_source_has_more_targets( tid, syn_id, lcid - 1, true );
        ++lcid;
      }
      // Mark last connection in sequence as not having successor. This is essential if connections are
      // delete, e.g., by structural plasticity, because we do not globally reset the more_targets flag.
      assert( lcid - 1 < syn_sources.size() );
      kernel().connection_manager.set_source_has_more_targets( tid, syn_id, lcid - 1, false );
    }

    // the connections are sorted by source, so the entries are sorted by node id and unique
    assert( std::adjacent_find( syn_compressible_sources.begin(),
              syn_compressible_sources.end(),
              []( const CompressibleSources::value_type& lhs, const CompressibleSources::value_type& rhs )
              { return lhs.first >= rhs.first; } )
      == syn_compressible_sources.end() );
  }
}

void
SourceTable::assemble_compressed_spike_data( const size_t tid )
{
#pragma omp single
  {
    const size_t num_threads = compressible_sources_.size();
    const size_t num_synapse_models = kernel().model_manager.get_num_connection_models();
    unique_compressible_sources_.assign( num_synapse_models, std::vector< UniqueSources >( num_threads ) );
    num_unique_compressible_sources_.assign(
      num_synapse_models, std::vector< size_t >( num_threads * num_threads, 0 ) );
  }  // of omp single; implicit barrier

  find_unique_sources_( tid );

  kernel().get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
  kernel().get_omp_synchronization_construction_stopwatch().stop();

  number_unique_sources_( tid );

  kernel().get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
  kernel().get_omp_synchronization_construction_stopwatch().stop();

#pragma omp single
  {
    allocate_compressed_spike_data_();
  }  // of omp single; implicit barrier

  write_own_column_( tid );

#ifndef NDEBUG
  kernel().get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
  kernel().get_omp_synchronization_construction_stopwatch().stop();
#pragma omp single
  {
    assert( is_result_of_serial_assembly_() );
  }  // of omp single; implicit barrier
#endif

  for ( auto& sources : compressible_sources_[ tid ] )
  {
    CompressibleSources().swap( sources );
  }
}

void
SourceTable::find_unique_sources_( const size_t tid )
{
  const size_t num_threads = compressible_sources_.size();
  const size_t max_node_id = kernel().node_manager.size();
  const size_t range_begin = 1 + tid * max_node_id / num_threads;
  const size_t range_end = 1 + ( tid + 1 ) * max_node_id / num_threads;
  const auto node_id_less = []( const CompressibleSources::value_type& entry, const size_t node_id )
  { return entry.first < node_id; };

  // The sources of every thread are sorted, so a merge over a heap of (node id, thread) pairs visits them in the order
  // (node id, thread), with memory proportional to the number of threads instead of a sorted copy of the range.
  std::vector< std::pair< CompressibleSources::const_iterator, CompressibleSources::const_iterator > > cursors(
    num_threads );
  std::vector< std::pair< size_t, size_t > > heap;
  heap.reserve( num_threads );
  const std::greater< std::pair< size_t, size_t > > heap_order;
  for ( synindex syn_id = 0; syn_id < unique_compressible_sources_.size(); ++syn_id )
  {
    for ( size_t target_thread = 0; target_thread < num_threads; ++target_thread )
    {
      const auto& sources = compressible_sources_[ target_thread ][ syn_id ];
      assert( sources.empty() or sources.back().first <= max_node_id );  // the ranges cover all sources
      const auto begin = std::lower_bound( sources.begin(), sources.end(), range_begin, node_id_less );
      cursors[ target_thread ] =
        std::make_pair( begin, std::lower_bound( begin, sources.end(), range_end, node_id_less ) );
      if ( begin != cursors[ target_thread ].second )
      {
        heap.emplace_back( begin->first, target_thread );
      }
    }
    std::make_heap( heap.begin(), heap.end(), heap_order );

    auto& unique_sources = unique_compressible_sources_[ syn_id ][ tid ];
    size_t* const num_unique_sources = &num_unique_compressible_sources_[ syn_id ][ tid * num_threads ];
    while ( not heap.empty() )
    {
      std::pop_heap( heap.begin(), heap.end(), heap_order );
      const auto [ node_id, target_thread ] = heap.back();
      heap.pop_back();
      if ( unique_sources.empty() or unique_sources.back().first != node_id )
      {
        unique_sources.emplace_back( node_id, CSDMapEntry( 0, target_thread ) );  // index set by number_unique_sources_
        ++num_unique_sources[ target_thread ];
      }
      auto& cursor = cursors[ target_thread ];
      if ( ++cursor.first != cursor.second )
      {
        heap.emplace_back( cursor.first->first, target_thread );
        std::push_heap( heap.begin(), heap.end(), heap_order );
      }
    }
  }
}

void
SourceTable::number_unique_sources_( const size_t tid )
{
  // The serial algorithm numbers the sources in the order (first thread, node id). The sources of range tid whose
  // first thread is t follow all sources of the threads before t and the sources of t in the ranges before tid.
  // Each thread sums the counts of all ranges: O(T^2) per thread, negligible also for T = 288.
  const size_t num_threads = compressible_sources_.size();
  std::vector< size_t > next_index( num_threads );
  for ( synindex syn_id = 0; syn_id < unique_compressible_sources_.size(); ++syn_id )
  {
    const std::vector< size_t >& num_unique_sources = num_unique_compressible_sources_[ syn_id ];
    size_t num_sources_of_previous_threads = 0;
    for ( size_t target_thread = 0; target_thread < num_threads; ++target_thread )
    {
      size_t num_in_previous_ranges = 0;
      size_t num_of_thread = 0;
      for ( size_t range = 0; range < num_threads; ++range )
      {
        const size_t n = num_unique_sources[ range * num_threads + target_thread ];
        num_in_previous_ranges += range < tid ? n : 0;
        num_of_thread += n;
      }
      next_index[ target_thread ] = num_sources_of_previous_threads + num_in_previous_ranges;
      num_sources_of_previous_threads += num_of_thread;
    }

    for ( auto& unique_source : unique_compressible_sources_[ syn_id ][ tid ] )
    {
      const size_t target_thread = unique_source.second.get_target_thread();
      unique_source.second = CSDMapEntry( next_index[ target_thread ]++, target_thread );
    }
  }
}

void
SourceTable::allocate_compressed_spike_data_()
{
  const size_t num_threads = compressible_sources_.size();
  const size_t num_synapse_models = unique_compressible_sources_.size();
  assembled_compressed_spike_data_.assign( num_synapse_models, std::vector< std::vector< SpikeData > >() );
  compressed_spike_data_map_.assign( num_synapse_models, std::map< size_t, CSDMapEntry >() );

  for ( synindex syn_id = 0; syn_id < num_synapse_models; ++syn_id )
  {
    // the ranges are ordered by node id, so every new entry goes to the end of the map
    auto& source_index_map = compressed_spike_data_map_[ syn_id ];
    for ( const auto& unique_sources : unique_compressible_sources_[ syn_id ] )
    {
      source_index_map.insert( unique_sources.begin(), unique_sources.end() );
    }
    assembled_compressed_spike_data_[ syn_id ].assign( source_index_map.size(),
      std::vector< SpikeData >( num_threads, SpikeData( invalid_targetindex, invalid_synindex, invalid_lcid, 0 ) ) );
  }
}

void
SourceTable::write_own_column_( const size_t tid )
{
  // The sources of thread tid and the unique sources of the ranges are both sorted by node id.
  for ( synindex syn_id = 0; syn_id < unique_compressible_sources_.size(); ++syn_id )
  {
    const auto& unique_sources = unique_compressible_sources_[ syn_id ];
    auto& compressed_spike_data = assembled_compressed_spike_data_[ syn_id ];
    size_t range = 0;
    size_t pos = 0;
    for ( const auto& source : compressible_sources_[ tid ][ syn_id ] )
    {
      while ( pos == unique_sources[ range ].size() or unique_sources[ range ][ pos ].first < source.first )
      {
        if ( pos == unique_sources[ range ].size() )
        {
          ++range;
          pos = 0;
        }
        else
        {
          ++pos;
        }
      }
      assert( unique_sources[ range ][ pos ].first == source.first );
      compressed_spike_data[ unique_sources[ range ][ pos ].second.get_source_index() ][ tid ] = source.second;
    }
  }
}

bool
SourceTable::is_result_of_serial_assembly_() const
{
  // Serial algorithm: visit the threads in ascending order and, for each thread, its sources in ascending order of
  // node id; a source gets the next row when it is seen for the first time.
  for ( synindex syn_id = 0; syn_id < assembled_compressed_spike_data_.size(); ++syn_id )
  {
    const auto& assembled_map = compressed_spike_data_map_[ syn_id ];
    std::map< size_t, CSDMapEntry > serial_map;
    for ( size_t target_thread = 0; target_thread < compressible_sources_.size(); ++target_thread )
    {
      for ( const auto& [ node_id, spike_data ] : compressible_sources_[ target_thread ][ syn_id ] )
      {
        const CSDMapEntry entry =
          serial_map.emplace( node_id, CSDMapEntry( serial_map.size(), target_thread ) ).first->second;
        const auto it = assembled_map.find( node_id );
        if ( entry.get_source_index() >= assembled_compressed_spike_data_[ syn_id ].size() )
        {
          return false;
        }
        const SpikeData& assembled =
          assembled_compressed_spike_data_[ syn_id ][ entry.get_source_index() ][ target_thread ];
        if ( it == assembled_map.end() or it->second.get_source_index() != entry.get_source_index()
          or it->second.get_target_thread() != entry.get_target_thread()
          or assembled.get_lcid() != spike_data.get_lcid() or assembled.get_tid() != spike_data.get_tid()
          or assembled.get_syn_id() != spike_data.get_syn_id() )
        {
          return false;
        }
      }
    }
    if ( serial_map.size() != assembled_map.size()
      or serial_map.size() != assembled_compressed_spike_data_[ syn_id ].size() )
    {
      return false;
    }
  }
  return true;
}

void
SourceTable::dump_sources() const
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
SourceTable::dump_compressible_sources() const
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
SourceTable::fill_compressed_spike_data( std::vector< std::vector< std::vector< SpikeData > > >& compressed_spike_data )
{
  // For each synapse type, and for each source neuron with at least one local target, compressed_spike_data holds one
  // SpikeData entry for each local thread that owns a local target. compressed_spike_data_map_ holds the index into
  // compressed_spike_data[ syn_id ] where the data for a given source is stored. Both were built by
  // assemble_compressed_spike_data().
  compressed_spike_data.clear();
  compressed_spike_data.swap( assembled_compressed_spike_data_ );
  std::vector< std::vector< UniqueSources > >().swap( unique_compressible_sources_ );
  std::vector< std::vector< size_t > >().swap( num_unique_compressible_sources_ );
}

// Argument name only needed if full logging is activated. Macro-protect to avoid unused argument warning.
void
SourceTable::dump_compressed_spike_data(
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

}  // namespace nest
