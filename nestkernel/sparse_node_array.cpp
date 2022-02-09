/*
 *  sparse_node_array.cpp
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

#include "sparse_node_array.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "node.h"

#include "kernel_manager.h"

nest::SparseNodeArray::NodeEntry::NodeEntry( Node& node, index node_id )
  : node_( &node )
  , node_id_( node_id )
{
  assert( node_id == node.get_node_id() );
}

nest::SparseNodeArray::SparseNodeArray()
  : nodes_()
  , max_node_id_( 0 )
  , local_min_node_id_( 0 )
  , local_max_node_id_( 0 )
  , scale_split_id_( 0 )
  , scale_split_idx_( 0 )
  , split_has_occured_( false )
  , adding_nodes_with_proxies_( false ) // meaningless default
{
}

void
nest::SparseNodeArray::add_local_node( Node& node )
{
  const index node_id = node.get_node_id();

  // protect against node ID 0
  assert( node_id > 0 );

  // local_min_node_id_ can only be 0 if no node has been stored
  assert( local_min_node_id_ > 0 or nodes_.size() == 0 );

  // local_min_node_id_ cannot be larger than local_max_node_id_
  assert( local_min_node_id_ <= local_max_node_id_ );

  // local_max_node_id_ cannot be larger than max_node_id_
  assert( local_max_node_id_ <= max_node_id_ );

  // node_id must exceed max_node_id_
  assert( node_id > max_node_id_ );

  // all is consistent, register node and update auxiliary variables
  nodes_.push_back( NodeEntry( node, node_id ) );
  if ( local_min_node_id_ == 0 ) // only first non-zero
  {
    local_min_node_id_ = node_id;
    adding_nodes_with_proxies_ = node.has_proxies();
  }
  local_max_node_id_ = node_id;
  max_node_id_ = node_id;

  if ( not split_has_occured_ )
  {
    if ( adding_nodes_with_proxies_ != node.has_proxies() )
    {
      split_has_occured_ = true;
    }
    else
    {
      scale_split_id_ = node_id;
      ++scale_split_idx_;
      if ( local_max_node_id_ > local_min_node_id_ )
      {
        const double size = static_cast< double >( nodes_.size() - 1 );
        id_idx_scale_[ 0 ] = size / ( local_max_node_id_ - local_min_node_id_ );
      }
    }
  }
  else
  {
    if ( local_max_node_id_ > scale_split_id_ )
    {
      const double size = static_cast< double >( nodes_.size() - scale_split_idx_ );
      id_idx_scale_[ 1 ] = size / ( local_max_node_id_ - scale_split_id_ );
    }
  }
  assert( id_idx_scale_[ 0 ] >= 0. );
  assert( id_idx_scale_[ 0 ] <= 1. );
  assert( id_idx_scale_[ 1 ] >= 0. );
  assert( id_idx_scale_[ 1 ] <= 1. );
}

void
nest::SparseNodeArray::update_max_node_id( index node_id )
{
  assert( node_id > 0 ); // minimum node ID is 1
  assert( node_id >= max_node_id_ );
  max_node_id_ = node_id;
}

nest::Node*
nest::SparseNodeArray::get_node_by_node_id( index node_id ) const
{
  // local_min_node_id_ cannot be larger than local_max_node_id_
  assert( local_min_node_id_ <= local_max_node_id_ );

  // local_max_node_id_ cannot be larger than max_node_id_
  assert( local_max_node_id_ <= max_node_id_ );

  if ( node_id < 1 or max_node_id_ < node_id )
  {
    throw UnknownNode();
  }

  // handle node_ids below or above range
  if ( node_id < local_min_node_id_ or local_max_node_id_ < node_id )
  {
    return 0;
  }

  const bool after_split = node_id > scale_split_id_;
  const index ref_id = after_split ? scale_split_id_ : local_min_node_id_;
  const size_t ref_idx = after_split ? scale_split_idx_ : 0;

  // now estimate index
  const size_t idx_guess =
    std::min( static_cast< size_t >( ref_idx + std::floor( id_idx_scale_[ after_split ] * ( node_id - ref_id ) ) ),
      nodes_.size() - 1 );

  // search left if necessary
  auto idx = idx_guess;
  while ( 0 < idx and node_id < nodes_[ idx ].node_id_ )
  {
    --idx;
  }

  // search right if necessary
  while ( idx < nodes_.size() and nodes_[ idx ].node_id_ < node_id )
  {
    ++idx;
  }

  // adjust scaling based on search steps required
  if ( std::abs( static_cast< long >( idx ) - static_cast< long >( idx_guess ) ) > 10 )
  {
    id_idx_scale_[ after_split ] = std::min( 1.0, static_cast< double >( idx - ref_idx ) / ( node_id - ref_id ) );
  }
  assert( id_idx_scale_[ after_split ] >= 0.0 );
  assert( id_idx_scale_[ after_split ] <= 1.0 );

  if ( idx < nodes_.size() and nodes_[ idx ].node_id_ == node_id )
  {
    return nodes_[ idx ].node_;
  }
  else
  {
    return 0;
  }
}
