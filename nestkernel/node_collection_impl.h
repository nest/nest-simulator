/*
 *  node_collection_impl.h
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

#ifndef NODE_COLLECTION_IMPL_H
#define NODE_COLLECTION_IMPL_H

#include "node_collection.h"

namespace nest
{

NodeCollectionPTR
NodeCollection::create( const std::vector< auto >& node_ids_vector )
{
  if ( node_ids_vector.empty() )
  {
    return NodeCollection::create_();
  }
  if ( not std::is_sorted( node_ids_vector.begin(), node_ids_vector.end() ) )
  {
    throw BadProperty( "Indices must be sorted in ascending order" );
  }
  return NodeCollection::create_( node_ids_vector );
}

NodeCollectionPTR
NodeCollection::create_( const std::vector< auto >& node_ids )
{
  size_t current_first = node_ids[ 0 ];
  size_t current_last = current_first;
  size_t current_model = kernel().modelrange_manager.get_model_id( node_ids[ 0 ] );

  std::vector< NodeCollectionPrimitive > parts;

  size_t old_node_id = current_first;
  for ( auto node_id = ++( node_ids.begin() ); node_id != node_ids.end(); ++node_id )
  {
    if ( *node_id == old_node_id )
    {
      throw BadProperty( "All node IDs in a NodeCollection have to be unique" );
    }
    old_node_id = *node_id;

    const size_t next_model = kernel().modelrange_manager.get_model_id( *node_id );

    if ( next_model == current_model and *node_id == ( current_last + 1 ) )
    {
      // node goes in Primitive
      ++current_last;
    }
    else
    {
      // store Primitive; node goes in new Primitive
      parts.emplace_back( current_first, current_last, current_model );
      current_first = *node_id;
      current_last = current_first;
      current_model = next_model;
    }
  }

  // now push last section we opened
  parts.emplace_back( current_first, current_last, current_model );

  if ( parts.size() == 1 )
  {
    return std::make_shared< NodeCollectionPrimitive >( parts[ 0 ] );
  }
  else
  {
    return std::make_shared< NodeCollectionComposite >( parts );
  }
}

} // namespace nest

#endif /* #ifndef NODE_COLLECTION_IMPL_H */
