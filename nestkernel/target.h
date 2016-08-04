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

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

/**
 * A structure containing all information required to uniquely
 * identify a target neuron on a (remote) machine. Used in TargetTable
 * for presynaptic part of connection infrastructure.
 */
struct Target
{
  unsigned int lcid : 27; //!< local connection index
  unsigned int  rank : 20; //!< rank of target neuron
  short tid : 10; //!< thread index
  char syn_index : 6; //!< synapse-type index
  bool processed : 1;
  Target();
  Target( const Target& target );
  Target( const thread tid, const thread rank, const synindex syn_index, const index lcid);
  void set_processed();
  bool is_processed() const;
  double_t get_offset() const;
};

inline
Target::Target()
  : lcid( 0 )
  , rank( 0 )
  , tid( 0 )
  , syn_index( 0 )
  , processed( false )
{
}

inline
Target::Target( const Target& target )
  : lcid( target.lcid )
  , rank( target.rank )
  , tid( target.tid )
  , syn_index( target.syn_index )
  , processed( false ) // always initialize as non-processed
{
}

inline
Target::Target( const thread tid, const thread rank, const synindex syn_index, const index lcid)
  : lcid( lcid )
  , rank( rank )
  , tid( tid )
  , syn_index( syn_index )
  , processed( false ) // always initialize as non-processed
{
  assert( lcid < 134217728 );
  assert( rank < 1048576 );
  assert( tid < 1024 );
  assert( syn_index < 64 );
}

inline void
Target::set_processed()
{
  processed = true;
}

inline bool
Target::is_processed() const
{
  return processed;
}

inline double_t
Target::get_offset() const
{
  return 0;
}

struct OffGridTarget : Target
{
  double_t offset;
  OffGridTarget();
  OffGridTarget( const Target& target, const double_t offset );
  double_t get_offset() const;
};

inline
OffGridTarget::OffGridTarget()
  : Target()
{
}

inline
OffGridTarget::OffGridTarget(const Target& target, const double_t offset )
  : Target( target )
  , offset( offset )
{
}

inline double_t
OffGridTarget::get_offset() const
{
  return offset;
}

} // namespace nest

#endif // TARGET_H
