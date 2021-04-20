/*
 *  grid_layer.h
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

#ifndef GRID_LAYER_H
#define GRID_LAYER_H

// Includes from spatial:
#include "layer.h"

namespace nest
{

/** Layer with neurons placed in a grid
 */
template < int D >
class GridLayer : public Layer< D >
{
public:
  typedef Position< D > key_type;
  typedef index mapped_type;
  typedef std::pair< Position< D >, index > value_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;

  /**
   * Iterator iterating over the nodes inside a Mask.
   */
  class masked_iterator
  {
  public:
    /**
     * Constructor for an invalid iterator
     */
    masked_iterator( const GridLayer< D >& layer )
      : layer_( layer )
      , node_()
      , mask_()
      , layer_size_()
    {
    }

    /**
     * Initialise an iterator to point to the first node inside the mask.
     */
    masked_iterator( const GridLayer< D >& layer, const Mask< D >& mask, const Position< D >& anchor );

    value_type operator*();

    /**
     * Move the iterator to the next node within the mask. May cause the
     * iterator to become invalid if there are no more nodes.
     */
    masked_iterator& operator++();

    /**
     * Postfix increment operator.
     */
    masked_iterator operator++( int )
    {
      masked_iterator tmp = *this;
      ++*this;
      return tmp;
    }

    /**
     * Iterators are equal if they point to the same node in the same layer.
     */
    bool operator==( const masked_iterator& other ) const
    {
      return ( other.layer_.get_metadata() == layer_.get_metadata() ) && ( other.node_ == node_ );
    }
    bool operator!=( const masked_iterator& other ) const
    {
      return ( other.layer_.get_metadata() != layer_.get_metadata() ) || ( other.node_ != node_ );
    }

  protected:
    const GridLayer< D >& layer_;
    int layer_size_;
    const Mask< D >* mask_;
    Position< D > anchor_;
    MultiIndex< D > node_;
  };

  GridLayer()
    : Layer< D >()
  {
  }

  GridLayer( const GridLayer& layer )
    : Layer< D >( layer )
    , dims_( layer.dims_ )
  {
  }

  /**
   * Get position of node. Only possible for local nodes.
   * @param sind index of node
   * @returns position of node.
   */
  Position< D > get_position( index sind ) const;

  /**
   * Get position of node. Also allowed for non-local nodes.
   * @param lid local index of node
   * @returns position of node.
   */
  Position< D > lid_to_position( index lid ) const;

  index gridpos_to_lid( Position< D, int > pos ) const;

  Position< D > gridpos_to_position( Position< D, int > gridpos ) const;

  using Layer< D >::get_global_positions_vector;

  std::vector< std::pair< Position< D >, index > > get_global_positions_vector( const AbstractMask& mask,
    const Position< D >& anchor,
    bool allow_oversized,
    NodeCollectionPTR node_collection );

  masked_iterator masked_begin( const Mask< D >& mask, const Position< D >& anchor );
  masked_iterator masked_end();

  Position< D, index > get_dims() const;

  void set_status( const DictionaryDatum& d );
  void get_status( DictionaryDatum& d ) const;

protected:
  Position< D, index > dims_; ///< number of nodes in each direction.

  template < class Ins >
  void insert_global_positions_( Ins iter, NodeCollectionPTR node_collection );
  void insert_global_positions_ntree_( Ntree< D, index >& tree, NodeCollectionPTR node_collection );
  void insert_global_positions_vector_( std::vector< std::pair< Position< D >, index > >& vec,
    NodeCollectionPTR node_collection );
};

template < int D >
Position< D, index >
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

  index new_size = 1;
  for ( int i = 0; i < D; ++i )
  {
    new_size *= new_dims[ i ];

    this->dims_[ i ] = static_cast< index >( new_dims[ i ] );
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
GridLayer< D >::get_status( DictionaryDatum& d ) const
{
  Layer< D >::get_status( d );

  ( *d )[ names::shape ] = std::vector< index >( dims_.get_vector() );
}

template < int D >
Position< D >
GridLayer< D >::lid_to_position( index lid ) const
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
GridLayer< D >::get_position( index sind ) const
{
  return lid_to_position( sind );
}

template < int D >
index
GridLayer< D >::gridpos_to_lid( Position< D, int > pos ) const
{
  index lid = 0;

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
  index i = 0;
  index lid_end = node_collection->size();

  NodeCollection::const_iterator gi = node_collection->begin();

  for ( ; ( gi < node_collection->end() ) && ( i < lid_end ); ++gi, ++i )
  {
    *iter++ = std::pair< Position< D >, index >( lid_to_position( i ), ( *gi ).node_id );
  }
}

template < int D >
void
GridLayer< D >::insert_global_positions_ntree_( Ntree< D, index >& tree, NodeCollectionPTR node_collection )
{
  insert_global_positions_( std::inserter( tree, tree.end() ), node_collection );
}

template < int D >
void
GridLayer< D >::insert_global_positions_vector_( std::vector< std::pair< Position< D >, index > >& vec,
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
        index( std::max(
          ceil( ( bbox.lower_left[ i ] - layer.lower_left_[ i ] ) * layer_.dims_[ i ] / layer.extent_[ i ] - 0.5 ),
          0.0 ) ),
        layer.dims_[ i ] );
      upper_right[ i ] = std::min(
        index( std::max(
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
inline std::pair< Position< D >, index > GridLayer< D >::masked_iterator::operator*()
{
  return std::pair< Position< D >, index >(
    layer_.gridpos_to_position( node_ ), layer_.node_collection_->operator[]( layer_.gridpos_to_lid( node_ ) ) );
}

template < int D >
typename GridLayer< D >::masked_iterator& GridLayer< D >::masked_iterator::operator++()
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
std::vector< std::pair< Position< D >, index > >
GridLayer< D >::get_global_positions_vector( const AbstractMask& mask,
  const Position< D >& anchor,
  bool,
  NodeCollectionPTR )
{
  std::vector< std::pair< Position< D >, index > > positions;

  const Mask< D >& mask_d = dynamic_cast< const Mask< D >& >( mask );
  for ( typename GridLayer< D >::masked_iterator mi = masked_begin( mask_d, anchor ); mi != masked_end(); ++mi )
  {
    positions.push_back( *mi );
  }

  return positions;
}

} // namespace nest

#endif
