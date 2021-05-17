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
 * This data structure stores the node IDs of presynaptic neurons
 * during postsynaptic connection creation, before the connection
 * information has been transferred to the presynaptic side. The core
 * structure is the three dimensional sources vector, which is
 * arranged as follows:
 * 1st dimension: threads
 * 2nd dimension: synapse types
 * 3rd dimension: node IDs
 * After all connections have been created, the information stored in
 * this structure is transferred to the presynaptic side and the
 * sources vector can be cleared.
 */
class SourceTable
{
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
   * constructing MPI buffers for communicating connections. Returns
   * false if i) this entry was already processed, or ii) this entry
   * is disabled (e.g., by structural plastcity) or iii) the reading
   * thread is not responsible for the particular part of the MPI
   * buffer where this entry would be written.
   */
  bool source_should_be_processed_( const thread rank_start, const thread rank_end, const Source& source ) const;

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
    const thread source_rank,
    TargetData& next_target_data ) const;

  /**
   * A structure to temporarily hold information about all process
   * local targets will be addressed by incoming spikes. Data from
   * this structure is transferred to the compressed_spike_data_
   * structure of ConnectionManager during construction of the
   * postsynaptic connection infrastructure. Arranged as a two
   * dimensional vector (thread|synapse) with an inner map (source
   * node id -> spike data).
   */
  std::vector< std::vector< std::map< index, SpikeData > > > compressible_sources_;

  /**
   * A structure to temporarily store locations of "unpacked spikes"
   * in the compressed_spike_data_ structure of
   * ConnectionManager. Data from this structure is transferred to the
   * presynaptic side during construction of the presynaptic
   * connection infrastructure. Arranged as a two dimensional vector
   * (thread|synapse) with an inner map (source node id -> index).
   */
  std::vector< std::vector< std::map< index, size_t > > > compressed_spike_data_map_;

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
  void add_source( const thread tid, const synindex syn_id, const index node_id, const bool is_primary );

  /**
   * Clears sources_.
   */
  void clear( const thread tid );

  /**
   * Returns true if sources_ has been cleared.
   */
  bool is_cleared() const;

  /**
   * Returns the next target data, according to the current_positions_.
   */
  bool get_next_target_data( const thread tid,
    const thread rank_start,
    const thread rank_end,
    thread& source_rank,
    TargetData& next_target_data );

  /**
   * Rejects the last target data, and resets the current_positions_
   * accordingly.
   */
  void reject_last_target_data( const thread tid );

  /**
   * Stores current_positions_ in saved_positions_.
   */
  void save_entry_point( const thread tid );

  /**
   * Restores current_positions_ from saved_positions_.
   */
  void restore_entry_point( const thread tid );

  /**
   * Resets saved_positions_ to end of sources_.
   */
  void reset_entry_point( const thread tid );

  /**
   * Returns the node ID of the source at tid|syn_id|lcid.
   */
  index get_node_id( const thread tid, const synindex syn_id, const index lcid ) const;

  /**
   * Returns a reference to all sources local on thread; necessary
   * for sorting.
   */
  std::vector< BlockVector< Source > >& get_thread_local_sources( const thread tid );

  /**
   * Determines maximal saved_positions_ after which it is safe to
   * delete sources during clean().
   */
  SourceTablePosition find_maximal_position() const;

  /**
   * Resets all processed flags. Needed for restructuring connection
   * tables, e.g., during structural plasticity update.
   */
  void reset_processed_flags( const thread tid );

  /**
   * Removes all entries marked as processed.
   */
  void clean( const thread tid );

  /**
   * Sets current_positions_ for this thread to minimal values so that
   * these are not considered in find_maximal_position().
   */
  void no_targets_to_process( const thread tid );

  /**
   * Computes MPI buffer positions for unique combination of source
   * node ID and synapse type across all threads for all secondary
   * connections.
   */
  void compute_buffer_pos_for_unique_secondary_sources( const thread tid,
    std::map< index, size_t >& buffer_pos_of_source_node_id_syn_id_ );

  /**
   * Finds the first entry in sources_ at the given thread id and
   * synapse type that is equal to snode_id.
   */
  index find_first_source( const thread tid, const synindex syn_id, const index snode_id ) const;

  /**
   * Marks entry in sources_ at given position as disabled.
   */
  void disable_connection( const thread tid, const synindex syn_id, const index lcid );

  /**
   * Removes all entries from sources_ that are marked as disabled.
   */
  index remove_disabled_sources( const thread tid, const synindex syn_id );

  /**
   * Returns node IDs for entries in sources_ for the given thread
   * id, synapse type and local connections ids.
   */
  void get_source_node_ids( const thread tid,
    const synindex syn_id,
    const std::vector< index >& source_lcids,
    std::vector< index >& sources );

  /**
   * Returns the number of unique node IDs for given thread id and
   * synapse type in sources_. This number corresponds to the number
   * of targets that need to be communicated during construction of
   * the presynaptic connection infrastructure.
   */
  size_t num_unique_sources( const thread tid, const synindex syn_id ) const;

  /**
   * Resizes sources_ according to total number of threads and
   * synapse types.
   */
  void resize_sources( const thread tid );

  /**
   * Encodes combination of node ID and synapse types as single
   * long number.
   */
  index pack_source_node_id_and_syn_id( const index source_node_id, const synindex syn_id ) const;

  void resize_compressible_sources();

  // creates maps of sources with more than one thread-local target
  void collect_compressible_sources( const thread tid );
  // fills the compressed_spike_data structure in ConnectionManager
  void fill_compressed_spike_data( std::vector< std::vector< std::vector< SpikeData > > >& compressed_spike_data );

  void clear_compressed_spike_data_map( const thread tid );
};

inline void
SourceTable::add_source( const thread tid, const synindex syn_id, const index node_id, const bool is_primary )
{
  const Source src( node_id, is_primary );
  sources_[ tid ][ syn_id ].push_back( src );
}

inline void
SourceTable::clear( const thread tid )
{
  for ( std::vector< BlockVector< Source > >::iterator it = sources_[ tid ].begin(); it != sources_[ tid ].end(); ++it )
  {
    it->clear();
  }
  sources_[ tid ].clear();
  is_cleared_[ tid ].set_true();
}

inline void
SourceTable::reject_last_target_data( const thread tid )
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
SourceTable::save_entry_point( const thread tid )
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
        static_cast< long >( sources_[ current_positions_[ tid ].tid ][ current_positions_[ tid ].syn_id ].size()
                                                 - 1 ) );
    }
    else
    {
      assert( current_positions_[ tid ].lcid == -1 );
      saved_positions_[ tid ].lcid = -1;
    }
    saved_entry_point_[ tid ].set_true();
  }
}

inline void
SourceTable::restore_entry_point( const thread tid )
{
  current_positions_[ tid ] = saved_positions_[ tid ];
  saved_entry_point_[ tid ].set_false();
}

inline void
SourceTable::reset_entry_point( const thread tid )
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
SourceTable::reset_processed_flags( const thread tid )
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
SourceTable::no_targets_to_process( const thread tid )
{
  current_positions_[ tid ].tid = -1;
  current_positions_[ tid ].syn_id = -1;
  current_positions_[ tid ].lcid = -1;
}

inline index
SourceTable::find_first_source( const thread tid, const synindex syn_id, const index snode_id ) const
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
      const index lcid = it - begin;
      return lcid;
    }
    ++it;
  }

  // no enabled entry with this snode ID found
  return invalid_index;
}

inline void
SourceTable::disable_connection( const thread tid, const synindex syn_id, const index lcid )
{
  // disabling a source changes its node ID to 2^62 -1
  // source here
  assert( not sources_[ tid ][ syn_id ][ lcid ].is_disabled() );
  sources_[ tid ][ syn_id ][ lcid ].disable();
}

inline void
SourceTable::get_source_node_ids( const thread tid,
  const synindex syn_id,
  const std::vector< index >& source_lcids,
  std::vector< index >& sources )
{
  for ( std::vector< index >::const_iterator cit = source_lcids.begin(); cit != source_lcids.end(); ++cit )
  {
    sources.push_back( sources_[ tid ][ syn_id ][ *cit ].get_node_id() );
  }
}

inline size_t
SourceTable::num_unique_sources( const thread tid, const synindex syn_id ) const
{
  size_t n = 0;
  index last_source = 0;
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

inline index
SourceTable::pack_source_node_id_and_syn_id( const index source_node_id, const synindex syn_id ) const
{
  assert( source_node_id < 72057594037927936 );
  assert( syn_id < invalid_synindex );
  // syn_id is maximally 256, so shifting node ID by 8 bits and storing
  // syn_id in the lowest 8 leads to a unique number
  return ( source_node_id << 8 ) + syn_id;
}

inline void
SourceTable::clear_compressed_spike_data_map( const thread tid )
{
  for ( synindex syn_id = 0; syn_id < compressed_spike_data_map_[ tid ].size(); ++syn_id )
  {
    compressed_spike_data_map_[ tid ][ syn_id ].clear();
  }
}

} // namespace nest

#endif // SOURCE_TABLE_H
