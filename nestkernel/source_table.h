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
 * along with a flag, whether this entry has been processed yet. Used
 * in SourceTable.
 */
struct Source
{
  // TODO@5g: use nest_types: index
  // TODO@5g: possible to define in nest_types?
  unsigned long gid : 63;
  unsigned long processed : 1;
  Source();
  Source( index );
};

inline
Source::Source()
  : gid( 0 )
  , processed( false )
{
}

inline
Source::Source( index gid )
  : gid( gid )
  , processed( false )
{
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
  std::vector< unsigned int > current_tid_;
  std::vector< unsigned int > current_syn_id_;
  std::vector< unsigned int > current_lcid_;
  std::vector< unsigned int > save_tid_;
  std::vector< unsigned int > save_syn_id_;
  std::vector< unsigned int > save_lcid_;
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
  void clear( const thread );
  //! returns true if the sources table has been cleared
  bool is_cleared() const;
  //! returns the next target data, according to the current_* positions
  void get_next_target_data( const thread tid, TargetData& next_target_data, const unsigned int rank_start, const unsigned int rank_end );
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
  // adding the last target data returned by get_next_target_data
  // could not be inserted into MPI buffer due to overflow. we hence
  // need to reduce the current lcid cound by one, since after this
  // function, save_entry_point is called, which uses the current_lcid
  // values as the save point. we also need to correct the processed
  // flag of the last entry.
  assert( current_lcid_[ tid ] > 0 );
  --current_lcid_[ tid ];
  ( *sources_[ current_tid_[ tid ] ] )[ current_syn_id_[ tid ] ][ current_lcid_[ tid ]  ].processed = false;
}

inline
void
nest::SourceTable::save_entry_point( const thread tid )
{
  if ( not saved_entry_point_[ tid ] )
  {
    save_tid_[ tid ] = current_tid_[ tid ];
    save_syn_id_[ tid ] = current_syn_id_[ tid ];
    save_lcid_[ tid ] = current_lcid_[ tid ];
    saved_entry_point_[ tid ] = true;
  }
}

inline
void
nest::SourceTable::restore_entry_point( const thread tid )
{
  current_tid_[ tid ] = save_tid_[ tid ];
  current_syn_id_[ tid ] = save_syn_id_[ tid ];
  current_lcid_[ tid ] = save_lcid_[ tid ];
  // we want to be able to store the positions again at a later point,
  // hence we need to set saved_entry_point to false
  saved_entry_point_[ tid ] = false;
}

inline
void
nest::SourceTable::reset_entry_point( const thread tid )
{
  save_tid_[ tid ] = 0;
  save_syn_id_[ tid ] = 0;
  save_lcid_[ tid ] = 0;
}

inline index
nest::SourceTable::get_gid( const thread tid, const synindex syn_id, const index lcid ) const
{
  std::map< synindex, synindex >::iterator it = synapse_ids_[ tid ]->find( syn_id );
  return (*sources_[ tid ])[ it->second ][ lcid ].gid;
}

} // namespace nest

#endif
