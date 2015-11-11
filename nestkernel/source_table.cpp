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
#include "target_table.h"
#include "kernel_manager.h"

nest::SourceTable::SourceTable()
{
}

nest::SourceTable::~SourceTable()
{
}

void
nest::SourceTable::initialize()
{
  thread num_threads = kernel().vp_manager.get_num_threads();
  synapse_ids_.resize( num_threads );
  sources_.resize( num_threads );
  is_cleared_.resize( num_threads );
  saved_entry_point_.resize( num_threads );
  current_tid_.resize( num_threads );
  current_syn_id_.resize( num_threads );
  current_lcid_.resize( num_threads );
  save_tid_.resize( num_threads );
  save_syn_id_.resize( num_threads );
  save_lcid_.resize( num_threads );

  for( thread tid = 0; tid < num_threads; ++tid)
  {
    synapse_ids_[ tid ] = new std::map< synindex, synindex >();
    sources_[ tid ] = new std::vector< std::vector< Source > >(
      0, std::vector< Source >( 0, Source() ) );
    is_cleared_[ tid ] = false;
    saved_entry_point_[ tid ] = false;
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
  for ( thread tid = 0; tid < kernel().vp_manager.get_num_threads(); ++tid )
  {
    all_cleared &= is_cleared_[ tid ];
  }
  return all_cleared;
}

void
nest::SourceTable::get_next_target_data( const thread tid, TargetData& next_target_data, const unsigned int rank_start, const unsigned int rank_end )
{
  while ( true )
  {
    if ( current_tid_[ tid ] == sources_.size() )
    {
      next_target_data.gid = invalid_index;
      return;
    }
    else
    {
      if ( current_syn_id_[ tid ] == sources_[ current_tid_[ tid ] ]->size() )
      {
        current_syn_id_[ tid ] = 0;
        ++current_tid_[ tid ];
        continue;
      }
      else
      {
        if ( current_lcid_[ tid ] == (*sources_[ current_tid_[ tid ] ])[ current_syn_id_[ tid ] ].size() )
        {
          current_lcid_[ tid ] = 0;
          ++current_syn_id_[ tid ];
          continue;
        }
        else
        {
          Source& current_source = ( *sources_[ current_tid_[ tid ] ] )[ current_syn_id_[ tid ] ][ current_lcid_[ tid ] ];
          const thread target_rank = kernel().mpi_manager.get_process_id_of_gid( current_source.gid );
          if ( rank_start <= target_rank && target_rank < rank_end )
          {
            if ( current_source.processed )
            {
              ++current_lcid_[ tid ];
              continue;
            }
            current_source.processed = true;
            next_target_data.gid = current_source.gid;
            next_target_data.target.tid = current_tid_[ tid ];
            next_target_data.target.rank = kernel().mpi_manager.get_rank();
            next_target_data.target.processed = false;
            next_target_data.target.syn_index = current_syn_id_[ tid ];
            next_target_data.target.lcid = current_lcid_[ tid ];
            ++current_lcid_[ tid ];
            return;
          }
          else
          {
            ++current_lcid_[ tid ];
            continue;
          }
        }
      }
    }
  }
}

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

void
nest::SourceTable::restore_entry_point( const thread tid )
{
  current_tid_[ tid ] = save_tid_[ tid ];
  current_syn_id_[ tid ] = save_syn_id_[ tid ];
  current_lcid_[ tid ] = save_lcid_[ tid ];
  saved_entry_point_[ tid ] = false;
}

void
nest::SourceTable::reset_entry_point( const thread tid )
{
  save_tid_[ tid ] = 0;
  save_syn_id_[ tid ] = 0;
  save_lcid_[ tid ] = 0;
}
