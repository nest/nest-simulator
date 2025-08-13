/*
 *  layer_impl.h
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
#include "grid_layer.h"
#include "layer.h"

namespace nest
{

template < int D >
void
MaskedLayer< D >::check_mask_( Layer< D >& layer, bool allow_oversized )
{
  if ( not mask_.get() )
  {
    mask_ = new AllMask< D >();
    return;
  }

  try // Try to cast to GridMask
  {
    const GridMask< D >& grid_mask = dynamic_cast< const GridMask< D >& >( *mask_ );

    // If the above cast succeeds, then this is a grid mask

    GridLayer< D >* grid_layer = dynamic_cast< GridLayer< D >* >( &layer );
    if ( grid_layer == 0 )
    {
      throw BadProperty( "Grid masks can only be used with grid layers." );
    }

    Position< D > ext = grid_layer->get_extent();
    Position< D, size_t > dims = grid_layer->get_dims();

    if ( not allow_oversized )
    {
      bool oversize = false;
      for ( int i = 0; i < D; ++i )
      {
        oversize |= layer.get_periodic_mask()[ i ]
          and ( grid_mask.get_lower_right()[ i ] - grid_mask.get_upper_left()[ i ] ) > static_cast< int >( dims[ i ] );
      }
      if ( oversize )
      {
        throw BadProperty(
          "Mask size must not exceed layer size; set allow_oversized_mask to "
          "override." );
      }
    }

    Position< D > lower_left = ext / dims * grid_mask.get_upper_left() - ext / dims * 0.5;
    Position< D > upper_right = ext / dims * grid_mask.get_lower_right() - ext / dims * 0.5;

    double y = lower_left[ 1 ];
    lower_left[ 1 ] = -upper_right[ 1 ];
    upper_right[ 1 ] = -y;

    mask_ = new BoxMask< D >( lower_left, upper_right );
  }
  catch ( std::bad_cast& )
  {

    // Not a grid mask

    try // Try to cast to correct dimension Mask
    {
      const Mask< D >& mask = dynamic_cast< const Mask< D >& >( *mask_ );

      if ( not allow_oversized )
      {
        const Box< D > bb = mask.get_bbox();
        bool oversize = false;
        for ( int i = 0; i < D; ++i )
        {
          oversize |=
            layer.get_periodic_mask()[ i ] and bb.upper_right[ i ] - bb.lower_left[ i ] > layer.get_extent()[ i ];
        }
        if ( oversize )
        {
          throw BadProperty(
            "Mask size must not exceed layer size; set allow_oversized_mask to "
            "override." );
        }
      }
    }
    catch ( std::bad_cast& )
    {
      throw BadProperty( "Mask is incompatible with layer." );
    }
  }
}

} // namespace nest
