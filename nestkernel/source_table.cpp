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
#include "source_table.h"
#include "kernel_manager.h"
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
  assert( sizeof(Source) == 8 );
  const thread num_threads = kernel().vp_manager.get_num_threads();
  synapse_ids_.resize( num_threads );
  sources_.resize( num_threads );
  is_cleared_.resize( num_threads, false );
  saved_entry_point_.resize( num_threads, false );
  current_positions_.resize( num_threads );
  saved_positions_.resize( num_threads );

  for( thread tid = 0; tid < num_threads; ++tid)
  {
    synapse_ids_[ tid ] = new std::map< synindex, synindex >();
    sources_[ tid ] = new std::vector< std::vector< Source > >(
      0, std::vector< Source >( 0, Source() ) );
    current_positions_[ tid ] = new SourceTablePosition();
    saved_positions_[ tid ] = new SourceTablePosition();
  }
}

void
nest::SourceTable::finalize()
{
  for( std::vector< std::map< synindex, synindex >* >::iterator it =
         synapse_ids_.begin(); it != synapse_ids_.end(); ++it )
  {
    delete *it;
  }
  synapse_ids_.clear();
  for( std::vector< std::vector< std::vector< Source > >* >::iterator it =
         sources_.begin(); it != sources_.end(); ++it )
  {
    delete *it;
  }
  sources_.clear();
  for ( std::vector< SourceTablePosition* >::iterator it = current_positions_.begin(); it != current_positions_.end(); ++it )
  {
    delete *it;
  }
  current_positions_.clear();
  for ( std::vector< SourceTablePosition* >::iterator it = saved_positions_.begin(); it != saved_positions_.end(); ++it )
  {
    delete *it;
  }
  saved_positions_.clear();
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

std::vector< std::vector< nest::Source > >&
nest::SourceTable::get_thread_local_sources( const thread tid )
{
  return *sources_[ tid ];
}

nest::SourceTablePosition
nest::SourceTable::find_maximal_position() const
{
  // max_position is initialized with zeros, so its values are always
  // positive.
  SourceTablePosition max_position( 0, 0, 0 );
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    if ( max_position < ( *saved_positions_[ tid ] ) )
    {
      max_position = ( *saved_positions_[ tid ]);
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
    for ( synindex syn_index = max_position.syn_index; syn_index < ( *sources_[ tid ] ).size(); ++syn_index )
    {
      std::vector< Source >& sources = ( *sources_[ tid ] )[ syn_index ];
      if ( max_position.syn_index == syn_index )
      {
        // we need to add 1 to max_position.lcid since
        // max_position.lcid can contain a valid entry which we do not
        // want to delete.
        if ( max_position.lcid + 1 < static_cast< long >( sources.size() ) )
        {
          sources.erase( sources.begin() + max_position.lcid + 1, sources.end() );
        }
      }
      else
      {
        sources.erase( sources.begin(), sources.end() );
      }
    }
  }
  else if ( max_position.tid < tid )
  {
    for ( synindex syn_index = 0; syn_index < ( *sources_[ tid ] ).size(); ++syn_index )
    {
      std::vector< Source >& sources = ( *sources_[ tid ] )[ syn_index ];
      sources.erase( sources.begin(), sources.end() );
    }
  }
}

void
nest::SourceTable::reserve( const thread tid,
  const synindex syn_id,
  const size_t count )
{
  std::map< synindex, synindex >::iterator it = synapse_ids_[ tid ]->find( syn_id );
  // if this synapse type is not known yet, create entry for new synapse vector
  if (it == synapse_ids_[ tid ]->end())
  {
    const index prev_n_synapse_types = synapse_ids_[ tid ]->size();
    (*synapse_ids_[ tid ])[ syn_id ] = prev_n_synapse_types;
    sources_[ tid ]->resize( prev_n_synapse_types + 1);
    (*sources_[ tid ])[ prev_n_synapse_types ].reserve( count );
  }
  // otherwise we can directly reserve
  else
  {
    (*sources_[ tid ])[ it->second ].reserve( count );
  }

}
