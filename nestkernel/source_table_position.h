/*
 *  source_table_position.h
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

#ifndef SOURCE_TABLE_POSITION_H
#define SOURCE_TABLE_POSITION_H

// C++ includes:
#include <cassert>
#include <iostream>
#include <vector>

#include "block_vector.h"

namespace nest
{

/**
 * Three-tuple to store position in 3d vector of sources.
 */
struct SourceTablePosition
{
  long tid;    //!< thread index
  long syn_id; //!< synapse-type index
  long lcid;   //!< local connection index

  SourceTablePosition();
  SourceTablePosition( const long tid, const long syn_id, const long lcid );
  SourceTablePosition( const SourceTablePosition& rhs );

  void seek_to_next_valid_index( const std::vector< std::vector< BlockVector< Source > > >& sources );
  void decrease();
  bool is_invalid() const;
};

inline SourceTablePosition::SourceTablePosition()
  : tid( -1 )
  , syn_id( -1 )
  , lcid( -1 )
{
}

inline SourceTablePosition::SourceTablePosition( const long tid, const long syn_id, const long lcid )
  : tid( tid )
  , syn_id( syn_id )
  , lcid( lcid )
{
}

inline SourceTablePosition::SourceTablePosition( const SourceTablePosition& rhs )
  : tid( rhs.tid )
  , syn_id( rhs.syn_id )
  , lcid( rhs.lcid )
{
}

inline void
SourceTablePosition::seek_to_next_valid_index( const std::vector< std::vector< BlockVector< Source > > >& sources )
{
  if ( lcid >= 0 )
  {
    return; // nothing to do if we are at a valid index
  }

  // we stay in this loop either until we can return a valid position,
  // i.e., lcid >= 0, or we have reached the end of the
  // multidimensional vector
  while ( lcid < 0 )
  {
    // first try finding a valid lcid by only decreasing synapse index
    --syn_id;
    if ( syn_id >= 0 )
    {
      lcid = sources[ tid ][ syn_id ].size() - 1;
      continue;
    }

    // if we can not find a valid lcid by decreasing synapse indices,
    // try decreasing thread index
    --tid;
    if ( tid >= 0 )
    {
      syn_id = sources[ tid ].size() - 1;
      if ( syn_id >= 0 )
      {
        lcid = sources[ tid ][ syn_id ].size() - 1;
      }
      continue;
    }

    // if we can not find a valid lcid by decreasing synapse or thread
    // indices, we have read all entries
    assert( tid < 0 );
    assert( syn_id < 0 );
    assert( lcid < 0 );
    return; // reached the end
  }
}

inline bool
SourceTablePosition::is_invalid() const
{
  if ( tid < 0 and syn_id < 0 and lcid < 0 )
  {
    return true;
  }
  else
  {
    return false;
  }
}

inline void
SourceTablePosition::decrease()
{
  --lcid;
}

inline bool operator==( const SourceTablePosition& lhs, const SourceTablePosition& rhs )
{
  return ( ( lhs.tid == rhs.tid ) and ( lhs.syn_id == rhs.syn_id ) and ( lhs.lcid == rhs.lcid ) );
}

inline bool operator!=( const SourceTablePosition& lhs, const SourceTablePosition& rhs )
{
  return not operator==( lhs, rhs );
}

inline bool operator<( const SourceTablePosition& lhs, const SourceTablePosition& rhs )
{
  if ( lhs.tid == rhs.tid )
  {
    if ( lhs.syn_id == rhs.syn_id )
    {
      return lhs.lcid < rhs.lcid;
    }
    else
    {
      return lhs.syn_id < rhs.syn_id;
    }
  }
  else
  {
    return lhs.tid < rhs.tid;
  }
}

inline bool operator>( const SourceTablePosition& lhs, const SourceTablePosition& rhs )
{
  return operator<( rhs, lhs );
}

inline bool operator<=( const SourceTablePosition& lhs, const SourceTablePosition& rhs )
{
  return not operator>( lhs, rhs );
}

inline bool operator>=( const SourceTablePosition& lhs, const SourceTablePosition& rhs )
{
  return not operator<( lhs, rhs );
}

} // namespace nest

#endif // SOURCE_TABLE_POSITION_H
