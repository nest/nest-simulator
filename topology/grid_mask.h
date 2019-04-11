/*
 *  grid_mask.h
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

#ifndef GRID_MASK_H
#define GRID_MASK_H

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

// Includes from topology:
#include "mask.h"
#include "position.h"
#include "topology_names.h"
#include "topologymodule.h"

namespace nest {
/**
 * Mask defined in terms of grid points rather than spacial
 * coordinates. Only suitable for grid layers.
 */
template < int D >
class GridMask : public AbstractMask {
public:
  /**
   * Parameters:
   * columns - horizontal size in grid coordinates
   * rows    - vertical size in grid coordinates
   * layers  - z-size in grid coordinates (for 3D layers)
   */
  GridMask( const DictionaryDatum& d );

  bool
  inside( const std::vector< double >& ) const
  {
    throw KernelException( "Grid mask must be applied to a grid layer." );
  }

  void set_anchor( const Position< D, int >& );

  DictionaryDatum get_dict() const;

  GridMask< D >*
  clone() const
  {
    return new GridMask( *this );
  }

  /**
   * @returns the name of this mask type.
   */
  static Name get_name();

  AbstractMask*
  intersect_mask( const AbstractMask& ) const
  {
    throw KernelException( "Grid masks can not be combined." );
  }

  AbstractMask*
  union_mask( const AbstractMask& ) const
  {
    throw KernelException( "Grid masks can not be combined." );
  }

  AbstractMask*
  minus_mask( const AbstractMask& ) const
  {
    throw KernelException( "Grid masks can not be combined." );
  }

  Position< D, int >
  get_upper_left() const
  {
    return upper_left_;
  }

  Position< D, int >
  get_lower_right() const
  {
    return lower_right_;
  }

protected:
  Position< D, int > upper_left_;
  Position< D, int > lower_right_;
};

template < int D >
GridMask< D >::GridMask( const DictionaryDatum& d )
{
  int columns = getValue< long >( d, names::columns );
  int rows = getValue< long >( d, names::rows );
  if ( D == 3 ) {
    int layers = getValue< long >( d, names::layers );
    lower_right_ = Position< D, int >( columns, rows, layers );
  }
  else if ( D == 2 ) {
    lower_right_ = Position< D, int >( columns, rows );
  }
  else {
    throw BadProperty( "Grid mask must be 2- or 3-dimensional." );
  }
}

template <>
inline Name
GridMask< 2 >::get_name()
{
  return names::grid;
}

template <>
inline Name
GridMask< 3 >::get_name()
{
  return names::grid3d;
}

template < int D >
DictionaryDatum
GridMask< D >::get_dict() const
{
  DictionaryDatum d( new Dictionary );
  DictionaryDatum maskd( new Dictionary );
  def< DictionaryDatum >( d, get_name(), maskd );
  def< long >( maskd, names::columns, lower_right_[ 0 ] - upper_left_[ 0 ] );
  def< long >( maskd, names::rows, lower_right_[ 1 ] - upper_left_[ 1 ] );
  if ( D >= 3 ) {
    def< long >( maskd, names::layers, lower_right_[ 2 ] - upper_left_[ 2 ] );
  }
  return d;
}

template < int D >
void
GridMask< D >::set_anchor( const Position< D, int >& anchor )
{
  lower_right_ = lower_right_ - upper_left_ - anchor;
  upper_left_ = -anchor;
}

} // namespace nest

#endif
