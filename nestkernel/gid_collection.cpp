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
#include "vp_manager_impl.h"
#include "mpi_manager_impl.h"


// C++ includes:
#include <algorithm> // copy

namespace nest
{

// function object for sorting a vector of GIDCollcetionPrimitives
struct PrimitiveSortObject
{
  bool operator()( GIDCollectionPrimitive& primitive_lhs, GIDCollectionPrimitive& primitive_rhs )
  {
    return primitive_lhs[ 0 ] < primitive_rhs[ 0 ];
  }

  bool operator()( const GIDCollectionPrimitive& primitive_lhs, const GIDCollectionPrimitive& primitive_rhs )
  {
    return primitive_lhs[ 0 ] < primitive_rhs[ 0 ];
  }
} primitiveSort;


gc_const_iterator::gc_const_iterator( GIDCollectionPTR collection_ptr,
  const GIDCollectionPrimitive& collection,
  size_t offset,
  size_t step )
  : coll_ptr_( collection_ptr )
  , element_idx_( offset )
  , part_idx_( 0 )
  , step_( step )
  , primitive_collection_( &collection )
  , composite_collection_( 0 )
{
  assert( not collection_ptr.get() or collection_ptr.get() == &collection );

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
  assert( not collection_ptr.get() or collection_ptr.get() == &collection );

  if ( ( part >= collection.parts_.size() or offset >= collection.parts_[ part ].size() )
    and not( part == collection.parts_.size() and offset == 0 ) // end iterator
    )
  {
    throw KernelException( "Invalid part or offset into GIDCollectionComposite" );
  }
}

gc_const_iterator::gc_const_iterator( const gc_const_iterator& gci )
  : coll_ptr_( gci.coll_ptr_ )
  , element_idx_( gci.element_idx_ )
  , part_idx_( gci.part_idx_ )
  , step_( gci.step_ )
  , primitive_collection_( gci.primitive_collection_ )
  , composite_collection_( gci.composite_collection_ )
{
}

void
gc_const_iterator::print_me( std::ostream& out ) const
{
  out << "[[" << this << " pc: " << primitive_collection_ << ", cc: " << composite_collection_ << ", px: " << part_idx_
      << ", ex: " << element_idx_ << "]]";
}

GIDCollectionPTR operator+( GIDCollectionPTR lhs, GIDCollectionPTR rhs )
{
  return lhs->operator+( rhs );
}

GIDCollection::GIDCollection()
  : fingerprint_( kernel().get_fingerprint() )
{
}

GIDCollectionPTR
GIDCollection::create( IntVectorDatum gidsdatum )
{
  if ( gidsdatum->size() == 0 )
  {
    return GIDCollection::create_();
  }

  std::vector< index > gids;
  gids.reserve( gidsdatum->size() );
  for ( std::vector< long >::const_iterator it = gidsdatum->begin(); it != gidsdatum->end(); ++it )
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
    return GIDCollection::create_();
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
GIDCollection::create_()
{
  return GIDCollectionPTR( new GIDCollectionPrimitive( 0, 0, invalid_index ) );
}

GIDCollectionPTR
GIDCollection::create_( const std::vector< index >& gids )
{
  index current_first = gids[ 0 ];
  index current_last = current_first;
  index current_model = kernel().modelrange_manager.get_model_id( gids[ 0 ] );

  std::vector< GIDCollectionPrimitive > parts;

  index old_gid = 0;
  for ( std::vector< index >::const_iterator gid = ++( gids.begin() ); gid != gids.end(); ++gid )
  {
    if ( *gid == old_gid )
    {
      throw BadProperty( "All GIDs in a GIDCollection have to be unique" );
    }
    old_gid = *gid;

    index next_model = kernel().modelrange_manager.get_model_id( *gid );

    if ( next_model == current_model and *gid == ( current_last + 1 ) )
    {
      // node goes in Primitive
      ++current_last;
    }
    else
    {
      // store Primitive; node goes in new Primitive
      parts.push_back( GIDCollectionPrimitive( current_first, current_last, current_model ) );
      current_first = *gid;
      current_last = current_first;
      current_model = next_model;
    }
  }

  // now push last section we opened
  parts.push_back( GIDCollectionPrimitive( current_first, current_last, current_model ) );

  if ( parts.size() == 1 )
  {
    return GIDCollectionPTR( new GIDCollectionPrimitive( parts[ 0 ] ) );
  }
  else
  {
    return GIDCollectionPTR( new GIDCollectionComposite( parts ) );
  }
}

bool
GIDCollection::valid() const
{
  return fingerprint_ == kernel().get_fingerprint();
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first, index last, index model_id, GIDCollectionMetadataPTR meta )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( meta )
{
  assert( first_ <= last_ );
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first, index last, index model_id )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( 0 )
{
  assert( first_ <= last_ );
}

GIDCollectionPrimitive::GIDCollectionPrimitive( index first, index last )
  : first_( first )
  , last_( last )
  , model_id_( invalid_index )
  , metadata_( 0 )
{
  assert( first_ <= last_ );

  // find the model_id
  const int first_model_id = kernel().modelrange_manager.get_model_id( first );
  for ( index gid = ++first; gid <= last; ++gid )
  {
    const int model_id = kernel().modelrange_manager.get_model_id( gid );
    if ( model_id != first_model_id )
    {
      throw BadProperty( "model ids does not match" );
    }
  }
  model_id_ = first_model_id;
}

GIDCollectionPrimitive::GIDCollectionPrimitive( const GIDCollectionPrimitive& rhs )
  : first_( rhs.first_ )
  , last_( rhs.last_ )
  , model_id_( rhs.model_id_ )
  , metadata_( rhs.metadata_ )
{
}

GIDCollectionPrimitive::GIDCollectionPrimitive()
  : first_( 0 )
  , last_( 0 )
  , model_id_( invalid_index )
  , metadata_( 0 )
{
}

ArrayDatum
GIDCollectionPrimitive::to_array() const
{
  ArrayDatum gids;
  gids.reserve( size() );
  for ( const_iterator it = begin(); it < end(); ++it )
  {
    gids.push_back( ( *it ).gid );
  }
  return gids;
}

GIDCollectionPTR GIDCollectionPrimitive::operator+( GIDCollectionPTR rhs ) const
{
  if ( ( get_metadata().get() or rhs->get_metadata().get() ) and not( get_metadata() == rhs->get_metadata() ) )
  {
    throw BadProperty( "Can only join GIDCollections with same metadata." );
  }
  if ( not valid() or not rhs->valid() )
  {
    throw KernelException( "InvalidGIDCollection" );
  }
  GIDCollectionPrimitive const* const rhs_ptr = dynamic_cast< GIDCollectionPrimitive const* >( rhs.get() );

  if ( rhs_ptr ) // if rhs is Primitive
  {
    if ( overlapping( *rhs_ptr ) )
    {
      throw BadProperty( "Cannot join overlapping GIDCollections." );
    }
    if ( ( last_ + 1 ) == rhs_ptr->first_ and model_id_ == rhs_ptr->model_id_ )
    // if contiguous and homogeneous
    {
      return GIDCollectionPTR( new GIDCollectionPrimitive( first_, rhs_ptr->last_, model_id_, metadata_ ) );
    }
    else if ( ( rhs_ptr->last_ + 1 ) == first_ and model_id_ == rhs_ptr->model_id_ )
    {
      return GIDCollectionPTR( new GIDCollectionPrimitive( rhs_ptr->first_, last_, model_id_, metadata_ ) );
    }
    else // not contiguous and homogeneous
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
    GIDCollectionComposite* rhs_ptr = dynamic_cast< GIDCollectionComposite* >( rhs.get() );
    assert( rhs_ptr );
    return rhs_ptr->operator+( *this ); // use Composite operator+
  }
}

GIDCollectionPrimitive::const_iterator
GIDCollectionPrimitive::local_begin( GIDCollectionPTR cp ) const
{
  size_t num_vps = kernel().vp_manager.get_num_virtual_processes();
  size_t current_vp = kernel().vp_manager.thread_to_vp( kernel().vp_manager.get_thread_id() );
  size_t vp_first_node = kernel().vp_manager.suggest_vp_for_gid( first_ );
  size_t offset = ( current_vp - vp_first_node + num_vps ) % num_vps;

  if ( offset >= size() ) // Too few GIDs to be shared among all vps.
  {
    return const_iterator( cp, *this, size() );
  }
  else
  {
    return const_iterator( cp, *this, offset, num_vps );
  }
}

GIDCollectionPrimitive::const_iterator
GIDCollectionPrimitive::MPI_local_begin( GIDCollectionPTR cp ) const
{
  size_t num_processes = kernel().mpi_manager.get_num_processes();
  size_t rank = kernel().mpi_manager.get_rank();
  size_t rank_first_node =
    kernel().mpi_manager.get_process_id_of_vp( kernel().vp_manager.suggest_vp_for_gid( first_ ) );
  size_t offset = ( rank - rank_first_node + num_processes ) % num_processes;
  if ( offset > size() ) // Too few GIDs to be shared among all MPI processes.
  {
    return const_iterator( cp, *this, size() );
  }
  else
  {
    return const_iterator( cp, *this, offset, num_processes );
  }
}

GIDCollectionPTR
GIDCollectionPrimitive::GIDCollectionPrimitive::slice( size_t start, size_t stop, size_t step ) const
{
  if ( not( start < stop ) )
  {
    throw BadParameter( "start < stop required." );
  }
  if ( not( stop <= size() ) )
  {
    throw BadParameter( "stop <= size() required." );
  }
  if ( not valid() )
  {
    throw KernelException( "InvalidGIDCollection" );
  }

  if ( step == 1 )
  {
    return GIDCollectionPTR( new GIDCollectionPrimitive( first_ + start, first_ + stop - 1, model_id_, metadata_ ) );
  }
  else
  {
    return GIDCollectionPTR( new GIDCollectionComposite( *this, start, stop, step ) );
  }
}

void
GIDCollectionPrimitive::print_me( std::ostream& out ) const
{
  std::string metadata = metadata_.get() ? metadata_->get_type() : "None";

  out << "GIDCollection("
      << "metadata=" << metadata << ", ";
  print_primitive( out );
  out << ")";
}

void
GIDCollectionPrimitive::print_primitive( std::ostream& out ) const
{
  std::string model = model_id_ != invalid_index ? kernel().model_manager.get_model( model_id_ )->get_name() : "none";

  out << "model=" << model << ", size=" << size();

  if ( size() == 1 )
  {
    out << ", first=" << first_;
  }
  else
  {
    out << ", first=" << first_ << ", last=" << last_;
  }
}

bool
GIDCollectionPrimitive::is_contiguous_ascending( GIDCollectionPrimitive& other )
{
  return ( ( last_ + 1 ) == other.first_ ) and ( model_id_ == other.model_id_ );
}

bool
GIDCollectionPrimitive::overlapping( const GIDCollectionPrimitive& rhs ) const
{
  return ( ( rhs.first_ <= last_ and rhs.first_ >= first_ ) or ( rhs.last_ <= last_ and rhs.last_ >= first_ ) );
}

GIDCollectionComposite::GIDCollectionComposite( const GIDCollectionPrimitive& primitive,
  size_t start,
  size_t stop,
  size_t step )
  : size_( floor( ( stop - start ) / ( float ) step ) + ( ( stop - start ) % step > 0 ) )
  , step_( step )
  , start_part_( 0 )
  , start_offset_( start )
  , stop_part_( stop == primitive.size() )
  , stop_offset_( stop == primitive.size() ? 0 : stop )
{
  parts_.reserve( 1 );
  parts_.push_back( primitive );
}

GIDCollectionComposite::GIDCollectionComposite( const GIDCollectionComposite& comp )
  : parts_( comp.parts_ )
  , size_( comp.size_ )
  , step_( 1 )
  , start_part_( 0 )
  , start_offset_( 0 )
  , stop_part_( 0 )
  , stop_offset_( 0 )
{
}

GIDCollectionComposite::GIDCollectionComposite( const std::vector< GIDCollectionPrimitive >& parts )
  : size_( 0 )
  , step_( 1 )
  , start_part_( 0 )
  , start_offset_( 0 )
  , stop_part_( 0 )
  , stop_offset_( 0 )
{
  if ( parts.size() < 1 )
  {
    throw BadProperty( "Cannot create an empty composite GIDCollection" );
  }

  GIDCollectionMetadataPTR meta = parts[ 0 ].get_metadata();
  parts_.reserve( parts.size() );
  for ( std::vector< GIDCollectionPrimitive >::const_iterator gc = parts.begin(); gc != parts.end(); ++gc )
  {
    if ( meta.get() and not( meta == ( *gc ).get_metadata() ) )
    {
      throw BadProperty( "all metadata in a GIDCollection must be the same" );
    }
    parts_.push_back( *gc );
    size_ += ( *gc ).size();
  }
  std::sort( parts_.begin(), parts_.end(), primitiveSort );
}

GIDCollectionComposite::GIDCollectionComposite( const GIDCollectionComposite& composite,
  size_t start,
  size_t stop,
  size_t step )
  : parts_( composite.parts_ )
  , size_( floor( ( stop - start ) / ( float ) step ) + ( ( stop - start ) % step > 0 ) )
  , step_( step )
  , start_part_( 0 )
  , start_offset_( 0 )
  , stop_part_( composite.parts_.size() )
  , stop_offset_( 0 )
{
  if ( stop - start < 1 )
  {
    throw BadProperty( "Cannot create an empty composite GIDCollection." );
  }
  if ( start > composite.size() or stop > composite.size() )
  {
    throw BadProperty( "Index out of range." );
  }
  if ( composite.step_ > 1 or composite.stop_part_ != 0 or composite.stop_offset_ != 0 )
  {
    throw BadProperty( "Cannot slice a sliced composite GIDCollection." );
  }

  size_t global_index = 0;
  for ( const_iterator it = composite.begin(); it < composite.end(); ++it )
  {
    if ( global_index == start )
    {
      it.get_current_part_offset( start_part_, start_offset_ );
    }
    else if ( global_index == stop )
    {
      it.get_current_part_offset( stop_part_, stop_offset_ );
      break;
    }
    ++global_index;
  }
}

GIDCollectionPTR GIDCollectionComposite::operator+( GIDCollectionPTR rhs ) const
{
  if ( get_metadata().get() and not( get_metadata() == rhs->get_metadata() ) )
  {
    throw BadProperty( "can only join GIDCollections with the same metadata" );
  }
  if ( not valid() or not rhs->valid() )
  {
    throw KernelException( "InvalidGIDCollection" );
  }
  if ( step_ > 1 or stop_part_ != 0 or stop_offset_ != 0 )
  {
    throw BadProperty( "Cannot add GIDCollection to a sliced composite." );
  }
  GIDCollectionPrimitive const* const rhs_ptr = dynamic_cast< GIDCollectionPrimitive const* >( rhs.get() );
  if ( rhs_ptr ) // if rhs is Primitive
  {
    // check primitives in the composite for overlap
    for ( std::vector< GIDCollectionPrimitive >::const_iterator this_it = parts_.begin(); this_it < parts_.end();
          ++this_it )
    {
      if ( this_it->overlapping( *rhs_ptr ) )
      {
        throw BadProperty( "Cannot join overlapping GIDCollections." );
      }
    }
    return GIDCollectionPTR( *this + *rhs_ptr );
  }
  else // rhs is Composite
  {
    GIDCollectionComposite const* const rhs_ptr = dynamic_cast< GIDCollectionComposite const* >( rhs.get() );
    if ( rhs_ptr->step_ > 1 or rhs_ptr->stop_part_ != 0 or rhs_ptr->stop_offset_ != 0 )
    {
      throw BadProperty( "Cannot add GIDCollection to a sliced composite." );
    }

    // check overlap between the two composites
    const GIDCollectionComposite* shortest, *longest;
    if ( size() < rhs_ptr->size() )
    {
      shortest = this;
      longest = rhs_ptr;
    }
    else
    {
      shortest = rhs_ptr;
      longest = this;
    }
    for ( GIDCollectionComposite::const_iterator short_it = shortest->begin(); short_it < shortest->end(); ++short_it )
    {
      if ( longest->contains( ( *short_it ).gid ) )
      {
        throw BadProperty( "Cannot join overlapping GIDCollections." );
      }
    }

    GIDCollectionComposite* new_composite = new GIDCollectionComposite( *this );
    new_composite->parts_.reserve( new_composite->parts_.size() + rhs_ptr->parts_.size() );
    for ( std::vector< GIDCollectionPrimitive >::const_iterator prim = rhs_ptr->parts_.begin();
          prim != rhs_ptr->parts_.end();
          ++prim )
    {
      new_composite->parts_.push_back( *prim );
      new_composite->size_ += ( *prim ).size();
    }
    std::sort( new_composite->parts_.begin(), new_composite->parts_.end(), primitiveSort );
    merge_parts( new_composite->parts_ );
    if ( new_composite->parts_.size() == 1 )
    {
      // If there is only a single primitive in the composite, we extract it,
      // ensuring that the composite is deleted from memory.
      GIDCollectionPrimitive new_primitive = new_composite->parts_[ 0 ];
      delete new_composite;
      return GIDCollectionPTR( new GIDCollectionPrimitive( new_primitive ) );
    }
    else
    {
      return GIDCollectionPTR( new_composite );
    }
  }
}

GIDCollectionPTR GIDCollectionComposite::operator+( const GIDCollectionPrimitive& rhs ) const
{
  if ( get_metadata().get() and not( get_metadata() == rhs.get_metadata() ) )
  {
    throw BadProperty( "can only join GIDCollections with the same metadata" );
  }

  // check primitives in the composites for overlap
  for ( std::vector< GIDCollectionPrimitive >::const_iterator this_it = parts_.begin(); this_it < parts_.end();
        ++this_it )
  {
    if ( this_it->overlapping( rhs ) )
    {
      throw BadProperty( "Cannot join overlapping GIDCollections." );
    }
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

GIDCollectionComposite::const_iterator
GIDCollectionComposite::local_begin( GIDCollectionPTR cp ) const
{
  size_t num_vps = kernel().vp_manager.get_num_virtual_processes();
  size_t current_vp = kernel().vp_manager.thread_to_vp( kernel().vp_manager.get_thread_id() );
  size_t vp_first_node = kernel().vp_manager.suggest_vp_for_gid( operator[]( 0 ) );
  size_t offset = ( current_vp - vp_first_node + num_vps ) % num_vps;

  size_t current_part = start_part_;
  size_t current_offset = start_offset_;
  if ( offset )
  {
    // First create an iterator at the start position.
    const_iterator tmp_it = const_iterator( cp, *this, start_part_, start_offset_, step_ );
    tmp_it += offset; // Go forward to the offset.
    // Get current position.
    tmp_it.get_current_part_offset( current_part, current_offset );
  }

  return const_iterator( cp, *this, current_part, current_offset, num_vps * step_ );
}

GIDCollectionComposite::const_iterator
GIDCollectionComposite::MPI_local_begin( GIDCollectionPTR cp ) const
{
  size_t num_processes = kernel().mpi_manager.get_num_processes();
  size_t rank = kernel().mpi_manager.get_rank();
  size_t rank_first_node =
    kernel().mpi_manager.get_process_id_of_vp( kernel().vp_manager.suggest_vp_for_gid( operator[]( 0 ) ) );
  size_t offset = ( rank - rank_first_node + num_processes ) % num_processes;

  size_t current_part = start_part_;
  size_t current_offset = start_offset_;
  if ( offset )
  {
    // First create an iterator at the start position.
    const_iterator tmp_it = const_iterator( cp, *this, start_part_, start_offset_, step_ );
    tmp_it += offset; // Go forward to the offset.
    // Get current position.
    tmp_it.get_current_part_offset( current_part, current_offset );
  }

  return const_iterator( cp, *this, current_part, current_offset, num_processes * step_ );
}

ArrayDatum
GIDCollectionComposite::to_array() const
{
  ArrayDatum gids;
  gids.reserve( size() );
  for ( const_iterator it = begin(); it < end(); ++it )
  {
    gids.push_back( ( *it ).gid );
  }
  return gids;
}

GIDCollectionPTR
GIDCollectionComposite::slice( size_t start, size_t stop, size_t step ) const
{
  if ( not valid() )
  {
    throw KernelException( "InvalidGIDCollection" );
  }
  return GIDCollectionPTR( new GIDCollectionComposite( *this, start, stop, step ) );
}

void
GIDCollectionComposite::merge_parts( std::vector< GIDCollectionPrimitive >& parts ) const
{
  bool did_merge = true; // initialize to enter the while loop
  size_t last_i = 0;
  while ( did_merge ) // if parts is changed, it has to be checked again
  {
    did_merge = false;
    for ( size_t i = last_i; i < parts.size() - 1; ++i )
    {
      if ( parts[ i ].is_contiguous_ascending( parts[ i + 1 ] ) )
      {
        GIDCollectionPTR merged_primitivesPTR =
          parts[ i ] + std::make_shared< GIDCollectionPrimitive >( parts[ i + 1 ] );
        GIDCollectionPrimitive const* const merged_primitives =
          dynamic_cast< GIDCollectionPrimitive const* >( merged_primitivesPTR.get() );

        parts[ i ] = *merged_primitives;
        parts.erase( parts.begin() + i + 1 );
        did_merge = true;
        last_i = i;
        break;
      }
    }
  }
}

void
GIDCollectionComposite::print_me( std::ostream& out ) const
{
  std::string metadata = get_metadata().get() ? get_metadata()->get_type() : "None";
  std::string gc = "GIDCollection(";
  std::string space( gc.size(), ' ' );

  if ( step_ > 1 or ( stop_part_ != 0 or stop_offset_ != 0 ) )
  {
    // Sliced composite GIDCollection

    size_t current_part = 0;
    size_t current_offset = 0;
    size_t previous_part = 4294967295; // initializing as large number (for now)
    index primitive_last = 0;

    size_t primitive_size = 0;
    GIDTriple gt;

    std::vector< std::string > string_vector;

    out << gc << "metadata=" << metadata << ",";
    for ( const_iterator it = begin(); it < end(); ++it )
    {
      it.get_current_part_offset( current_part, current_offset );
      if ( current_part != previous_part ) // New primitive
      {
        if ( it != begin() )
        {
          // Need to count the primitive, so can't start at begin()
          out << "\n" + space << "model=" << kernel().model_manager.get_model( gt.model_id )->get_name()
              << ", size=" << primitive_size << ", ";
          if ( primitive_size == 1 )
          {
            out << "first=" << gt.gid << ", last=" << gt.gid << ";";
          }
          else
          {
            out << "first=" << gt.gid << ", last=";
            out << primitive_last;
            if ( step_ > 1 )
            {
              out << ", step=" << step_ << ";";
            }
          }
        }
        primitive_size = 1;
        gt = *it;
      }
      else
      {
        ++primitive_size;
      }
      primitive_last = ( *it ).gid;
      previous_part = current_part;
    }

    // Need to also print the last primitive
    out << "\n" + space << "model=" << kernel().model_manager.get_model( gt.model_id )->get_name()
        << ", size=" << primitive_size << ", ";
    if ( primitive_size == 1 )
    {
      out << "first=" << gt.gid << ", last=" << gt.gid;
    }
    else
    {
      out << "first=" << gt.gid << ", last=";
      out << primitive_last;
      if ( step_ > 1 )
      {
        out << ", step=" << step_;
      }
    }
  }
  else
  {
    // None-sliced Composite GIDCollection
    out << gc << "metadata=" << metadata << ",";
    for ( std::vector< GIDCollectionPrimitive >::const_iterator it = parts_.begin(); it != parts_.end(); ++it )
    {
      if ( it == parts_.end() - 1 )
      {
        out << "\n" + space;
        it->print_primitive( out );
      }
      else
      {
        out << "\n" + space;
        it->print_primitive( out );
        out << ";";
      }
    }
  }
  out << ")";
}

} // namespace nest
