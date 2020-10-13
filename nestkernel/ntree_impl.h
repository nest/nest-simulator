/*
 *  ntree_impl.h
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

#ifndef NTREE_IMPL_H
#define NTREE_IMPL_H

#include "ntree.h"

// Includes from spatial:
#include "mask.h"

namespace nest
{

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
typename Ntree< D, T, max_capacity, max_depth >::iterator& Ntree< D, T, max_capacity, max_depth >::iterator::
operator++()
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
  while ( ntree_ && ( ntree_ != top_ ) && ( ntree_->my_subquad_ == N - 1 ) )
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
        anchor_[ i ] = nest::mod( anchor_[ i ] + mask_bb.lower_left[ i ] - ntree_->lower_left_[ i ],
                         ntree_->extent_[ i ] ) - mask_bb.lower_left[ i ] + ntree_->lower_left_[ i ];
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
    /*
          for(int i=0;i<anchors_.size();++i) {
            std::cout << anchors_[i] << std::endl;
          }
          std::cout << "---" << std::endl;
    */
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

    if ( ntree_->nodes_.empty() || ( not mask_->inside( ntree_->nodes_[ node_ ].first - anchor_ ) ) )
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
    while ( ntree_ && ( ntree_ != allin_top_ ) && ( ntree_->my_subquad_ == N - 1 ) )
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
    while ( ntree_ && ( ntree_ != top_ ) && ( ntree_->my_subquad_ == N - 1 ) )
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
  Ntree< D, T, max_capacity, max_depth >::masked_iterator::
  operator++()
{
  ++node_;

  if ( allin_top_ == 0 )
  {
    while (
      ( node_ < ntree_->nodes_.size() ) && ( not anchored_position_inside_mask( ntree_->nodes_[ node_ ].first ) ) )
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
        ( node_ < ntree_->nodes_.size() ) && ( not anchored_position_inside_mask( ntree_->nodes_[ node_ ].first ) ) )
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
    r += ( 1 << i ) * ( pos[ i ] < lower_left_[ i ] + extent_[ i ] / 2 ? 0 : 1 );
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
    // Map position into standard range when using periodic b.c.
    // Only necessary when inserting positions during source driven connect when
    // target has periodic b.c. May be inefficient.

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

  if ( leaf_ && ( nodes_.size() >= max_capacity ) && ( my_depth_ < max_depth ) )
  {
    split_();
  }
  if ( leaf_ )
  {

    assert( ( pos >= lower_left_ ) && ( pos < lower_left_ + extent_ ) );

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
}

#endif
