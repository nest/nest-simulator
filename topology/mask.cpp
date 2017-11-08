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
EllipseMask< 2 >::inside( const Position< 2 >& p ) const
{
  const double new_x = ( p[ 0 ] - center_[ 0 ] ) * azimuth_cos_
    + ( p[ 1 ] - center_[ 1 ] ) * azimuth_sin_;
  const double new_y = ( p[ 0 ] - center_[ 0 ] ) * azimuth_sin_
    - ( p[ 1 ] - center_[ 1 ] ) * azimuth_cos_;

  return std::pow( new_x, 2 ) * x_scale_ + std::pow( new_y, 2 ) * y_scale_ <= 1;
}

template <>
bool
EllipseMask< 3 >::inside( const Position< 3 >& p ) const
{
  // The new x, y, z values are calculated using rotation matrices:
  // [new_x, new_y, new_z] =
  //       R_y(-polar)*R_z(azimuth)*[x - x_c, y - y_c, z - z_c]
  // where R_z(t) = [cos(t) sin(t) 0; sin(t) -cos(t) 0; 0 0 1] and
  //       R_y(-t) = [cos(t) 0 -sin(t); 0 1 0; sin(t) 0 cos(t)]

  // See https://en.wikipedia.org/wiki/Rotation_matrix for more.

  const double new_x =
    ( ( p[ 0 ] - center_[ 0 ] ) * azimuth_cos_
      + ( p[ 1 ] - center_[ 1 ] ) * azimuth_sin_ ) * polar_cos_
    - ( p[ 2 ] - center_[ 2 ] ) * polar_sin_;

  const double new_y = ( ( p[ 0 ] - center_[ 0 ] ) * azimuth_sin_
    - ( p[ 1 ] - center_[ 1 ] ) * azimuth_cos_ );

  const double new_z =
    ( ( p[ 0 ] - center_[ 0 ] ) * azimuth_cos_
      + ( p[ 1 ] - center_[ 1 ] ) * azimuth_sin_ ) * polar_sin_
    + ( p[ 2 ] - center_[ 2 ] ) * polar_cos_;

  return std::pow( new_x, 2 ) * x_scale_ + std::pow( new_y, 2 ) * y_scale_
    + std::pow( new_z, 2 ) * z_scale_
    <= 1;
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

template <>
bool
EllipseMask< 2 >::inside( const Box< 2 >& b ) const
{
  Position< 2 > p = b.lower_left;

  // Test if all corners are inside ellipse
  if ( not inside( p ) )
  {
    return false; // lower left corner not inside ellipse
  }

  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) )
  {
    return false; // upper left corner not inside ellipse
  }

  p[ 1 ] = b.upper_right[ 1 ];
  if ( not inside( p ) )
  {
    return false; // upper right corner not inside ellipse
  }

  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) )
  {
    return false; // lower right corner not inside ellipse
  }

  return true;
}

template <>
bool
EllipseMask< 3 >::inside( const Box< 3 >& b ) const
{
  Position< 3 > p = b.lower_left;

  // Test if all corners are inside ellipsoid
  if ( not inside( p ) )
  {
    return false; // first lower left corner not inside ellipsoid
  }

  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) )
  {
    return false; // second lower left corner not inside ellipsoid
  }

  p[ 1 ] = b.upper_right[ 1 ];
  if ( not inside( p ) )
  {
    return false; // second lower right corner not inside ellipsoid
  }

  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) )
  {
    return false; // first lower right corner not inside ellipsoid
  }

  p[ 2 ] = b.upper_right[ 2 ];
  if ( not inside( p ) )
  {
    return false; // first upper right corner not inside ellipsoid
  }

  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) )
  {
    return false; // second upper right corner not inside ellipsoid
  }

  p[ 1 ] = b.lower_left[ 1 ];
  if ( not inside( p ) )
  {
    return false; // second upper left corner not inside ellipsoid
  }

  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) )
  {
    return false; // first upper left corner not inside ellipsoid
  }

  return true;
}

}
