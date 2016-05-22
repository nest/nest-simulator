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
  const thread num_threads = kernel().vp_manager.get_num_threads();
  synapse_ids_.resize( num_threads );
  sources_.resize( num_threads );
  is_cleared_.resize( num_threads, false );
  saved_entry_point_.resize( num_threads, false );
  current_positions_.resize( num_threads );
  saved_positions_.resize( num_threads );
  current_first_source_.resize( num_threads, 0 );

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

// TODO@5g: benchmark with and without reserving memory for synapses
// TODO@5g: if we use reserve, we need to make sure the synapse type is known
// void
// nest::SourceTable::reserve( thread tid, synindex syn_id, index n_sources )
// {
//   std::map< synindex, synindex >::iterator it = synapse_ids_[ tid ]->find( syn_id );
//   synindex syn_index = it->second;
//   index prev_n_sources = (*sources_[ tid ])[ syn_index ].size();
//   (*sources_[ tid ])[ syn_index ].reserve( prev_n_sources + n_sources );
// }

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
