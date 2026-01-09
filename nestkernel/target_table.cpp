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

// Includes from nestkernel:
#include "target_table.h"
#include "kernel_manager.h"
#include "model_manager.h"
#include "mpi_manager.h"
#include "vp_manager.h"

// Includes from libnestutil
#include "vector_util.h"

#include <node_manager.h>

void
nest::TargetTable::initialize()
{
  const size_t num_threads = kernel::manager< VPManager >.get_num_threads();
  targets_.resize( num_threads );
  secondary_send_buffer_pos_.resize( num_threads );

#pragma omp parallel
  {
    const size_t tid = kernel::manager< VPManager >.get_thread_id();
    targets_[ tid ] = std::vector< std::vector< Target > >();
    secondary_send_buffer_pos_[ tid ] = std::vector< std::vector< std::vector< size_t > > >();
  } // of omp parallel
}

void
nest::TargetTable::finalize()
{
  std::vector< std::vector< std::vector< Target > > >().swap( targets_ );
  std::vector< std::vector< std::vector< std::vector< size_t > > > >().swap( secondary_send_buffer_pos_ );
}

void
nest::TargetTable::prepare( const size_t tid )
{
  // add one to max_num_local_nodes to avoid possible overflow in case
  // of rounding errors
  const size_t num_local_nodes = kernel::manager< NodeManager >.get_max_num_local_nodes() + 1;

  targets_[ tid ].resize( num_local_nodes );

  secondary_send_buffer_pos_[ tid ].resize( num_local_nodes );

  for ( size_t lid = 0; lid < num_local_nodes; ++lid )
  {
    // resize to maximal possible synapse-type index
    secondary_send_buffer_pos_[ tid ][ lid ].resize( kernel::manager< ModelManager >.get_num_connection_models() );
  }
}

void
nest::TargetTable::compress_secondary_send_buffer_pos( const size_t tid )
{
  for ( std::vector< std::vector< std::vector< size_t > > >::iterator it = secondary_send_buffer_pos_[ tid ].begin();
        it != secondary_send_buffer_pos_[ tid ].end();
        ++it )
  {
    for ( std::vector< std::vector< size_t > >::iterator iit = it->begin(); iit != it->end(); ++iit )
    {
      std::sort( iit->begin(), iit->end() );
      const std::vector< size_t >::iterator new_it = std::unique( iit->begin(), iit->end() );
      iit->resize( std::distance( iit->begin(), new_it ) );
    }
  }
}

void
nest::TargetTable::add_target( const size_t tid, const size_t target_rank, const TargetData& target_data )
{
  const size_t lid = target_data.get_source_lid();

  vector_util::grow( targets_[ tid ][ lid ] );

  if ( target_data.is_primary() )
  {
    const TargetDataFields& target_fields = target_data.target_data;

    targets_[ tid ][ lid ].push_back(
      Target( target_fields.get_tid(), target_rank, target_fields.get_syn_id(), target_fields.get_lcid() ) );
  }
  else
  {
    const SecondaryTargetDataFields& secondary_fields = target_data.secondary_data;
    const size_t send_buffer_pos = secondary_fields.get_recv_buffer_pos()
      + kernel::manager< MPIManager >.get_send_displacement_secondary_events_in_int( target_rank );
    const synindex syn_id = secondary_fields.get_syn_id();

    assert( syn_id < secondary_send_buffer_pos_[ tid ][ lid ].size() );
    secondary_send_buffer_pos_[ tid ][ lid ][ syn_id ].push_back( send_buffer_pos );
  }
}

const std::vector< nest::Target >&
nest::TargetTable::get_targets( const size_t tid, const size_t lid ) const
{
  return targets_[ tid ][ lid ];
}

const std::vector< size_t >&
nest::TargetTable::get_secondary_send_buffer_positions( const size_t tid,
  const size_t lid,
  const synindex syn_id ) const
{
  assert( syn_id < secondary_send_buffer_pos_[ tid ][ lid ].size() );
  return secondary_send_buffer_pos_[ tid ][ lid ][ syn_id ];
}

void
nest::TargetTable::clear( const size_t tid )
{
  targets_[ tid ].clear();
  secondary_send_buffer_pos_[ tid ].clear();
}
