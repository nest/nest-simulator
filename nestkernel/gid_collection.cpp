/*
 *  gid_collection.cpp
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

#include "gid_collection.h"

// C++ includes:
#include <algorithm> // copy

namespace nest
{

GIDCollection::GIDCollection( index first, index last )
  : is_range_( true )
{
  gid_range_.first = first;
  gid_range_.second = last;
}

GIDCollection::GIDCollection( IntVectorDatum gids )
  : is_range_( false )
{
  gid_array_.resize( gids->size() );
  std::copy( gids->begin(), gids->end(), gid_array_.begin() );
}

GIDCollection::GIDCollection( TokenArray gids )
  : is_range_( false )
{
  gid_array_.resize( gids.size() );
  for ( size_t i = 0; i < gids.size(); ++i )
  {
    gid_array_[ i ] = gids[ i ];
  }
}

void
GIDCollection::print_me( std::ostream& out ) const
{
  out << "[[is_range=" << is_range_ << ",size=" << size() << ",";
  if ( is_range_ )
  {
    out << "(" << gid_range_.first << ".." << gid_range_.second << ")";
  }
  else
  {
    out << "(" << gid_array_[ 0 ] << ".." << gid_array_[ gid_array_.size() - 1 ] << ")";
  }
  out << "]]";
}

} // namespace nest
