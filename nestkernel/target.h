/*
 *  target.h
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

#ifndef TARGET_H
#define TARGET_H

// C++ includes:
#include <cassert>

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_types.h"
#include "static_assert.h"

namespace nest
{

/**
 * This class implements a 64-bit target neuron identifier type.
 *
 * It uniquely identifies a target neuron on a (remote) machine.
 * Used in TargetTable for the presynaptic part
 * of the connection infrastructure.
 *
 * The bitwise layout of the neuron identifier for the "standard" CMAKE option:
 *
 *  +-------- processed flag
 *  |   +---- synapse-type id (syn_id)
 *  |   |
 *  ||----------||--thread--||---------rank----------||----local connection id (lcid)----|
 *  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000
 *  |       |  |       |  |       |  |       |  |       |  |       |  |       |  |       |
 *  63      56 55      48 47      40 39      32 31      24 23      16 15      8  7       0
 *
 * The bitwise layout of the neuron identifier for the "hpc" CMAKE option:
 *
 *  +-------- processed flag
 *  |   +---- synapse-type id (syn_id)
 *  |   |
 *  ||-----||---thread----||---------rank------------||----local connection id (lcid)----|
 *  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000  0000 0000
 *  |       |  |       |  |       |  |       |  |       |  |       |  |       |  |       |
 *  63      56 55      48 47      40 39      32 31      24 23      16 15      8  7       0
 *
 * Other custom layouts can be chosen by providing a list of 5
 * numbers, representing the bits required for rank, thread, synapse
 * id, local connection id and processed flag, respectively. The number
 * of bits needs to sum to 64. The processed flag must always use one
 * bit.
 */

enum enum_status_target_id
{
  TARGET_ID_PROCESSED,
  TARGET_ID_UNPROCESSED
};

class Target
{
private:
  uint64_t remote_target_id_;

  static constexpr uint8_t BITPOS_LCID = 0U;
  static constexpr uint8_t BITPOS_RANK = NUM_BITS_LCID;
  static constexpr uint8_t BITPOS_TID = BITPOS_RANK + NUM_BITS_RANK;
  static constexpr uint8_t BITPOS_SYN_ID = BITPOS_TID + NUM_BITS_TID;
  static constexpr uint8_t BITPOS_PROCESSED_FLAG = BITPOS_SYN_ID + NUM_BITS_SYN_ID;

  using bits_for_processed_flag = StaticAssert< NUM_BITS_PROCESSED_FLAG == 1U >::success;
  using position_of_processed_flag = StaticAssert< BITPOS_PROCESSED_FLAG == 63U >::success;

  // generate bit-masks used in bit-operations
  static constexpr uint64_t MASK_LCID = generate_bit_mask( NUM_BITS_LCID, BITPOS_LCID );
  static constexpr uint64_t MASK_RANK = generate_bit_mask( NUM_BITS_RANK, BITPOS_RANK );
  static constexpr uint64_t MASK_TID = generate_bit_mask( NUM_BITS_TID, BITPOS_TID );
  static constexpr uint64_t MASK_SYN_ID = generate_bit_mask( NUM_BITS_SYN_ID, BITPOS_SYN_ID );
  static constexpr uint64_t MASK_PROCESSED_FLAG = generate_bit_mask( NUM_BITS_PROCESSED_FLAG, BITPOS_PROCESSED_FLAG );

public:
  Target();
  Target( const Target& target );
  Target( const size_t tid, const size_t rank, const synindex syn_id, const size_t lcid );

  Target& operator=( const Target& );

  /**
   * Set local connection id.
   */
  void set_lcid( const size_t lcid );

  /**
   * Return local connection id.
   */
  size_t get_lcid() const;

  /**
   * Set rank.
   */
  void set_rank( const size_t rank );

  /**
   * Return rank.
   */
  size_t get_rank() const;

  /**
   * Set thread id.
   */
  void set_tid( const size_t tid );

  /**
   * Return thread id.
   */
  size_t get_tid() const;

  /**
   * Set the synapse-type id.
   */
  void set_syn_id( const synindex syn_id );

  /**
   * Return synapse-type id.
   */
  synindex get_syn_id() const;

  /**
   * Set the status of the target identifier: processed or unprocessed.
   */
  void set_status( enum_status_target_id status );

  /**
   * Get the status of the target identifier: processed or unprocessed.
   */
  enum_status_target_id get_status() const;

  /**
   * Return the status od the target identifier: processed or unprocessed.
   */
  bool is_processed() const;

  /**
   * Return offset.
   */
  double get_offset() const;

  /**
   *  Set the status of the target identifier to processed
   */
  void mark_for_removal();
};

//!< check legal size
using success_target_size = StaticAssert< sizeof( Target ) == 8 >::success;

class OffGridTarget : public Target
{
private:
  double offset_;

public:
  OffGridTarget();
  OffGridTarget( const Target& target, const double offset );
  double get_offset() const;
};

} // namespace nest

#endif /* #ifndef TARGET_H */
