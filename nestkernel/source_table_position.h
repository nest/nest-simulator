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

namespace nest
{

/**
 * Tuple to store position in 3d vector of sources.
 **/
struct SourceTablePosition
{
  long tid;       //!< thread index
  long syn_index; //!< synapse-type index
  long lcid;      //!< local connection index
  SourceTablePosition();
  SourceTablePosition( const long tid, const long syn_index, const long lcid );
  SourceTablePosition( const SourceTablePosition& rhs );
};

inline SourceTablePosition::SourceTablePosition()
  : tid( -1 )
  , syn_index( -1 )
  , lcid( -1 )
{
}

inline SourceTablePosition::SourceTablePosition( const long tid,
  const long syn_index,
  const long lcid )
  : tid( tid )
  , syn_index( syn_index )
  , lcid( lcid )
{
}

inline SourceTablePosition::SourceTablePosition(
  const SourceTablePosition& rhs )
  : tid( rhs.tid )
  , syn_index( rhs.syn_index )
  , lcid( rhs.lcid )
{
}

inline bool operator==( const SourceTablePosition& lhs,
  const SourceTablePosition& rhs )
{
  return ( ( lhs.tid == rhs.tid ) && ( lhs.syn_index == rhs.syn_index )
    && ( lhs.lcid == rhs.lcid ) );
}

inline bool operator!=( const SourceTablePosition& lhs,
  const SourceTablePosition& rhs )
{
  return !operator==( lhs, rhs );
}

inline bool operator<( const SourceTablePosition& lhs,
  const SourceTablePosition& rhs )
{
  if ( lhs.tid == rhs.tid )
  {
    if ( lhs.syn_index == rhs.syn_index )
    {
      return lhs.lcid < rhs.lcid;
    }
    else
    {
      return lhs.syn_index < rhs.syn_index;
    }
  }
  else
  {
    return lhs.tid < rhs.tid;
  }
}

inline bool operator>( const SourceTablePosition& lhs,
  const SourceTablePosition& rhs )
{
  return operator<( rhs, lhs );
}

inline bool operator<=( const SourceTablePosition& lhs,
  const SourceTablePosition& rhs )
{
  return !operator>( lhs, rhs );
}

inline bool operator>=( const SourceTablePosition& lhs,
  const SourceTablePosition& rhs )
{
  return !operator<( lhs, rhs );
}

} // namespace nest

#endif // SOURCE_TABLE_POSITION_H
