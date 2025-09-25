/*
 *  source.cpp
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

#include "source.h"
#include <cassert>

namespace nest
{

Source::Source()
  : node_id_( 0 )
  , processed_( false )
  , primary_( true )
{
}

Source::Source( const uint64_t node_id, const bool is_primary )
  : node_id_( node_id )
  , processed_( false )
  , primary_( is_primary )
{
  assert( node_id <= MAX_NODE_ID );
}

void Source::set_node_id( const uint64_t node_id )
{
  assert( node_id <= MAX_NODE_ID );
  node_id_ = node_id;
}

uint64_t Source::get_node_id() const
{
  return node_id_;
}

void Source::set_processed( const bool processed )
{
  processed_ = processed;
}

bool Source::is_processed() const
{
  return processed_;
}

void Source::set_primary( const bool primary )
{
  primary_ = primary;
}

bool Source::is_primary() const
{
  return primary_;
}

void Source::disable()
{
  node_id_ = DISABLED_NODE_ID;
}

bool Source::is_disabled() const
{
  return node_id_ == DISABLED_NODE_ID;
}

bool operator<( const Source& lhs, const Source& rhs )
{
  return lhs.node_id_ < rhs.node_id_;
}

bool operator>( const Source& lhs, const Source& rhs )
{
  return rhs < lhs;
}

bool operator==( const Source& lhs, const Source& rhs )
{
  return lhs.node_id_ == rhs.node_id_;
}

} // namespace nest
