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
#include "kernel_manager.h"

// C++ includes:
#include <algorithm> // copy

namespace nest
{

GIDCollectionPrimitive::GIDCollectionPrimitive( index first, index last, index model_id )
{
  first_ = first;
  last_ = last;
  model_id = model_id;
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first, index last)
{
  first_ = first;
  last_ = last;
  
  // find the model_id
  const int model_id = kernel().node_manager.get_node( first )->get_model_id();
  for (index gid = ++first; gid <= last; ++gid)
  {
    if ( model_id != kernel().node_manager.get_node( gid )->get_model_id() )
      {
        throw BadProperty( "model ids does not match" );
      }
  }
  model_id_ = model_id;
}

GIDCollectionPrimitive::GIDCollectionPrimitive( TokenArray gids )
{
  //assert(false); // Constructor should not be used.
  throw BadProperty( "constructor will be removed." );
}

GIDCollectionPrimitive::GIDCollectionPrimitive( IntVectorDatum gids )
{
  //assert(false); // Constructor should not be used.
  throw BadProperty( "constructor will be removed." );
}

void
GIDCollectionPrimitive::print_me( std::ostream& out ) const
{
  out << "[[size=" << size() << ",";
  out << "(" << first_ << ".." << last_ << ")";
  out << "]]";
}

} // namespace nest
