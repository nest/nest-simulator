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

// Includes from nestkernel:
#include "connection_manager_impl.h"
#include "node_manager_impl.h"
#include "kernel_manager.h"
#include "source_table.h"
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
  const thread num_threads = kernel().vp_manager.get_num_threads();
  synapse_ids_.resize( num_threads );
  sources_.resize( num_threads );
  is_cleared_.resize( num_threads );
  saved_entry_point_.resize( num_threads );
  current_positions_.resize( num_threads );
  saved_positions_.resize( num_threads );
  last_sorted_source_.resize( num_threads );

#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    synapse_ids_[ tid ] = new std::map< synindex, synindex >();
    sources_[ tid ] = new std::vector< std::vector< Source >* >( 0 );
    current_positions_[ tid ] = new SourceTablePosition();
    saved_positions_[ tid ] = new SourceTablePosition();
    is_cleared_[ tid ] = false;
    saved_entry_point_[ tid ] = false;
    last_sorted_source_[ tid ] = new std::vector< size_t >( 0 );
  } // of omp parallel
}

void
nest::SourceTable::finalize()
{
  for ( std::vector< std::map< synindex, synindex >* >::iterator it =
          synapse_ids_.begin();
        it != synapse_ids_.end();
        ++it )
  {
    delete *it;
  }
  synapse_ids_.clear();
  if ( not is_cleared() )
  {
    for ( size_t tid = 0; tid < sources_.size(); ++tid )
    {
      clear( tid );
    }
  }
  for ( std::vector< std::vector< std::vector< Source >* >* >::iterator it =
          sources_.begin();
        it != sources_.end();
        ++it )
  {
    delete *it;
  }
  sources_.clear();
  for ( std::vector< SourceTablePosition* >::iterator it =
          current_positions_.begin();
        it != current_positions_.end();
        ++it )
  {
    delete *it;
  }
  current_positions_.clear();
  for (
    std::vector< SourceTablePosition* >::iterator it = saved_positions_.begin();
    it != saved_positions_.end();
    ++it )
  {
    delete *it;
  }
  saved_positions_.clear();

  for ( std::vector< std::vector< size_t >* >::iterator it =
          last_sorted_source_.begin();
        it != last_sorted_source_.end();
        ++it )
  {
    delete *it;
  }
  last_sorted_source_.clear();
}

bool
nest::SourceTable::is_cleared() const
{
  bool all_cleared = true;
  // we only return true, if is_cleared is true for all threads
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    all_cleared &= is_cleared_[ tid ];
  }
  return all_cleared;
}

std::vector< std::vector< nest::Source >* >&
nest::SourceTable::get_thread_local_sources( const thread tid )
{
  return *sources_[ tid ];
}

nest::SourceTablePosition
nest::SourceTable::find_maximal_position() const
{
  SourceTablePosition max_position( -1, -1, -1 );
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( max_position < ( *saved_positions_[ tid ] ) )
    {
      max_position = ( *saved_positions_[ tid ] );
    }
  }
  return max_position;
}

void
nest::SourceTable::clean( const thread tid )
{
  // find maximal position in source table among threads to make sure
  // unprocessed entries are not removed. given this maximal position,
  // we can savely delete all larger entries since they will not be
  // touched any more.
  const SourceTablePosition max_position = find_maximal_position();
  // we need to distinguish whether we are in the vector corresponding
  // to max position or above. we can delete all entries above the
  // maximal position, otherwise we need to respect to indices.
  if ( max_position.tid == tid )
  {
    for ( synindex syn_index = max_position.syn_index;
          syn_index < ( *sources_[ tid ] ).size();
          ++syn_index )
    {
      if ( max_position.syn_index == syn_index )
      {
        std::vector< Source >& sources = *( *sources_[ tid ] )[ syn_index ];
        // we need to add 1 to max_position.lcid since
        // max_position.lcid can contain a valid entry which we do not
        // want to delete.
        if ( max_position.lcid + 1 < static_cast< long >( sources.size() ) )
        {
          const size_t deleted_elements =
            sources.end() - ( sources.begin() + max_position.lcid + 1 );
          sources.erase(
            sources.begin() + max_position.lcid + 1, sources.end() );
          if ( deleted_elements > min_deleted_elements_ )
          {
            std::vector< Source >( sources.begin(), sources.end() )
              .swap( sources );
          }
        }
      }
      else
      {
        assert( max_position.syn_index < syn_index );
        ( *sources_[ tid ] )[ syn_index ]->clear();
        delete ( *sources_[ tid ] )[ syn_index ];
        ( *sources_[ tid ] )[ syn_index ] = NULL;
      }
    }
  }
  else if ( max_position.tid < tid )
  {
    for ( synindex syn_index = 0; syn_index < ( *sources_[ tid ] ).size();
          ++syn_index )
    {
      ( *sources_[ tid ] )[ syn_index ]->clear();
      delete ( *sources_[ tid ] )[ syn_index ];
      ( *sources_[ tid ] )[ syn_index ] = NULL;
    }
  }
  else
  {
    // do nothing
  }
}

void
nest::SourceTable::reserve( const thread tid,
  const synindex syn_id,
  const size_t count )
{
  std::map< synindex, synindex >::iterator it =
    synapse_ids_[ tid ]->find( syn_id );
  // if this synapse type is not known yet, create entry for new synapse vector
  if ( it == synapse_ids_[ tid ]->end() )
  {
    const index prev_n_synapse_types = synapse_ids_[ tid ]->size();
    ( *synapse_ids_[ tid ] )[ syn_id ] = prev_n_synapse_types;
    sources_[ tid ]->resize( prev_n_synapse_types + 1 );
    ( *sources_[ tid ] )[ prev_n_synapse_types ] =
      new std::vector< Source >( 0 );
    ( *sources_[ tid ] )[ prev_n_synapse_types ]->reserve( count );
  }
  // otherwise we can directly reserve
  else
  {
    ( *sources_[ tid ] )[ it->second ]->reserve( ( *sources_[ tid ] )[ it->second ]->size() + count );
  }
}

nest::index
nest::SourceTable::remove_disabled_sources( const thread tid,
  const synindex syn_index )
{
  const index max_size = ( *( *sources_[ tid ] )[ syn_index ] ).size();
  index i = max_size - 1;

  while ( ( *( *sources_[ tid ] )[ syn_index ] )[ i ].is_disabled() && i >= 0 )
  {
    --i;
  }
  ++i;

  ( *( *sources_[ tid ] )[ syn_index ] )
    .erase( ( *( *sources_[ tid ] )[ syn_index ] ).begin() + i,
      ( *( *sources_[ tid ] )[ syn_index ] ).end() );

  if ( i == max_size )
  {
    return invalid_index;
  }
  else
  {
    return i;
  }
}

void
nest::SourceTable::print_sources( const thread tid,
  const synindex syn_index ) const
{
  if ( syn_index >= ( *sources_[ tid ] ).size() )
  {
    return;
  }

  index prev_gid = 0;
  std::cout << "-------------SOURCES-------------------\n";
  for ( std::vector< Source >::const_iterator it =
          ( *( *sources_[ tid ] )[ syn_index ] ).begin();
        it != ( *( *sources_[ tid ] )[ syn_index ] ).end();
        ++it )
  {
    if ( prev_gid != it->get_gid() )
    {
      std::cout << std::endl;
      prev_gid = it->get_gid();
    }
    std::cout << "(" << it->get_gid() << ", " << it->is_disabled() << ")";
  }
  std::cout << std::endl;
  std::cout << "---------------------------------------\n";
}

void
nest::SourceTable::compute_buffer_pos_for_unique_secondary_sources( const thread tid,
  std::map< index, size_t >& gid_to_buffer_pos )
{
#pragma omp critical
  {
    std::cout<< "computing buffer unique" << std::endl;
  }
#pragma omp single
  {
    unique_secondary_sources_.clear();
  }

  // collect all unique sources
  for ( size_t syn_index = 0; syn_index < sources_[ tid ]->size();
        ++syn_index )
  {
    const synindex syn_id =
      kernel().connection_manager.get_syn_id( tid, syn_index );
    if ( not kernel()
         .model_manager.get_synapse_prototype( syn_id, tid )
         .is_primary() )
    {
      const size_t event_size =
        kernel()
        .model_manager.get_secondary_event_prototype( syn_id, tid )
        .prototype_size();
      for ( std::vector< Source >::const_iterator cit =
              ( *sources_[ tid ] )[ syn_index ]->begin();
            cit != ( *sources_[ tid ] )[ syn_index ]->end();
            ++cit )
      {
#pragma omp critical
        {
          unique_secondary_sources_.insert( std::pair< index, size_t >( cit->get_gid(), event_size ) );
        }
      }
    }
  }
#pragma omp barrier

#pragma omp single
  {
    // given all unique sources, calculate maximal chunksize per rank
    // and fill vector of unique sources
    std::vector< size_t > count_per_rank(
      kernel().mpi_manager.get_num_processes(), 0 );
    for ( std::set< std::pair< index, size_t > >::const_iterator cit = unique_secondary_sources_.begin();
          cit != unique_secondary_sources_.end(); ++cit )
    {
      count_per_rank[ kernel().node_manager.get_process_id_of_gid( cit->first ) ] += cit->second;
    }

    // determine maximal chunksize across all MPI ranks
    std::vector< long > max_count(
      1, *std::max_element( count_per_rank.begin(), count_per_rank.end() ) );
    kernel().mpi_manager.communicate_Allreduce_max_in_place( max_count );

    kernel().mpi_manager.set_chunk_size_secondary_events(
      max_count[ 0 ] + 1 );

    // offsets in receive buffer
    std::vector< size_t > buffer_position_by_rank(
      kernel().mpi_manager.get_num_processes(), 0 );
    for ( size_t rank = 0; rank < buffer_position_by_rank.size(); ++rank )
    {
      buffer_position_by_rank[ rank ] = rank * kernel().mpi_manager.get_chunk_size_secondary_events();
    }

    for ( std::set< std::pair< index, size_t > >::const_iterator cit = unique_secondary_sources_.begin();
          cit != unique_secondary_sources_.end(); ++cit )
    {
      const thread target_rank = kernel().node_manager.get_process_id_of_gid( cit->first );
      gid_to_buffer_pos.insert( std::pair< index, size_t >( cit->first, buffer_position_by_rank[ target_rank ] ) );
      buffer_position_by_rank[ target_rank ] += cit->second;
    }
  } // of omp single
}
