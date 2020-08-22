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

namespace nest
{
/**
 * Mask defined in terms of grid points rather than spacial
 * coordinates. Only suitable for grid layers.
 */
template < int D >
class GridMask : public AbstractMask
{
public:
  /**
   * Parameters:
   * shape - size in grid coordinates
             (length 2 for 2D layers or length 3 for 3D layers)
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
  std::vector< long > shape = getValue< std::vector< long > >( d, names::shape );

  if ( D == 2 )
  {
    lower_right_ = Position< D, int >( shape[ 0 ], shape[ 1 ] );
  }
  else if ( D == 3 )
  {
    lower_right_ = Position< D, int >( shape[ 0 ], shape[ 1 ], shape[ 2 ] );
  }
  else
  {
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

  long shape_x = lower_right_[ 0 ] - upper_left_[ 0 ];
  long shape_y = lower_right_[ 1 ] - upper_left_[ 1 ];
  std::vector< long > shape_dim{ shape_x, shape_y };

  if ( D == 3 )
  {
    long shape_z = lower_right_[ 2 ] - upper_left_[ 2 ];
    shape_dim.push_back( shape_z );
  }
  def< std::vector< long > >( maskd, names::shape, shape_dim );

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
