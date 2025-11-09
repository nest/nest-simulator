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
#include "kernel_manager.h"
#include "node.h"


nest::SparseNodeArray::NodeEntry::NodeEntry( Node& node, size_t node_id )
  : node_( &node )
  , node_id_( node_id )
{
}

nest::SparseNodeArray::SparseNodeArray()
  : nodes_()
  , global_max_node_id_( 0 )
  , local_min_node_id_( 0 )
  , local_max_node_id_( 0 )
  , left_scale_( 1.0 )
  , right_scale_( 1.0 )
  , split_node_id_( 0 )
  , split_idx_( 0 )
  , have_split_( false )
  , left_side_has_proxies_( false ) // meaningless initial value
{
}


void
nest::SparseNodeArray::clear()
{
  nodes_.clear();

  global_max_node_id_ = 0;
  local_min_node_id_ = 0;
  local_max_node_id_ = 0;
  left_scale_ = 1.0;
  right_scale_ = 1.0;
  split_node_id_ = 0;
  split_idx_ = 0;
  have_split_ = false;
  left_side_has_proxies_ = false;
}

void
nest::SparseNodeArray::add_local_node( Node& node )
{
  const size_t node_id = node.get_node_id();

  // ensure increasing order
  assert( node_id > local_max_node_id_ );

  nodes_.push_back( NodeEntry( node, node_id ) );
  local_max_node_id_ = node_id;

  // mark array inconsistent until set_max_node_id() called
  global_max_node_id_ = 0;

  // set up when first node is added
  if ( local_min_node_id_ == 0 )
  {
    local_min_node_id_ = node_id;
    left_side_has_proxies_ = node.has_proxies();

    // we now know which scale applies on which side of the split
    const double proxy_scale = 1.0 / static_cast< double >( kernel::manager< VPManager >.get_num_virtual_processes() );
    if ( left_side_has_proxies_ )
    {
      left_scale_ = proxy_scale;
    }
    else
    {
      right_scale_ = proxy_scale;
    }
  }

  if ( not have_split_ )
  {
    if ( left_side_has_proxies_ != node.has_proxies() )
    {
      // node is first past splitting point
      have_split_ = true;
    }
    else
    {
      ++split_idx_; // index one beyond the node
    }
  }
}

void
nest::SparseNodeArray::set_max_node_id( size_t node_id )
{
  assert( node_id > 0 ); // minimum node ID is 1
  assert( node_id >= local_max_node_id_ );
  global_max_node_id_ = node_id;
  if ( not have_split_ )
  {
    split_node_id_ = global_max_node_id_ + 1;
  }
}

nest::Node*
nest::SparseNodeArray::get_node_by_node_id( size_t node_id ) const
{
  assert( is_consistent_() );

  if ( node_id < 1 or global_max_node_id_ < node_id )
  {
    throw UnknownNode();
  }

  // handle node_ids below or above range
  if ( node_id < local_min_node_id_ or local_max_node_id_ < node_id )
  {
    return nullptr;
  }

  // Find base index and node ID for estimating location of desired node in array.
  // In the expression for base_id, split_node_id_ will only be used if we are on the
  // right side, when the value is well-defined.
  const bool left_side = node_id < split_node_id_;
  const double scale = left_side ? left_scale_ : right_scale_;
  const size_t base_idx = left_side ? 0 : split_idx_;
  const size_t base_id = left_side ? local_min_node_id_ : split_node_id_;

  // estimate index, limit to array size for safety size
  auto idx =
    std::min( static_cast< size_t >( base_idx + std::floor( scale * ( node_id - base_id ) ) ), nodes_.size() - 1 );

  // search left if necessary
  while ( 0 < idx and node_id < nodes_[ idx ].node_id_ )
  {
    --idx;
  }

  // search right if necessary
  while ( idx < nodes_.size() and nodes_[ idx ].node_id_ < node_id )
  {
    ++idx;
  }

  if ( idx < nodes_.size() and nodes_[ idx ].node_id_ == node_id )
  {
    return nodes_[ idx ].node_;
  }
  else
  {
    return nullptr;
  }
}
size_t
nest::SparseNodeArray::NodeEntry::get_node_id() const
{

  assert( node_id_ > 0 );
  return node_id_;
}

nest::Node*
nest::SparseNodeArray::NodeEntry::get_node() const
{

  assert( node_ );
  return node_;
}

bool
nest::SparseNodeArray::is_consistent_() const
{

  return nodes_.size() == 0 or global_max_node_id_ > 0;
}

size_t
nest::SparseNodeArray::get_max_node_id() const
{

  return global_max_node_id_;
}

nest::Node*
nest::SparseNodeArray::get_node_by_index( size_t idx ) const
{

  assert( idx < nodes_.size() );
  return nodes_[ idx ].node_;
}

size_t
nest::SparseNodeArray::size() const
{

  return nodes_.size();
}

nest::SparseNodeArray::const_iterator
nest::SparseNodeArray::end() const
{

  return nodes_.end();
}

nest::SparseNodeArray::const_iterator
nest::SparseNodeArray::begin() const
{

  return nodes_.begin();
}
