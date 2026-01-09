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

  size_t get_source_index() const;
  size_t get_target_thread() const;

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
   * dimensional vector (thread|synapse) with an inner map (source
   * node id -> spike data).
   */
  std::vector< std::vector< std::map< size_t, SpikeData > > > compressible_sources_;

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

  // creates maps of sources with more than one thread-local target
  void collect_compressible_sources( const size_t tid );
  // fills the compressed_spike_data structure in ConnectionManager
  void fill_compressed_spike_data( std::vector< std::vector< std::vector< SpikeData > > >& compressed_spike_data );

  void clear_compressed_spike_data_map();

  void dump_sources() const;
  void dump_compressible_sources() const;
  void dump_compressed_spike_data(
    const std::vector< std::vector< std::vector< SpikeData > > >& compressed_spike_data ) const;
};


} // namespace nest

#endif /* #ifndef SOURCE_TABLE_H */
