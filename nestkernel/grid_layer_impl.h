/*
 *  grid_layer_impl.h
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

#ifndef GRID_LAYER_IMPL_H
#define GRID_LAYER_IMPL_H

// Includes from spatial:
#include "grid_layer.h"

namespace nest
{


template < int D >
Position< D, size_t >
GridLayer< D >::get_dims() const
{
  return dims_;
}

template < int D >
void
GridLayer< D >::set_status( const DictionaryDatum& d )
{
  std::vector< long > new_dims( D );

  updateValue< std::vector< long > >( d, names::shape, new_dims );

  size_t new_size = 1;
  for ( int i = 0; i < D; ++i )
  {
    new_size *= new_dims[ i ];

    this->dims_[ i ] = static_cast< size_t >( new_dims[ i ] );
  }

  if ( new_size != this->node_collection_->size() )
  {
    throw BadProperty( "Total size of layer must be unchanged." );
  }

  if ( d->known( names::extent ) )
  {
    Position< D > center = this->get_center();
    this->extent_ = getValue< std::vector< double > >( d, names::extent );
    this->lower_left_ = center - this->extent_ / 2;
  }
  if ( d->known( names::center ) )
  {
    this->lower_left_ = getValue< std::vector< double > >( d, names::center );
    this->lower_left_ -= this->extent_ / 2;
  }

  Layer< D >::set_status( d );
}

template < int D >
void
GridLayer< D >::get_status( DictionaryDatum& d, NodeCollection const* nc ) const
{
  Layer< D >::get_status( d, nc );

  ( *d )[ names::shape ] = std::vector< size_t >( dims_.get_vector() );
}

template < int D >
Position< D >
GridLayer< D >::lid_to_position( size_t lid ) const
{
  Position< D, int > gridpos;
  for ( int i = D - 1; i > 0; --i )
  {
    gridpos[ i ] = lid % dims_[ i ];
    lid = lid / dims_[ i ];
  }
  assert( lid < dims_[ 0 ] );
  gridpos[ 0 ] = lid;
  return gridpos_to_position( gridpos );
}

template < int D >
Position< D >
GridLayer< D >::gridpos_to_position( Position< D, int > gridpos ) const
{
  // grid layer uses "matrix convention", i.e. reversed y axis
  Position< D > ext = this->extent_;
  Position< D > upper_left = this->lower_left_;
  if ( D > 1 )
  {
    upper_left[ 1 ] += ext[ 1 ];
    ext[ 1 ] = -ext[ 1 ];
  }
  return upper_left + ext / dims_ * gridpos + ext / dims_ * 0.5;
}

template < int D >
Position< D >
GridLayer< D >::get_position( size_t lid ) const
{
  return lid_to_position( lid );
}

template < int D >
size_t
GridLayer< D >::gridpos_to_lid( Position< D, int > pos ) const
{
  size_t lid = 0;

  // In case of periodic boundaries, allow grid positions outside layer
  for ( int i = 0; i < D; ++i )
  {
    if ( this->periodic_[ i ] )
    {
      pos[ i ] %= int( dims_[ i ] );
      if ( pos[ i ] < 0 )
      {
        pos[ i ] += dims_[ i ];
      }
    }
  }

  for ( int i = 0; i < D; ++i )
  {
    lid *= dims_[ i ];
    lid += pos[ i ];
  }

  return lid;
}

template < int D >
template < class Ins >
void
GridLayer< D >::insert_global_positions_( Ins iter, NodeCollectionPTR node_collection )
{
  for ( auto gi = node_collection->begin(); gi < node_collection->end(); ++gi )
  {
    const auto triple = *gi;
    *iter++ = std::pair< Position< D >, size_t >( lid_to_position( triple.nc_index ), triple.node_id );
  }
}

template < int D >
void
GridLayer< D >::insert_global_positions_ntree_( Ntree< D, size_t >& tree, NodeCollectionPTR node_collection )
{
  insert_global_positions_( std::inserter( tree, tree.end() ), node_collection );
}

template < int D >
void
GridLayer< D >::insert_global_positions_vector_( std::vector< std::pair< Position< D >, size_t > >& vec,
  NodeCollectionPTR node_collection )
{
  insert_global_positions_( std::back_inserter( vec ), node_collection );
}

template < int D >
inline typename GridLayer< D >::masked_iterator
GridLayer< D >::masked_begin( const Mask< D >& mask, const Position< D >& anchor )
{
  return masked_iterator( *this, mask, anchor );
}

template < int D >
inline typename GridLayer< D >::masked_iterator
GridLayer< D >::masked_end()
{
  return masked_iterator( *this );
}

template < int D >
GridLayer< D >::masked_iterator::masked_iterator( const GridLayer< D >& layer,
  const Mask< D >& mask,
  const Position< D >& anchor )
  : layer_( layer )
  , mask_( &mask )
  , anchor_( anchor )
{
  layer_size_ = layer.global_size();

  Position< D, int > lower_left;
  Position< D, int > upper_right;
  Box< D > bbox = mask.get_bbox();
  bbox.lower_left += anchor;
  bbox.upper_right += anchor;
  for ( int i = 0; i < D; ++i )
  {
    if ( layer.periodic_[ i ] )
    {
      lower_left[ i ] =
        ceil( ( bbox.lower_left[ i ] - layer.lower_left_[ i ] ) * layer_.dims_[ i ] / layer.extent_[ i ] - 0.5 );
      upper_right[ i ] =
        round( ( bbox.upper_right[ i ] - layer.lower_left_[ i ] ) * layer_.dims_[ i ] / layer.extent_[ i ] );
    }
    else
    {
      lower_left[ i ] = std::min(
        size_t( std::max(
          ceil( ( bbox.lower_left[ i ] - layer.lower_left_[ i ] ) * layer_.dims_[ i ] / layer.extent_[ i ] - 0.5 ),
          0.0 ) ),
        layer.dims_[ i ] );
      upper_right[ i ] = std::min(
        size_t( std::max(
          round( ( bbox.upper_right[ i ] - layer.lower_left_[ i ] ) * layer_.dims_[ i ] / layer.extent_[ i ] ), 0.0 ) ),
        layer.dims_[ i ] );
    }
  }
  if ( D > 1 )
  {
    // grid layer uses "matrix convention", i.e. reversed y axis
    int tmp = lower_left[ 1 ];
    lower_left[ 1 ] = layer.dims_[ 1 ] - upper_right[ 1 ];
    upper_right[ 1 ] = layer.dims_[ 1 ] - tmp;
  }

  node_ = MultiIndex< D >( lower_left, upper_right );

  if ( not mask_->inside( layer_.gridpos_to_position( node_ ) - anchor_ ) )
  {
    ++( *this );
  }
}

template < int D >
inline std::pair< Position< D >, size_t >
GridLayer< D >::masked_iterator::operator*()
{
  return std::pair< Position< D >, size_t >(
    layer_.gridpos_to_position( node_ ), layer_.node_collection_->operator[]( layer_.gridpos_to_lid( node_ ) ) );
}

template < int D >
typename GridLayer< D >::masked_iterator&
GridLayer< D >::masked_iterator::operator++()
{
  do
  {
    ++node_;

    if ( node_ == node_.get_upper_right() )
    {
      // Mark as invalid
      node_ = MultiIndex< D >();
      return *this;
    }

  } while ( not mask_->inside( layer_.gridpos_to_position( node_ ) - anchor_ ) );

  return *this;
}

template < int D >
std::vector< std::pair< Position< D >, size_t > >
GridLayer< D >::get_global_positions_vector( const AbstractMask& mask,
  const Position< D >& anchor,
  bool,
  NodeCollectionPTR )
{
  std::vector< std::pair< Position< D >, size_t > > positions;

  const Mask< D >& mask_d = dynamic_cast< const Mask< D >& >( mask );
  for ( typename GridLayer< D >::masked_iterator mi = masked_begin( mask_d, anchor ); mi != masked_end(); ++mi )
  {
    positions.push_back( *mi );
  }

  return positions;
}


} // namespace nest

#endif
