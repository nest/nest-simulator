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

gc_const_iterator::gc_const_iterator( const GIDCollectionPrimitive& collection,
  size_t offset )
  : coll_ptr_( 0 )
  , element_idx_( offset )
  , part_idx_( 0 )
  , primitive_collection_( &collection )
  , composite_collection_( 0 )
{
  if ( offset > collection.size() ) // allow == size() for end iterator
  {
    throw KernelException( "Invalid offset into GIDCollectionPrimitive" );
  }
}

gc_const_iterator::gc_const_iterator( const GIDCollectionComposite& collection,
  size_t part,
  size_t offset )
  : coll_ptr_( 0 )
  , element_idx_( offset )
  , part_idx_( part )
  , primitive_collection_( 0 )
  , composite_collection_( &collection )
{
  if ( ( part >= collection.parts_.size()
         or offset >= collection.parts_[ part ].size() )
    and not( part == collection.parts_.size() and offset == 0 ) // end iterator
    )
  {
    throw KernelException(
      "Invalid part or offset into GIDCollectionComposite" );
  }
}

gc_const_iterator::gc_const_iterator( GIDCollectionPTR collection_ptr,
  const GIDCollectionPrimitive& collection,
  size_t offset )
  : coll_ptr_( collection_ptr )
  , element_idx_( offset )
  , part_idx_( 0 )
  , primitive_collection_( &collection )
  , composite_collection_( 0 )
{
  assert( collection_ptr.get() == &collection );
  collection_ptr.unlock();

  if ( offset > collection.size() ) // allow == size() for end iterator
  {
    throw KernelException( "Invalid offset into GIDCollectionPrimitive" );
  }
}

gc_const_iterator::gc_const_iterator( GIDCollectionPTR collection_ptr,
  const GIDCollectionComposite& collection,
  size_t part,
  size_t offset )
  : coll_ptr_( collection_ptr )
  , element_idx_( offset )
  , part_idx_( part )
  , primitive_collection_( 0 )
  , composite_collection_( &collection )
{
  assert( collection_ptr.get() == &collection );
  collection_ptr.unlock();

  if ( ( part >= collection.parts_.size()
         or offset >= collection.parts_[ part ].size() )
    and not( part == collection.parts_.size() and offset == 0 ) // end iterator
    )
  {
    throw KernelException(
      "Invalid part or offset into GIDCollectionComposite" );
  }
}

gc_const_iterator::gc_const_iterator( const gc_const_iterator& gci )
  : element_idx_( gci.element_idx_ )
  , part_idx_( gci.part_idx_ )
  , primitive_collection_( gci.primitive_collection_ )
  , composite_collection_( gci.composite_collection_ )
{
}

void
gc_const_iterator::print_me( std::ostream& out ) const
{
  out << "[[" << this << " pc: " << primitive_collection_
      << ", cc: " << composite_collection_ << ", px: " << part_idx_
      << ", ex: " << element_idx_ << "]]";
}

GIDCollectionPTR operator+( GIDCollectionPTR lhs, GIDCollectionPTR rhs )
{
  return lhs->operator+( rhs );
}

GIDCollectionPTR
GIDCollection::create( IntVectorDatum gidsdatum )
{
  if ( gidsdatum->size() == 0 )
  {
    throw KernelException( "Cannot create empty GIDCollection" );
  }

  std::vector< index > gids;
  gids.reserve( gidsdatum->size() );
  for ( std::vector< long >::const_iterator it = gidsdatum->begin();
        it != gidsdatum->end();
        ++it )
  {
    gids.push_back( static_cast< index >( getValue< long >( *it ) ) );
  }
  std::sort( gids.begin(), gids.end() );

  return GIDCollection::create_( gids );
}

GIDCollectionPTR
GIDCollection::create( TokenArray gidsarray )
{
  if ( gidsarray.size() == 0 )
  {
    throw BadProperty( "Cannot create empty GIDCollection" );
  }

  std::vector< index > gids;
  gids.reserve( gidsarray.size() );
  for ( Token const* it = gidsarray.begin(); it != gidsarray.end(); ++it )
  {
    gids.push_back( static_cast< index >( getValue< long >( *it ) ) );
  }
  std::sort( gids.begin(), gids.end() );

  return GIDCollection::create_( gids );
}

GIDCollectionPTR
GIDCollection::create_( const std::vector< index >& gids )
{
  index current_first = gids[ 0 ];
  index current_last = current_first;
  index current_model =
    kernel().node_manager.get_node( gids[ 0 ] )->get_model_id();

  std::vector< GIDCollectionPrimitive > parts;

  for ( std::vector< index >::const_iterator gid = ++( gids.begin() );
        gid != gids.end();
        ++gid )
  {
    index next_model = kernel().node_manager.get_node( *gid )->get_model_id();

    if ( next_model == current_model and *gid == ( current_last + 1 ) )
    {
      // node goes in Primitive
      ++current_last;
    }
    else
    {
      // store Primitive; node goes in new Primitive
      parts.push_back(
        GIDCollectionPrimitive( current_first, current_last, current_model ) );
      current_first = *gid;
      current_last = current_first;
      current_model = next_model;
    }
  }

  // now push last section we opened
  parts.push_back(
    GIDCollectionPrimitive( current_first, current_last, current_model ) );

  if ( parts.size() == 1 )
  {
    return GIDCollectionPTR( new GIDCollectionPrimitive( parts[ 0 ] ) );
  }
  else
  {
    return GIDCollectionPTR( new GIDCollectionComposite( parts ) );
  }
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

ArrayDatum
GIDCollectionPrimitive::to_array() const
{
  ArrayDatum gids;
  gids.reserve( size() );
  for ( const_iterator it = begin(); it != end(); ++it )
  {
    gids.push_back( ( *it ).gid );
  }
  return gids;
}

ArrayDatum
GIDCollectionComposite::to_array() const
{
  ArrayDatum gids;
  gids.reserve( size() );
  for ( const_iterator it = begin(); it != end(); ++it )
  {
    gids.push_back( ( *it ).gid );
  }
  return gids;
}

GIDCollectionPTR GIDCollectionPrimitive::operator+( GIDCollectionPTR rhs ) const
{
  if ( get_metadata().valid() and not( get_metadata() == rhs->get_metadata() ) )
  {
    throw BadProperty( "can only join GIDCollections with same metadata" );
  }
  GIDCollectionPrimitive const* const rhs_ptr =
    dynamic_cast< GIDCollectionPrimitive const* >( rhs.get() );
  rhs.unlock();

  if ( rhs_ptr ) // if rhs is Primitive
  {
    if ( ( last_ + 1 ) == rhs_ptr->first_ and model_id_ == rhs_ptr->model_id_ )
    // if contiguous and homogenous
    {
      return GIDCollectionPTR( new GIDCollectionPrimitive(
        first_, rhs_ptr->last_, model_id_, metadata_ ) );
    }
    else if ( ( rhs_ptr->last_ + 1 ) == first_
      and model_id_ == rhs_ptr->model_id_ )
    {
      return GIDCollectionPTR( new GIDCollectionPrimitive(
        rhs_ptr->first_, last_, model_id_, metadata_ ) );
    }
    else // not contiguous and homogenous
    {
      std::vector< GIDCollectionPrimitive > primitives;
      primitives.reserve( 2 );
      primitives.push_back( *this );
      primitives.push_back( *rhs_ptr );
      return GIDCollectionPTR( new GIDCollectionComposite( primitives ) );
    }
  }
  else // if rhs is not Primitive, i.e. Composite
  {
    GIDCollectionComposite* rhs_ptr =
      dynamic_cast< GIDCollectionComposite* >( rhs.get() );
    rhs.unlock();
    assert( rhs_ptr );
    return rhs_ptr->operator+( *this ); // use Composite operator+
  }
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
      new GIDCollectionComposite( *this, start, stop, step ) );
  }
}

void // TODO: is this needed?
  GIDCollectionPrimitive::print_me( std::ostream& out ) const
{
  out << "[[" << this << " model=" << model_id_ << ", size=" << size() << " ";
  out << "(" << first_ << ".." << last_ << ")";
  out << "]]";
}

// make a Primitive from start to stop with step into a Composite
GIDCollectionComposite::GIDCollectionComposite(
  const GIDCollectionPrimitive& prim,
  size_t start,
  size_t stop,
  size_t step )
{
  throw KernelException( "not implemented yet" );
  // TODO: Use a single primitive in parts_, with the optional step parameter?
}

// Composite copy constructor
GIDCollectionComposite::GIDCollectionComposite(
  const GIDCollectionComposite& comp )
  : parts_( comp.parts_ )
  , size_( comp.size_ )
{
}

// construct Composite from a vector of Primitives
GIDCollectionComposite::GIDCollectionComposite(
  const std::vector< GIDCollectionPrimitive >& parts )
  : size_( 0 )
{
  if ( parts.size() < 1 )
  {
    throw BadProperty( "Cannot create empty GIDCollection" );
  }

  GIDCollectionMetadataPTR meta = parts[ 0 ].get_metadata();
  parts_.reserve( parts.size() );
  for (
    std::vector< GIDCollectionPrimitive >::const_iterator gc = parts.begin();
    gc != parts.end();
    ++gc )
  {
    if ( meta.valid() and not( meta == ( *gc ).get_metadata() ) )
    {
      throw BadProperty( "all metadata in a GIDCollection must be the same" );
    }
    parts_.push_back( *gc );
    size_ += ( *gc ).size();
  }
}

GIDCollectionPTR GIDCollectionComposite::operator+( GIDCollectionPTR rhs ) const
{
  if ( get_metadata().valid() and not( get_metadata() == rhs->get_metadata() ) )
  {
    throw BadProperty( "can only join GIDCollections with the same metadata" );
  }
  GIDCollectionPrimitive const* const rhs_ptr =
    dynamic_cast< GIDCollectionPrimitive const* >( rhs.get() );
  rhs.unlock();
  if ( rhs_ptr ) // if rhs is Primitive
  {
    return GIDCollectionPTR( *this + *rhs_ptr );
  }
  else // if rhs is not Primitive, i.e. Composite
  {
    GIDCollectionComposite const* const rhs_ptr =
      dynamic_cast< GIDCollectionComposite const* >( rhs.get() );
    rhs.unlock();
    GIDCollectionComposite* new_composite = new GIDCollectionComposite( *this );
    new_composite->parts_.reserve(
      new_composite->parts_.size() + rhs_ptr->parts_.size() );
    for ( std::vector< GIDCollectionPrimitive >::const_iterator prim =
            rhs_ptr->parts_.begin();
          prim != rhs_ptr->parts_.end();
          ++prim )
    {
      new_composite->parts_.push_back( *prim );
      new_composite->size_ += ( *prim ).size();
    }
    return GIDCollectionPTR( new_composite );
  }
}

// function object for sorting a vector of GIDCollcetionPrimitives
class primitiveSort
{
public:
  bool operator()( GIDCollectionPrimitive primitive_1, GIDCollectionPrimitive primitive_2 )
  {
    return primitive_1[ 0 ] < primitive_2[ 0 ];
  }
};

GIDCollectionPTR GIDCollectionComposite::operator+(
  const GIDCollectionPrimitive& rhs ) const
{
  if ( get_metadata().valid() and not( get_metadata() == rhs.get_metadata() ) )
  {
    throw BadProperty( "can only join GIDCollections with the same metadata" );
  }

  std::vector< GIDCollectionPrimitive > new_parts = parts_;
  new_parts.push_back( rhs );
  return GIDCollectionPTR( new GIDCollectionComposite( new_parts ) );
}


GIDCollectionPTR
GIDCollectionComposite::slice( size_t first, size_t last, size_t step ) const
{
  throw KernelException( "not implemented yet" );
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
  out << "[[" << this << " size=" << size() << ": ";
  for (
    std::vector< GIDCollectionPrimitive >::const_iterator it = parts_.begin();
    it != parts_.end();
    ++it )
  {
    it->print_me( out );
  }
  out << "]]";
}

} // namespace nest
