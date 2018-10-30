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
#include "nest_types.h"
#include "static_assert.h"

namespace nest
{

/**
 * Contains all information required to uniquely identify a target
 * neuron on a (remote) machine. Used in TargetTable for presynaptic
 * part of connection infrastructure.
 */
class Target
{
private:
  uint64_t data_;

  // define masks to select correct bits in data_
  static constexpr uint64_t MASK_LCID = 0x0000000007FFFFFF;
  static constexpr uint64_t MASK_RANK = 0x00007FFFF8000000;
  static constexpr uint64_t MASK_TID = 0x01FF800000000000;
  static constexpr uint64_t MASK_SYN_ID = 0x7E00000000000000;
  static constexpr uint64_t MASK_PROCESSED = 0x8000000000000000;

  // define shifts to arrive at correct bits; note: the size of these
  // variables is most likely not enough for exascale computers, or
  // very small number of threads; if any issues are encountered with
  // these values, we can introduce compiler flags that rearrange these
  // sizes according to the target platform/application
  static constexpr uint8_t SHIFT_LCID = 0;
  static constexpr uint8_t SHIFT_RANK = 27;
  static constexpr uint8_t SHIFT_TID = 47;
  static constexpr uint8_t SHIFT_SYN_ID = 57;
  static constexpr uint8_t SHIFT_PROCESSED = 63;

  // maximal sizes are determined by bitshifts
  static constexpr int MAX_LCID = 134217728; // 2 ** 27
  static constexpr int MAX_RANK = 1048576;   // 2 ** 20
  static constexpr int MAX_TID = 1024;       // 2 ** 10
  static constexpr int MAX_SYN_ID = 64;      // 2 ** 6

public:
  Target();
  Target( const Target& target );
  Target( const thread tid,
    const thread rank,
    const synindex syn_id,
    const index lcid );

  /**
   * Sets the local connection ID.
   */
  void set_lcid( const index lcid );

  /**
   * Returns the local connection ID.
   */
  index get_lcid() const;

  /**
   * Sets the rank.
   */
  void set_rank( const thread rank );

  /**
   * Returns the rank.
   */
  thread get_rank() const;

  /**
   * Sets the target ID.
   */
  void set_tid( const thread tid );

  /**
   * Returns the target ID.
   */
  thread get_tid() const;

  /**
   * Sets the synapse-type ID.
   */
  void set_syn_id( const synindex syn_id );

  /**
   * Returns the synapse-type ID.
   */
  synindex get_syn_id() const;

  /**
   * Sets whether Target is processed.
   */
  void set_is_processed( const bool processed );

  /**
   * Returns whether Target is processed.
   */
  bool is_processed() const;

  /**
   * Returns offset.
   */
  double get_offset() const;
};

//!< check legal size
typedef StaticAssert< sizeof( Target ) == 8 >::success success_target_size;

inline Target::Target()
  : data_( 0 )
{
}

inline Target::Target( const Target& target )
  : data_( target.data_ )
{
  set_is_processed( false ); // always initialize as non-processed
}

inline Target::Target( const thread tid,
  const thread rank,
  const synindex syn_id,
  const index lcid )
  : data_( 0 )
{
  assert( tid < MAX_TID );
  assert( rank < MAX_RANK );
  assert( syn_id < MAX_SYN_ID );
  assert( lcid < MAX_LCID );
  set_lcid( lcid );
  set_rank( rank );
  set_tid( tid );
  set_syn_id( syn_id );
  set_is_processed( false ); // always initialize as non-processed
}

inline void
Target::set_lcid( const index lcid )
{
  assert( lcid < MAX_LCID );
  // Reset corresponding bits using complement of mask and write new
  // bits by shifting input appropriately. Need to cast to long first,
  // to avoid overflow of input by left shifts.
  data_ = ( data_ & ( ~MASK_LCID ) )
    | ( static_cast< uint64_t >( lcid ) << SHIFT_LCID );
}

inline index
Target::get_lcid() const
{
  return ( data_ & MASK_LCID ) >> SHIFT_LCID;
}

inline void
Target::set_rank( const thread rank )
{
  assert( rank < MAX_RANK );
  data_ = ( data_ & ( ~MASK_RANK ) )
    | ( static_cast< uint64_t >( rank ) << SHIFT_RANK );
}

inline thread
Target::get_rank() const
{
  return ( data_ & MASK_RANK ) >> SHIFT_RANK;
}

inline void
Target::set_tid( const thread tid )
{
  assert( tid < MAX_TID );
  data_ =
    ( data_ & ( ~MASK_TID ) ) | ( static_cast< uint64_t >( tid ) << SHIFT_TID );
}

inline thread
Target::get_tid() const
{
  return ( data_ & MASK_TID ) >> SHIFT_TID;
}

inline void
Target::set_syn_id( const synindex syn_id )
{
  assert( syn_id < MAX_SYN_ID );
  data_ = ( data_ & ( ~MASK_SYN_ID ) )
    | ( static_cast< uint64_t >( syn_id ) << SHIFT_SYN_ID );
}

inline synindex
Target::get_syn_id() const
{
  return ( data_ & MASK_SYN_ID ) >> SHIFT_SYN_ID;
}

inline void
Target::set_is_processed( const bool processed )
{
  data_ = ( data_ & ( ~MASK_PROCESSED ) )
    | ( static_cast< uint64_t >( processed ) << SHIFT_PROCESSED );
}

inline bool
Target::is_processed() const
{
  return ( data_ & MASK_PROCESSED ) >> SHIFT_PROCESSED;
}

inline double
Target::get_offset() const
{
  return 0;
}

class OffGridTarget : public Target
{
private:
  double offset_;

public:
  OffGridTarget();
  OffGridTarget( const Target& target, const double offset );
  double get_offset() const;
};

inline OffGridTarget::OffGridTarget()
  : Target()
  , offset_( 0 )
{
}

inline OffGridTarget::OffGridTarget( const Target& target, const double offset )
  : Target( target )
  , offset_( offset )
{
}

inline double
OffGridTarget::get_offset() const
{
  return offset_;
}

} // namespace nest

#endif // TARGET_H
