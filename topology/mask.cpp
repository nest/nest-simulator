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

  if ( not inside( p ) )
  {
    return false; // (0,0)
  }
  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) )
  {
    return false; // (0,1)
  }
  p[ 1 ] = b.upper_right[ 1 ];
  if ( not inside( p ) )
  {
    return false; // (1,1)
  }
  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) )
  {
    return false; // (1,0)
  }

  return true;
}

template <>
bool
BallMask< 3 >::inside( const Box< 3 >& b ) const
{
  Position< 3 > p = b.lower_left;

  // Test if all corners are inside sphere

  if ( not inside( p ) )
  {
    return false; // (0,0,0)
  }
  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) )
  {
    return false; // (0,0,1)
  }
  p[ 1 ] = b.upper_right[ 1 ];
  if ( not inside( p ) )
  {
    return false; // (0,1,1)
  }
  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) )
  {
    return false; // (0,1,0)
  }
  p[ 2 ] = b.upper_right[ 2 ];
  if ( not inside( p ) )
  {
    return false; // (1,1,0)
  }
  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) )
  {
    return false; // (1,1,1)
  }
  p[ 1 ] = b.lower_left[ 1 ];
  if ( not inside( p ) )
  {
    return false; // (1,0,1)
  }
  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) )
  {
    return false; // (1,0,0)
  }

  return true;
}

template <>
void
BoxMask< 2 >::calculate_min_max_values_()
{
  // Rotate the corners of the box to find the new minimum and maximum x and y
  // values to define the bounding box of the rotated box. If the box is not
  // rotated, the min values correspond to the lower_left values and the max
  // values correspond to the upper_right values.

  if ( not is_rotated_ )
  {
    min_values_ = lower_left_;
    max_values_ = upper_right_;
  }
  else
  {
    const Position< 2 > lower_left_cos = ( lower_left_ - cntr_ ) * azimuth_cos_;
    const Position< 2 > lower_left_sin = ( lower_left_ - cntr_ ) * azimuth_sin_;
    const Position< 2 > upper_right_cos = ( upper_right_ - cntr_ ) * azimuth_cos_;
    const Position< 2 > upper_right_sin = ( upper_right_ - cntr_ ) * azimuth_sin_;

    const double rotatedLLx = lower_left_cos[ 0 ] - lower_left_sin[ 1 ] + cntr_[ 0 ];
    const double rotatedLLy = lower_left_sin[ 0 ] + lower_left_cos[ 1 ] + cntr_[ 1 ];

    const double rotatedLRx = upper_right_cos[ 0 ] - lower_left_sin[ 1 ] + cntr_[ 0 ];
    const double rotatedLRy = upper_right_sin[ 0 ] + lower_left_cos[ 1 ] + cntr_[ 1 ];

    const double rotatedURx = upper_right_cos[ 0 ] - upper_right_sin[ 1 ] + cntr_[ 0 ];
    const double rotatedURy = upper_right_sin[ 0 ] + upper_right_cos[ 1 ] + cntr_[ 1 ];

    const double rotatedULx = lower_left_cos[ 0 ] - upper_right_sin[ 1 ] + cntr_[ 0 ];
    const double rotatedULy = lower_left_sin[ 0 ] + upper_right_cos[ 1 ] + cntr_[ 1 ];

    const double rotated_x[] = { rotatedLLx, rotatedLRx, rotatedURx, rotatedULx };
    const double rotated_y[] = { rotatedLLy, rotatedLRy, rotatedURy, rotatedULy };

    min_values_[ 0 ] = *std::min_element( rotated_x, rotated_x + 4 );
    min_values_[ 1 ] = *std::min_element( rotated_y, rotated_y + 4 );
    max_values_[ 0 ] = *std::max_element( rotated_x, rotated_x + 4 );
    max_values_[ 1 ] = *std::max_element( rotated_y, rotated_y + 4 );
  }
}

template <>
void
BoxMask< 3 >::calculate_min_max_values_()
{
  /*
   * Rotate the corners of the box to find the new minimum and maximum x, y and
   * z values to define the bounding box of the rotated box. We need to rotate
   * all eight corners. If the box is not rotated, the min values correspond
   * to the lower_left values and the max values correspond to the upper_right
   * values.
   *
   *        LLH      LHH
   *       *--------*
   *      /|       /|
   *     / |LLL   / |LHL
   *    *--*-----*--*
   * HLH| /   HHH| /
   *    |/       |/
   *    *--------*
   * HLL      HHL
   *
   *                       max_values_
   *       *-----*------------*
   *     /    /     \        /|
   *    *---/----------\----* |
   *    | /               \ | |
   *    *                   * |
   *    | \               / | |
   *    *    \          /   * |
   *    | \      \    /   / | |
   *    |    \      *   /   | *
   *    |       \   | /     |/
   *    *-----------*-------*
   * min_values_
   */

  if ( not is_rotated_ )
  {
    min_values_ = lower_left_;
    max_values_ = upper_right_;
  }
  else
  {
    const Position< 3 > lower_left_cos = ( lower_left_ - cntr_ ) * azimuth_cos_;
    const Position< 3 > lower_left_sin = ( lower_left_ - cntr_ ) * azimuth_sin_;
    const Position< 3 > upper_right_cos = ( upper_right_ - cntr_ ) * azimuth_cos_;
    const Position< 3 > upper_right_sin = ( upper_right_ - cntr_ ) * azimuth_sin_;

    const double lower_left_polar_cos = ( lower_left_[ 2 ] - cntr_[ 2 ] ) * polar_cos_;
    const double lower_left_polar_sin = ( lower_left_[ 2 ] - cntr_[ 2 ] ) * polar_sin_;
    const double upper_right_polar_cos = ( upper_right_[ 2 ] - cntr_[ 2 ] ) * polar_cos_;
    const double upper_right_polar_sin = ( upper_right_[ 2 ] - cntr_[ 2 ] ) * polar_sin_;

    const double rotatedLLLx =
      ( lower_left_cos[ 0 ] - lower_left_sin[ 1 ] ) * polar_cos_ - lower_left_polar_sin + cntr_[ 0 ];
    const double rotatedLLLy = lower_left_sin[ 0 ] + lower_left_cos[ 1 ] + cntr_[ 1 ];
    const double rotatedLLLz =
      ( lower_left_cos[ 0 ] - lower_left_sin[ 1 ] ) * polar_sin_ + lower_left_polar_cos + cntr_[ 2 ];

    const double rotatedLLHx =
      ( lower_left_cos[ 0 ] - lower_left_sin[ 1 ] ) * polar_cos_ - upper_right_polar_sin + cntr_[ 0 ];
    const double rotatedLLHy = lower_left_sin[ 0 ] + lower_left_cos[ 1 ] + cntr_[ 1 ];
    const double rotatedLLHz =
      ( lower_left_cos[ 0 ] - lower_left_sin[ 1 ] ) * polar_sin_ + upper_right_polar_cos + cntr_[ 2 ];

    const double rotatedHLLx =
      ( upper_right_cos[ 0 ] - lower_left_sin[ 1 ] ) * polar_cos_ - lower_left_polar_sin + cntr_[ 0 ];
    const double rotatedHLLy = upper_right_sin[ 0 ] + lower_left_cos[ 1 ] + cntr_[ 1 ];
    const double rotatedHLLz =
      ( upper_right_cos[ 0 ] - lower_left_sin[ 1 ] ) * polar_sin_ + lower_left_polar_cos + cntr_[ 2 ];

    const double rotatedHLHx =
      ( upper_right_cos[ 0 ] - lower_left_sin[ 1 ] ) * polar_cos_ - upper_right_polar_sin + cntr_[ 0 ];
    const double rotatedHLHy = upper_right_sin[ 0 ] + lower_left_cos[ 1 ] + cntr_[ 1 ];
    const double rotatedHLHz =
      ( upper_right_cos[ 0 ] - lower_left_sin[ 1 ] ) * polar_sin_ + upper_right_polar_cos + cntr_[ 2 ];

    const double rotatedHHHx =
      ( upper_right_cos[ 0 ] - upper_right_sin[ 1 ] ) * polar_cos_ - upper_right_polar_sin + cntr_[ 0 ];
    const double rotatedHHHy = upper_right_sin[ 0 ] + upper_right_cos[ 1 ] + cntr_[ 1 ];
    const double rotatedHHHz =
      ( upper_right_cos[ 0 ] - upper_right_sin[ 1 ] ) * polar_sin_ + upper_right_polar_cos + cntr_[ 2 ];

    const double rotatedHHLx =
      ( upper_right_cos[ 0 ] - upper_right_sin[ 1 ] ) * polar_cos_ - lower_left_polar_sin + cntr_[ 0 ];
    const double rotatedHHLy = upper_right_sin[ 0 ] + upper_right_cos[ 1 ] + cntr_[ 1 ];
    const double rotatedHHLz =
      ( upper_right_cos[ 0 ] - upper_right_sin[ 1 ] ) * polar_sin_ + lower_left_polar_cos + cntr_[ 2 ];

    const double rotatedLHHx =
      ( lower_left_cos[ 0 ] - upper_right_sin[ 1 ] ) * polar_cos_ - upper_right_polar_sin + cntr_[ 0 ];
    const double rotatedLHHy = lower_left_sin[ 0 ] + upper_right_cos[ 1 ] + cntr_[ 1 ];
    const double rotatedLHHz =
      ( lower_left_cos[ 0 ] - upper_right_sin[ 1 ] ) * polar_sin_ + upper_right_polar_cos + cntr_[ 2 ];

    const double rotatedLHLx =
      ( lower_left_cos[ 0 ] - upper_right_sin[ 1 ] ) * polar_cos_ - lower_left_polar_sin + cntr_[ 0 ];
    const double rotatedLHLy = lower_left_sin[ 0 ] + upper_right_cos[ 1 ] + cntr_[ 1 ];
    const double rotatedLHLz =
      ( lower_left_cos[ 0 ] - upper_right_sin[ 1 ] ) * polar_sin_ + lower_left_polar_cos + cntr_[ 2 ];

    const double rotated_x[] = {
      rotatedLLLx, rotatedLLHx, rotatedHLLx, rotatedHLHx, rotatedHHHx, rotatedHHLx, rotatedLHHx, rotatedLHLx
    };
    const double rotated_y[] = {
      rotatedLLLy, rotatedLLHy, rotatedHLLy, rotatedHLHy, rotatedHHHy, rotatedHHLy, rotatedLHHy, rotatedLHLy
    };
    const double rotated_z[] = {
      rotatedLLLz, rotatedLLHz, rotatedHLLz, rotatedHLHz, rotatedHHHz, rotatedHHLz, rotatedLHHz, rotatedLHLz
    };

    min_values_[ 0 ] = *std::min_element( rotated_x, rotated_x + 8 );
    min_values_[ 1 ] = *std::min_element( rotated_y, rotated_y + 8 );
    min_values_[ 2 ] = *std::min_element( rotated_z, rotated_z + 8 );
    max_values_[ 0 ] = *std::max_element( rotated_x, rotated_x + 8 );
    max_values_[ 1 ] = *std::max_element( rotated_y, rotated_y + 8 );
    max_values_[ 2 ] = *std::max_element( rotated_z, rotated_z + 8 );
  }
}

template <>
bool
BoxMask< 2 >::inside( const Position< 2 >& p ) const
{
  // If the box is not rotated we just check if the point is inside the box.
  if ( not is_rotated_ )
  {
    return ( lower_left_ <= p ) && ( p <= upper_right_ );
  }

  // If we have a rotated box, we rotate the point down to the unrotated box,
  // and check if it is inside said unrotated box.

  // The new x, y values are calculated using a rotation matrix:
  // [new_x, new_y] = R(-azimuth)*[x - x_c, y - y_c]
  // where R(-t) = [cos(t) sin(t); -sin(t) cos(t)]

  // See https://en.wikipedia.org/wiki/Rotation_matrix for more.

  const double new_x = p[ 0 ] * azimuth_cos_ - cntr_x_az_cos_ + p[ 1 ] * azimuth_sin_ - cntr_y_az_sin_ + cntr_[ 0 ];
  const double new_y = -p[ 0 ] * azimuth_sin_ + cntr_x_az_sin_ + p[ 1 ] * azimuth_cos_ - cntr_y_az_cos_ + cntr_[ 1 ];
  const Position< 2 > new_p( new_x, new_y );

  // We need to add a small epsilon in case of rounding errors.
  return ( lower_left_ - eps_ <= new_p ) && ( new_p <= upper_right_ + eps_ );
}

template <>
bool
BoxMask< 3 >::inside( const Position< 3 >& p ) const
{
  // If the box is not rotated we just check if the point is inside the box.
  if ( not is_rotated_ )
  {
    return ( lower_left_ <= p ) && ( p <= upper_right_ );
  }

  // If we have a rotated box, we rotate the point down to the unrotated box,
  // and check if it is inside said unrotated box.

  // The new x, y, z values are calculated using rotation matrices:
  // [new_x, new_y, new_z] =
  //       R_y(-polar)*R_z(-azimuth)*[x - x_c, y - y_c, z - z_c]
  // where R_z(-t) = [cos(t) sin(t) 0; -sin(t) cos(t) 0; 0 0 1] and
  //       R_y(-t) = [cos(t) 0 -sin(t); 0 1 0; sin(t) 0 cos(t)]

  // See https://en.wikipedia.org/wiki/Rotation_matrix for more.

  const double new_x = p[ 0 ] * az_cos_pol_cos_ - cntr_x_az_cos_pol_cos_ + p[ 1 ] * az_sin_pol_cos_
    - cntr_y_az_sin_pol_cos_ - p[ 2 ] * polar_sin_ + cntr_z_pol_sin_ + cntr_[ 0 ];
  const double new_y = -p[ 0 ] * azimuth_sin_ + cntr_x_az_sin_ + p[ 1 ] * azimuth_cos_ - cntr_y_az_cos_ + cntr_[ 1 ];
  const double new_z = p[ 0 ] * az_cos_pol_sin_ - cntr_x_az_cos_pol_sin_ + p[ 1 ] * az_sin_pol_sin_
    - cntr_y_az_sin_pol_sin_ + p[ 2 ] * polar_cos_ - cntr_z_pol_cos_ + cntr_[ 2 ];

  const Position< 3 > new_p( new_x, new_y, new_z );

  // We need to add a small epsilon in case of rounding errors.
  return ( lower_left_ - eps_ <= new_p ) && ( new_p <= upper_right_ + eps_ );
}

template <>
bool
EllipseMask< 2 >::inside( const Position< 2 >& p ) const
{
  const double new_x = ( p[ 0 ] - center_[ 0 ] ) * azimuth_cos_ + ( p[ 1 ] - center_[ 1 ] ) * azimuth_sin_;
  const double new_y = ( p[ 0 ] - center_[ 0 ] ) * azimuth_sin_ - ( p[ 1 ] - center_[ 1 ] ) * azimuth_cos_;

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
    ( ( p[ 0 ] - center_[ 0 ] ) * azimuth_cos_ + ( p[ 1 ] - center_[ 1 ] ) * azimuth_sin_ ) * polar_cos_
    - ( p[ 2 ] - center_[ 2 ] ) * polar_sin_;

  const double new_y = ( ( p[ 0 ] - center_[ 0 ] ) * azimuth_sin_ - ( p[ 1 ] - center_[ 1 ] ) * azimuth_cos_ );

  const double new_z =
    ( ( p[ 0 ] - center_[ 0 ] ) * azimuth_cos_ + ( p[ 1 ] - center_[ 1 ] ) * azimuth_sin_ ) * polar_sin_
    + ( p[ 2 ] - center_[ 2 ] ) * polar_cos_;

  return std::pow( new_x, 2 ) * x_scale_ + std::pow( new_y, 2 ) * y_scale_ + std::pow( new_z, 2 ) * z_scale_ <= 1;
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
