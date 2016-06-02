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
#include <vector>
#include <map>
#include <cassert>

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

struct TargetData;

/**
 * A data structure that stores the global id of a presynaptic neuron
 * and the number of local targets, along with a flag, whether this
 * entry has been processed yet. Used in SourceTable.
 */
struct Source
{
  index gid : 45; //!< gid of source
  unsigned int target_count : 18; //!< number of local targets
  bool processed;
  Source();
  Source( index );
};

inline
Source::Source()
  : gid( 0 )
  , target_count( 0 )
  , processed( false )
{
}

inline
Source::Source( index gid )
  : gid( gid )
  , target_count( 0 )
  , processed( false )
{
}

inline bool
operator<( const Source& lhs, const Source& rhs )
{
  return ( lhs.gid < rhs.gid );
}

inline bool
operator>( const Source& lhs, const Source& rhs )
{
  return operator<( rhs, lhs );
}

/**
 * Tuple to store position in 3d vector of sources.
 **/
struct SourceTablePosition
{
  thread tid; //!< thread index
  synindex syn_index; //!< synapse-type index
  index lcid; //!< local connection index
  SourceTablePosition();
  void reset();
};

inline
SourceTablePosition::SourceTablePosition()
  : tid( 0 )
  , syn_index( 0 )
  , lcid( 0 )
{
}

inline void
SourceTablePosition::reset()
{
  tid = 0;
  syn_index = 0;
  lcid = 0;
}

/** This data structure that stores the global ids of presynaptic
 * neuron during postsynaptic connection creation, before the
 * connection information has been transferred to the presynaptic
 * side. The core structure is the 3 dimensional sources vector, which
 * is arranged as follows:
 * 1st dimension: threads
 * 2nd dimension: synapse types
 * 3rd dimension: global ids
 * After all connections have been created, the information stored in
 * this structure is transferred to the presynaptic side and the
 * sources vector is cleared.
 */
class SourceTable
{
private:
  //! 3d structure storing gids of presynaptic neurons
  std::vector< std::vector< std::vector< Source > >* > sources_;
  //! mapping from synapse ids (according to NEST) to indices in
  //! sources_
  std::vector< std::map< synindex, synindex >* > synapse_ids_;
  //! whether the 3d structure has been deleted
  std::vector< bool > is_cleared_;
  //! the following six members are needed during the readout of the
  //! sources_ table. every time it is requested we return the next
  //! member of the sources table (see below).
  std::vector< SourceTablePosition* > current_positions_;
  std::vector< SourceTablePosition* > saved_positions_;
  std::vector< Source* > current_first_source_;
  //! if we detect an overflow in one of the MPI buffers, we save our
  //! current position in the sources table (see above) and continue
  //! at that point in the next communication round, while filling up
  //! (possible) remaining parts of the MPI buffer.
  std::vector< bool > saved_entry_point_;

public:
  SourceTable();
  ~SourceTable();
  //! initialize data structure
  void initialize();
  //! delete data structures
  void finalize();
  //! reserve memory to avoid expensive copying of vectors during
  //! connection creation
  // void reserve( thread, synindex, index );
  //! adds a source to the sources table
  void add_source( const thread tid, const synindex syn_id, const index gid );
  //! clears the sources table
  void clear( const thread tid );
  //! returns true if the sources table has been cleared
  bool is_cleared() const;
  //! returns the next target data, according to the current_* positions
  bool get_next_target_data( const thread tid, const thread rank_start, const thread rank_end, thread& target_rank, TargetData& next_target_data );
  //! rejects the last target data, and resets the current_* positions accordingly
  void reject_last_target_data( const thread tid );
  //! stores the current_* positions
  void save_entry_point( const thread tid );
  //! restores the current_* positions
  void restore_entry_point( const thread tid );
  //! resets the save_* positions to zero
  void reset_entry_point( const thread tid );
  //! returns the global id of the source at tid|syn_id|lcid
  index get_gid( const thread tid, const synindex syn_id, const index lcid ) const;
  unsigned int get_target_count( const thread tid, const synindex syn_index, const index lcid ) const;
  //! returns a reference to all sources local on thread tid (used for sorting)
  std::vector< std::vector< Source > >& get_thread_local_sources( const thread tid );
  //! resets all processed flags. needed for restructuring connection
  //! tables.
  void reset_processed_flags( const thread tid );
};

inline
void
nest::SourceTable::add_source( const thread tid, const synindex syn_id, const index gid)
{
  // the sources table is not ordered by synapse ids, to avoid wasting
  // memory, hence we need to determine for each source we are adding
  // the correct synapse index according to its synapse id
  std::map< synindex, synindex >::iterator it = synapse_ids_[ tid ]->find( syn_id );
  const Source src( gid );
  // if this synapse type is not known yet, create entry for new synapse vector
  if (it == synapse_ids_[ tid ]->end())
  {
    const index prev_n_synapse_types = synapse_ids_[ tid ]->size();
    (*synapse_ids_[ tid ])[ syn_id ] = prev_n_synapse_types;
    sources_[ tid ]->resize( prev_n_synapse_types + 1);
    (*sources_[ tid ])[ prev_n_synapse_types ].push_back( src );
  }
  // otherwise we can directly add the new source
  else
  {
    (*sources_[ tid ])[ it->second ].push_back( src );
  }
}

inline
void
nest::SourceTable::clear( const thread tid )
{
  sources_[ tid ]->clear();
  is_cleared_[ tid ] = true;
}

inline
void
nest::SourceTable::reject_last_target_data( const thread tid )
{
  SourceTablePosition& current_position = *current_positions_[ tid ];
  // adding the last target data returned by get_next_target_data
  // could not be inserted into MPI buffer due to overflow. we hence
  // need to correct the processed flag of the last entry (see
  // source_table_impl.h)
  assert( current_position.lcid > 0 );
  ( *sources_[ current_position.tid ] )[ current_position.syn_index ][ current_position.lcid - 1 ].processed = false;
  // need to decrease target count as well
  --(*current_first_source_[ tid ]).target_count;
}

inline
void
nest::SourceTable::save_entry_point( const thread tid )
{
  if ( not saved_entry_point_[ tid ] )
  {
    SourceTablePosition& current_position = *current_positions_[ tid ];
    SourceTablePosition& saved_position = *saved_positions_[ tid ];
    saved_position.tid = current_position.tid;
    saved_position.syn_index = current_position.syn_index;
    if ( current_position.lcid > 0 )
    {
      saved_position.lcid = current_position.lcid - 1;
    }
    else
    {
      saved_position.lcid = 0;
    }
    saved_entry_point_[ tid ] = true;
  }
}

inline
void
nest::SourceTable::restore_entry_point( const thread tid )
{
  *current_positions_[ tid ] = *saved_positions_[ tid ];
  saved_entry_point_[ tid ] = false;
  current_first_source_[ tid ] = 0;
}

inline
void
nest::SourceTable::reset_entry_point( const thread tid )
{
  saved_positions_[ tid ]->reset();
  current_positions_[ tid ]->reset();
  current_first_source_[ tid ] = 0;
}

inline index
nest::SourceTable::get_gid( const thread tid, const synindex syn_id, const index lcid ) const
{
  std::map< synindex, synindex >::iterator it = synapse_ids_[ tid ]->find( syn_id );
  return (*sources_[ tid ])[ it->second ][ lcid ].gid;
}

inline unsigned int
nest::SourceTable::get_target_count( const thread tid, const synindex syn_index, const index lcid ) const
{
  return (*sources_[ tid ])[ syn_index ][ lcid ].target_count;
}

inline void
SourceTable::reset_processed_flags( const thread tid )
{
  for( std::vector< std::vector< Source > >::iterator it = (*sources_[ tid ]).begin();
       it != (*sources_[ tid ]).end(); ++it )
  {
    for( std::vector< Source >::iterator iit = it->begin(); iit != it->end(); ++iit )
    {
      iit->processed = false;
    }
  }
}

} // namespace nest

#endif // SOURCE_TABLE_H
