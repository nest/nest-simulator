/*
 *  target_table.h
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
 * A structure containing all information required to uniquely
 * identify a target neuron on a (remote) machine. Used in TargetTable
 * for presynaptic part of connection infrastructure.
 */
class Target
{
private:
  unsigned long data_;
  // define masks to select correct bits in data_
  static const unsigned long lcid_mask = 0x0000000007FFFFFF;
  static const unsigned long rank_mask = 0x00007FFFF8000000;
  static const unsigned long tid_mask = 0x01FF800000000000;
  static const unsigned long syn_index_mask = 0x7E00000000000000;
  static const unsigned long processed_mask = 0x8000000000000000;
  // define shifts to arrive at correct bits
  static const size_t lcid_shift = 0;
  static const size_t rank_shift = 27;
  static const size_t tid_shift = 47;
  static const size_t syn_index_shift = 57;
  static const size_t processed_shift = 63;

public:
  Target();
  Target( const Target& target );
  Target( const thread tid,
    const thread rank,
    const synindex syn_index,
    const index lcid );
  void set_lcid( const size_t lcid );
  size_t get_lcid() const;
  void set_rank( const unsigned int rank );
  unsigned int get_rank() const;
  void set_tid( const unsigned int tid );
  unsigned int get_tid() const;
  void set_syn_index( const unsigned char syn_index );
  unsigned char get_syn_index() const;
  void set_processed( const bool processed );
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
  set_processed( false ); // always initialize as non-processed
}

inline Target::Target( const thread tid,
  const thread rank,
  const synindex syn_index,
  const index lcid )
  : data_( 0 )
{
  set_lcid( lcid );
  set_rank( rank );
  set_tid( tid );
  set_syn_index( syn_index );
  set_processed( false ); // always initialize as non-processed
}

inline void
Target::set_lcid( const size_t lcid )
{
  assert( lcid < 134217728 );
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
  assert( rank < 1048576 );
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
  assert( tid < 1024 );
  data_ = ( data_ & ( ~tid_mask ) )
    | ( static_cast< unsigned long >( tid ) << tid_shift );
}

inline unsigned int
Target::get_tid() const
{
  return ( data_ & tid_mask ) >> tid_shift;
}

inline void
Target::set_syn_index( const unsigned char syn_index )
{
  assert( syn_index < 64 );
  data_ = ( data_ & ( ~syn_index_mask ) )
    | ( static_cast< unsigned long >( syn_index ) << syn_index_shift );
}

inline unsigned char
Target::get_syn_index() const
{
  return ( data_ & syn_index_mask ) >> syn_index_shift;
}

inline void
Target::set_processed( const bool processed )
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
  double offset;

public:
  OffGridTarget();
  OffGridTarget( const Target& target, const double offset );
  double get_offset() const;
};

inline OffGridTarget::OffGridTarget()
  : Target()
  , offset( 0 )
{
}

inline OffGridTarget::OffGridTarget( const Target& target, const double offset )
  : Target( target )
  , offset( offset )
{
}

inline double
OffGridTarget::get_offset() const
{
  return offset;
}

} // namespace nest

#endif // TARGET_H
