/*
 *  node_collection.cpp
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

#include "node_collection.h"
#include "kernel_manager.h"
#include "vp_manager_impl.h"
#include "mpi_manager_impl.h"


// C++ includes:
#include <algorithm> // copy
#include <numeric>   // accumulate


namespace nest
{

// function object for sorting a vector of NodeCollectionPrimitives
struct PrimitiveSortObject
{
  bool operator()( NodeCollectionPrimitive& primitive_lhs, NodeCollectionPrimitive& primitive_rhs )
  {
    return primitive_lhs[ 0 ] < primitive_rhs[ 0 ];
  }

  bool operator()( const NodeCollectionPrimitive& primitive_lhs, const NodeCollectionPrimitive& primitive_rhs )
  {
    return primitive_lhs[ 0 ] < primitive_rhs[ 0 ];
  }
} primitiveSort;


nc_const_iterator::nc_const_iterator( NodeCollectionPTR collection_ptr,
  const NodeCollectionPrimitive& collection,
  size_t offset,
  size_t step )
  : coll_ptr_( collection_ptr )
  , element_idx_( offset )
  , part_idx_( 0 )
  , step_( step )
  , primitive_collection_( &collection )
  , composite_collection_( nullptr )
{
  assert( not collection_ptr.get() or collection_ptr.get() == &collection );

  if ( offset > collection.size() ) // allow == size() for end iterator
  {
    throw KernelException( "Invalid offset into NodeCollectionPrimitive" );
  }
}

nc_const_iterator::nc_const_iterator( NodeCollectionPTR collection_ptr,
  const NodeCollectionComposite& collection,
  size_t part,
  size_t offset,
  size_t step )
  : coll_ptr_( collection_ptr )
  , element_idx_( offset )
  , part_idx_( part )
  , step_( step )
  , primitive_collection_( nullptr )
  , composite_collection_( &collection )
{
  assert( not collection_ptr.get() or collection_ptr.get() == &collection );

  if ( ( part >= collection.parts_.size() or offset >= collection.parts_[ part ].size() )
    and not( part == collection.parts_.size() and offset == 0 ) // end iterator
    )
  {
    throw KernelException( "Invalid part or offset into NodeCollectionComposite" );
  }
}

void
nc_const_iterator::print_me( std::ostream& out ) const
{
  out << "[[" << this << " pc: " << primitive_collection_ << ", cc: " << composite_collection_ << ", px: " << part_idx_
      << ", ex: " << element_idx_ << "]]";
}

NodeCollectionPTR operator+( NodeCollectionPTR lhs, NodeCollectionPTR rhs )
{
  return lhs->operator+( rhs );
}

NodeCollection::NodeCollection()
  : fingerprint_( kernel().get_fingerprint() )
{
}

NodeCollectionPTR
NodeCollection::create( const IntVectorDatum& node_idsdatum )
{
  if ( node_idsdatum->size() == 0 )
  {
    return NodeCollection::create_();
  }

  std::vector< index > node_ids;
  node_ids.reserve( node_idsdatum->size() );
  for ( std::vector< long >::const_iterator it = node_idsdatum->begin(); it != node_idsdatum->end(); ++it )
  {
    node_ids.push_back( static_cast< index >( getValue< long >( *it ) ) );
  }

  if ( not std::is_sorted( node_ids.begin(), node_ids.end() ) )
  {
    throw BadProperty( "Node IDs must be sorted in ascending order" );
  }
  return NodeCollection::create_( node_ids );
}

NodeCollectionPTR
NodeCollection::create( const TokenArray& node_idsarray )
{
  if ( node_idsarray.size() == 0 )
  {
    return NodeCollection::create_();
  }

  std::vector< index > node_ids;
  node_ids.reserve( node_idsarray.size() );
  for ( const auto& node_id_token : node_idsarray )
  {
    node_ids.push_back( static_cast< index >( getValue< long >( node_id_token ) ) );
  }

  if ( not std::is_sorted( node_ids.begin(), node_ids.end() ) )
  {
    throw BadProperty( "Node IDs must be sorted in ascending order" );
  }
  return NodeCollection::create_( node_ids );
}


NodeCollectionPTR
NodeCollection::create( const index node_id )
{
  return NodeCollection::create_( { node_id } );
}

NodeCollectionPTR
NodeCollection::create( const std::vector< index >& node_ids_vector )
{
  if ( node_ids_vector.size() == 0 )
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
NodeCollection::create_()
{
  return NodeCollectionPTR( new NodeCollectionPrimitive( 0, 0, invalid_index ) );
}

NodeCollectionPTR
NodeCollection::create_( const std::vector< index >& node_ids )
{
  index current_first = node_ids[ 0 ];
  index current_last = current_first;
  index current_model = kernel().modelrange_manager.get_model_id( node_ids[ 0 ] );

  std::vector< NodeCollectionPrimitive > parts;

  index old_node_id = current_first;
  for ( auto node_id = ++( node_ids.begin() ); node_id != node_ids.end(); ++node_id )
  {
    if ( *node_id == old_node_id )
    {
      throw BadProperty( "All node IDs in a NodeCollection have to be unique" );
    }
    old_node_id = *node_id;

    index next_model = kernel().modelrange_manager.get_model_id( *node_id );

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
    return NodeCollectionPTR( new NodeCollectionPrimitive( parts[ 0 ] ) );
  }
  else
  {
    return NodeCollectionPTR( new NodeCollectionComposite( parts ) );
  }
}

bool
NodeCollection::valid() const
{
  return fingerprint_ == kernel().get_fingerprint();
}

NodeCollectionPrimitive::NodeCollectionPrimitive( index first,
  index last,
  index model_id,
  NodeCollectionMetadataPTR meta )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( meta )
{
  assert( first_ <= last_ );
}

NodeCollectionPrimitive::NodeCollectionPrimitive( index first, index last, index model_id )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( nullptr )
{
  assert( first_ <= last_ );
}

NodeCollectionPrimitive::NodeCollectionPrimitive( index first, index last )
  : first_( first )
  , last_( last )
  , model_id_( invalid_index )
  , metadata_( nullptr )
{
  assert( first_ <= last_ );

  // find the model_id
  const int first_model_id = kernel().modelrange_manager.get_model_id( first );
  for ( index node_id = ++first; node_id <= last; ++node_id )
  {
    const int model_id = kernel().modelrange_manager.get_model_id( node_id );
    if ( model_id != first_model_id )
    {
      throw BadProperty( "model ids does not match" );
    }
  }
  model_id_ = first_model_id;
}

NodeCollectionPrimitive::NodeCollectionPrimitive( const NodeCollectionPrimitive& rhs )
  : first_( rhs.first_ )
  , last_( rhs.last_ )
  , model_id_( rhs.model_id_ )
  , metadata_( rhs.metadata_ )
{
}

NodeCollectionPrimitive::NodeCollectionPrimitive()
  : first_( 0 )
  , last_( 0 )
  , model_id_( invalid_index )
  , metadata_( nullptr )
{
}

ArrayDatum
NodeCollectionPrimitive::to_array() const
{
  ArrayDatum node_ids;
  node_ids.reserve( size() );
  for ( const_iterator it = begin(); it < end(); ++it )
  {
    node_ids.push_back( ( *it ).node_id );
  }
  return node_ids;
}

NodeCollectionPTR NodeCollectionPrimitive::operator+( NodeCollectionPTR rhs ) const
{
  if ( not valid() or not rhs->valid() )
  {
    throw KernelException( "InvalidNodeCollection" );
  }
  if ( rhs->empty() )
  {
    return NodeCollectionPTR( new NodeCollectionPrimitive( *this ) );
  }
  if ( empty() )
  {
    auto const* const rhs_ptr = dynamic_cast< NodeCollectionPrimitive const* >( rhs.get() );
    if ( rhs_ptr )
    {
      // rhs is primitive
      return std::make_shared< NodeCollectionPrimitive >( *rhs_ptr );
    }
    else
    {
      // rhs is composite
      auto const* const rhs_ptr = dynamic_cast< NodeCollectionComposite const* >( rhs.get() );
      assert( rhs_ptr );
      return std::make_shared< NodeCollectionComposite >( *rhs_ptr );
    }
  }
  if ( ( get_metadata().get() or rhs->get_metadata().get() ) and not( get_metadata() == rhs->get_metadata() ) )
  {
    throw BadProperty( "Can only join NodeCollections with same metadata." );
  }

  auto const* const rhs_ptr = dynamic_cast< NodeCollectionPrimitive const* >( rhs.get() );

  if ( rhs_ptr ) // if rhs is Primitive
  {
    if ( overlapping( *rhs_ptr ) )
    {
      throw BadProperty( "Cannot join overlapping NodeCollections." );
    }
    if ( ( last_ + 1 ) == rhs_ptr->first_ and model_id_ == rhs_ptr->model_id_ )
    // if contiguous and homogeneous
    {
      return NodeCollectionPTR( new NodeCollectionPrimitive( first_, rhs_ptr->last_, model_id_, metadata_ ) );
    }
    else if ( ( rhs_ptr->last_ + 1 ) == first_ and model_id_ == rhs_ptr->model_id_ )
    {
      return NodeCollectionPTR( new NodeCollectionPrimitive( rhs_ptr->first_, last_, model_id_, metadata_ ) );
    }
    else // not contiguous and homogeneous
    {
      std::vector< NodeCollectionPrimitive > primitives;
      primitives.reserve( 2 );
      primitives.push_back( *this );
      primitives.push_back( *rhs_ptr );
      return NodeCollectionPTR( new NodeCollectionComposite( primitives ) );
    }
  }
  else // if rhs is not Primitive, i.e. Composite
  {
    auto* rhs_ptr = dynamic_cast< NodeCollectionComposite* >( rhs.get() );
    assert( rhs_ptr );
    return rhs_ptr->operator+( *this ); // use Composite operator+
  }
}

NodeCollectionPrimitive::const_iterator
NodeCollectionPrimitive::local_begin( NodeCollectionPTR cp ) const
{
  size_t num_vps = kernel().vp_manager.get_num_virtual_processes();
  size_t current_vp = kernel().vp_manager.thread_to_vp( kernel().vp_manager.get_thread_id() );
  size_t vp_first_node = kernel().vp_manager.node_id_to_vp( first_ );
  size_t offset = ( current_vp - vp_first_node + num_vps ) % num_vps;

  if ( offset >= size() ) // Too few node IDs to be shared among all vps.
  {
    return const_iterator( cp, *this, size() );
  }
  else
  {
    return const_iterator( cp, *this, offset, num_vps );
  }
}

NodeCollectionPrimitive::const_iterator
NodeCollectionPrimitive::MPI_local_begin( NodeCollectionPTR cp ) const
{
  size_t num_processes = kernel().mpi_manager.get_num_processes();
  size_t rank = kernel().mpi_manager.get_rank();
  size_t rank_first_node = kernel().mpi_manager.get_process_id_of_vp( kernel().vp_manager.node_id_to_vp( first_ ) );
  size_t offset = ( rank - rank_first_node + num_processes ) % num_processes;
  if ( offset > size() ) // Too few node IDs to be shared among all MPI processes.
  {
    return const_iterator( cp, *this, size() );
  }
  else
  {
    return const_iterator( cp, *this, offset, num_processes );
  }
}

NodeCollectionPTR
NodeCollectionPrimitive::NodeCollectionPrimitive::slice( size_t start, size_t stop, size_t step ) const
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
    throw KernelException( "InvalidNodeCollection" );
  }

  NodeCollectionPTR sliced_nc;
  if ( step == 1 )
  {
    sliced_nc = std::make_shared< NodeCollectionPrimitive >( first_ + start, first_ + stop - 1, model_id_, metadata_ );
  }
  else
  {
    sliced_nc = std::make_shared< NodeCollectionComposite >( *this, start, stop, step );
  }
  if ( metadata_ )
  {
    metadata_->slice( start, stop, step, sliced_nc );
  }

  return sliced_nc;
}

void
NodeCollectionPrimitive::print_me( std::ostream& out ) const
{
  out << "NodeCollection(";
  if ( empty() )
  {
    out << "<empty>";
  }
  else
  {
    std::string metadata = metadata_.get() ? metadata_->get_type() : "None";
    out << "metadata=" << metadata << ", ";
    print_primitive( out );
  }
  out << ")";
}

void
NodeCollectionPrimitive::print_primitive( std::ostream& out ) const
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
NodeCollectionPrimitive::is_contiguous_ascending( NodeCollectionPrimitive& other )
{
  return ( ( last_ + 1 ) == other.first_ ) and ( model_id_ == other.model_id_ );
}

bool
NodeCollectionPrimitive::overlapping( const NodeCollectionPrimitive& rhs ) const
{
  return ( ( rhs.first_ <= last_ and rhs.first_ >= first_ ) or ( rhs.last_ <= last_ and rhs.last_ >= first_ ) );
}

NodeCollectionComposite::NodeCollectionComposite( const NodeCollectionPrimitive& primitive,
  size_t start,
  size_t stop,
  size_t step )
  : size_( std::floor( ( stop - start ) / ( float ) step ) + ( ( stop - start ) % step > 0 ) )
  , step_( step )
  , start_part_( 0 )
  , start_offset_( start )
  , stop_part_( stop == primitive.size() )
  , stop_offset_( stop == primitive.size() ? 0 : stop )
{
  parts_.reserve( 1 );
  parts_.push_back( primitive );
}

NodeCollectionComposite::NodeCollectionComposite( const NodeCollectionComposite& comp )
  : parts_( comp.parts_ )
  , size_( comp.size_ )
  , step_( comp.step_ )
  , start_part_( comp.start_part_ )
  , start_offset_( comp.start_offset_ )
  , stop_part_( comp.stop_part_ )
  , stop_offset_( comp.stop_offset_ )
{
}

NodeCollectionComposite::NodeCollectionComposite( const std::vector< NodeCollectionPrimitive >& parts )
  : size_( 0 )
  , step_( 1 )
  , start_part_( 0 )
  , start_offset_( 0 )
  , stop_part_( 0 )
  , stop_offset_( 0 )
{
  if ( parts.size() < 1 )
  {
    throw BadProperty( "Cannot create an empty composite NodeCollection" );
  }

  NodeCollectionMetadataPTR meta = parts[ 0 ].get_metadata();
  parts_.reserve( parts.size() );
  for ( const auto& part : parts )
  {
    if ( meta.get() and not( meta == part.get_metadata() ) )
    {
      throw BadProperty( "all metadata in a NodeCollection must be the same" );
    }
    parts_.push_back( part );
    size_ += part.size();
  }
  std::sort( parts_.begin(), parts_.end(), primitiveSort );
}

NodeCollectionComposite::NodeCollectionComposite( const NodeCollectionComposite& composite,
  size_t start,
  size_t stop,
  size_t step )
  : parts_( composite.parts_ )
  , size_( std::floor( ( stop - start ) / ( float ) step ) + ( ( stop - start ) % step > 0 ) )
  , step_( step )
  , start_part_( 0 )
  , start_offset_( 0 )
  , stop_part_( composite.parts_.size() )
  , stop_offset_( 0 )
{
  if ( stop - start < 1 )
  {
    throw BadProperty( "Cannot create an empty composite NodeCollection." );
  }
  if ( start > composite.size() or stop > composite.size() )
  {
    throw BadProperty( "Index out of range." );
  }

  if ( composite.step_ > 1 or composite.stop_part_ != 0 or composite.stop_offset_ != 0 )
  {
    // The NodeCollection is sliced
    if ( size_ > 1 )
    {
      // Creating a sliced NC with more than one node ID from a sliced NC is impossible.
      throw BadProperty( "Cannot slice a sliced composite NodeCollection." );
    }
    // we have a single single node ID, must just find where it is.
    const_iterator it = composite.begin() + start;
    it.get_current_part_offset( start_part_, start_offset_ );
    stop_part_ = start_part_;
    stop_offset_ = start_offset_ + 1;
  }
  else
  {
    // The NodeCollection is not sliced

    // Iterate through the composite to find where to start and stop.
    // TODO: There is some room for improvement here. Can go through parts instead, similar to in find().
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
}

NodeCollectionPTR NodeCollectionComposite::operator+( NodeCollectionPTR rhs ) const
{
  if ( rhs->empty() )
  {
    return NodeCollectionPTR( new NodeCollectionComposite( *this ) );
  }
  if ( get_metadata().get() and not( get_metadata() == rhs->get_metadata() ) )
  {
    throw BadProperty( "can only join NodeCollections with the same metadata" );
  }
  if ( not valid() or not rhs->valid() )
  {
    throw KernelException( "InvalidNodeCollection" );
  }
  if ( step_ > 1 or stop_part_ != 0 or stop_offset_ != 0 )
  {
    throw BadProperty( "Cannot add NodeCollection to a sliced composite." );
  }
  auto const* const rhs_ptr = dynamic_cast< NodeCollectionPrimitive const* >( rhs.get() );
  if ( rhs_ptr ) // if rhs is Primitive
  {
    // check primitives in the composite for overlap
    for ( auto part_it = parts_.begin(); part_it < parts_.end(); ++part_it )
    {
      if ( part_it->overlapping( *rhs_ptr ) )
      {
        throw BadProperty( "Cannot join overlapping NodeCollections." );
      }
    }
    return NodeCollectionPTR( *this + *rhs_ptr );
  }
  else // rhs is Composite
  {
    auto const* const rhs_ptr = dynamic_cast< NodeCollectionComposite const* >( rhs.get() );
    if ( rhs_ptr->step_ > 1 or rhs_ptr->stop_part_ != 0 or rhs_ptr->stop_offset_ != 0 )
    {
      throw BadProperty( "Cannot add NodeCollection to a sliced composite." );
    }

    // check overlap between the two composites
    const NodeCollectionComposite* shortest, *longest;
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
    for ( NodeCollectionComposite::const_iterator short_it = shortest->begin(); short_it < shortest->end(); ++short_it )
    {
      if ( longest->contains( ( *short_it ).node_id ) )
      {
        throw BadProperty( "Cannot join overlapping NodeCollections." );
      }
    }

    auto* new_composite = new NodeCollectionComposite( *this );
    new_composite->parts_.reserve( new_composite->parts_.size() + rhs_ptr->parts_.size() );
    for ( const auto& part : rhs_ptr->parts_ )
    {
      new_composite->parts_.push_back( part );
      new_composite->size_ += part.size();
    }
    std::sort( new_composite->parts_.begin(), new_composite->parts_.end(), primitiveSort );
    merge_parts( new_composite->parts_ );
    if ( new_composite->parts_.size() == 1 )
    {
      // If there is only a single primitive in the composite, we extract it,
      // ensuring that the composite is deleted from memory.
      NodeCollectionPrimitive new_primitive = new_composite->parts_[ 0 ];
      delete new_composite;
      return NodeCollectionPTR( new NodeCollectionPrimitive( new_primitive ) );
    }
    else
    {
      return NodeCollectionPTR( new_composite );
    }
  }
}

NodeCollectionPTR NodeCollectionComposite::operator+( const NodeCollectionPrimitive& rhs ) const
{
  if ( get_metadata().get() and not( get_metadata() == rhs.get_metadata() ) )
  {
    throw BadProperty( "can only join NodeCollections with the same metadata" );
  }

  // check primitives in the composites for overlap
  for ( auto part_it = parts_.begin(); part_it < parts_.end(); ++part_it )
  {
    if ( part_it->overlapping( rhs ) )
    {
      throw BadProperty( "Cannot join overlapping NodeCollections." );
    }
  }

  std::vector< NodeCollectionPrimitive > new_parts = parts_;
  new_parts.push_back( rhs );
  std::sort( new_parts.begin(), new_parts.end(), primitiveSort );
  merge_parts( new_parts );
  if ( new_parts.size() == 1 )
  {
    return NodeCollectionPTR( new NodeCollectionPrimitive( new_parts[ 0 ] ) );
  }
  else
  {
    return NodeCollectionPTR( new NodeCollectionComposite( new_parts ) );
  }
}

NodeCollectionComposite::const_iterator
NodeCollectionComposite::local_begin( NodeCollectionPTR cp ) const
{
  size_t num_vps = kernel().vp_manager.get_num_virtual_processes();
  size_t current_vp = kernel().vp_manager.thread_to_vp( kernel().vp_manager.get_thread_id() );
  size_t vp_first_node = kernel().vp_manager.node_id_to_vp( operator[]( 0 ) );
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

NodeCollectionComposite::const_iterator
NodeCollectionComposite::MPI_local_begin( NodeCollectionPTR cp ) const
{
  size_t num_processes = kernel().mpi_manager.get_num_processes();
  size_t rank = kernel().mpi_manager.get_rank();
  size_t rank_first_node =
    kernel().mpi_manager.get_process_id_of_vp( kernel().vp_manager.node_id_to_vp( operator[]( 0 ) ) );
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
NodeCollectionComposite::to_array() const
{
  ArrayDatum node_ids;
  node_ids.reserve( size() );
  for ( const_iterator it = begin(); it < end(); ++it )
  {
    node_ids.push_back( ( *it ).node_id );
  }
  return node_ids;
}

NodeCollectionPTR
NodeCollectionComposite::slice( size_t start, size_t stop, size_t step ) const
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
    throw KernelException( "InvalidNodeCollection" );
  }

  auto new_composite = NodeCollectionComposite( *this, start, stop, step );

  if ( step == 1 and new_composite.start_part_ == new_composite.stop_part_ )
  {
    // Return only the primitive
    return new_composite.parts_[ new_composite.start_part_ ].slice(
      new_composite.start_offset_, new_composite.stop_offset_ );
  }
  return NodeCollectionPTR( new NodeCollectionComposite( new_composite ) );
}

void
NodeCollectionComposite::merge_parts( std::vector< NodeCollectionPrimitive >& parts ) const
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
        NodeCollectionPTR merged_primitivesPTR =
          parts[ i ] + std::make_shared< NodeCollectionPrimitive >( parts[ i + 1 ] );
        auto const* const merged_primitives =
          dynamic_cast< NodeCollectionPrimitive const* >( merged_primitivesPTR.get() );

        parts[ i ] = *merged_primitives;
        parts.erase( parts.begin() + i + 1 );
        did_merge = true;
        last_i = i;
        break;
      }
    }
  }
}

bool
NodeCollectionComposite::contains( index node_id ) const
{
  long lower = 0;
  long upper = parts_.size() - 1;
  while ( lower <= upper )
  {
    long middle = std::floor( ( lower + upper ) / 2.0 );

    if ( ( *( parts_[ middle ].begin() + ( parts_[ middle ].size() - 1 ) ) ).node_id < node_id )
    {
      lower = middle + 1;
    }
    else if ( node_id < ( *( parts_[ middle ].begin() ) ).node_id )
    {
      upper = middle - 1;
    }
    else
    {
      if ( step_ > 1 )
      {
        index start_node_id = ( *( parts_[ middle ].begin() + start_offset_ ) ).node_id;
        index end_node_id = ( *( parts_[ middle ].begin() + ( parts_[ middle ].size() - 1 ) ) ).node_id;
        index stop_node_id =
          stop_part_ != ( size_t ) middle ? end_node_id : ( *( parts_[ middle ].begin() + stop_offset_ ) ).node_id;

        return node_id >= start_node_id and ( ( node_id - start_node_id ) % step_ ) == 0 and node_id <= stop_node_id;
      }
      return true;
    }
  }
  return false;
}

long
NodeCollectionComposite::find( const index node_id ) const
{
  if ( step_ > 1 or start_part_ > 0 or start_offset_ > 0 or ( stop_part_ > 0 and stop_part_ != parts_.size() )
    or stop_offset_ > 0 )
  {
    // Composite is sliced, we must iterate to find the index.
    auto it = begin();
    long index = 0;
    for ( const_iterator it = begin(); it < end(); ++it, ++index )
    {
      if ( ( *it ).node_id == node_id )
      {
        return index;
      }
    }
    return -1;
  }
  else
  {
    // using the same algorithm as contains(), but returns the node ID if found.
    long lower = 0;
    long upper = parts_.size() - 1;
    while ( lower <= upper )
    {
      long middle = std::floor( ( lower + upper ) / 2.0 );

      if ( ( *( parts_[ middle ].begin() + ( parts_[ middle ].size() - 1 ) ) ).node_id < node_id )
      {
        lower = middle + 1;
      }
      else if ( node_id < ( *( parts_[ middle ].begin() ) ).node_id )
      {
        upper = middle - 1;
      }
      else
      {
        auto size_accu = []( long a, const NodeCollectionPrimitive& b )
        {
          return a + b.size();
        };
        long sum_pre = std::accumulate( parts_.begin(), parts_.begin() + middle, ( long ) 0, size_accu );
        return sum_pre + parts_[ middle ].find( node_id );
      }
    }
    return -1;
  }
}

void
NodeCollectionComposite::print_me( std::ostream& out ) const
{
  std::string metadata = "None";
  std::string nc = "NodeCollection(";
  std::string space( nc.size(), ' ' );

  if ( step_ > 1 or ( stop_part_ != 0 or stop_offset_ != 0 ) )
  {
    // Sliced composite NodeCollection

    size_t current_part = 0;
    size_t current_offset = 0;
    size_t previous_part = std::numeric_limits< size_t >::infinity();
    index primitive_last = 0;

    size_t primitive_size = 0;
    NodeIDTriple first_in_primitive = *begin();

    std::vector< std::string > string_vector;

    out << nc << "metadata=" << metadata << ",";
    for ( const_iterator it = begin(); it < end(); ++it )
    {
      it.get_current_part_offset( current_part, current_offset );
      if ( current_part != previous_part ) // New primitive
      {
        if ( it != begin() )
        {
          // Need to count the primitive, so can't start at begin()
          out << "\n" + space << "model=" << kernel().model_manager.get_model( first_in_primitive.model_id )->get_name()
              << ", size=" << primitive_size << ", ";
          if ( primitive_size == 1 )
          {
            out << "first=" << first_in_primitive.node_id << ", last=" << first_in_primitive.node_id << ";";
          }
          else
          {
            out << "first=" << first_in_primitive.node_id << ", last=";
            out << primitive_last;
            if ( step_ > 1 )
            {
              out << ", step=" << step_ << ";";
            }
          }
        }
        primitive_size = 1;
        first_in_primitive = *it;
      }
      else
      {
        ++primitive_size;
      }
      primitive_last = ( *it ).node_id;
      previous_part = current_part;
    }

    // Need to also print the last primitive
    out << "\n" + space << "model=" << kernel().model_manager.get_model( first_in_primitive.model_id )->get_name()
        << ", size=" << primitive_size << ", ";
    if ( primitive_size == 1 )
    {
      out << "first=" << first_in_primitive.node_id << ", last=" << first_in_primitive.node_id;
    }
    else
    {
      out << "first=" << first_in_primitive.node_id << ", last=";
      out << primitive_last;
      if ( step_ > 1 )
      {
        out << ", step=" << step_;
      }
    }
  }
  else
  {
    // None-sliced Composite NodeCollection
    out << nc << "metadata=" << metadata << ",";
    for ( auto it = parts_.begin(); it != parts_.end(); ++it )
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
