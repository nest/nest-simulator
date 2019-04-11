/*
 *  mask_impl.h
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

#ifndef MASK_IMPL_H
#define MASK_IMPL_H

#include "mask.h"

namespace nest {

template < int D >
AbstractMask*
Mask< D >::intersect_mask( const AbstractMask& other ) const
{
  const Mask* other_d = dynamic_cast< const Mask* >( &other );
  if ( other_d == 0 ) {
    throw BadProperty( "Masks must have same number of dimensions." );
  }
  return new IntersectionMask< D >( *this, *other_d );
}

template < int D >
AbstractMask*
Mask< D >::union_mask( const AbstractMask& other ) const
{
  const Mask* other_d = dynamic_cast< const Mask* >( &other );
  if ( other_d == 0 ) {
    throw BadProperty( "Masks must have same number of dimensions." );
  }
  return new UnionMask< D >( *this, *other_d );
}

template < int D >
AbstractMask*
Mask< D >::minus_mask( const AbstractMask& other ) const
{
  const Mask* other_d = dynamic_cast< const Mask* >( &other );
  if ( other_d == 0 ) {
    throw BadProperty( "Masks must have same number of dimensions." );
  }
  return new DifferenceMask< D >( *this, *other_d );
}

template < int D >
bool
Mask< D >::inside( const std::vector< double >& pt ) const
{
  return inside( Position< D >( pt ) );
}

template < int D >
bool
Mask< D >::outside( const Box< D >& b ) const
{
  Box< D > bb = get_bbox();
  for ( int i = 0; i < D; ++i ) {
    if ( ( b.upper_right[ i ] < bb.lower_left[ i ] ) || ( b.lower_left[ i ] > bb.upper_right[ i ] ) ) {
      return true;
    }
  }
  return false;
}

template <>
bool
BoxMask< 2 >::inside( const Position< 2 >& p ) const
{
  // If the box is not rotated we just check if the point is inside the box.
  if ( not is_rotated_ ) {
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
  if ( not is_rotated_ ) {
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

template < int D >
bool
BoxMask< D >::inside( const Box< D >& b ) const
{
  return ( inside( b.lower_left ) and inside( b.upper_right ) );
}

template < int D >
bool
BoxMask< D >::outside( const Box< D >& b ) const
{
  // Note: There could be some inconsistencies with the boundaries. For the
  // inside() function we had to add an epsilon because of rounding errors that
  // can occur if GIDs are on the boundary if we have rotation. This might lead
  // to overlap of the inside and outside functions. None of the tests have
  // picked up any problems with this potential overlap as of yet (autumn 2017),
  // so we don't know if it is an actual problem.
  for ( int i = 0; i < D; ++i ) {
    if ( ( b.upper_right[ i ] < min_values_[ i ] ) || ( b.lower_left[ i ] > max_values_[ i ] ) ) {
      return true;
    }
  }
  return false;
}

template <>
void
BoxMask< 2 >::calculate_min_max_values_()
{
  // Rotate the corners of the box to find the new minimum and maximum x and y
  // values to define the bounding box of the rotated box. If the box is not
  // rotated, the min values correspond to the lower_left values and the max
  // values correspond to the upper_right values.

  if ( not is_rotated_ ) {
    min_values_ = lower_left_;
    max_values_ = upper_right_;
  }
  else {
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

  if ( not is_rotated_ ) {
    min_values_ = lower_left_;
    max_values_ = upper_right_;
  }
  else {
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

template < int D >
Box< D >
BoxMask< D >::get_bbox() const
{
  return Box< D >( min_values_, max_values_ );
}

template < int D >
Mask< D >*
BoxMask< D >::clone() const
{
  return new BoxMask( *this );
}

template < int D >
DictionaryDatum
BoxMask< D >::get_dict() const
{
  DictionaryDatum d( new Dictionary );
  DictionaryDatum maskd( new Dictionary );
  def< DictionaryDatum >( d, get_name(), maskd );
  def< std::vector< double > >( maskd, names::lower_left, lower_left_ );
  def< std::vector< double > >( maskd, names::upper_right, upper_right_ );
  def< double >( maskd, names::azimuth_angle, azimuth_angle_ );
  def< double >( maskd, names::polar_angle, polar_angle_ );
  return d;
}

template < int D >
bool
BallMask< D >::inside( const Position< D >& p ) const
{
  return ( p - center_ ).length() <= radius_;
}

template <>
bool
BallMask< 2 >::inside( const Box< 2 >& b ) const
{
  Position< 2 > p = b.lower_left;

  // Test if all corners are inside circle

  if ( not inside( p ) ) {
    return false; // (0,0)
  }

  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) ) {
    return false; // (0,1)
  }

  p[ 1 ] = b.upper_right[ 1 ];
  if ( not inside( p ) ) {
    return false; // (1,1)
  }

  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) ) {
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

  if ( not inside( p ) ) {
    return false; // (0,0,0)
  }

  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) ) {
    return false; // (0,0,1)
  }

  p[ 1 ] = b.upper_right[ 1 ];
  if ( not inside( p ) ) {
    return false; // (0,1,1)
  }

  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) ) {
    return false; // (0,1,0)
  }

  p[ 2 ] = b.upper_right[ 2 ];
  if ( not inside( p ) ) {
    return false; // (1,1,0)
  }

  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) ) {
    return false; // (1,1,1)
  }

  p[ 1 ] = b.lower_left[ 1 ];
  if ( not inside( p ) ) {
    return false; // (1,0,1)
  }

  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) ) {
    return false; // (1,0,0)
  }

  return true;
}

template < int D >
bool
BallMask< D >::outside( const Box< D >& b ) const
{
  // Currently only checks if the box is outside the bounding box of
  // the ball. This could be made more refined.
  for ( int i = 0; i < D; ++i ) {
    if ( ( b.upper_right[ i ] < center_[ i ] - radius_ ) || ( b.lower_left[ i ] > center_[ i ] + radius_ ) ) {
      return true;
    }
  }
  return false;
}

template < int D >
Box< D >
BallMask< D >::get_bbox() const
{
  Box< D > bb( center_, center_ );
  for ( int i = 0; i < D; ++i ) {
    bb.lower_left[ i ] -= radius_;
    bb.upper_right[ i ] += radius_;
  }
  return bb;
}

template < int D >
Mask< D >*
BallMask< D >::clone() const
{
  return new BallMask( *this );
}

template < int D >
DictionaryDatum
BallMask< D >::get_dict() const
{
  DictionaryDatum d( new Dictionary );
  DictionaryDatum maskd( new Dictionary );
  def< DictionaryDatum >( d, get_name(), maskd );
  def< double >( maskd, names::radius, radius_ );
  def< std::vector< double > >( maskd, names::anchor, center_ );
  return d;
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
  if ( not inside( p ) ) {
    return false; // lower left corner not inside ellipse
  }

  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) ) {
    return false; // upper left corner not inside ellipse
  }

  p[ 1 ] = b.upper_right[ 1 ];
  if ( not inside( p ) ) {
    return false; // upper right corner not inside ellipse
  }

  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) ) {
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
  if ( not inside( p ) ) {
    return false; // first lower left corner not inside ellipsoid
  }

  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) ) {
    return false; // second lower left corner not inside ellipsoid
  }

  p[ 1 ] = b.upper_right[ 1 ];
  if ( not inside( p ) ) {
    return false; // second lower right corner not inside ellipsoid
  }

  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) ) {
    return false; // first lower right corner not inside ellipsoid
  }

  p[ 2 ] = b.upper_right[ 2 ];
  if ( not inside( p ) ) {
    return false; // first upper right corner not inside ellipsoid
  }

  p[ 0 ] = b.upper_right[ 0 ];
  if ( not inside( p ) ) {
    return false; // second upper right corner not inside ellipsoid
  }

  p[ 1 ] = b.lower_left[ 1 ];
  if ( not inside( p ) ) {
    return false; // second upper left corner not inside ellipsoid
  }

  p[ 0 ] = b.lower_left[ 0 ];
  if ( not inside( p ) ) {
    return false; // first upper left corner not inside ellipsoid
  }

  return true;
}

template < int D >
void
EllipseMask< D >::create_bbox_()
{
  // Currently assumes 3D when constructing the radius vector. This could be
  // avoided with more if tests, but the vector is only made once and is not
  // big. The construction of the box is done in accordance with the actual
  // dimensions.
  std::vector< double > radii( 3 );
  if ( azimuth_angle_ == 0.0 and polar_angle_ == 0.0 ) {
    radii[ 0 ] = major_axis_ / 2.0;
    radii[ 1 ] = minor_axis_ / 2.0;
    radii[ 2 ] = polar_axis_ / 2.0;
  }
  else {
    // If the ellipse or ellipsoid is tilted, we make the boundary box
    // quadratic, with the length of the sides equal to the axis with greatest
    // length. This could be more refined.
    const double greatest_semi_axis = std::max( major_axis_, polar_axis_ ) / 2.0;
    radii[ 0 ] = greatest_semi_axis;
    radii[ 1 ] = greatest_semi_axis;
    radii[ 2 ] = greatest_semi_axis;
  }

  for ( int i = 0; i < D; ++i ) {
    bbox_.lower_left[ i ] = center_[ i ] - radii[ i ];
    bbox_.upper_right[ i ] = center_[ i ] + radii[ i ];
  }
}

template < int D >
bool
EllipseMask< D >::outside( const Box< D >& b ) const
{
  // Currently only checks if the box is outside the bounding box of
  // the ellipse. This could be made more refined.

  const Box< D >& bb = bbox_;

  for ( int i = 0; i < D; ++i ) {
    if ( ( b.upper_right[ i ] < bb.lower_left[ i ] ) || ( b.lower_left[ i ] > bb.upper_right[ i ] ) ) {
      return true;
    }
  }
  return false;
}

template < int D >
Box< D >
EllipseMask< D >::get_bbox() const
{
  return bbox_;
}

template < int D >
Mask< D >*
EllipseMask< D >::clone() const
{
  return new EllipseMask( *this );
}

template < int D >
DictionaryDatum
EllipseMask< D >::get_dict() const
{
  DictionaryDatum d( new Dictionary );
  DictionaryDatum maskd( new Dictionary );
  def< DictionaryDatum >( d, get_name(), maskd );
  def< double >( maskd, names::major_axis, major_axis_ );
  def< double >( maskd, names::minor_axis, minor_axis_ );
  def< double >( maskd, names::polar_axis, polar_axis_ );
  def< std::vector< double > >( maskd, names::anchor, center_ );
  def< double >( maskd, names::azimuth_angle, azimuth_angle_ );
  def< double >( maskd, names::polar_angle, polar_angle_ );
  return d;
}


template < int D >
bool
IntersectionMask< D >::inside( const Position< D >& p ) const
{
  return mask1_->inside( p ) && mask2_->inside( p );
}

template < int D >
bool
IntersectionMask< D >::inside( const Box< D >& b ) const
{
  return mask1_->inside( b ) && mask2_->inside( b );
}

template < int D >
bool
IntersectionMask< D >::outside( const Box< D >& b ) const
{
  return mask1_->outside( b ) || mask2_->outside( b );
}

template < int D >
Box< D >
IntersectionMask< D >::get_bbox() const
{
  Box< D > bb = mask1_->get_bbox();
  Box< D > bb2 = mask2_->get_bbox();
  for ( int i = 0; i < D; ++i ) {
    if ( bb2.lower_left[ i ] > bb.lower_left[ i ] ) {
      bb.lower_left[ i ] = bb2.lower_left[ i ];
    }
    if ( bb2.upper_right[ i ] < bb.upper_right[ i ] ) {
      bb.upper_right[ i ] = bb2.upper_right[ i ];
    }
  }
  return bb;
}

template < int D >
Mask< D >*
IntersectionMask< D >::clone() const
{
  return new IntersectionMask( *this );
}

template < int D >
bool
UnionMask< D >::inside( const Position< D >& p ) const
{
  return mask1_->inside( p ) || mask2_->inside( p );
}

template < int D >
bool
UnionMask< D >::inside( const Box< D >& b ) const
{
  return mask1_->inside( b ) || mask2_->inside( b );
}

template < int D >
bool
UnionMask< D >::outside( const Box< D >& b ) const
{
  return mask1_->outside( b ) && mask2_->outside( b );
}

template < int D >
Box< D >
UnionMask< D >::get_bbox() const
{
  Box< D > bb = mask1_->get_bbox();
  Box< D > bb2 = mask2_->get_bbox();
  for ( int i = 0; i < D; ++i ) {
    if ( bb2.lower_left[ i ] < bb.lower_left[ i ] ) {
      bb.lower_left[ i ] = bb2.lower_left[ i ];
    }
    if ( bb2.upper_right[ i ] > bb.upper_right[ i ] ) {
      bb.upper_right[ i ] = bb2.upper_right[ i ];
    }
  }
  return bb;
}

template < int D >
Mask< D >*
UnionMask< D >::clone() const
{
  return new UnionMask( *this );
}

template < int D >
bool
DifferenceMask< D >::inside( const Position< D >& p ) const
{
  return mask1_->inside( p ) && not mask2_->inside( p );
}

template < int D >
bool
DifferenceMask< D >::inside( const Box< D >& b ) const
{
  return mask1_->inside( b ) && mask2_->outside( b );
}

template < int D >
bool
DifferenceMask< D >::outside( const Box< D >& b ) const
{
  return mask1_->outside( b ) || mask2_->inside( b );
}

template < int D >
Box< D >
DifferenceMask< D >::get_bbox() const
{
  return mask1_->get_bbox();
}

template < int D >
Mask< D >*
DifferenceMask< D >::clone() const
{
  return new DifferenceMask( *this );
}

template < int D >
bool
ConverseMask< D >::inside( const Position< D >& p ) const
{
  return m_->inside( -p );
}

template < int D >
bool
ConverseMask< D >::inside( const Box< D >& b ) const
{
  return m_->inside( Box< D >( -b.upper_right, -b.lower_left ) );
}

template < int D >
bool
ConverseMask< D >::outside( const Box< D >& b ) const
{
  return m_->outside( Box< D >( -b.upper_right, -b.lower_left ) );
}

template < int D >
Box< D >
ConverseMask< D >::get_bbox() const
{
  Box< D > bb = m_->get_bbox();
  return Box< D >( -bb.upper_right, -bb.lower_left );
}

template < int D >
Mask< D >*
ConverseMask< D >::clone() const
{
  return new ConverseMask( *this );
}

template < int D >
bool
AnchoredMask< D >::inside( const Position< D >& p ) const
{
  return m_->inside( p - anchor_ );
}

template < int D >
bool
AnchoredMask< D >::inside( const Box< D >& b ) const
{
  return m_->inside( Box< D >( b.lower_left - anchor_, b.upper_right - anchor_ ) );
}

template < int D >
bool
AnchoredMask< D >::outside( const Box< D >& b ) const
{
  return m_->outside( Box< D >( b.lower_left - anchor_, b.upper_right - anchor_ ) );
}

template < int D >
Box< D >
AnchoredMask< D >::get_bbox() const
{
  Box< D > bb = m_->get_bbox();
  return Box< D >( bb.lower_left + anchor_, bb.upper_right + anchor_ );
}

template < int D >
Mask< D >*
AnchoredMask< D >::clone() const
{
  return new AnchoredMask( *this );
}

template < int D >
DictionaryDatum
AnchoredMask< D >::get_dict() const
{
  DictionaryDatum d = m_->get_dict();
  def< std::vector< double > >( d, names::anchor, anchor_ );
  return d;
}

} // namespace nest

#endif
