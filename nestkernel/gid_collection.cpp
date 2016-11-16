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

GIDCollectionPTR 
GIDCollection::create_GIDCollection( index first, index last ) const
{
  index it_first = first;
  index it_last = it_first;
  index prim_model_id = -1;
  index node_model_id;
  std::vector<GIDCollectionPrimitive> prims;
  bool first_node_exists = false;
  
  for (index gid = first; gid <= last; ++gid)
  {
    try
    {
      node_model_id = kernel().node_manager.get_node( gid )->get_model_id();
      if ( not first_node_exists )
      {
        // first node
        it_first = gid;
        it_last = it_first;
        prim_model_id = node_model_id;
        first_node_exists = true;
      }
      else if ( node_model_id == prim_model_id )
      {
        // node goes in Primitive
        ++it_last;
      }
      else
      {
        // store Primitive; node goes in new primitive
        prims.push_back( *( new GIDCollectionPrimitive( it_first, it_last, 
                                                            prim_model_id ) ) );
        it_first = gid;
        it_last = gid;
        prim_model_id = node_model_id;
      }
    }
    catch ( UnknownNode )
    {
      prim_model_id = -1; // sends the next found node into a new Primitive
    }
  }
  
  if (not first_node_exists )
  {
    throw UnknownNode();
  }
  else
  {
    prims.push_back( *( new GIDCollectionPrimitive( it_first, it_last, 
                                                            prim_model_id ) ) );
  }
  if ( prims.size() == 1 ) // find type of GIDCollection
  {
    return GIDCollectionPTR( prims[0] );
  }
  else
  {
    return GIDCollectionPTR( new GIDCollectionComposite( prims ) );
  }
}

GIDCollectionPTR 
GIDCollection::create_GIDCollection( IntVectorDatum gids ) const
{
  std::sort( gids->begin(), gids->end() );
  
  index it_first = *( gids->begin() );
  index it_last = it_first;
  index prim_model_id = -1;
  index node_model_id;
  index it_to_gid;
  std::vector<GIDCollectionPrimitive> prims;
  bool first_node_exists = false;
  
  for ( std::vector< long >::const_iterator gid = gids->begin(); gid != gids->end(); ++gid )
  {
    try
    {
      node_model_id = kernel().node_manager.get_node( *gid )->get_model_id();
      it_to_gid = *gid; // *gid is signed; want to avoid warnings when comparing
      if ( not first_node_exists )
      {
        // first node
        it_first = *gid;
        it_last = it_first;
        prim_model_id = node_model_id;
        first_node_exists = true;
      }
      else if ( node_model_id == prim_model_id and (it_last + 1) == it_to_gid )
      {
        // node goes in Primitive
        ++it_last;
      }
      else
      {
        // store Primitive; node goes in new Primitive
        prims.push_back( *( new GIDCollectionPrimitive( it_first, it_last, 
                                                            prim_model_id ) ) );
        it_first = *gid;
        it_last = *gid;
        prim_model_id = node_model_id;
      }
    }
    catch ( UnknownNode )
    {
      prim_model_id = -1; // sends the next found node into a new Primitive
    }
  }
  
  if (not first_node_exists )
  {
    throw UnknownNode();
  }
  else
  {
    prims.push_back( *( new GIDCollectionPrimitive( it_first, it_last, 
                                                            prim_model_id ) ) );
  }
  
  if ( prims.size() == 1 ) // find type of GIDCollection
  {
    return GIDCollectionPTR( prims[0] );
  }
  else
  {
    return GIDCollectionPTR( new GIDCollectionComposite( prims ) );
  }
}

GIDCollectionPTR 
GIDCollection::create_GIDCollection( TokenArray gids ) const
{
  std::vector< index > gids_vector;
  std::copy( gids.begin(), gids.end(), gids_vector.begin() );
  std::sort( gids_vector.begin(), gids_vector.end() );
  
  index it_first = *( gids_vector.begin() );
  index it_last = it_first;
  index prim_model_id = -1;
  index node_model_id;
  std::vector<GIDCollectionPrimitive> prims;
  bool first_node_exists = false;
  
  for ( std::vector< index >::const_iterator gid = gids_vector.begin(); 
                                              gid != gids_vector.end(); ++gid )
  {
    try
    {
      node_model_id = kernel().node_manager.get_node( *gid )->get_model_id();
      if ( not first_node_exists )
      {
        // first node
        it_first = *gid;
        it_last = it_first;
        prim_model_id = node_model_id;
        first_node_exists = true;
      }
      else if ( node_model_id == prim_model_id and (it_last + 1) == *gid )
      {
        // node goes in Primitive
        ++it_last;
      }
      else
      {
        // store Primitive; node goes in new Primitive
        prims.push_back( *( new GIDCollectionPrimitive( it_first, it_last, 
                                                            prim_model_id ) ) );
        it_first = *gid;
        it_last = *gid;
        prim_model_id = node_model_id;
      }
    }
    catch ( UnknownNode )
    {
      prim_model_id = -1; // sends the next found node into a new Primitive
    }
  }
  
  if (not first_node_exists )
  {
    throw UnknownNode();
  }
  else
  {
    prims.push_back( *( new GIDCollectionPrimitive( it_first, it_last, 
                                                            prim_model_id ) ) );
  }
  if ( prims.size() == 1 ) // find type of GIDCollection
  {
    return GIDCollectionPTR( prims[0] );
  }
  else
  {
    return GIDCollectionPTR( new GIDCollectionComposite( prims ) );
  }
}

GIDCollectionPTR 
GIDCollection::create_GIDCollection( const ArrayDatum iterable ) const
{
  std::vector< index > gids_vector;
  std::copy( iterable.begin(), iterable.end(), gids_vector.begin() );
  std::sort( gids_vector.begin(), gids_vector.end() );
  
  index it_first = *( gids_vector.begin() );
  index it_last = it_first;
  index prim_model_id = -1;
  index node_model_id;
  std::vector<GIDCollectionPrimitive> prims;
  bool first_node_exists = false;
  
  for ( std::vector< index >::const_iterator gid = gids_vector.begin(); 
                                                  gid != gids_vector.end(); ++gid )
  {
    try
    {
      node_model_id = kernel().node_manager.get_node( *gid )->get_model_id();
      if ( not first_node_exists )
      {
        // first node
        it_first = *gid;
        it_last = it_first;
        prim_model_id = node_model_id;
        first_node_exists = true;
      }
      else if ( node_model_id == prim_model_id and (it_last + 1) == *gid )
      {
        // node goes in Primitive
        ++it_last;
      }
      else
      {
        // store Primitive; node goes in new Primitive
        prims.push_back( *( new GIDCollectionPrimitive( it_first, it_last, 
                                                            prim_model_id ) ) );
        it_first = *gid;
        it_last = *gid;
        prim_model_id = node_model_id;
      }
    }
    catch ( UnknownNode )
    {
      prim_model_id = -1; // sends the next found node into a new Primitive
    }
  }
  if (not first_node_exists )
  {
    throw UnknownNode();
  }
  else
  {
    prims.push_back( *( new GIDCollectionPrimitive( it_first, it_last, 
                                                            prim_model_id ) ) );
  }
  if ( prims.size() == 1 ) // find type of GIDCollection
  {
    return GIDCollectionPTR( prims[0] );
  }
  else
  {
    return GIDCollectionPTR( new GIDCollectionComposite( prims ) );
  }
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first, index last,
		                             index model_id, GIDCollectionMetadataPTR meta )
: first_( first )
, last_( last )
, model_id_( model_id )
, metadata_( meta )
{
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first, index last,
		                                            index model_id )
: first_( first )
, last_( last )
, model_id_( model_id )
, metadata_( 0 )
{
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first, index last)
: first_( first )
, last_( last )
, model_id_( 0 )
, metadata_( 0 )
{
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

GIDCollectionPrimitive::GIDCollectionPrimitive( const GIDCollectionPrimitive& rhs )
: first_( rhs.first_ )
, last_( rhs.last_ )
, model_id_( rhs.model_id_ )
, metadata_( rhs.metadata_ )
{
}

GIDCollectionPTR GIDCollectionPrimitive::operator+( GIDCollectionPTR rhs ) const
{
  if ( not ( get_metadata() == rhs->get_metadata() ) )
  {
    throw BadProperty( "can only join GIDCollections with same metadata" );
  }
  GIDCollectionPrimitive const * const rhs_ptr = 
    dynamic_cast< GIDCollectionPrimitive const * >( rhs.get() );
  rhs.unlock();
  
  if ( rhs_ptr ) // if rhs is Primitive
  {
    if ( ( last_ + 1 ) == rhs_ptr->first_ and model_id_ == rhs_ptr->model_id_ ) 
    // if contiguous and homogenous
    {
      return GIDCollectionPTR ( new GIDCollectionPrimitive( first_, 
                                 rhs_ptr->last_, model_id_, metadata_ ) );
    }
    else if ( ( rhs_ptr->last_ + 1 ) == first_ 
                                           and model_id_ == rhs_ptr->model_id_ )
    {
      return GIDCollectionPTR ( new GIDCollectionPrimitive( rhs_ptr->first_, 
                                                           last_, model_id_ ) );
    }
    else // not contiguous and homogenous
    {
      std::vector<GIDCollectionPrimitive> primitives;
      primitives.reserve( 2 );
      primitives.push_back(*this);
      primitives.push_back(*rhs_ptr);
      return GIDCollectionPTR ( new GIDCollectionComposite( primitives ) );
    }
  }
  else // if rhs is not Primitive, i.e. Composite
  {
    GIDCollectionComposite * rhs_ptr = 
      dynamic_cast< GIDCollectionComposite * >( rhs.get() );
    rhs.unlock();
    return GIDCollectionPTR( *rhs_ptr + *this ); // use Composite operator+
  }  
}

GIDCollectionPTR 
GIDCollectionPrimitive::GIDCollectionPrimitive::slice( size_t start,
                                                       size_t stop,
															                         size_t step ) const
{
	  if ( not ( start < stop ) )
	  {
		throw BadParameter( "start < stop required." );
	  }
	  if ( not ( stop <= size() ) )
	  {
	    throw BadParameter( "stop <= size() required." );
	  }

  if ( step == 1 )
  {
    return GIDCollectionPTR( new GIDCollectionPrimitive( first_ + start,
                             first_ + stop - 1, model_id_, metadata_ ) );
  }
  else
  {
    return GIDCollectionPTR( new GIDCollectionComposite( *this, start, stop, step ) );
  }
}

void // TODO: is this needed?
GIDCollectionPrimitive::print_me( std::ostream& out ) const
{
  out << "[[size=" << size() << ",";
  out << "(" << first_ << ".." << last_ << ")";
  out << "]]";
}

// make a Primitive from start to stop with step into a Composite
GIDCollectionComposite::GIDCollectionComposite( const GIDCollectionPrimitive& 
                                  prim, size_t start, size_t stop, size_t step )
{
  throw KernelException("not implemented yet");
  // TODO: Use a single primitive in parts_, with the optional step parameter?
}

// Composite copy constructor
GIDCollectionComposite::GIDCollectionComposite( const GIDCollectionComposite& comp )
  : parts_( comp.parts_ )
  , size_( comp.size_ )
{
}

// construct Composite from a vector of Primitives
GIDCollectionComposite::GIDCollectionComposite( const std::vector<GIDCollectionPrimitive> prims )
  : size_( 0 )
{
  parts_.reserve( prims.size() );
  for ( std::vector<GIDCollectionPrimitive>::const_iterator gc = prims.begin(); 
                                                       gc != prims.end(); ++gc )
  {
    if ( not ( ( *gc ).get_metadata() == prims[0].get_metadata() ))
    {
      throw BadProperty( "all metadata in a GIDCollection must be the same" );
    }
    parts_.push_back( *gc );
    size_ += ( *gc ).size();
  }
}

GIDCollectionPTR
GIDCollectionComposite::operator+( GIDCollectionPTR rhs ) const
{
  if ( not ( get_metadata() == rhs->get_metadata() ) )
  {
    throw BadProperty( "can only join GIDCollections with the same metadata" );
  }
  GIDCollectionPrimitive const * const rhs_ptr = 
    dynamic_cast< GIDCollectionPrimitive const * >( rhs.get() );
  rhs.unlock();
  if ( rhs_ptr ) // if rhs is Primitive
  {
    return GIDCollectionPTR( *this + *rhs_ptr );
  }
  else // if rhs is not Primitive, i.e. Composite
  {
    GIDCollectionComposite const * const rhs_ptr = 
      dynamic_cast< GIDCollectionComposite const * >( rhs.get() );
    rhs.unlock();
    GIDCollectionComposite* new_composite = new GIDCollectionComposite( *this );
    new_composite->parts_.reserve( new_composite->parts_.size() 
                                   + rhs_ptr->parts_.size() );
    for ( std::vector<GIDCollectionPrimitive>::const_iterator prim = 
            rhs_ptr->parts_.begin(); prim != rhs_ptr->parts_.end(); ++prim )
    {
       new_composite->parts_.push_back( *prim );
       new_composite->size_ += ( *prim ).size();
    }
    return GIDCollectionPTR( new_composite );
  }
}

GIDCollectionPTR
GIDCollectionComposite::operator+( const GIDCollectionPrimitive& rhs ) const
{
  GIDCollectionComposite* new_composite = new GIDCollectionComposite( *this );
  new_composite->parts_.push_back( rhs );
  new_composite->size_ += rhs.size();  
  return GIDCollectionPTR( new_composite );
}

GIDCollectionPTR
GIDCollectionComposite::slice( size_t first, size_t last, size_t step ) const
{
  throw KernelException("not implemented yet");
  /* TODO: IMPLEMENT THIS
  bool primitive = true;
  std::vector<index> gids; // temp storage for gids to be returned
  size_t i_count = 0; // index count
  size_t next_step = 0; // index of next to save
  for ( const_iterator gid = begin(); gid != end; ++gid, ++i )
  {
    if ( i_count == next_step )
    {
      
      next_step += step;
    }
  }
    
  return GIDCollectionPTR(0);
  */
}

void
GIDCollectionComposite::print_me( std::ostream& out ) const
{
  throw KernelException("not implemented");
  /*
  out << "[[size=" << size() << ",";
  out << "(" << first_ << ".." << last_ << ")";
  out << "]]";
  */
}

} // namespace nest
