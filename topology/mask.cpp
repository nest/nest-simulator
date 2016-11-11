/*
 *  mask.cpp
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

#include "mask.h"

// Explicit specializations behave as normal methods and must
// be defined here to avoid duplicate symbols.

namespace nest
{

template <>
bool
BallMask< 2 >::inside( const Box< 2 >& b ) const
{
  Position< 2 > p = b.lower_left;

  // Test if all corners are inside circle

  if ( !inside( p ) )
    return false; // (0,0)
  p[ 0 ] = b.upper_right[ 0 ];
  if ( !inside( p ) )
    return false; // (0,1)
  p[ 1 ] = b.upper_right[ 1 ];
  if ( !inside( p ) )
    return false; // (1,1)
  p[ 0 ] = b.lower_left[ 0 ];
  if ( !inside( p ) )
    return false; // (1,0)

  return true;
}

template <>
bool
BallMask< 3 >::inside( const Box< 3 >& b ) const
{
  Position< 3 > p = b.lower_left;

  // Test if all corners are inside sphere

  if ( !inside( p ) )
    return false; // (0,0,0)
  p[ 0 ] = b.upper_right[ 0 ];
  if ( !inside( p ) )
    return false; // (0,0,1)
  p[ 1 ] = b.upper_right[ 1 ];
  if ( !inside( p ) )
    return false; // (0,1,1)
  p[ 0 ] = b.lower_left[ 0 ];
  if ( !inside( p ) )
    return false; // (0,1,0)
  p[ 2 ] = b.upper_right[ 2 ];
  if ( !inside( p ) )
    return false; // (1,1,0)
  p[ 0 ] = b.upper_right[ 0 ];
  if ( !inside( p ) )
    return false; // (1,1,1)
  p[ 1 ] = b.lower_left[ 1 ];
  if ( !inside( p ) )
    return false; // (1,0,1)
  p[ 0 ] = b.lower_left[ 0 ];
  if ( !inside( p ) )
    return false; // (1,0,0)

  return true;
}

}
