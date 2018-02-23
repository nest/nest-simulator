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

namespace nest
{

/**
 * Contains all information required to uniquely identify a target
 * neuron on a (remote) machine. Used in TargetTable for presynaptic
 * part of connection infrastructure.
 */
class Target // TODO@5g: write tests for reading and writing fields -> Jakob
{
private:
  unsigned long data_;

  // define masks to select correct bits in data_
  static const unsigned long lcid_mask = 0x0000000007FFFFFF;
  static const unsigned long rank_mask = 0x00007FFFF8000000;
  static const unsigned long tid_mask = 0x01FF800000000000;
  static const unsigned long syn_id_mask = 0x7E00000000000000;
  static const unsigned long processed_mask = 0x8000000000000000;

  // define shifts to arrive at correct bits; note: the size of these
  // variables is most likely not enough for exascale computers, or
  // very small number of threads; if any issues are encountered with
  // these values, we can introduce compilerflags that rearrange these
  // sizes according to the target platform/application
  static const size_t lcid_shift = 0;
  static const size_t rank_shift = 27;
  static const size_t tid_shift = 47;
  static const size_t syn_id_shift = 57;
  static const size_t processed_shift = 63;

  // maximal sizes are determined by bitshifts
  static const int max_lcid_ = 134217728; // 2 ** 27
  static const int max_rank_ = 1048576; // 2 ** 20
  static const int max_tid_ = 1024; // 2 ** 10
  static const int max_syn_id_ = 64; // 2 ** 6

public:
  Target();
  Target( const Target& target );
  Target( const thread tid,
    const thread rank,
    const synindex syn_id,
    const index lcid );
  void set_lcid( const size_t lcid );
  size_t get_lcid() const;
  void set_rank( const unsigned int rank );
  unsigned int get_rank() const;
  void set_tid( const unsigned int tid );
  unsigned int get_tid() const;
  void set_syn_id( const unsigned char syn_id );
  unsigned char get_syn_id() const;
  void set_is_processed( const bool processed );
  bool is_processed() const;
  double get_offset() const;
};

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
  assert( tid < max_tid_ );
  assert( rank < max_rank_ );
  assert( syn_id < max_syn_id_ );
  assert( lcid < max_lcid_ );
  set_lcid( lcid );
  set_rank( rank );
  set_tid( tid );
  set_syn_id( syn_id );
  set_is_processed( false ); // always initialize as non-processed
}

inline void
Target::set_lcid( const size_t lcid )
{
  assert( lcid < max_lcid_ );
  // reset corresponding bits using complement of mask and write new
  // bits by shifting input appropiately. need to cast to long first,
  // to avoid overflow of input by left shifts.
  data_ = ( data_ & ( ~lcid_mask ) )
    | ( static_cast< unsigned long >( lcid ) << lcid_shift );
}

inline size_t
Target::get_lcid() const
{
  return ( data_ & lcid_mask ) >> lcid_shift;
}

inline void
Target::set_rank( const unsigned int rank )
{
  assert( rank < max_rank_ );
  data_ = ( data_ & ( ~rank_mask ) )
    | ( static_cast< unsigned long >( rank ) << rank_shift );
}

inline unsigned int
Target::get_rank() const
{
  return ( data_ & rank_mask ) >> rank_shift;
}

inline void
Target::set_tid( const unsigned int tid )
{
  assert( tid < max_tid_ );
  data_ = ( data_ & ( ~tid_mask ) )
    | ( static_cast< unsigned long >( tid ) << tid_shift );
}

inline unsigned int
Target::get_tid() const
{
  return ( data_ & tid_mask ) >> tid_shift;
}

inline void
Target::set_syn_id( const unsigned char syn_id )
{
  assert( syn_id < max_syn_id_ );
  data_ = ( data_ & ( ~syn_id_mask ) )
    | ( static_cast< unsigned long >( syn_id ) << syn_id_shift );
}

inline unsigned char
Target::get_syn_id() const
{
  return ( data_ & syn_id_mask ) >> syn_id_shift;
}

inline void
Target::set_is_processed( const bool processed )
{
  data_ = ( data_ & ( ~processed_mask ) )
    | ( static_cast< unsigned long >( processed ) << processed_shift );
}

inline bool
Target::is_processed() const
{
  return ( data_ & processed_mask ) >> processed_shift;
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
