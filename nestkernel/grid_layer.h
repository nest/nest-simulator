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

/**
 * Layer with neurons placed in a grid
 */
template < int D >
class GridLayer : public Layer< D >
{
public:
  typedef Position< D > key_type;
  typedef size_t mapped_type;
  typedef std::pair< Position< D >, size_t > value_type;
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
    masked_iterator
    operator++( int )
    {
      masked_iterator tmp = *this;
      ++*this;
      return tmp;
    }

    /**
     * Iterators are equal if they point to the same node in the same layer.
     */
    bool
    operator==( const masked_iterator& other ) const
    {
      return ( other.layer_.get_metadata() == layer_.get_metadata() ) and ( other.node_ == node_ );
    }
    bool
    operator!=( const masked_iterator& other ) const
    {
      return ( other.layer_.get_metadata() != layer_.get_metadata() ) or ( other.node_ != node_ );
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
   *
   * @param sind index of node
   * @returns position of node.
   */
  Position< D > get_position( size_t sind ) const override;

  /**
   * Get position of node. Also allowed for non-local nodes.
   *
   * @param lid global index of node within layer
   * @returns position of node.
   */
  Position< D > lid_to_position( size_t lid ) const;

  size_t gridpos_to_lid( Position< D, int > pos ) const;

  Position< D > gridpos_to_position( Position< D, int > gridpos ) const;

  using Layer< D >::get_global_positions_vector;

  std::vector< std::pair< Position< D >, size_t > > get_global_positions_vector( const AbstractMask& mask,
    const Position< D >& anchor,
    bool allow_oversized,
    NodeCollectionPTR node_collection );

  masked_iterator masked_begin( const Mask< D >& mask, const Position< D >& anchor );
  masked_iterator masked_end();

  Position< D, size_t > get_dims() const;

  void set_status( const DictionaryDatum& d ) override;
  void get_status( DictionaryDatum& d, NodeCollection const* ) const override;

protected:
  Position< D, size_t > dims_; ///< number of nodes in each direction.

  template < class Ins >
  void insert_global_positions_( Ins iter, NodeCollectionPTR node_collection );
  void insert_global_positions_ntree_( Ntree< D, size_t >& tree, NodeCollectionPTR node_collection ) override;
  void insert_global_positions_vector_( std::vector< std::pair< Position< D >, size_t > >& vec,
    NodeCollectionPTR node_collection ) override;
};

} // namespace nest

#endif
