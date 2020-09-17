/*
 *  ntree.h
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

#ifndef NTREE_H
#define NTREE_H

// C++ includes:
#include <bitset>
#include <utility>
#include <vector>
#include <iterator>

// Includes from spatial:
#include "position.h"

namespace nest
{

class AbstractMask;

template < int D >
class Mask;

/**
 * A Ntree object represents a subtree or leaf in a Ntree structure. Any
 * ntree covers a specific region in space. A leaf ntree contains a list
 * of items and their corresponding positions. A branch ntree contains a
 * list of N=1<<D other ntrees, each covering a region corresponding to the
 * upper-left, lower-left, upper-right and lower-left corner of their
 * mother ntree.
 *
 */
template < int D, class T, int max_capacity = 100, int max_depth = 10 >
class Ntree
{
public:
  static const int N = 1 << D;

  typedef Position< D > key_type;
  typedef T mapped_type;
  typedef std::pair< Position< D >, T > value_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;

  /**
   * Iterator iterating the nodes in a Quadtree.
   */
  class iterator
  {
  public:
    /**
     * Initialize an invalid iterator.
     */
    iterator()
      : ntree_( 0 )
      , top_( 0 )
      , node_( 0 )
    {
    }

    /**
     * Initialize an iterator to point to the first node in the first
     * non-empty leaf within the tree below this Ntree.
     */
    iterator( Ntree& q );

    /**
     * Initialize an iterator to point to the nth node in this Ntree,
     * which must be a leaf. The top of the tree is the first ancestor of
     * the Ntree.
     */
    iterator( Ntree& q, index n );

    value_type& operator*()
    {
      return ntree_->nodes_[ node_ ];
    }
    value_type* operator->()
    {
      return &ntree_->nodes_[ node_ ];
    }

    /**
     * Move the iterator to the next node within the tree. May cause the
     * iterator to become invalid if there are no more nodes.
     */
    iterator& operator++();

    /**
     * Postfix increment operator.
     */
    iterator operator++( int )
    {
      iterator tmp = *this;
      ++*this;
      return tmp;
    }

    /**
     * Iterators are equal if they point to the same node in the same
     * ntree.
     */
    bool operator==( const iterator& other ) const
    {
      return ( other.ntree_ == ntree_ ) && ( other.node_ == node_ );
    }
    bool operator!=( const iterator& other ) const
    {
      return ( other.ntree_ != ntree_ ) || ( other.node_ != node_ );
    }

  protected:
    /**
     * Move to the next leaf quadrant, or set ntree_ to 0 if there are no
     * more leaves.
    */
    void next_leaf_();

    Ntree* ntree_;
    Ntree* top_;
    index node_;
  };

  /**
   * Iterator iterating the nodes in a Quadtree inside a Mask.
   */
  class masked_iterator
  {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair< Position< D >, T >;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = long int;
    /**
     * Initialize an invalid iterator.
     */
    masked_iterator()
      : ntree_( 0 )
      , top_( 0 )
      , allin_top_( 0 )
      , node_( 0 )
      , mask_( 0 )
    {
    }

    /**
     * Initialize an iterator to point to the first leaf node inside the
     * mask within the tree below this Ntree.
     */
    masked_iterator( Ntree& q, const Mask< D >& mask, const Position< D >& anchor );

    value_type& operator*()
    {
      return ntree_->nodes_[ node_ ];
    }
    value_type* operator->()
    {
      return &ntree_->nodes_[ node_ ];
    }

    /**
     * Move the iterator to the next node inside the mask within the
     * tree. May cause the iterator to become invalid if there are no
     * more nodes.
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
     * Iterators are equal if they point to the same node in the same
     * ntree.
     */
    bool operator==( const masked_iterator& other ) const
    {
      return ( other.ntree_ == ntree_ ) && ( other.node_ == node_ );
    }
    bool operator!=( const masked_iterator& other ) const
    {
      return ( other.ntree_ != ntree_ ) || ( other.node_ != node_ );
    }

  protected:
    /**
     * Initialize
     */
    void init_();

    /**
     * Find the next leaf which is not outside the mask.
     */
    void next_leaf_();

    /**
     * Find the first leaf which is not outside the mask. If no leaf is
     * found below the current quadrant, will continue to next_leaf_().
     */
    void first_leaf_();

    /**
     * Set the allin_top_ to the current quadrant, and find the first
     * leaf below the current quadrant.
     */
    void first_leaf_inside_();

    /**
     * Go to the next anchor image.
     */
    void next_anchor_();

    bool
    anchored_position_inside_mask( const Position< D >& position )
    {
      // Create anchored position in two steps to avoid creating a new Position object.
      anchored_position_ = position;
      anchored_position_ -= anchor_;
      return mask_->inside( anchored_position_ );
    }

    Ntree* ntree_;
    Ntree* top_;
    Ntree* allin_top_;
    index node_;
    const Mask< D >* mask_;
    Position< D > anchor_;
    Position< D > anchored_position_;
    std::vector< Position< D > > anchors_;
    index current_anchor_;
  };

  /**
   * Create a Ntree that covers the region defined by the two
   * input positions.
   * @param lower_left  Lower left corner of ntree.
   * @param extent      Size (width,height) of ntree.
   */
  Ntree( const Position< D >& lower_left,
    const Position< D >& extent,
    std::bitset< D > periodic = 0,
    Ntree* parent = 0,
    int subquad = 0 );

  /**
   * Delete Ntree recursively.
   */
  ~Ntree();

  /**
   * Traverse quadtree structure from current ntree.
   * Inserts node in correct leaf in quadtree.
   * @returns iterator pointing to inserted node.
   */
  iterator insert( Position< D > pos, const T& node );

  /**
   * std::multimap like insert method
   */
  iterator insert( const value_type& val );

  /**
   * STL container compatible insert method (the first argument is ignored)
   */
  iterator insert( iterator, const value_type& val );

  /**
   * @returns member nodes in ntree and their position.
   */
  std::vector< value_type > get_nodes();

  /**
   * Applies a Mask to this ntree.
   * @param mask    mask to apply.
   * @param anchor  position to center mask in.
   * @returns member nodes in ntree inside mask.
   */
  std::vector< value_type > get_nodes( const Mask< D >& mask, const Position< D >& anchor );

  /**
   * This function returns a node iterator which will traverse the
   * subtree below this Ntree.
   * @returns iterator for nodes in quadtree.
   */
  iterator
  begin()
  {
    return iterator( *this );
  }

  iterator
  end()
  {
    return iterator();
  }

  /**
   * This function returns a masked node iterator which will traverse the
   * subtree below this Ntree, skipping nodes outside the mask.
   * @returns iterator for nodes in quadtree.
   */
  masked_iterator
  masked_begin( const Mask< D >& mask, const Position< D >& anchor )
  {
    return masked_iterator( *this, mask, anchor );
  }

  masked_iterator
  masked_end()
  {
    return masked_iterator();
  }

  /**
   * @returns true if ntree is a leaf.
   */
  bool is_leaf() const;

protected:
  /**
   * Change a leaf ntree to a regular ntree with four
   * children regions.
   */
  void split_();

  /**
   * Append this ntree's nodes to the vector
   */
  void append_nodes_( std::vector< value_type >& );

  /**
   * Append this ntree's nodes inside the mask to the vector
   */
  void append_nodes_( std::vector< value_type >&, const Mask< D >&, const Position< D >& );

  /**
   * @returns the subquad number for this position
   */
  int subquad_( const Position< D >& );

  Position< D > lower_left_;
  Position< D > extent_;

  bool leaf_;

  std::vector< value_type > nodes_;

  Ntree* parent_;
  int my_subquad_; ///< This Ntree's subquad number within parent
  int my_depth_;   ///< This Ntree's depth in the tree
  Ntree* children_[ N ];
  std::bitset< D > periodic_; ///< periodic b.c.

  friend class iterator;
  friend class masked_iterator;
};

template < int D, class T, int max_capacity, int max_depth >
Ntree< D, T, max_capacity, max_depth >::Ntree( const Position< D >& lower_left,
  const Position< D >& extent,
  std::bitset< D > periodic,
  Ntree< D, T, max_capacity, max_depth >* parent,
  int subquad )
  : lower_left_( lower_left )
  , extent_( extent )
  , leaf_( true )
  , parent_( parent )
  , my_subquad_( subquad )
  , my_depth_( parent ? parent->my_depth_ + 1 : 0 )
  , periodic_( periodic )
{
}

template < int D, class T, int max_capacity, int max_depth >
Ntree< D, T, max_capacity, max_depth >::~Ntree()
{
  if ( leaf_ )
  {
    // if T is a vector class, we do not delete the pointees
    return;
  }

  for ( size_t n = 0; n < static_cast< size_t >( N ); ++n )
  {
    delete children_[ n ]; // calls destructor in child, thus recursing
  }
}

template < int D, class T, int max_capacity, int max_depth >
Ntree< D, T, max_capacity, max_depth >::iterator::iterator( Ntree& q, index n )
  : ntree_( &q )
  , top_( &q )
  , node_( n )
{
  assert( ntree_->leaf_ );

  // First ancestor
  while ( top_->parent_ )
  {
    top_ = top_->parent_;
  }
}

template < int D, class T, int max_capacity, int max_depth >
bool
Ntree< D, T, max_capacity, max_depth >::is_leaf() const
{
  return leaf_;
}


template < int D, class T, int max_capacity, int max_depth >
std::vector< std::pair< Position< D >, T > >
Ntree< D, T, max_capacity, max_depth >::get_nodes()
{
  std::vector< std::pair< Position< D >, T > > result;
  append_nodes_( result );
  return result;
}

template < int D, class T, int max_capacity, int max_depth >
std::vector< std::pair< Position< D >, T > >
Ntree< D, T, max_capacity, max_depth >::get_nodes( const Mask< D >& mask, const Position< D >& anchor )
{
  std::vector< std::pair< Position< D >, T > > result;
  append_nodes_( result, mask, anchor );
  return result;
}

template < int D, class T, int max_capacity, int max_depth >
typename Ntree< D, T, max_capacity, max_depth >::iterator
Ntree< D, T, max_capacity, max_depth >::insert( const std::pair< Position< D >, T >& val )
{
  return insert( val.first, val.second );
}

template < int D, class T, int max_capacity, int max_depth >
typename Ntree< D, T, max_capacity, max_depth >::iterator
Ntree< D, T, max_capacity, max_depth >::insert( iterator, const std::pair< Position< D >, T >& val )
{
  return insert( val.first, val.second );
}

} // namespace nest

#endif
