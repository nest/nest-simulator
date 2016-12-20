/*
 *  source.h
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

#ifndef SOURCE_H
#define SOURCE_H

// C++ include
#include <cassert>

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

/**
 * A data structure that stores the global id of a presynaptic neuron
 * and the number of local targets, along with a flag, whether this
 * entry has been processed yet. Used in SourceTable.
 */
struct Source
{
  unsigned long gid : 62; //!< gid of source
  bool processed : 1; //!< whether this target has already been moved to the MPI
                      //buffer
  bool is_primary : 1;
  Source();
  explicit Source( const index gid, const bool is_primary );
  void disable();
  bool is_disabled() const;
  static const size_t disabled_marker = 4611686018427387904 - 1; // 2 ** 62 - 1
};

inline Source::Source()
  : gid( 0 )
  , processed( false )
  , is_primary( true )
{
}

inline Source::Source( const index gid, const bool is_primary )
  : gid( gid )
  , processed( false )
  , is_primary( is_primary )
{
  assert( gid < disabled_marker );
}

inline void
Source::disable()
{
  gid = disabled_marker;
}

inline bool
Source::is_disabled() const
{
  return gid == disabled_marker;
}

inline bool operator<( const Source& lhs, const Source& rhs )
{
  return ( lhs.gid < rhs.gid );
}

inline bool operator>( const Source& lhs, const Source& rhs )
{
  return operator<( rhs, lhs );
}

inline bool operator==( const Source& lhs, const Source& rhs )
{
  return ( lhs.gid == rhs.gid );
}

} // namespace nest

#endif // SOURCE_H
