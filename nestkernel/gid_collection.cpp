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

GIDCollectionPTR operator+( GIDCollectionPTR lhs, GIDCollectionPTR rhs )
{
  throw KernelException( "not yet implemented" );
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first,
  index last,
  index model_id,
  GIDCollectionMetadataPTR meta )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( meta )
{
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first,
  index last,
  index model_id )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( 0 )
{
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first, index last )
  : first_( first )
  , last_( last )
  , model_id_( 0 )
  , metadata_( 0 )
{
  // find the model_id
  const int model_id = kernel().node_manager.get_node( first )->get_model_id();
  for ( index gid = ++first; gid <= last; ++gid )
  {
    if ( model_id != kernel().node_manager.get_node( gid )->get_model_id() )
    {
      throw BadProperty( "model ids does not match" );
    }
  }
  model_id_ = model_id;
}

GIDCollectionPrimitive::GIDCollectionPrimitive(
  const GIDCollectionPrimitive& rhs )
  : first_( rhs.first_ )
  , last_( rhs.last_ )
  , model_id_( rhs.model_id_ )
  , metadata_( rhs.metadata_ )
{
}

ArrayDatum GIDCollectionPrimitive::to_array() const
{
  ArrayDatum gids;
  gids.reserve( size() );
  for ( const_iterator it = begin(); it != end() ; ++it )
  {
	gids.push_back( (*it).gid );
  }
  return gids;
}

GIDCollectionPTR
GIDCollectionPrimitive::GIDCollectionPrimitive::slice( size_t start,
  size_t stop,
  size_t step ) const
{
  if ( not( start < stop ) )
  {
    throw BadParameter( "start < stop required." );
  }
  if ( not( stop <= size() ) )
  {
    throw BadParameter( "stop <= size() required." );
  }

  if ( step == 1 )
  {
    return GIDCollectionPTR( new GIDCollectionPrimitive(
      first_ + start, first_ + stop - 1, model_id_, metadata_ ) );
  }
  else
  {
    return GIDCollectionPTR(
      0 ); // new GIDCollectionComposite( *this, start, stop, step ) );
  }
}


GIDCollectionComposite::GIDCollectionComposite( TokenArray gids )
{
  // assert(false); // Constructor should not be used.
  throw BadProperty( "constructor will be removed." );
}

GIDCollectionComposite::GIDCollectionComposite( IntVectorDatum gids )
{
  // assert(false); // Constructor should not be used.
  throw BadProperty( "constructor will be removed." );
}

GIDCollectionComposite::GIDCollectionComposite(
  const GIDCollectionPrimitive& prim,
  size_t start,
  size_t stop,
  size_t step )

{
}

void
GIDCollectionPrimitive::print_me( std::ostream& out ) const
{
  out << "[[size=" << size() << ",";
  out << "(" << first_ << ".." << last_ << ")";
  out << "]]";
}

} // namespace nest
