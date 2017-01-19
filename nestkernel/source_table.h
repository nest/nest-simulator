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
#include "source.h"
#include "source_table_position.h"

namespace nest
{

class TargetData;

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
  std::vector< std::vector< std::vector< Source >* >* > sources_;
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
  //! if we detect an overflow in one of the MPI buffers, we save our
  //! current position in the sources table (see above) and continue
  //! at that point in the next communication round, while filling up
  //! (possible) remaining parts of the MPI buffer.
  std::vector< bool > saved_entry_point_;
  //! minimal number of elements that need to be deleted before
  //! reallocation happens
  static const size_t min_deleted_elements_ = 10000000;

  //! stores the index of the end of the ordered sections in sources
  //! vectors
  // TODO@5g: rename
  std::vector< std::vector< size_t >* > last_sorted_source_;

  //! set of unique sources, required to determine secondary events
  //! buffer positions
  std::set< std::pair< index, size_t > > unique_secondary_sources_;

public:
  SourceTable();
  ~SourceTable();
  //! initialize data structure
  void initialize();
  //! delete data structures
  void finalize();
  //! reserve memory to avoid expensive copying of vectors during
  //! connection creation
  void reserve( const thread tid, const synindex syn_id, const size_t count );
  //! adds a source to the sources table
  void add_source( const thread tid,
    const synindex syn_id,
    const index gid,
    const bool is_primary );
  //! clears the sources table
  void clear( const thread tid );
  //! returns true if the sources table has been cleared
  bool is_cleared() const;
  //! returns the next target data, according to the current_* positions
  bool get_next_target_data( const thread tid,
    const thread rank_start,
    const thread rank_end,
    thread& target_rank,
    TargetData& next_target_data );
  //! rejects the last target data, and resets the current_* positions
  //accordingly
  void reject_last_target_data( const thread tid );
  //! stores the current_* positions
  void save_entry_point( const thread tid );
  //! restores the current_* positions
  void restore_entry_point( const thread tid );
  //! resets the save_* positions to zero
  void reset_entry_point( const thread tid );
  //! returns the global id of the source at tid|syn_id|lcid
  index
  get_gid( const thread tid, const synindex syn_id, const index lcid ) const;
  //! returns a reference to all sources local on thread tid (used for sorting)
  std::vector< std::vector< Source >* >& get_thread_local_sources(
    const thread tid );
  //! Determines maximal saved position after which it is save to
  //! delete sources.
  SourceTablePosition find_maximal_position() const;
  //! resets all processed flags. needed for restructuring connection
  //! tables.
  void reset_processed_flags( const thread tid );
  //! Removes all entries marked as processed.
  void clean( const thread tid );
  //! Sets saved_positions for this thread to minimal values so that
  //! these are not considered in find_maximal_position.
  void no_targets_to_process( const thread tid );

  void compute_buffer_pos_for_unique_secondary_sources( const thread tid, std::map< index, size_t >& gid_to_buffer_pos );

  void reset_last_sorted_source( const thread tid );

  void update_last_sorted_source( const thread tid );

  index find_first_source( const thread tid, const synindex syn_index, const index sgid ) const;

  void find_all_sources( const thread tid,
    const index sgid,
    const synindex syn_index,
    std::vector< index >& matchings_lcids );

  void disable_connection( const thread tid,
    const synindex syn_index,
    const index lcid );

  index remove_disabled_sources( const thread tid, const synindex syn_index );

  void print_sources( const thread tid, const synindex syn_index ) const;

  void get_source_gids( const thread tid,
    const synindex syn_index,
    const std::vector< index >& source_lcids,
    std::vector< index >& sources );
};

inline void
SourceTable::add_source( const thread tid,
  const synindex syn_id,
  const index gid,
  const bool is_primary )
{
  // the sources table is not ordered by synapse ids, to avoid wasting
  // memory, hence we need to determine for each source we are adding
  // the correct synapse index according to its synapse id
  std::map< synindex, synindex >::iterator it =
    synapse_ids_[ tid ]->find( syn_id );
  const Source src( gid, is_primary );
  // if this synapse type is not known yet, create entry for new synapse vector
  if ( it == synapse_ids_[ tid ]->end() )
  {
    const index prev_n_synapse_types = synapse_ids_[ tid ]->size();
    ( *synapse_ids_[ tid ] )[ syn_id ] = prev_n_synapse_types;
    sources_[ tid ]->resize( prev_n_synapse_types + 1 );
    ( *sources_[ tid ] )[ prev_n_synapse_types ] =
      new std::vector< Source >( 0 );
    ( *sources_[ tid ] )[ prev_n_synapse_types ]->push_back( src );
  }
  // otherwise we can directly add the new source
  else
  {
    // use 1.5 growth strategy (see, e.g.,
    // https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md)
    if ( ( *sources_[ tid ] )[ it->second ]->size() == ( *sources_[ tid ] )[ it->second ]->capacity() )
    {
      ( *sources_[ tid ] )[ it->second ]->reserve( ( ( *sources_[ tid ] )[ it->second ]->size() * 3 + 1 ) / 2 );
    }
    ( *sources_[ tid ] )[ it->second ]->push_back( src );
  }
}

inline void
SourceTable::clear( const thread tid )
{
  for ( std::vector< std::vector< Source >* >::iterator it =
          sources_[ tid ]->begin();
        it != sources_[ tid ]->end();
        ++it )
  {
    ( *it )->clear();
    delete *it;
  }
  sources_[ tid ]->clear();
  is_cleared_[ tid ] = true;
}

inline void
SourceTable::reject_last_target_data( const thread tid )
{
  // adding the last target data returned by get_next_target_data
  // could not be inserted into MPI buffer due to overflow. we hence
  // need to correct the processed flag of the last entry (see
  // source_table_impl.h)
  assert( ( *current_positions_[ tid ] ).lcid + 1
    < static_cast< long >(
            ( *sources_[ ( *current_positions_[ tid ] )
                           .tid ] )[ ( *current_positions_[ tid ] ).syn_index ]
              ->size() ) );
  ( *( *sources_[ ( *current_positions_[ tid ] )
                    .tid ] )[ ( *current_positions_[ tid ] )
                                .syn_index ] )[ ( *current_positions_[ tid ] )
                                                  .lcid + 1 ].set_processed( false );
}

inline void
SourceTable::save_entry_point( const thread tid )
{
  if ( not saved_entry_point_[ tid ] )
  {
    ( *saved_positions_[ tid ] ).tid = ( *current_positions_[ tid ] ).tid;
    ( *saved_positions_[ tid ] ).syn_index =
      ( *current_positions_[ tid ] ).syn_index;
    // if tid and syn_index are valid entries, also store valid entry for lcid
    if ( ( *current_positions_[ tid ] ).tid > -1
      && ( *current_positions_[ tid ] ).syn_index > -1 )
    {
      // either store current_position.lcid + 1, since this can
      // contain non-processed entry (see reject_last_target_data()) or
      // store maximal value for lcid.
      ( *saved_positions_[ tid ] ).lcid = std::min(
        ( *current_positions_[ tid ] ).lcid + 1,
        static_cast< long >(
          ( *sources_[ ( *current_positions_[ tid ] )
                         .tid ] )[ ( *current_positions_[ tid ] ).syn_index ]
            ->size() - 1 ) );
    }
    else
    {
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
    ( *saved_positions_[ tid ] ).syn_index =
      ( *sources_[ ( *saved_positions_[ tid ] ).tid ] ).size() - 1;
  }
  else
  {
    ( *saved_positions_[ tid ] ).syn_index = -1;
  }
  if ( ( *saved_positions_[ tid ] ).syn_index > -1 )
  {
    ( *saved_positions_[ tid ] ).lcid =
      ( *sources_[ ( *saved_positions_[ tid ] )
                     .tid ] )[ ( *saved_positions_[ tid ] ).syn_index ]->size()
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
  std::map< synindex, synindex >::iterator it =
    synapse_ids_[ tid ]->find( syn_id );
  return ( *( *sources_[ tid ] )[ it->second ] )[ lcid ].get_gid();
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
  ( *current_positions_[ tid ] ).syn_index = -1;
  ( *current_positions_[ tid ] ).lcid = -1;
}

inline void
SourceTable::reset_last_sorted_source( const thread tid )
{
  ( *last_sorted_source_[ tid ] ).resize( ( *sources_[ tid ] ).size(), 0 );
  for ( synindex syn_index = 0; syn_index < ( *sources_[ tid ] ).size();
        ++syn_index )
  {
    ( *last_sorted_source_[ tid ] )[ syn_index ] = 0;
  }
}

inline void
SourceTable::update_last_sorted_source( const thread tid )
{
  ( *last_sorted_source_[ tid ] ).resize( ( *sources_[ tid ] ).size(), 0 );
  for ( synindex syn_index = 0; syn_index < ( *sources_[ tid ] ).size();
        ++syn_index )
  {
    ( *last_sorted_source_[ tid ] )[ syn_index ] =
      ( *( *sources_[ tid ] )[ syn_index ] ).size();
  }
}

inline index
SourceTable::find_first_source( const thread tid, const synindex syn_index, const index sgid ) const
{
  // binary search in sorted sources
  const std::vector< Source >::const_iterator begin =
    ( *( *sources_[ tid ] )[ syn_index ] ).begin();
  const std::vector< Source >::const_iterator end_of_sorted =
    begin + ( *last_sorted_source_[ tid ] )[ syn_index ];
  std::vector< Source >::const_iterator it =
    std::lower_bound( begin, end_of_sorted, Source( sgid, true ) );
  if ( it != end_of_sorted && it->get_gid() == sgid )
  {
    index lcid = it - begin;
    return lcid;
  }

  return invalid_index;
}

inline void
SourceTable::find_all_sources( const thread tid,
  const index sgid,
  const synindex syn_index,
  std::vector< index >& matching_lcids )
{
  // iterate over unsorted sources
  const std::vector< Source >::const_iterator begin =
    ( *( *sources_[ tid ] )[ syn_index ] ).begin();
  const std::vector< Source >::const_iterator end_of_sorted =
    begin + ( *last_sorted_source_[ tid ] )[ syn_index ];
  const std::vector< Source >::const_iterator end =
    ( *( *sources_[ tid ] )[ syn_index ] ).end();
  for ( std::vector< Source >::const_iterator it = end_of_sorted; it != end; ++it )
  {
    if ( it->get_gid() == sgid )
    {
      index lcid = it - begin;
      matching_lcids.push_back( lcid );
    }
  }
}

inline void
SourceTable::disable_connection( const thread tid,
  const synindex syn_index,
  const index lcid )
{
  // disabling a source changes its gid to 2^62 -1, hence only smaller
  // lcids that this remain sorted and we need to update last sorted
  // source here
  if ( lcid < ( *last_sorted_source_[ tid ] )[ syn_index ] )
  {
    ( *last_sorted_source_[ tid ] )[ syn_index ] = lcid;
  }
  ( *( *sources_[ tid ] )[ syn_index ] )[ lcid ].disable();
}

inline void
SourceTable::get_source_gids( const thread tid,
  const synindex syn_index,
  const std::vector< index >& source_lcids,
  std::vector< index >& sources )
{
  for ( std::vector< index >::const_iterator cit = source_lcids.begin();
        cit != source_lcids.end();
        ++cit )
  {
    sources.push_back( ( *( *sources_[ tid ] )[ syn_index ] )[ *cit ].get_gid() );
  }
}


} // namespace nest

#endif // SOURCE_TABLE_H
