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

// Includes from topology:
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
      , depth_( -1 )
    {
    }

    /**
     * Initialize an iterator to point to the first node inside the mask.
     */
    masked_iterator( const GridLayer< D >& layer,
      const Mask< D >& mask,
      const Position< D >& anchor,
      const Selector& filter );

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
      return ( other.layer_.get_gid() == layer_.get_gid() )
        && ( other.node_ == node_ ) && ( other.depth_ == depth_ );
    }
    bool operator!=( const masked_iterator& other ) const
    {
      return ( other.layer_.get_gid() != layer_.get_gid() )
        || ( other.node_ != node_ ) || ( other.depth_ != depth_ );
    }

  protected:
    const GridLayer< D >& layer_;
    int layer_size_;
    const Mask< D >* mask_;
    Position< D > anchor_;
    Selector filter_;
    MultiIndex< D > node_;
    int depth_;
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
   * @param sind subnet index of node
   * @returns position of node identified by Subnet local index value.
   */
  Position< D > get_position( index sind ) const;

  /**
   * Get position of node. Also allowed for non-local nodes.
   * @param lid local index of node
   * @returns position of node identified by Subnet local index value.
   */
  Position< D > lid_to_position( index lid ) const;

  index gridpos_to_lid( Position< D, int > pos ) const;

  Position< D > gridpos_to_position( Position< D, int > gridpos ) const;

  /**
   * Returns nodes at a given discrete layerspace position.
   * @param pos  Discrete position in layerspace.
   * @returns vector of gids at the depth column covering
   *          the input position.
   */
  std::vector< index > get_nodes( Position< D, int > pos );

  using Layer< D >::get_global_positions_vector;

  std::vector< std::pair< Position< D >, index > > get_global_positions_vector(
    Selector filter,
    const AbstractMask& mask,
    const Position< D >& anchor,
    bool allow_oversized );

  masked_iterator masked_begin( const Mask< D >& mask,
    const Position< D >& anchor,
    const Selector& filter );
  masked_iterator masked_end();

  Position< D, index > get_dims() const;

  void set_status( const DictionaryDatum& d );
  void get_status( DictionaryDatum& d ) const;

protected:
  Position< D, index > dims_; ///< number of nodes in each direction.

  template < class Ins >
  void insert_global_positions_( Ins iter, const Selector& filter );
  void insert_global_positions_ntree_( Ntree< D, index >& tree,
    const Selector& filter );
  void insert_global_positions_vector_(
    std::vector< std::pair< Position< D >, index > >& vec,
    const Selector& filter );
  void insert_local_positions_ntree_( Ntree< D, index >& tree,
    const Selector& filter );
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
  Position< D, index > new_dims = dims_;
  updateValue< long >( d, names::columns, new_dims[ 0 ] );
  if ( D >= 2 )
  {
    updateValue< long >( d, names::rows, new_dims[ 1 ] );
  }
  if ( D >= 3 )
  {
    updateValue< long >( d, names::layers, new_dims[ 2 ] );
  }

  index new_size = this->depth_;
  for ( int i = 0; i < D; ++i )
  {
    new_size *= new_dims[ i ];
  }

  if ( new_size != this->global_size() )
  {
    throw BadProperty( "Total size of layer must be unchanged." );
  }

  this->dims_ = new_dims;

  Layer< D >::set_status( d );
}

template < int D >
void
GridLayer< D >::get_status( DictionaryDatum& d ) const
{
  Layer< D >::get_status( d );

  DictionaryDatum topology_dict =
    getValue< DictionaryDatum >( ( *d )[ names::topology ] );

  ( *topology_dict )[ names::columns ] = dims_[ 0 ];
  if ( D >= 2 )
  {
    ( *topology_dict )[ names::rows ] = dims_[ 1 ];
  }
  if ( D >= 3 )
  {
    ( *topology_dict )[ names::layers ] = dims_[ 2 ];
  }
}

template < int D >
Position< D >
GridLayer< D >::lid_to_position( index lid ) const
{
  lid %= this->global_size() / this->depth_;
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
  return lid_to_position( this->nodes_[ sind ]->get_lid() );
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
std::vector< index >
GridLayer< D >::get_nodes( Position< D, int > pos )
{
  std::vector< index > gids;
  index lid = gridpos_to_lid( pos );
  index layer_size = this->global_size() / this->depth_;

  for ( int d = 0; d < this->depth_; ++d )
  {
    gids.push_back( this->gids_[ lid + d * layer_size ] );
  }

  return gids;
}

template < int D >
void
GridLayer< D >::insert_local_positions_ntree_( Ntree< D, index >& tree,
  const Selector& filter )
{
  std::vector< Node* >::const_iterator nodes_begin;
  std::vector< Node* >::const_iterator nodes_end;

  if ( filter.select_depth() )
  {
    nodes_begin = this->local_begin( filter.depth );
    nodes_end = this->local_end( filter.depth );
  }
  else
  {
    nodes_begin = this->local_begin();
    nodes_end = this->local_end();
  }

  for ( std::vector< Node* >::const_iterator node_it = nodes_begin;
        node_it != nodes_end;
        ++node_it )
  {

    if ( filter.select_model()
      && ( ( *node_it )->get_model_id() != filter.model ) )
    {
      continue;
    }

    tree.insert( std::pair< Position< D >, index >(
      lid_to_position( ( *node_it )->get_lid() ), ( *node_it )->get_gid() ) );
  }
}

template < int D >
template < class Ins >
void
GridLayer< D >::insert_global_positions_( Ins iter, const Selector& filter )
{
  index i = 0;
  index lid_end = this->gids_.size();

  if ( filter.select_depth() )
  {
    const index nodes_per_layer = this->gids_.size() / this->depth_;
    i = nodes_per_layer * filter.depth;
    lid_end = nodes_per_layer * ( filter.depth + 1 );
    if ( ( i >= this->gids_.size() ) or ( lid_end > this->gids_.size() ) )
    {
      throw BadProperty( "Selected depth out of range" );
    }
  }

  Multirange::iterator gi = this->gids_.begin();
  // Advance iterator to first gid at selected depth
  for ( index j = 0; j < i; ++j )
  {
    ++gi;
  }

  for ( ; ( gi != this->gids_.end() ) && ( i < lid_end ); ++gi, ++i )
  {

    if ( filter.select_model()
      && ( ( int ) kernel().modelrange_manager.get_model_id( *gi )
           != filter.model ) )
    {
      continue;
    }
    *iter++ = std::pair< Position< D >, index >( lid_to_position( i ), *gi );
  }
}

template < int D >
void
GridLayer< D >::insert_global_positions_ntree_( Ntree< D, index >& tree,
  const Selector& filter )
{
  insert_global_positions_( std::inserter( tree, tree.end() ), filter );
}

template < int D >
void
GridLayer< D >::insert_global_positions_vector_(
  std::vector< std::pair< Position< D >, index > >& vec,
  const Selector& filter )
{
  insert_global_positions_( std::back_inserter( vec ), filter );
}

template < int D >
inline typename GridLayer< D >::masked_iterator
GridLayer< D >::masked_begin( const Mask< D >& mask,
  const Position< D >& anchor,
  const Selector& filter )
{
  return masked_iterator( *this, mask, anchor, filter );
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
  const Position< D >& anchor,
  const Selector& filter )
  : layer_( layer )
  , mask_( &mask )
  , anchor_( anchor )
  , filter_( filter )
{
  layer_size_ = layer.global_size() / layer.depth_;

  Position< D, int > lower_left;
  Position< D, int > upper_right;
  Box< D > bbox = mask.get_bbox();
  bbox.lower_left += anchor;
  bbox.upper_right += anchor;
  for ( int i = 0; i < D; ++i )
  {
    if ( layer.periodic_[ i ] )
    {
      lower_left[ i ] = ceil( ( bbox.lower_left[ i ] - layer.lower_left_[ i ] )
          * layer_.dims_[ i ] / layer.extent_[ i ]
        - 0.5 );
      upper_right[ i ] =
        round( ( bbox.upper_right[ i ] - layer.lower_left_[ i ] )
          * layer_.dims_[ i ] / layer.extent_[ i ] );
    }
    else
    {
      lower_left[ i ] = std::min(
        index( std::max( ceil( ( bbox.lower_left[ i ] - layer.lower_left_[ i ] )
                             * layer_.dims_[ i ] / layer.extent_[ i ]
                           - 0.5 ),
          0.0 ) ),
        layer.dims_[ i ] );
      upper_right[ i ] = std::min(
        index(
          std::max( round( ( bbox.upper_right[ i ] - layer.lower_left_[ i ] )
                      * layer_.dims_[ i ] / layer.extent_[ i ] ),
            0.0 ) ),
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

  if ( filter_.select_depth() )
  {
    depth_ = filter_.depth;
  }
  else
  {
    depth_ = 0;
  }

  if ( ( not mask_->inside( layer_.gridpos_to_position( node_ ) - anchor_ ) )
    or ( filter_.select_model() && ( kernel().modelrange_manager.get_model_id(
                                       layer_.gids_[ depth_ * layer_size_ ] )
                                     != index( filter_.model ) ) ) )
  {
    ++( *this );
  }
}

template < int D >
inline std::pair< Position< D >, index > GridLayer< D >::masked_iterator::
operator*()
{
  assert( depth_ >= 0 );
  return std::pair< Position< D >, index >( layer_.gridpos_to_position( node_ ),
    layer_.gids_[ layer_.gridpos_to_lid( node_ ) + depth_ * layer_size_ ] );
}

template < int D >
typename GridLayer< D >::masked_iterator& GridLayer< D >::masked_iterator::
operator++()
{
  if ( depth_ == -1 )
  {
    return *this;
  } // Invalid (end) iterator

  if ( not filter_.select_depth() )
  {
    depth_++;
    if ( depth_ >= layer_.depth_ )
    {
      depth_ = 0;
    }
    else
    {
      if ( filter_.select_model() && ( kernel().modelrange_manager.get_model_id(
                                         layer_.gids_[ depth_ * layer_size_ ] )
                                       != index( filter_.model ) ) )
      {
        return operator++();
      }
      else
      {
        return *this;
      }
    }
  }

  do
  {
    ++node_;

    if ( node_ == node_.get_upper_right() )
    {
      // Mark as invalid
      depth_ = -1;
      node_ = MultiIndex< D >();
      return *this;
    }

  } while (
    not mask_->inside( layer_.gridpos_to_position( node_ ) - anchor_ ) );

  if ( filter_.select_model()
    && ( kernel().modelrange_manager.get_model_id(
           layer_.gids_[ depth_ * layer_size_ ] ) != index( filter_.model ) ) )
  {
    return operator++();
  }

  return *this;
}

template < int D >
std::vector< std::pair< Position< D >, index > >
GridLayer< D >::get_global_positions_vector( Selector filter,
  const AbstractMask& mask,
  const Position< D >& anchor,
  bool )
{
  std::vector< std::pair< Position< D >, index > > positions;

  const Mask< D >& mask_d = dynamic_cast< const Mask< D >& >( mask );
  for ( typename GridLayer< D >::masked_iterator mi =
          masked_begin( mask_d, anchor, filter );
        mi != masked_end();
        ++mi )
  {
    positions.push_back( *mi );
  }

  return positions;
}

} // namespace nest

#endif
