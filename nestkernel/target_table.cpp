/*
 *  target_table.cpp
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

#include "target_table.h"

// Includes from nestkernel:
#include "kernel_manager.h"

const nest::index nest::TargetData::empty_marker = std::numeric_limits< index >::max();
const nest::index nest::TargetData::complete_marker = std::numeric_limits< index >::max() - 1;

nest::TargetTable::TargetTable()
{
}

nest::TargetTable::~TargetTable()
{
}

void
nest::TargetTable::initialize()
{
  thread num_threads = kernel().vp_manager.get_num_threads();
  targets_.resize( num_threads );
  for( thread tid = 0; tid < num_threads; ++tid)
  {
    targets_[ tid ] = new std::vector< std::vector< Target > >(
      0, std::vector< Target >( 0, Target() ) );
  }
}

void
nest::TargetTable::finalize()
{
  for( std::vector< std::vector< std::vector< Target > >* >::iterator it =
         targets_.begin(); it != targets_.end(); ++it )
  {
    delete *it;
  }
  targets_.clear();
}

void
nest::TargetTable::prepare( const thread tid )
{
  targets_[ tid ]->resize( kernel().node_manager.get_max_num_local_nodes() );
}

// TODO@5g: benchmark with and without reserving memory for synapses
// TODO@5g: if we use reserve, we need to make sure the synapse type is known
// void
// nest::TargetTable::reserve( thread tid, synindex syn_id, index n_targets )
// {
// }

bool
nest::TargetTable::get_next_spike_data( const thread tid, const thread current_tid, const index lid, index& rank, SpikeData& next_spike_data, const unsigned int rank_start, const unsigned int rank_end )
{
  while ( true )
  {
    if ( current_target_index_[ tid ] == (*targets_[ current_tid ])[ lid ].size() )
    {
      current_target_index_[ tid ] = 0;
      return false;
    }
    else
    {
      Target& current_target = (*targets_[ current_tid ])[ lid ][ current_target_index_[ tid ] ];
      if ( rank_start <= current_target.rank && current_target.rank < rank_end )
      {
        if ( current_target.processed )
        {
          ++current_target_index_[ tid ];
          continue;
        }
        else
        {
          current_target.processed = true;
          rank = current_target.rank;
          next_spike_data.tid = current_target.tid;
          next_spike_data.syn_index = current_target.syn_index;
          next_spike_data.lcid = current_target.lcid;
          ++current_target_index_[ tid ];
          return true;
        }
      }
      else
      {
        ++current_target_index_[ tid ];
        continue;
      }
    }
  }
}

void
nest::TargetTable::reject_last_spike_data( const thread tid, const thread current_tid, const index lid )
{
  assert( current_target_index_[ tid ] > 0 );
  --current_target_index_[ tid ];
  ( *targets_[ current_tid ])[ lid ][ current_target_index_[ tid ] ].processed = false;
}
