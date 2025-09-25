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
  std::vector< size_t > begin_; //!< first entry for rank
  std::vector< size_t > end_;   //!< one beyond last entry for rank
  std::vector< size_t > idx_;   //!< next entry in rank to write to

public:
  SendBufferPosition();

  /**
   * Returns begin index of specified rank in MPI buffer.
   */
  size_t begin( const size_t rank ) const;

  /**
   * Returns end index of specified rank in MPI buffer.
   */
  size_t end( const size_t rank ) const;

  /**
   * Returns current index of specified rank in MPI buffer.
   */
  size_t idx( const size_t rank ) const;

  /**
   * Returns whether the part of the buffer on the specified rank has been filled.
   *
   * @param rank Rank denoting which part of the buffer we check
   */
  bool is_chunk_filled( const size_t rank ) const;

  void increase( const size_t rank );
};


/**
 * This class simplifies keeping track of write position in MPI buffer
 * while collocating targets.
 */
class TargetSendBufferPosition
{
private:
  size_t begin_rank_;
  size_t end_rank_;
  size_t max_size_;
  size_t num_target_data_written_;
  size_t send_recv_count_per_rank_;
  std::vector< size_t > idx_;
  std::vector< size_t > begin_;
  std::vector< size_t > end_;

  size_t rank_to_index_( const size_t rank ) const;

public:
  TargetSendBufferPosition( const AssignedRanks& assigned_ranks, const unsigned int send_recv_count_per_rank );

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

} // namespace nest

#endif /* SEND_BUFFER_POSITION_H */
