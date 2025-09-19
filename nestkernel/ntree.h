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
#include <iterator>
#include <utility>
#include <vector>

// Includes from spatial:
#include "position.h"

namespace nest
{

class AbstractMask;

template < int D >
class Mask;

/**
 * A Ntree object represents a subtree or leaf in a Ntree structure.
 *
 * Any ntree covers a specific region in space. A leaf ntree contains a list
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
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::pair< Position< D >, T >;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = long int;

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
    explicit iterator( Ntree& q );

    /**
     * Initialize an iterator to point to the nth node in this Ntree,
     * which must be a leaf. The top of the tree is the first ancestor of
     * the Ntree.
     */
    iterator( Ntree& q, size_t n );

    value_type&
    operator*()
    {
      return ntree_->nodes_[ node_ ];
    }
    value_type*
    operator->()
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
    iterator
    operator++( int )
    {
      iterator tmp = *this;
      ++*this;
      return tmp;
    }

    /**
     * Iterators are equal if they point to the same node in the same
     * ntree.
     */
    bool
    operator==( const iterator& other ) const
    {
      return other.ntree_ == ntree_ and ( other.node_ == node_ );
    }
    bool
    operator!=( const iterator& other ) const
    {
      return ( other.ntree_ != ntree_ ) or ( other.node_ != node_ );
    }

  protected:
    /**
     * Move to the next leaf quadrant, or set ntree_ to 0 if there are no
     * more leaves.
     */
    void next_leaf_();

    Ntree* ntree_;
    Ntree* top_;
    size_t node_;
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

    value_type&
    operator*()
    {
      return ntree_->nodes_[ node_ ];
    }
    value_type*
    operator->()
    {
      return &ntree_->nodes_[ node_ ];
    }

    /**
     * Move the iterator to the next node inside the mask within the
     * tree.
     *
     * May cause the iterator to become invalid if there are no
     * more nodes.
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
     * Iterators are equal if they point to the same node in the same
     * ntree.
     */
    bool
    operator==( const masked_iterator& other ) const
    {
      return other.ntree_ == ntree_ and ( other.node_ == node_ );
    }
    bool
    operator!=( const masked_iterator& other ) const
    {
      return ( other.ntree_ != ntree_ ) or ( other.node_ != node_ );
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
     * Find the first leaf which is not outside the mask.
     *
     * If no leaf is found below the current quadrant, will continue to next_leaf_().
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
    size_t node_;
    const Mask< D >* mask_;
    Position< D > anchor_;
    Position< D > anchored_position_;
    std::vector< Position< D > > anchors_;
    size_t current_anchor_;
  };

  /**
   * Create a Ntree that covers the region defined by the two
   * input positions.
   *
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

  using const_iterator = iterator;

  /**
   * Traverse quadtree structure from current ntree.
   *
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
  iterator insert( iterator, value_type&& val );

  /**
   * @returns member nodes in ntree and their position.
   */
  std::vector< value_type > get_nodes();

  /**
   * Applies a Mask to this ntree.
   *
   * @param mask    mask to apply.
   * @param anchor  position to center mask in.
   * @returns member nodes in ntree inside mask.
   */
  std::vector< value_type > get_nodes( const Mask< D >& mask, const Position< D >& anchor );

  /**
   * This function returns a node iterator which will traverse the
   * subtree below this Ntree.
   *
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
   *
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
Ntree< D, T, max_capacity, max_depth >::iterator::iterator( Ntree& q, size_t n )
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
template < int D, class T, int max_capacity, int max_depth >
typename Ntree< D, T, max_capacity, max_depth >::iterator
Ntree< D, T, max_capacity, max_depth >::insert( iterator, std::pair< Position< D >, T >&& val )
{
  return insert( val.first, val.second );
}

template < int D, class T, int max_capacity, int max_depth >
Ntree< D, T, max_capacity, max_depth >::iterator::iterator( Ntree& q )
  : ntree_( &q )
  , top_( &q )
  , node_( 0 )
{
  // First leaf
  while ( not ntree_->is_leaf() )
  {
    ntree_ = ntree_->children_[ 0 ];
  }
  // Find the first non-empty leaf
  while ( ntree_->nodes_.empty() )
  {

    next_leaf_();
    if ( ntree_ == 0 )
    {
      break;
    }
  }
}

template < int D, class T, int max_capacity, int max_depth >
typename Ntree< D, T, max_capacity, max_depth >::iterator&
Ntree< D, T, max_capacity, max_depth >::iterator::operator++()
{
  node_++;

  while ( node_ >= ntree_->nodes_.size() )
  {

    next_leaf_();

    node_ = 0;
    if ( ntree_ == 0 )
    {
      break;
    }
  }

  return *this;
}

template < int D, class T, int max_capacity, int max_depth >
void
Ntree< D, T, max_capacity, max_depth >::iterator::next_leaf_()
{

  // If we are on the last subntree, move up
  while ( ntree_ and ( ntree_ != top_ ) and ntree_->my_subquad_ == N - 1 )
  {
    ntree_ = ntree_->parent_;
  }

  // Since we stop at the top, this should never happen!
  assert( ntree_ != 0 );

  // If we have reached the top, mark as invalid and return
  if ( ntree_ == top_ )
  {
    ntree_ = 0;
    return;
  }

  // Move to next sibling
  ntree_ = ntree_->parent_->children_[ ntree_->my_subquad_ + 1 ];

  // Move down if this is not a leaf.
  while ( not ntree_->is_leaf() )
  {
    ntree_ = ntree_->children_[ 0 ];
  }
}

// Proper mod which returns non-negative numbers
static inline double
mod( double x, double p )
{
  x = std::fmod( x, p );
  if ( x < 0 )
  {
    x += p;
  }
  return x;
}

template < int D, class T, int max_capacity, int max_depth >
Ntree< D, T, max_capacity, max_depth >::masked_iterator::masked_iterator( Ntree< D, T, max_capacity, max_depth >& q,
  const Mask< D >& mask,
  const Position< D >& anchor )
  : ntree_( &q )
  , top_( &q )
  , allin_top_( 0 )
  , node_( 0 )
  , mask_( &mask )
  , anchor_( anchor )
  , anchors_()
  , current_anchor_( 0 )
{
  if ( ntree_->periodic_.any() )
  {
    Box< D > mask_bb = mask_->get_bbox();

    // Move lower left corner of mask into main image of layer
    for ( int i = 0; i < D; ++i )
    {
      if ( ntree_->periodic_[ i ] )
      {
        anchor_[ i ] =
          nest::mod( anchor_[ i ] + mask_bb.lower_left[ i ] - ntree_->lower_left_[ i ], ntree_->extent_[ i ] )
          - mask_bb.lower_left[ i ] + ntree_->lower_left_[ i ];
      }
    }
    anchors_.push_back( anchor_ );

    // Add extra anchors for each dimension where this is needed
    // (Assumes that the mask is not wider than the layer)
    for ( int i = 0; i < D; ++i )
    {
      if ( ntree_->periodic_[ i ] )
      {
        int n = anchors_.size();
        if ( ( anchor_[ i ] + mask_bb.upper_right[ i ] - ntree_->lower_left_[ i ] ) > ntree_->extent_[ i ] )
        {
          for ( int j = 0; j < n; ++j )
          {
            Position< D > p = anchors_[ j ];
            p[ i ] -= ntree_->extent_[ i ];
            anchors_.push_back( p );
          }
        }
      }
    }
  }

  init_();
}

template < int D, class T, int max_capacity, int max_depth >
void
Ntree< D, T, max_capacity, max_depth >::masked_iterator::init_()
{
  node_ = 0;
  allin_top_ = 0;
  ntree_ = top_;

  if ( mask_->outside( Box< D >( ntree_->lower_left_ - anchor_, ntree_->lower_left_ - anchor_ + ntree_->extent_ ) ) )
  {

    next_anchor_();
  }
  else
  {

    if ( mask_->inside( Box< D >( ntree_->lower_left_ - anchor_, ntree_->lower_left_ - anchor_ + ntree_->extent_ ) ) )
    {
      first_leaf_inside_();
    }
    else
    {
      first_leaf_();
    }

    if ( ntree_->nodes_.empty() or ( not mask_->inside( ntree_->nodes_[ node_ ].first - anchor_ ) ) )
    {
      ++( *this );
    }
  }
}

template < int D, class T, int max_capacity, int max_depth >
void
Ntree< D, T, max_capacity, max_depth >::masked_iterator::next_anchor_()
{
  ++current_anchor_;
  if ( current_anchor_ >= anchors_.size() )
  {
    // Done. Mark as invalid.
    ntree_ = 0;
    node_ = 0;
  }
  else
  {
    anchor_ = anchors_[ current_anchor_ ];
    init_();
  }
}

template < int D, class T, int max_capacity, int max_depth >
void
Ntree< D, T, max_capacity, max_depth >::masked_iterator::next_leaf_()
{

  // There are two states: the initial state, and "all in". In the
  // all in state, we are in a subtree which is completely inside
  // the mask. The allin_top_ is the top of this subtree. When
  // exiting the subtree, the state changes to the initial
  // state. In the initial state, we must check each quadrant to
  // see if it is completely inside or outside the mask. If inside,
  // we go all in. If outside, we move on to the next leaf. If
  // neither, keep going until we find a leaf. Upon exiting from
  // this function, we are either done (ntree_==0), or on a leaf
  // node which at least intersects with the mask. If allin_top_!=0,
  // the leaf is completely inside the mask.

  if ( allin_top_ )
  {
    // state: all in

    // If we are on the last subtree, move up
    while ( ntree_ and ( ntree_ != allin_top_ ) and ntree_->my_subquad_ == N - 1 )
    {
      ntree_ = ntree_->parent_;
    }

    // Since we stop at the top, this should never happen!
    assert( ntree_ != 0 );

    // If we reached the allin_top_, we are no longer all in.
    if ( ntree_ != allin_top_ )
    {

      // Move to next sibling
      ntree_ = ntree_->parent_->children_[ ntree_->my_subquad_ + 1 ];

      // Move down if this is not a leaf.
      while ( not ntree_->is_leaf() )
      {
        ntree_ = ntree_->children_[ 0 ];
      }
      return;
    }

    allin_top_ = 0;
    // Will continue as not all in.
  }

  // state: Not all in

  do
  {

    // If we are on the last subtree, move up
    while ( ntree_ and ( ntree_ != top_ ) and ntree_->my_subquad_ == N - 1 )
    {
      ntree_ = ntree_->parent_;
    }

    // Since we stop at the top, this should never happen!
    assert( ntree_ != 0 );

    // If we have reached the top, mark as invalid and return
    if ( ntree_ == top_ )
    {
      return next_anchor_();
    }

    // Move to next sibling
    ntree_ = ntree_->parent_->children_[ ntree_->my_subquad_ + 1 ];
    // Create anchored position in two steps to avoid creating a new Position object.
    anchored_position_ = ntree_->lower_left_;
    anchored_position_ -= anchor_;

    if ( mask_->inside( Box< D >( anchored_position_, anchored_position_ + ntree_->extent_ ) ) )
    {
      return first_leaf_inside_();
    }

  } while ( mask_->outside( Box< D >( anchored_position_, anchored_position_ + ntree_->extent_ ) ) );

  return first_leaf_();
}

template < int D, class T, int max_capacity, int max_depth >
void
Ntree< D, T, max_capacity, max_depth >::masked_iterator::first_leaf_()
{
  while ( not ntree_->is_leaf() )
  {

    ntree_ = ntree_->children_[ 0 ];

    if ( mask_->inside( Box< D >( ntree_->lower_left_ - anchor_, ntree_->lower_left_ - anchor_ + ntree_->extent_ ) ) )
    {
      return first_leaf_inside_();
    }

    if ( mask_->outside( Box< D >( ntree_->lower_left_ - anchor_, ntree_->lower_left_ - anchor_ + ntree_->extent_ ) ) )
    {
      return next_leaf_();
    }
  }
}


template < int D, class T, int max_capacity, int max_depth >
void
Ntree< D, T, max_capacity, max_depth >::masked_iterator::first_leaf_inside_()
{

  allin_top_ = ntree_;

  while ( not ntree_->is_leaf() )
  {
    ntree_ = ntree_->children_[ 0 ];
  }
}

template < int D, class T, int max_capacity, int max_depth >
typename Ntree< D, T, max_capacity, max_depth >::masked_iterator&
Ntree< D, T, max_capacity, max_depth >::masked_iterator::operator++()
{
  ++node_;

  if ( allin_top_ == 0 )
  {
    while (
      ( node_ < ntree_->nodes_.size() ) and ( not anchored_position_inside_mask( ntree_->nodes_[ node_ ].first ) ) )
    {
      ++node_;
    }
  }

  while ( node_ >= ntree_->nodes_.size() )
  {
    next_leaf_();

    node_ = 0;
    if ( ntree_ == 0 )
    {
      break;
    }

    if ( allin_top_ == 0 )
    {
      while (
        ( node_ < ntree_->nodes_.size() ) and ( not anchored_position_inside_mask( ntree_->nodes_[ node_ ].first ) ) )
      {
        ++node_;
      }
    }
  }

  return *this;
}

template < int D, class T, int max_capacity, int max_depth >
int
Ntree< D, T, max_capacity, max_depth >::subquad_( const Position< D >& pos )
{
  int r = 0;
  for ( int i = 0; i < D; ++i )
  {
    // Comparing against an epsilon value in case there are round-off errors.
    // Using a negative epsilon value because the round-off error may go both ways
    // and the difference we check against may therefore be +/- 10^-16.
    const bool in_left_half =
      ( ( lower_left_[ i ] + extent_[ i ] / 2 ) - pos[ i ] ) > -std::numeric_limits< double >::epsilon();
    r += ( 1 << i ) * ( in_left_half ? 0 : 1 );
  }

  return r;
}

template < int D, class T, int max_capacity, int max_depth >
void
Ntree< D, T, max_capacity, max_depth >::append_nodes_( std::vector< std::pair< Position< D >, T > >& v )
{
  if ( leaf_ )
  {
    std::copy( nodes_.begin(), nodes_.end(), std::back_inserter( v ) );
  }
  else
  {
    for ( int i = 0; i < N; ++i )
    {
      children_[ i ]->append_nodes_( v );
    }
  }
}

template < int D, class T, int max_capacity, int max_depth >
void
Ntree< D, T, max_capacity, max_depth >::append_nodes_( std::vector< std::pair< Position< D >, T > >& v,
  const Mask< D >& mask,
  const Position< D >& anchor )
{
  if ( mask.outside( Box< D >( lower_left_ - anchor, lower_left_ - anchor + extent_ ) ) )
  {
    return;
  }
  if ( mask.inside( Box< D >( lower_left_ - anchor, lower_left_ - anchor + extent_ ) ) )
  {
    return append_nodes_( v );
  }
  if ( leaf_ )
  {

    for ( typename std::vector< std::pair< Position< D >, T > >::iterator i = nodes_.begin(); i != nodes_.end(); ++i )
    {
      if ( mask.inside( i->first - anchor ) )
      {
        v.push_back( *i );
      }
    }
  }
  else
  {
    for ( int i = 0; i < N; ++i )
    {
      children_[ i ]->append_nodes_( v, mask, anchor );
    }
  }
}

template < int D, class T, int max_capacity, int max_depth >
typename Ntree< D, T, max_capacity, max_depth >::iterator
Ntree< D, T, max_capacity, max_depth >::insert( Position< D > pos, const T& node )
{
  if ( periodic_.any() )
  {
    // Map position into standard range when using periodic b.c. Only necessary when
    // inserting positions during source driven connect when target has periodic b.c.
    // May be inefficient.
    for ( int i = 0; i < D; ++i )
    {
      if ( periodic_[ i ] )
      {
        pos[ i ] = lower_left_[ i ] + std::fmod( pos[ i ] - lower_left_[ i ], extent_[ i ] );
        if ( pos[ i ] < lower_left_[ i ] )
        {
          pos[ i ] += extent_[ i ];
        }
      }
    }
  }

  if ( leaf_ and ( nodes_.size() >= max_capacity ) and my_depth_ < max_depth )
  {
    split_();
  }
  if ( leaf_ )
  {

    for ( int i = 0; i < D; ++i )
    {
      // Comparing against an epsilon value in case there are round-off errors.
      // Using a negative epsilon value because the round-off error may go both ways
      // and the difference we check against may therefore be +/- 10^-16.
      assert( ( pos - lower_left_ )[ i ] > -std::numeric_limits< double >::epsilon()
        and ( lower_left_ + extent_ - pos )[ i ] > -std::numeric_limits< double >::epsilon() );
    }

    nodes_.push_back( std::pair< Position< D >, T >( pos, node ) );

    return iterator( *this, nodes_.size() - 1 );
  }
  else
  {

    return children_[ subquad_( pos ) ]->insert( pos, node );
  }
}

template < int D, class T, int max_capacity, int max_depth >
void
Ntree< D, T, max_capacity, max_depth >::split_()
{
  assert( leaf_ );

  for ( int j = 0; j < N; ++j )
  {
    Position< D > lower_left = lower_left_;
    for ( int i = 0; i < D; ++i )
    {
      if ( j & ( 1 << i ) )
      {
        lower_left[ i ] += extent_[ i ] * 0.5;
      }
    }

    children_[ j ] = new Ntree< D, T, max_capacity, max_depth >( lower_left, extent_ * 0.5, 0, this, j );
  }

  for ( typename std::vector< std::pair< Position< D >, T > >::iterator i = nodes_.begin(); i != nodes_.end(); ++i )
  {
    children_[ subquad_( i->first ) ]->insert( i->first, i->second );
  }

  nodes_.clear();

  leaf_ = false;
}

} // namespace nest

#endif
