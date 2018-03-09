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
#include "nest_types.h"
#include "mpi_manager.h"
#include "source.h"
#include "source_table_position.h"

namespace nest
{

class TargetData;

/** This data structure stores the global ids of presynaptic neurons
 * during postsynaptic connection creation, before the connection
 * information has been transferred to the presynaptic side. The core
 * structure is the three dimensional sources vector, which is
 * arranged as follows:
 * 1st dimension: threads
 * 2nd dimension: synapse types
 * 3rd dimension: global ids
 * After all connections have been created, the information stored in
 * this structure is transferred to the presynaptic side and the
 * sources vector can be cleared.
 */
class SourceTable
{
private:
  //! 3d structure storing gids of presynaptic neurons
  std::vector< std::vector< std::vector< Source >* >* > sources_;

  //! whether the 3d structure has been deleted
  std::vector< bool > is_cleared_;

  //! these are needed during readout of sources_
  std::vector< SourceTablePosition* > current_positions_;
  std::vector< SourceTablePosition* > saved_positions_;

  //! if we detect an overflow in one of the MPI buffer parts, we save
  //! our current position in sources_ to continue at that point in
  //! the next communication round, while filling up (possible)
  //! remaining parts of the MPI buffer.
  std::vector< bool > saved_entry_point_;

  //! minimal number of sources that need to be deleted per synapse
  //! type and thread before a reallocation of the respective vector
  //! is performed; balances number of reallocations and memory usage;
  //! see SourceTable::clean()
  static const size_t min_deleted_elements_ = 1000000;

  //! stores the index of the end of the sorted sections in sources
  //! vectors
  std::vector< std::vector< size_t >* > last_sorted_source_;

public:
  SourceTable();
  ~SourceTable();

  //! initialize data structure
  void initialize();

  //! delete data structures
  void finalize();

  //! reserve memory to avoid expensive reallocation of vectors during
  //! connection creation
  void reserve( const thread tid, const synindex syn_id, const size_t count );

  //! adds a source to sources_
  void add_source( const thread tid,
    const synindex syn_id,
    const index gid,
    const bool is_primary );

  //! clears sources_
  void clear( const thread tid );

  //! returns true if sources_ has been cleared
  bool is_cleared() const;

  //! returns the next target data, according to the current_positions_
  bool get_next_target_data( const thread tid,
    const thread rank_start,
    const thread rank_end,
    thread& source_rank,
    TargetData& next_target_data );

  //! rejects the last target data, and resets the current_positions_
  //! accordingly
  void reject_last_target_data( const thread tid );

  //! stores current_positions_ in saved_positions_
  void save_entry_point( const thread tid );

  //! restores current_positions_ from saved_positions_
  void restore_entry_point( const thread tid );

  //! resets saved_positions_ to end of sources_
  void reset_entry_point( const thread tid );

  //! returns the global id of the source at tid|syn_id|lcid
  index
  get_gid( const thread tid, const synindex syn_id, const index lcid ) const;

  //! returns a reference to all sources local on thread; necessary
  //! for sorting
  std::vector< std::vector< Source >* >& get_thread_local_sources(
    const thread tid );

  //! determines maximal saved_positions_ after which it is save to
  //! delete sources during clean()
  SourceTablePosition find_maximal_position() const;

  //! resets all processed flags. needed for restructuring connection
  //! tables, e.g., during structural plasticity update
  void reset_processed_flags( const thread tid );

  //! removes all entries marked as processed
  void clean( const thread tid );

  //! sets saved_positions for this thread to minimal values so that
  //! these are not considered in find_maximal_position
  void no_targets_to_process( const thread tid );

  //! computes MPI buffer positions for unique combination of source
  //! gid and synapse type across all threads for all secondary
  //! connections
  void compute_buffer_pos_for_unique_secondary_sources( const thread tid, std::map< index, size_t >& buffer_pos_of_source_gid_syn_id_ );

  //! sets last_sorted_source_ to beginning of sources_ to make sure
  //! all entries are considered during sorting of connections
  void reset_last_sorted_source( const thread tid );

  //! sets last_sorted_source_ to end of sources_; is done after
  //! sorting connections, to mark all entries as sorted
  void update_last_sorted_source( const thread tid );

  //! finds the first entry in sources_ at the given thread id and
  //! synapse type, that is equal to sgid
  index find_first_source( const thread tid, const synindex syn_id, const index sgid ) const;

  //! find all entries in sources_ at the given thread id and synapse
  //! type, that are equal to sgid in range of unsorted sources
  void find_all_sources_unsorted( const thread tid,
    const index sgid,
    const synindex syn_id,
    std::vector< index >& matchings_lcids );

  //! marks entry in sources_ at given position as disabled
  void disable_connection( const thread tid,
    const synindex syn_id,
    const index lcid );

  //! removes all entries from sources_ that are marked as disabled
  index remove_disabled_sources( const thread tid, const synindex syn_id );

  //TODO@5g: remove?
  void print_sources( const thread tid, const synindex syn_id ) const;

  //! returns global ids for entries in sources_ for the given thread
  //! id, synapse type and local connections ids
  void get_source_gids( const thread tid,
    const synindex syn_id,
    const std::vector< index >& source_lcids,
    std::vector< index >& sources );

  //! returns the number of unique global ids for given thread id and
  //! synapse type in sources_; this number corresponds to the number
  //! of targets that need to be communicated during construction of
  //! the presynaptic connection infrastructure
  size_t num_unique_sources( const thread tid, const synindex syn_id ) const;

  //! resizes sources_ according to total number of threads and
  //! synapse types
  void resize_sources( const thread tid );

  //! encodes combination of global id and synapse types as single
  //! long number
  index pack_source_gid_and_syn_id( const index source_gid, const synindex syn_id ) const;
};

inline void
SourceTable::add_source( const thread tid,
  const synindex syn_id,
  const index gid,
  const bool is_primary )
{
  const Source src( gid, is_primary );

  // use 1.5 growth strategy (see, e.g.,
  // https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md)
  if ( ( *sources_[ tid ] )[ syn_id ]->size() == ( *sources_[ tid ] )[ syn_id ]->capacity() )
  {
    ( *sources_[ tid ] )[ syn_id ]->reserve( ( ( *sources_[ tid ] )[ syn_id ]->size() * 3 + 1 ) / 2 );
  }

  ( *sources_[ tid ] )[ syn_id ]->push_back( src );
}

inline void
SourceTable::clear( const thread tid )
{
  for ( std::vector< std::vector< Source >* >::iterator it =
          sources_[ tid ]->begin();
        it != sources_[ tid ]->end();
        ++it )
  {
    if ( ( *it ) != NULL )
    {
      ( *it )->clear();
      delete *it;
    }
  }
  sources_[ tid ]->clear();
  is_cleared_[ tid ] = true;
}

inline void
SourceTable::reject_last_target_data( const thread tid )
{
  // the last target data returned by get_next_target_data() could not
  // be inserted into MPI buffer due to overflow. we hence need to
  // correct the processed flag of the last entry (see
  // source_table_impl.h)
  assert( ( *current_positions_[ tid ] ).lcid + 1
    < static_cast< long >
	  ( ( *sources_[ ( *current_positions_[ tid ] ).tid ] )
	    [ ( *current_positions_[ tid ] ).syn_id ]->size() ) );

  ( *( *sources_[ ( *current_positions_[ tid ] ).tid ] )
    [ ( *current_positions_[ tid ] ).syn_id ] )
    [ ( *current_positions_[ tid ] ).lcid + 1 ].set_processed( false );
}

inline void
SourceTable::save_entry_point( const thread tid )
{
  if ( not saved_entry_point_[ tid ] )
  {
    ( *saved_positions_[ tid ] ).tid = ( *current_positions_[ tid ] ).tid;
    ( *saved_positions_[ tid ] ).syn_id =
      ( *current_positions_[ tid ] ).syn_id;
    // TODO@5g: set lcid here?
    // ( *saved_positions_[ tid ] ).lcid =
    //   ( *current_positions_[ tid ] ).lcid;

    // if tid and syn_id are valid entries, also store valid entry for lcid
    if ( ( *current_positions_[ tid ] ).tid > -1
      and ( *current_positions_[ tid ] ).syn_id > -1 )
    {
      // either store current_position.lcid + 1, since this can
      // contain non-processed entry (see reject_last_target_data()) or
      // store maximal value for lcid.
      ( *saved_positions_[ tid ] ).lcid = std::min(
        ( *current_positions_[ tid ] ).lcid + 1,
        static_cast< long >(
          ( *sources_[ ( *current_positions_[ tid ] ).tid ] )
	  [ ( *current_positions_[ tid ] ).syn_id ]->size() - 1 ) );
    }
    else
    {
      assert( ( *current_positions_[ tid ] ).lcid == -1 );
      ( *saved_positions_[ tid ] ).lcid = -1;
    }
    saved_entry_point_[ tid ] = true;
  }
}

inline void
SourceTable::restore_entry_point( const thread tid )
{
  *current_positions_[ tid ] = *saved_positions_[ tid ];
  saved_entry_point_[ tid ] = false;
}

inline void
SourceTable::reset_entry_point( const thread tid )
{
  // since we read the source table backwards, we need to set saved
  // values to the biggest possible value. these will be used to
  // initialize current_positions_ correctly upon calling
  // restore_entry_point. however, this can only be done if other
  // values have valid values.
  ( *saved_positions_[ tid ] ).tid = sources_.size() - 1;
  if ( ( *saved_positions_[ tid ] ).tid > -1 )
  {
    ( *saved_positions_[ tid ] ).syn_id =
      ( *sources_[ ( *saved_positions_[ tid ] ).tid ] ).size() - 1;
  }
  else
  {
    ( *saved_positions_[ tid ] ).syn_id = -1;
  }
  if ( ( *saved_positions_[ tid ] ).syn_id > -1 )
  {
    ( *saved_positions_[ tid ] ).lcid =
      ( *sources_[ ( *saved_positions_[ tid ] )
                     .tid ] )[ ( *saved_positions_[ tid ] ).syn_id ]->size()
      - 1;
  }
  else
  {
    ( *saved_positions_[ tid ] ).lcid = -1;
  }
}

inline index
SourceTable::get_gid( const thread tid,
  const synindex syn_id,
  const index lcid ) const
{
  return ( *( *sources_[ tid ] )[ syn_id ] )[ lcid ].get_gid();
}

inline void
SourceTable::reset_processed_flags( const thread tid )
{
  for ( std::vector< std::vector< Source >* >::iterator it =
          ( *sources_[ tid ] ).begin();
        it != ( *sources_[ tid ] ).end();
        ++it )
  {
    for ( std::vector< Source >::iterator iit = ( *it )->begin();
          iit != ( *it )->end();
          ++iit )
    {
      iit->set_processed( false );
    }
  }
}

inline void
SourceTable::no_targets_to_process( const thread tid )
{
  ( *current_positions_[ tid ] ).tid = -1;
  ( *current_positions_[ tid ] ).syn_id = -1;
  ( *current_positions_[ tid ] ).lcid = -1;
}

inline void
SourceTable::reset_last_sorted_source( const thread tid )
{
  ( *last_sorted_source_[ tid ] ).resize( ( *sources_[ tid ] ).size(), 0 );
  for ( synindex syn_id = 0; syn_id < ( *sources_[ tid ] ).size();
        ++syn_id )
  {
    ( *last_sorted_source_[ tid ] )[ syn_id ] = 0;
  }
}

inline void
SourceTable::update_last_sorted_source( const thread tid )
{
  ( *last_sorted_source_[ tid ] ).resize( ( *sources_[ tid ] ).size(), 0 );
  for ( synindex syn_id = 0; syn_id < ( *sources_[ tid ] ).size();
        ++syn_id )
  {
    ( *last_sorted_source_[ tid ] )[ syn_id ] =
      ( *( *sources_[ tid ] )[ syn_id ] ).size();
  }
}

inline index
SourceTable::find_first_source( const thread tid, const synindex syn_id, const index sgid ) const
{
  // binary search in sorted sources
  const std::vector< Source >::const_iterator begin =
    ( *( *sources_[ tid ] )[ syn_id ] ).begin();
  const std::vector< Source >::const_iterator end_of_sorted =
    begin + ( *last_sorted_source_[ tid ] )[ syn_id ];
  std::vector< Source >::const_iterator it =
    std::lower_bound( begin, end_of_sorted, Source( sgid, true ) );

  // source found by binary search could be disabled, iterate through
  // sources until a valid one is found
  while ( it != end_of_sorted )
  {
    if ( it->get_gid() == sgid and not it->is_disabled() )
    {
      const index lcid = it - begin;
      return lcid;
    }
    ++it;
  }

  // no enabled entry with this sgid found
  return invalid_index;
}

inline void
SourceTable::find_all_sources_unsorted( const thread tid,
  const index sgid,
  const synindex syn_id,
  std::vector< index >& matching_lcids )
{
  // iterate over unsorted sources
  const std::vector< Source >::const_iterator begin =
    ( *( *sources_[ tid ] )[ syn_id ] ).begin();
  const std::vector< Source >::const_iterator end_of_sorted =
    begin + ( *last_sorted_source_[ tid ] )[ syn_id ];
  const std::vector< Source >::const_iterator end =
    ( *( *sources_[ tid ] )[ syn_id ] ).end();
  for ( std::vector< Source >::const_iterator it = end_of_sorted; it != end; ++it )
  {
    if ( it->get_gid() == sgid )
    {
      matching_lcids.push_back( it - begin );
    }
  }
}

inline void
SourceTable::disable_connection( const thread tid,
  const synindex syn_id,
  const index lcid )
{
  // disabling a source changes its gid to 2^62 -1, hence only smaller
  // lcids that this remain sorted and we need to update last sorted
  // source here
  if ( lcid < ( *last_sorted_source_[ tid ] )[ syn_id ] )
  {
    ( *last_sorted_source_[ tid ] )[ syn_id ] = lcid;
  }
  assert( not ( *( *sources_[ tid ] )[ syn_id ] )[ lcid ].is_disabled() );
  ( *( *sources_[ tid ] )[ syn_id ] )[ lcid ].disable();
}

inline void
SourceTable::get_source_gids( const thread tid,
  const synindex syn_id,
  const std::vector< index >& source_lcids,
  std::vector< index >& sources )
{
  for ( std::vector< index >::const_iterator cit = source_lcids.begin();
        cit != source_lcids.end();
        ++cit )
  {
    sources.push_back( ( *( *sources_[ tid ] )[ syn_id ] )[ *cit ].get_gid() );
  }
}

inline size_t
SourceTable::num_unique_sources( const thread tid, const synindex syn_id ) const
{
  size_t n = 0;
  index last_source = 0;
  for ( std::vector< Source >::const_iterator cit = ( *( *sources_[ tid ] )[ syn_id ] ).begin();
        cit != ( *( *sources_[ tid ] )[ syn_id ] ).end(); ++cit )
  {
    if ( last_source != ( *cit ).get_gid() )
    {
      last_source = ( *cit ).get_gid();
      ++n;
    }
  }
  return n;
}

inline index
SourceTable::pack_source_gid_and_syn_id( const index source_gid, const synindex syn_id ) const
{
  assert( source_gid < 72057594037927936 );
  assert( syn_id < invalid_synindex );
  // syn_id is maximally 256, so shifting gid by 8 bits and storing
  // syn_id in the lowest 8 leads to a unique number
  return ( source_gid << 8 ) + syn_id;
}

} // namespace nest

#endif // SOURCE_TABLE_H
