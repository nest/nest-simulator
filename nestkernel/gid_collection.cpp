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
  , step_( 1 )
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
  size_t offset,
  size_t step )
  : coll_ptr_( 0 )
  , element_idx_( offset )
  , part_idx_( part )
  , step_( step )
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
  , step_( 1 )
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
  size_t offset,
  size_t step )
  : coll_ptr_( collection_ptr )
  , element_idx_( offset )
  , part_idx_( part )
  , step_( step )
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
  : coll_ptr_( gci.coll_ptr_ )
  , element_idx_( gci.element_idx_ )
  , part_idx_( gci.part_idx_ )
  , step_( 1 )
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
    throw BadProperty( "Can only join GIDCollections with same metadata." );
  }
  GIDCollectionPrimitive const* const rhs_ptr =
    dynamic_cast< GIDCollectionPrimitive const* >( rhs.get() );
  rhs.unlock();

  if ( rhs_ptr ) // if rhs is Primitive
  {
    if ( ( rhs_ptr->first_ <= last_ and rhs_ptr->first_ >= first_ )
      or ( rhs_ptr->last_ <= last_ and rhs_ptr->last_ >= first_ ) )
    {
      throw BadProperty( "Cannot join overlapping GIDCollections." );
    }
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

void
GIDCollectionPrimitive::print_me( std::ostream& out ) const
{
  out << "[[" << this << " model=" << model_id_ << ", size=" << size() << " ";
  out << "(" << first_ << ".." << last_ << ")";
  out << "]]";
}

bool
GIDCollectionPrimitive::is_contigous_ascending( GIDCollectionPrimitive& next )
{
  return ( ( last_ + 1 ) == next.first_ ) and ( model_id_ == next.model_id_ );
}

// make a Primitive from start to stop with step into a Composite
GIDCollectionComposite::GIDCollectionComposite(
  const GIDCollectionPrimitive& primitive,
  size_t start,
  size_t stop,
  size_t step )
  : size_( 0 )
  , step_( 1 )
  , start_part_( 0 )
  , start_offset_( 0 )
  , stop_part_( 0 )
  , stop_offset_( 0 )
{
  for ( const_iterator it = primitive.begin() + start;
        it != primitive.begin() + stop;
        it += step )
  {
    parts_.push_back(
      GIDCollectionPrimitive( ( *it ).gid, ( *it ).gid, ( *it ).model_id ) );
    ++size_;
  }
}

// Composite copy constructor
GIDCollectionComposite::GIDCollectionComposite(
  const GIDCollectionComposite& comp )
  : parts_( comp.parts_ )
  , size_( comp.size_ )
  , step_( 1 )
  , start_part_( 0 )
  , start_offset_( 0 )
  , stop_part_( 0 )
  , stop_offset_( 0 )
{
}

// function object for sorting a vector of GIDCollcetionPrimitives
struct
{
  bool operator()( GIDCollectionPrimitive& primitive_1,
    GIDCollectionPrimitive& primitive_2 )
  {
    return primitive_1[ 0 ] < primitive_2[ 0 ];
  }
} primitiveSort;

// construct Composite from a vector of Primitives
GIDCollectionComposite::GIDCollectionComposite(
  const std::vector< GIDCollectionPrimitive >& parts )
  : size_( 0 )
  , step_( 1 )
  , start_part_( 0 )
  , start_offset_( 0 )
  , stop_part_( 0 )
  , stop_offset_( 0 )
{
  if ( parts.size() < 1 )
  {
    throw BadProperty( "Cannot create an empty GIDCollection" );
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
  std::sort( parts_.begin(), parts_.end(), primitiveSort );
}

GIDCollectionComposite::GIDCollectionComposite(
  const GIDCollectionComposite& composite,
  size_t start,
  size_t stop,
  size_t step )
  : size_( 0 )
  , step_( 0 )
  , start_part_( 0 )
  , start_offset_( 0 )
  , stop_part_( composite.parts_.size() )
  , stop_offset_( 0 )
{
  if ( stop - start < 1 )
  {
    throw BadProperty( "Cannot create an empty GIDCollection." );
  }
  if ( start > composite.size() or stop > composite.size() )
  {
    throw BadProperty( "Index out of range." );
  }
  size_t index = 0;
  for ( const_iterator it = composite.begin(); it != composite.end(); ++it )
  {
    if ( index == start )
    {
      it.set_current_part_offset( start_part_, start_offset_ );
    }
    else if ( index == stop ) // TODO: verify correct stop
    {
      it.set_current_part_offset( stop_part_, stop_offset_ );
      break;
    }
    ++index;
  }
  step_ = step;
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
    for ( const_iterator it = begin(); it != end(); ++it )
    {
      if ( rhs_ptr->contains( ( *it ).gid ) )
      {
        throw BadProperty( "Cannot join overlapping GIDCollections." );
      }
    }
    return GIDCollectionPTR( *this + *rhs_ptr );
  }
  else // if rhs is not Primitive, i.e. Composite
  {
    GIDCollectionComposite const* const rhs_ptr =
      dynamic_cast< GIDCollectionComposite const* >( rhs.get() );
    rhs.unlock();

    for ( const_iterator it = begin(); it != end(); ++it )
    {
      if ( rhs_ptr->contains( ( *it ).gid ) )
      {
        throw BadProperty( "Cannot join overlapping GIDCollections." );
      }
    }

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
    std::sort( new_composite->parts_.begin(),
      new_composite->parts_.end(),
      primitiveSort );
    merge_parts( new_composite->parts_ );
    if ( new_composite->parts_.size() == 1 )
    {
      return GIDCollectionPTR(
        new GIDCollectionPrimitive( new_composite->parts_[ 0 ] ) );
    }
    else
    {
      return GIDCollectionPTR( new_composite );
    }
  }
}

GIDCollectionPTR GIDCollectionComposite::operator+(
  const GIDCollectionPrimitive& rhs ) const
{
  if ( get_metadata().valid() and not( get_metadata() == rhs.get_metadata() ) )
  {
    throw BadProperty( "can only join GIDCollections with the same metadata" );
  }

  std::vector< GIDCollectionPrimitive > new_parts = parts_;
  new_parts.push_back( rhs );
  std::sort( new_parts.begin(), new_parts.end(), primitiveSort );
  merge_parts( new_parts );
  if ( new_parts.size() == 1 )
  {
    return GIDCollectionPTR( new GIDCollectionPrimitive( new_parts[ 0 ] ) );
  }
  else
  {
    return GIDCollectionPTR( new GIDCollectionComposite( new_parts ) );
  }
}


GIDCollectionPTR
GIDCollectionComposite::slice( size_t first, size_t last, size_t step ) const
{
  return GIDCollectionPTR( new GIDCollectionComposite( *this, first, last, step ) );
}

void
GIDCollectionComposite::merge_parts(
  std::vector< GIDCollectionPrimitive >& parts ) const
{
  bool did_merge = true;
  while ( did_merge )
  {
    did_merge = false;
    for ( size_t i = 0; i < parts.size() - 1; ++i )
    {
      if ( parts[ i ].is_contigous_ascending( parts[ i + 1 ] ) )
      {
        GIDCollectionPTR merged_primitivesPTR =
          parts[ i ] + GIDCollectionPTR( parts[ i + 1 ] );
        GIDCollectionPrimitive const* const merged_primitives =
          dynamic_cast< GIDCollectionPrimitive const* >(
            merged_primitivesPTR.get() );
        merged_primitivesPTR.unlock();

        parts[ i ] = *merged_primitives;
        parts.erase( parts.begin() + i + 1 );
        did_merge = true;
        break;
      }
    }
  }
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
