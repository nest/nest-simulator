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

nest::SparseNodeArray::NodeEntry::NodeEntry( Node& node, index gid )
  : node_( &node )
  , gid_( gid )
{
  assert( gid == node.get_gid() );
}

nest::SparseNodeArray::SparseNodeArray()
  : nodes_()
  , max_gid_( 0 )
  , local_min_gid_( 0 )
  , local_max_gid_( 0 )
  , gid_idx_scale_( 1.0 )
{
}

void
nest::SparseNodeArray::add_local_node( Node& node )
{
  const index gid = node.get_gid();

  // protect against GID 0
  assert( gid > 0 );

  // local_min_gid_ can only be 0 if no node has been stored
  assert( local_min_gid_ > 0 or nodes_.size() == 0 );

  // local_min_gid_ cannot be larger than local_max_gid_
  assert( local_min_gid_ <= local_max_gid_ );

  // local_max_gid_ cannot be larger than max_gid_
  assert( local_max_gid_ <= max_gid_ );

  // gid must exceed max_gid_
  assert( gid > max_gid_ );

  // all is consistent, register node and update auxiliary variables
  nodes_.push_back( NodeEntry( node, gid ) );
  if ( local_min_gid_ == 0 ) // only first non-zero
  {
    local_min_gid_ = gid;
  }
  local_max_gid_ = gid;
  max_gid_ = gid;

  // implies nodes_.size() > 1
  if ( local_max_gid_ > local_min_gid_ )
  {
    double size = static_cast< double >( nodes_.size() - 1 );
    gid_idx_scale_ = size / ( local_max_gid_ - local_min_gid_ );
  }
  assert( gid_idx_scale_ > 0. );
  assert( gid_idx_scale_ <= 1. );
}

void
nest::SparseNodeArray::update_max_gid( index gid )
{
  assert( gid > 0 ); // minimum GID is 1
  assert( gid >= max_gid_ );
  max_gid_ = gid;
}

nest::Node*
nest::SparseNodeArray::get_node_by_gid( index gid ) const
{
  // local_min_gid_ cannot be larger than local_max_gid_
  assert( local_min_gid_ <= local_max_gid_ );

  // local_max_gid_ cannot be larger than max_gid_
  assert( local_max_gid_ <= max_gid_ );

  if ( gid < 1 or max_gid_ < gid )
  {
    throw UnknownNode();
  }

  // handle gids below or above range
  if ( gid < local_min_gid_ or local_max_gid_ < gid )
  {
    return 0;
  }

  // now estimate index
  size_t idx = std::floor( gid_idx_scale_ * ( gid - local_min_gid_ ) );
  assert( idx < nodes_.size() );

  // search left if necessary
  while ( 0 < idx and gid < nodes_[ idx ].gid_ )
  {
    --idx;
  }
  // search right if necessary
  while ( idx < nodes_.size() and nodes_[ idx ].gid_ < gid )
  {
    ++idx;
  }
  if ( idx < nodes_.size() and nodes_[ idx ].gid_ == gid )
  {
    return nodes_[ idx ].node_;
  }
  else
  {
    return 0;
  }
}
