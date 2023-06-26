/*
 *  send_buffer_position.h
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

#ifndef SEND_BUFFER_POSITION_H
#define SEND_BUFFER_POSITION_H

// C++ includes:
#include <cassert>
#include <limits>
#include <vector>

// Includes from nestkernel:
#include "vp_manager.h"

namespace nest
{

/**
 * This class simplifies keeping track of write position in MPI buffer
 * while collocating spikes.
 */
class SendBufferPosition
{
private:
  size_t begin_rank_;
  size_t end_rank_;
  size_t max_size_;
  size_t num_spike_data_written_;
  size_t send_recv_count_per_rank_;
  std::vector< size_t > idx_;
  std::vector< size_t > begin_;
  std::vector< size_t > end_;

  size_t rank_to_index_( const size_t rank ) const;

public:
  SendBufferPosition( const AssignedRanks& assigned_ranks, const unsigned int send_recv_count_per_rank );

  /**
   * Returns current index of specified rank in MPI buffer.
   */
  unsigned int idx( const size_t rank ) const;

  /**
   * Returns begin index of specified rank in MPI buffer.
   */
  unsigned int begin( const size_t rank ) const;

  /**
   * Returns end index of specified rank in MPI buffer.
   */
  unsigned int end( const size_t rank ) const;

  /**
   * Returns whether the part of the buffer on the specified rank has been
   * filled.
   *
   * @param rank Rank denoting which part of the buffer we check
   */
  bool is_chunk_filled( const size_t rank ) const;

  /**
   * Returns whether the parts of the  MPI buffer assigned to this thread has
   * been filled.
   */
  bool are_all_chunks_filled() const;

  void increase( const size_t rank );
};

inline SendBufferPosition::SendBufferPosition( const AssignedRanks& assigned_ranks,
  const unsigned int send_recv_count_per_rank )
  : begin_rank_( assigned_ranks.begin )
  , end_rank_( assigned_ranks.end )
  , max_size_( assigned_ranks.max_size )
  , num_spike_data_written_( 0 )
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

inline size_t
SendBufferPosition::rank_to_index_( const size_t rank ) const
{
  assert( begin_rank_ <= rank );
  assert( rank < end_rank_ );
  return rank % max_size_;
}

inline unsigned int
SendBufferPosition::idx( const size_t rank ) const
{
  return idx_[ rank_to_index_( rank ) ];
}

inline unsigned int
SendBufferPosition::begin( const size_t rank ) const
{
  return begin_[ rank_to_index_( rank ) ];
}

inline unsigned int
SendBufferPosition::end( const size_t rank ) const
{
  return end_[ rank_to_index_( rank ) ];
}

inline bool
SendBufferPosition::is_chunk_filled( const size_t rank ) const
{
  return idx( rank ) == end( rank );
}

inline bool
SendBufferPosition::are_all_chunks_filled() const
{
  return num_spike_data_written_ == send_recv_count_per_rank_ * idx_.size();
}

inline void
SendBufferPosition::increase( const size_t rank )
{
  ++idx_[ rank_to_index_( rank ) ];
  ++num_spike_data_written_;
}

} // namespace nest

#endif /* SEND_BUFFER_POSITION_H */
