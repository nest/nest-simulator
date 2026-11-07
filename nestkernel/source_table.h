/*
 *  source_table.h
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

#ifndef SOURCE_TABLE_H
#define SOURCE_TABLE_H

// C++ includes:
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <vector>

// Includes from nestkernel:
#include "mpi_manager.h"
#include "nest_types.h"
#include "per_thread_bool_indicator.h"
#include "source.h"
#include "source_table_position.h"
#include "spike_data.h"

// Includes from libnestutil
#include "block_vector.h"
#include "vector_util.h"

namespace nest
{

class TargetData;

/**
 * Entry of compressed_spike_data_map_.
 *
 * Combines source node index and target node thread.
 */
class CSDMapEntry
{
public:
  CSDMapEntry( size_t source_index, size_t target_thread )
    : source_index_( source_index )
    , target_thread_( target_thread )
  {
    // MAX_LCID cannot be used since the value is used to mark invalid entries.
    // MAX_TID is defined based on the NUM_BITS_TID field width and thus much smaller than invalid_thread and can be
    // used.
    assert( source_index < MAX_LCID );
    assert( target_thread <= MAX_TID );
  }

  size_t
  get_source_index() const
  {
    return source_index_;
  }
  size_t
  get_target_thread() const
  {
    return target_thread_;
  }

private:
  size_t source_index_ : NUM_BITS_LCID;
  size_t target_thread_ : NUM_BITS_TID;
};

//! check legal size
using success_csdmapentry_size = StaticAssert< sizeof( CSDMapEntry ) == 8 >::success;

/**
 * This data structure stores the node IDs of presynaptic neurons
 * during postsynaptic connection creation, before the connection
 * information has been transferred to the presynaptic side. The core
 * structure is the three dimensional sources vector, which is
 * arranged as follows:
 *
 * 1st dimension: threads
 * 2nd dimension: synapse types
 * 3rd dimension: node IDs
 * After all connections have been created, the information stored in
 * this structure is transferred to the presynaptic side and the
 * sources vector can be cleared.
 */
class SourceTable
{
  friend class ConnectionManager;

  //! Sources of one thread and synapse type: (node id, spike data) pairs, sorted by node id, one entry per source
  using CompressibleSources = std::vector< std::pair< size_t, SpikeData > >;

  //! Unique sources of one range of node ids with their entries of compressed_spike_data_map_, sorted by node id
  using UniqueSources = std::vector< std::pair< size_t, CSDMapEntry > >;

private:
  /**
   * 3D structure storing node IDs of presynaptic neurons.
   */
  std::vector< std::vector< BlockVector< Source > > > sources_;

  /**
   * Whether the 3D structure has been deleted.
   */
  PerThreadBoolIndicator is_cleared_;

  //! Needed during readout of sources_.
  std::vector< SourceTablePosition > current_positions_;
  //! Needed during readout of sources_.
  std::vector< SourceTablePosition > saved_positions_;

  /**
   * If we detect an overflow in one of the MPI buffer parts, we save
   * our current position in sources_ to continue at that point in
   * the next communication round, while filling up (possible)
   * remaining parts of the MPI buffer.
   */
  PerThreadBoolIndicator saved_entry_point_;

  /**
   * Minimal number of sources that need to be deleted per synapse
   * type and thread before a reallocation of the respective vector
   * is performed. Balances number of reallocations and memory usage.
   *
   * @see SourceTable::clean()
   */
  static const size_t min_deleted_elements_ = 1000000;

  /**
   * Returns whether this Source object should be considered when
   * constructing MPI buffers for communicating connections.
   *
   * Returns false if i) this entry was already processed, or ii) this entry
   * is disabled (e.g., by structural plastcity) or iii) the reading
   * thread is not responsible for the particular part of the MPI
   * buffer where this entry would be written.
   */
  bool source_should_be_processed_( const size_t rank_start, const size_t rank_end, const Source& source ) const;

  /**
   * Returns true if the following entry in the SourceTable has the
   * same source gid.
   */
  bool next_entry_has_same_source_( const SourceTablePosition& current_position, const Source& current_source ) const;

  /**
   * Returns true if the previous entry in the SourceTable has the
   * same source gid.
   */
  bool previous_entry_has_same_source_( const SourceTablePosition& current_position,
    const Source& current_source ) const;

  /**
   * Fills the fields of a TargetData during construction of *
   * presynaptic connection infrastructure.
   */
  bool populate_target_data_fields_( const SourceTablePosition& current_position,
    const Source& current_source,
    const size_t source_rank,
    TargetData& next_target_data ) const;

  /**
   * A structure to temporarily hold information about all process
   * local targets will be addressed by incoming spikes.
   *
   * Data from this structure is transferred to the compressed_spike_data_
   * structure of ConnectionManager during construction of the
   * postsynaptic connection infrastructure. Arranged as a two
   * dimensional vector (thread|synapse) with an inner vector of
   * (source node id, spike data) pairs sorted by source node id.
   */
  std::vector< std::vector< CompressibleSources > > compressible_sources_;

  /**
   * Unique sources of all threads with their entries in compressed_spike_data_map_.
   *
   * Arranged as a two dimensional vector (synapse|range of source node ids) with an inner vector of (source node id,
   * map entry) pairs sorted by source node id. Thread t determines the unique sources of range t, so that the ranges
   * together cover all node ids. Only used while assemble_compressed_spike_data() runs.
   */
  std::vector< std::vector< UniqueSources > > unique_compressible_sources_;

  /**
   * Number of unique sources per range and first thread.
   *
   * Arranged as a vector over synapses; entry [ range * num_threads + thread ] counts the sources of the range that
   * appear first in compressible_sources_ of the thread. Only used while assemble_compressed_spike_data() runs.
   */
  std::vector< std::vector< size_t > > num_unique_compressible_sources_;

  /**
   * Compressed spike data built by assemble_compressed_spike_data().
   *
   * Handed over to ConnectionManager by fill_compressed_spike_data().
   */
  std::vector< std::vector< std::vector< SpikeData > > > assembled_compressed_spike_data_;

  /**
   * A structure to temporarily store locations of "unpacked spikes"
   * in the compressed_spike_data_ structure of
   * ConnectionManager.
   *
   * Data from this structure is transferred to the
   * presynaptic side during construction of the presynaptic
   * connection infrastructure. Arranged as a one-dimensional vector
   * over synapse ids with an inner map (source node id -> (source_index+target_thread).
   */
  std::vector< std::map< size_t, CSDMapEntry > > compressed_spike_data_map_;

  //! Step 1 of assemble_compressed_spike_data(): unique sources of node-id range tid and their first threads.
  void find_unique_sources_( const size_t tid );

  //! Step 2 of assemble_compressed_spike_data(): row of every unique source of range tid in the serial order.
  void number_unique_sources_( const size_t tid );

  /**
   * Allocate one row per source in index order and build compressed_spike_data_map_ (one thread).
   *
   * One thread allocates and initializes the rows on purpose: the memory placement of the structure stays as with the
   * serial construction, and placement is a separate concern (copies of the structure per block of threads). The pass
   * is serial in sources x threads: negligible for PD14, but about 4 GB to initialize for the multi-area model with 128
   * threads per process. Combined with the copies, write_own_column_() could write directly into the copy of its block
   * and make this pass unnecessary.
   */
  void allocate_compressed_spike_data_();

  //! Step 3 of assemble_compressed_spike_data(): thread tid writes its column of the compressed spike data.
  void write_own_column_( const size_t tid );

  /**
   * Whether assembled_compressed_spike_data_ and compressed_spike_data_map_ equal the result of the serial algorithm.
   *
   * Used in assertions only; compressible_sources_ must still hold the sources of all threads.
   */
  bool is_result_of_serial_assembly_() const;

public:
  SourceTable();
  ~SourceTable();

  /**
   * Initialize data structure.
   */
  void initialize();

  /**
   * Delete data structures.
   */
  void finalize();

  /**
   * Adds a source to sources_.
   */
  void add_source( const size_t tid, const synindex syn_id, const size_t node_id, const bool is_primary );

  /**
   * Clears sources_.
   */
  void clear( const size_t tid );

  /**
   * Returns true if sources_ has been cleared.
   */
  bool is_cleared() const;

  /**
   * Returns the next target data, according to the current_positions_.
   */
  bool get_next_target_data( const size_t tid,
    const size_t rank_start,
    const size_t rank_end,
    size_t& source_rank,
    TargetData& next_target_data );

  /**
   * Rejects the last target data, and resets the current_positions_
   * accordingly.
   */
  void reject_last_target_data( const size_t tid );

  /**
   * Stores current_positions_ in saved_positions_.
   */
  void save_entry_point( const size_t tid );

  /**
   * Restores current_positions_ from saved_positions_.
   */
  void restore_entry_point( const size_t tid );

  /**
   * Resets saved_positions_ to end of sources_.
   */
  void reset_entry_point( const size_t tid );

  /**
   * Returns the node ID of the source at tid|syn_id|lcid.
   */
  size_t get_node_id( const size_t tid, const synindex syn_id, const size_t lcid ) const;

  /**
   * Returns a reference to all sources local on thread; necessary
   * for sorting.
   */
  std::vector< BlockVector< Source > >& get_thread_local_sources( const size_t tid );

  /**
   * Determines maximal saved_positions_ after which it is safe to
   * delete sources during clean().
   */
  SourceTablePosition find_maximal_position() const;

  /**
   * Resets all processed flags. Needed for restructuring connection
   * tables, e.g., during structural plasticity update.
   */
  void reset_processed_flags( const size_t tid );

  /**
   * Removes all entries marked as processed.
   */
  void clean( const size_t tid );

  /**
   * Sets current_positions_ for this thread to minimal values so that
   * these are not considered in find_maximal_position().
   */
  void no_targets_to_process( const size_t tid );

  /**
   * Computes MPI buffer positions for unique combination of source
   * node ID and synapse type across all threads for all secondary
   * connections.
   */
  void compute_buffer_pos_for_unique_secondary_sources( const size_t tid,
    std::map< size_t, size_t >& buffer_pos_of_source_node_id_syn_id_ );

  /**
   * Finds the first non-disabled entry in sources_ at the given thread id and synapse type that has sender equal to
   * snode_id.
   *
   * @returns local connection id (lcid) or `invalid_index`
   *
   * @note For compressed spikes, it uses binary search, otherwise linear search.
   */
  size_t find_first_source( const size_t tid, const synindex syn_id, const size_t snode_id ) const;

  /**
   * Marks entry in sources_ at given position as disabled.
   */
  void disable_connection( const size_t tid, const synindex syn_id, const size_t lcid );

  /**
   * Removes all entries from sources_ that are marked as disabled.
   */
  size_t remove_disabled_sources( const size_t tid, const synindex syn_id );

  /**
   * Returns node IDs for entries in sources_ for the given thread
   * id, synapse type and local connections ids.
   */
  void get_source_node_ids( const size_t tid,
    const synindex syn_id,
    const std::vector< size_t >& source_lcids,
    std::vector< size_t >& sources );

  /**
   * Returns the number of unique node IDs for given thread id and
   * synapse type in sources_.i
   *
   * This number corresponds to the number
   * of targets that need to be communicated during construction of
   * the presynaptic connection infrastructure.
   */
  size_t num_unique_sources( const size_t tid, const synindex syn_id ) const;

  /**
   * Resizes sources_ according to total number of threads and synapse types.
   */
  void resize_sources();

  /**
   * Encodes combination of node ID and synapse types as single
   * long number.
   */
  size_t pack_source_node_id_and_syn_id( const size_t source_node_id, const synindex syn_id ) const;

  void resize_compressible_sources();

  /**
   * Collect the sources of thread tid with at least one local target.
   *
   * Guarantees that compressible_sources_[ tid ][ syn_id ] is sorted by node id and holds one entry per source.
   */
  void collect_compressible_sources( const size_t tid );

  /**
   * Build the compressed spike data and compressed_spike_data_map_ from compressible_sources_ in parallel.
   *
   * Must be called by all threads after collect_compressible_sources() has finished on all threads. Thread tid finds
   * the unique sources in one range of source node ids, and later writes its own column of the compressed spike data.
   * The result is identical to the serial algorithm: a source has the index it would get by visiting the threads in
   * ascending order and, for each thread, its sources in ascending order of node id. One thread allocates the rows
   * in this order.
   */
  void assemble_compressed_spike_data( const size_t tid );

  //! Hand over the compressed spike data built by assemble_compressed_spike_data() and free the temporaries.
  void fill_compressed_spike_data( std::vector< std::vector< std::vector< SpikeData > > >& compressed_spike_data );

  void clear_compressed_spike_data_map();

  void dump_sources() const;
  void dump_compressible_sources() const;
  void dump_compressed_spike_data(
    const std::vector< std::vector< std::vector< SpikeData > > >& compressed_spike_data ) const;
};

inline void
SourceTable::add_source( const size_t tid, const synindex syn_id, const size_t node_id, const bool is_primary )
{
  const Source src( node_id, is_primary );
  sources_[ tid ][ syn_id ].push_back( src );
}

inline void
SourceTable::clear( const size_t tid )
{
  for ( std::vector< BlockVector< Source > >::iterator it = sources_[ tid ].begin(); it != sources_[ tid ].end(); ++it )
  {
    it->clear();
  }
  sources_[ tid ].clear();
  is_cleared_.set_true( tid );
}

inline void
SourceTable::reject_last_target_data( const size_t tid )
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

inline void
SourceTable::save_entry_point( const size_t tid )
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

inline void
SourceTable::restore_entry_point( const size_t tid )
{
  current_positions_[ tid ] = saved_positions_[ tid ];
  saved_entry_point_.set_false( tid );
}

inline void
SourceTable::reset_entry_point( const size_t tid )
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

inline void
SourceTable::reset_processed_flags( const size_t tid )
{
  for ( std::vector< BlockVector< Source > >::iterator it = sources_[ tid ].begin(); it != sources_[ tid ].end(); ++it )
  {
    for ( BlockVector< Source >::iterator iit = it->begin(); iit != it->end(); ++iit )
    {
      iit->set_processed( false );
    }
  }
}

inline void
SourceTable::no_targets_to_process( const size_t tid )
{
  current_positions_[ tid ].tid = -1;
  current_positions_[ tid ].syn_id = -1;
  current_positions_[ tid ].lcid = -1;
}

inline void
SourceTable::disable_connection( const size_t tid, const synindex syn_id, const size_t lcid )
{
  // disabling a source changes its node ID to 2^62 -1
  // source here
  assert( not sources_[ tid ][ syn_id ][ lcid ].is_disabled() );
  sources_[ tid ][ syn_id ][ lcid ].disable();
}

inline void
SourceTable::get_source_node_ids( const size_t tid,
  const synindex syn_id,
  const std::vector< size_t >& source_lcids,
  std::vector< size_t >& sources )
{
  for ( std::vector< size_t >::const_iterator cit = source_lcids.begin(); cit != source_lcids.end(); ++cit )
  {
    sources.push_back( sources_[ tid ][ syn_id ][ *cit ].get_node_id() );
  }
}

inline size_t
SourceTable::num_unique_sources( const size_t tid, const synindex syn_id ) const
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

inline size_t
SourceTable::pack_source_node_id_and_syn_id( const size_t source_node_id, const synindex syn_id ) const
{
  assert( source_node_id < 72057594037927936 );
  assert( syn_id < invalid_synindex );
  // syn_id is maximally 256, so shifting node ID by 8 bits and storing
  // syn_id in the lowest 8 leads to a unique number
  return ( source_node_id << 8 ) + syn_id;
}

inline void
SourceTable::clear_compressed_spike_data_map()
{
  for ( auto& source_index_map : compressed_spike_data_map_ )
  {
    source_index_map.clear();
  }
}

}  // namespace nest

#endif /* #ifndef SOURCE_TABLE_H */
