/*
 *  send_buffer_position.cpp
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
#include "kernel_manager.h"

#include "send_buffer_position.h"

nest::SendBufferPosition::SendBufferPosition()
  : begin_( kernel::manager< MPIManager >.get_num_processes(), 0 )
  , end_( kernel::manager< MPIManager >.get_num_processes(), 0 )
  , idx_( kernel::manager< MPIManager >.get_num_processes(), 0 )
{
  const size_t num_procs = kernel::manager< MPIManager >.get_num_processes();
  const size_t send_recv_count_per_rank = kernel::manager< MPIManager >.get_send_recv_count_spike_data_per_rank();

  for ( size_t rank = 0; rank < num_procs; ++rank )
  {
    begin_[ rank ] = rank * send_recv_count_per_rank;
    end_[ rank ] = ( rank + 1 ) * send_recv_count_per_rank;
    idx_[ rank ] = begin_[ rank ];
  }
}

void
nest::TargetSendBufferPosition::increase( const size_t rank )
{

  ++idx_[ rank_to_index_( rank ) ];
  ++num_target_data_written_;
}

bool
nest::TargetSendBufferPosition::are_all_chunks_filled() const
{

  return num_target_data_written_ == send_recv_count_per_rank_ * idx_.size();
}

bool
nest::TargetSendBufferPosition::is_chunk_filled( const size_t rank ) const
{

  return idx( rank ) == end( rank );
}

unsigned int
nest::TargetSendBufferPosition::end( const size_t rank ) const
{

  return end_[ rank_to_index_( rank ) ];
}

unsigned int
nest::TargetSendBufferPosition::begin( const size_t rank ) const
{

  return begin_[ rank_to_index_( rank ) ];
}

unsigned int
nest::TargetSendBufferPosition::idx( const size_t rank ) const
{

  return idx_[ rank_to_index_( rank ) ];
}

size_t
nest::TargetSendBufferPosition::rank_to_index_( const size_t rank ) const
{

  assert( begin_rank_ <= rank );
  assert( rank < end_rank_ );
  return rank % max_size_;
}

nest::TargetSendBufferPosition::TargetSendBufferPosition( const AssignedRanks& assigned_ranks,
  const unsigned int send_recv_count_per_rank )
  : begin_rank_( assigned_ranks.begin )
  , end_rank_( assigned_ranks.end )
  , max_size_( assigned_ranks.max_size )
  , num_target_data_written_( 0 )
  , send_recv_count_per_rank_( send_recv_count_per_rank )
{

  idx_.resize( assigned_ranks.size );
  begin_.resize( assigned_ranks.size );
  end_.resize( assigned_ranks.size );
  for ( size_t rank = assigned_ranks.begin; rank < assigned_ranks.end; ++rank )
  {
    // thread-local index of (global) rank
    const size_t lr_idx = rank % assigned_ranks.max_size;
    assert( lr_idx < assigned_ranks.size );
    idx_[ lr_idx ] = rank * send_recv_count_per_rank;
    begin_[ lr_idx ] = rank * send_recv_count_per_rank;
    end_[ lr_idx ] = ( rank + 1 ) * send_recv_count_per_rank;
  }
}

void
nest::SendBufferPosition::increase( const size_t rank )
{

  ++idx_[ rank ];
}

bool
nest::SendBufferPosition::is_chunk_filled( const size_t rank ) const
{

  return idx_[ rank ] == end_[ rank ];
}

size_t
nest::SendBufferPosition::end( const size_t rank ) const
{

  return end_[ rank ];
}

size_t
nest::SendBufferPosition::begin( const size_t rank ) const
{

  return begin_[ rank ];
}

size_t
nest::SendBufferPosition::idx( const size_t rank ) const
{

  return idx_[ rank ];
}
