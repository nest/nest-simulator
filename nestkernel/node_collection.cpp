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

// Includes from nestkernel:
#include "kernel_manager.h"
#include "mpi_manager_impl.h"
#include "node.h"
#include "vp_manager_impl.h"

// C++ includes:
#include <algorithm> // copy
#include <numeric>   // accumulate


namespace nest
{

// function object for sorting a vector of NodeCollectionPrimitives
const struct PrimitiveSortOp
{
  bool
  operator()( const NodeCollectionPrimitive& primitive_lhs, const NodeCollectionPrimitive& primitive_rhs ) const
  {
    return primitive_lhs[ 0 ] < primitive_rhs[ 0 ];
  }
} primitive_sort_op;


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
nc_const_iterator::composite_update_indices_()
{
  // If we went past the size of the primitive, we need to adjust the element
  // and primitive part indices.
  size_t primitive_size = composite_collection_->parts_[ part_idx_ ].size();
  while ( element_idx_ >= primitive_size )
  {
    element_idx_ = element_idx_ - primitive_size;
    ++part_idx_;
    if ( part_idx_ < composite_collection_->parts_.size() )
    {
      primitive_size = composite_collection_->parts_[ part_idx_ ].size();
    }
  }
  // If we went past the end of the composite, we need to adjust the
  // position of the iterator.
  if ( composite_collection_->is_sliced_ )
  {
    assert( composite_collection_->end_offset_ != 0 or composite_collection_->end_part_ != 0 );
    if ( part_idx_ >= composite_collection_->end_part_ and element_idx_ >= composite_collection_->end_offset_ )
    {
      part_idx_ = composite_collection_->end_part_;
      element_idx_ = composite_collection_->end_offset_;
    }
  }
  else if ( part_idx_ >= composite_collection_->parts_.size() )
  {
    auto end_of_composite = composite_collection_->end();
    part_idx_ = end_of_composite.part_idx_;
    element_idx_ = end_of_composite.element_idx_;
  }
}

void
nc_const_iterator::print_me( std::ostream& out ) const
{
  out << "[[" << this << " pc: " << primitive_collection_ << ", cc: " << composite_collection_ << ", px: " << part_idx_
      << ", ex: " << element_idx_ << "]]";
}

NodeIDTriple
nc_const_iterator::operator*() const
{
  NodeIDTriple gt;
  if ( primitive_collection_ )
  {
    gt.node_id = primitive_collection_->first_ + element_idx_;
    if ( gt.node_id > primitive_collection_->last_ )
    {
      throw KernelException( "Invalid NodeCollection iterator (primitive element beyond last element)" );
    }
    gt.model_id = primitive_collection_->model_id_;
    gt.lid = element_idx_;
  }
  else
  {
    // for efficiency we check each value instead of simply checking against
    // composite_collection->end()
    if ( not( part_idx_ < composite_collection_->end_part_
           or ( part_idx_ == composite_collection_->end_part_
             and element_idx_ < composite_collection_->end_offset_ ) ) )
    {
      throw KernelException( "Invalid NodeCollection iterator (composite element beyond specified end element)" );
    }

    // Add to local placement from NodeCollectionPrimitives that comes before the
    // current one.
    gt.lid = 0;
    for ( const auto& part : composite_collection_->parts_ )
    {
      // Using a stripped-down comparison of Primitives to avoid redundant and potentially expensive comparisons of
      // metadata.
      const auto& current_part = composite_collection_->parts_[ part_idx_ ];
      if ( part.first_ == current_part.first_ and part.last_ == current_part.last_ )
      {
        break;
      }
      gt.lid += part.size();
    }

    gt.node_id = composite_collection_->parts_[ part_idx_ ][ element_idx_ ];
    gt.model_id = composite_collection_->parts_[ part_idx_ ].model_id_;
    gt.lid += element_idx_;
  }
  return gt;
}

nc_const_iterator&
nc_const_iterator::operator++()
{
  element_idx_ += step_;
  if ( primitive_collection_ )
  {
    if ( element_idx_ >= primitive_collection_->size() )
    {
      element_idx_ = primitive_collection_->size();
    }
  }
  else
  {
    composite_update_indices_();
  }
  return *this;
}

nc_const_iterator
nc_const_iterator::operator++( int )
{
  nc_const_iterator tmp = *this;
  ++( *this );
  return tmp;
}

NodeCollectionPTR
operator+( NodeCollectionPTR lhs, NodeCollectionPTR rhs )
{
  return lhs->operator+( rhs );
}

NodeCollection::NodeCollection()
  : fingerprint_( kernel().get_fingerprint() )
{
}

NodeCollectionPTR
NodeCollection::create( const IntVectorDatum& node_ids_datum )
{
  if ( node_ids_datum->empty() )
  {
    return NodeCollection::create_();
  }

  std::vector< size_t > node_ids;
  node_ids.reserve( node_ids_datum->size() );
  for ( const auto& datum : *node_ids_datum )
  {
    node_ids.push_back( static_cast< size_t >( getValue< long >( datum ) ) );
  }

  if ( not std::is_sorted( node_ids.begin(), node_ids.end() ) )
  {
    throw BadProperty( "Node IDs must be sorted in ascending order" );
  }
  return NodeCollection::create_( node_ids );
}

NodeCollectionPTR
NodeCollection::create( const TokenArray& node_ids_array )
{
  if ( node_ids_array.empty() )
  {
    return NodeCollection::create_();
  }

  std::vector< size_t > node_ids;
  node_ids.reserve( node_ids_array.size() );
  for ( const auto& node_id_token : node_ids_array )
  {
    node_ids.push_back( static_cast< size_t >( getValue< long >( node_id_token ) ) );
  }

  if ( not std::is_sorted( node_ids.begin(), node_ids.end() ) )
  {
    throw BadProperty( "Node IDs must be sorted in ascending order" );
  }
  return NodeCollection::create_( node_ids );
}

NodeCollectionPTR
NodeCollection::create( const size_t node_id )
{
  return NodeCollection::create_( { node_id } );
}

NodeCollectionPTR
NodeCollection::create( const Node* node )
{
  if ( node )
  {
    return NodeCollection::create( node->get_node_id() );
  }
  return NodeCollection::create_();
}

NodeCollectionPTR
NodeCollection::create( const std::vector< size_t >& node_ids_vector )
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
NodeCollection::create_()
{
  return std::make_shared< NodeCollectionPrimitive >();
}

NodeCollectionPTR
NodeCollection::create_( const std::vector< size_t >& node_ids )
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

bool
NodeCollection::valid() const
{
  return fingerprint_ == kernel().get_fingerprint();
}

NodeCollectionPrimitive::NodeCollectionPrimitive( size_t first,
  size_t last,
  size_t model_id,
  NodeCollectionMetadataPTR meta )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( meta )
  , nodes_have_no_proxies_( not kernel().model_manager.get_node_model( model_id_ )->has_proxies() )
{
  assert_consistent_model_ids_( model_id_ );

  assert( first_ <= last_ );
}

NodeCollectionPrimitive::NodeCollectionPrimitive( size_t first, size_t last, size_t model_id )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( nullptr )
  , nodes_have_no_proxies_( not kernel().model_manager.get_node_model( model_id_ )->has_proxies() )
{
  assert( first_ <= last_ );
}

NodeCollectionPrimitive::NodeCollectionPrimitive( size_t first, size_t last )
  : first_( first )
  , last_( last )
  , model_id_( invalid_index )
  , metadata_( nullptr )
{
  assert( first_ <= last_ );

  // find the model_id
  const auto first_model_id = kernel().modelrange_manager.get_model_id( first );
  const auto init_index = first + 1;
  for ( size_t node_id = init_index; node_id <= last; ++node_id )
  {
    const auto model_id = kernel().modelrange_manager.get_model_id( node_id );
    if ( model_id != first_model_id )
    {
      throw BadProperty( "model ids does not match" );
    }
  }
  model_id_ = first_model_id;
  nodes_have_no_proxies_ = not kernel().model_manager.get_node_model( model_id_ )->has_proxies();
}

NodeCollectionPrimitive::NodeCollectionPrimitive()
  : first_( 0 )
  , last_( 0 )
  , model_id_( invalid_index )
  , metadata_( nullptr )
  , nodes_have_no_proxies_( false )
{
}

ArrayDatum
NodeCollectionPrimitive::to_array() const
{
  ArrayDatum node_ids;
  node_ids.reserve( size() );
  for ( auto it = begin(); it < end(); ++it )
  {
    node_ids.push_back( ( *it ).node_id );
  }
  return node_ids;
}

NodeCollectionPTR
NodeCollectionPrimitive::operator+( NodeCollectionPTR rhs ) const
{
  if ( not valid() or not rhs->valid() )
  {
    throw KernelException(
      "InvalidNodeCollection: note that ResetKernel invalidates all previously created NodeCollections." );
  }
  if ( rhs->empty() )
  {
    return std::make_shared< NodeCollectionPrimitive >( *this );
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
      return std::make_shared< NodeCollectionPrimitive >( first_, rhs_ptr->last_, model_id_, metadata_ );
    }
    else if ( ( rhs_ptr->last_ + 1 ) == first_ and model_id_ == rhs_ptr->model_id_ )
    {
      return std::make_shared< NodeCollectionPrimitive >( rhs_ptr->first_, last_, model_id_, metadata_ );
    }
    else // not contiguous and homogeneous
    {
      std::vector< NodeCollectionPrimitive > primitives;
      primitives.reserve( 2 );
      primitives.push_back( *this );
      primitives.push_back( *rhs_ptr );
      return std::make_shared< NodeCollectionComposite >( primitives );
    }
  }
  else // if rhs is not Primitive, i.e. Composite
  {
    auto const* const rhs_ptr = dynamic_cast< NodeCollectionComposite* >( rhs.get() );
    assert( rhs_ptr );
    return rhs_ptr->operator+( *this ); // use Composite operator+
  }
}

NodeCollectionPrimitive::const_iterator
NodeCollectionPrimitive::local_begin( NodeCollectionPTR cp ) const
{
  const size_t num_vps = kernel().vp_manager.get_num_virtual_processes();
  const size_t current_vp = kernel().vp_manager.thread_to_vp( kernel().vp_manager.get_thread_id() );
  const size_t vp_first_node = kernel().vp_manager.node_id_to_vp( first_ );
  const size_t offset = ( current_vp - vp_first_node + num_vps ) % num_vps;

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
  const size_t num_processes = kernel().mpi_manager.get_num_processes();
  const size_t rank = kernel().mpi_manager.get_rank();
  const size_t rank_first_node =
    kernel().mpi_manager.get_process_id_of_vp( kernel().vp_manager.node_id_to_vp( first_ ) );
  const size_t offset = ( rank - rank_first_node + num_processes ) % num_processes;

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
NodeCollectionPrimitive::slice( size_t start, size_t end, size_t step ) const
{
  if ( not( start < end ) )
  {
    throw BadParameter( "start < stop required." );
  }
  if ( not( end <= size() ) )
  {
    throw BadParameter( "stop <= size() required." );
  }
  if ( not valid() )
  {
    throw KernelException(
      "InvalidNodeCollection: note that ResetKernel invalidates all previously created NodeCollections." );
  }

  NodeCollectionPTR sliced_nc;
  if ( step == 1 and not metadata_ )
  {
    // Create primitive NodeCollection passing node IDs.
    // Subtract 1 because "end" is one past last element to take while constructor expects ID of last node.
    sliced_nc = std::make_shared< NodeCollectionPrimitive >( first_ + start, first_ + end - 1, model_id_ );
  }
  else
  {
    sliced_nc = std::make_shared< NodeCollectionComposite >( *this, start, end, step );
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
  const std::string model =
    model_id_ != invalid_index ? kernel().model_manager.get_node_model( model_id_ )->get_name() : "none";

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
NodeCollectionPrimitive::is_contiguous_ascending( const NodeCollectionPrimitive& other ) const
{
  return ( ( last_ + 1 ) == other.first_ ) and ( model_id_ == other.model_id_ );
}

bool
NodeCollectionPrimitive::overlapping( const NodeCollectionPrimitive& rhs ) const
{
  return ( ( rhs.first_ <= last_ and rhs.first_ >= first_ ) or ( rhs.last_ <= last_ and rhs.last_ >= first_ ) );
}

void
NodeCollectionPrimitive::assert_consistent_model_ids_( const size_t expected_model_id ) const
{
  for ( size_t node_id = first_; node_id <= last_; ++node_id )
  {
    const auto model_id = kernel().modelrange_manager.get_model_id( node_id );
    if ( model_id != expected_model_id )
    {
      const auto node_model = kernel().modelrange_manager.get_model_of_node_id( model_id )->get_name();
      const auto expected_model = kernel().modelrange_manager.get_model_of_node_id( expected_model_id )->get_name();
      const auto message = "All nodes must have the same model (node with ID " + std::to_string( node_id )
        + " has model " + node_model + ", expected " + expected_model + ")";
      throw BadProperty( message );
    }
  }
}

NodeCollectionComposite::NodeCollectionComposite( const NodeCollectionPrimitive& primitive,
  size_t start,
  size_t end,
  size_t step )
  : parts_()
  , size_( ( end - start - 1 ) / step + 1 )
  , step_( step )
  , start_part_( 0 )
  , start_offset_( start )
  // If end is at the end of the primitive, set the end to the first in the next (nonexistent) part,
  // for consistency with iterator comparisons.
  , end_part_( end == primitive.size() ? 1 : 0 )
  , end_offset_( end == primitive.size() ? 0 : end )
  , is_sliced_( start != 0 or end != primitive.size() or step > 1 )
{
  parts_.push_back( primitive );
}

NodeCollectionComposite::NodeCollectionComposite( const std::vector< NodeCollectionPrimitive >& parts )
  : size_( 0 )
  , step_( 1 )
  , start_part_( 0 )
  , start_offset_( 0 )
  , end_part_( parts.size() )
  , end_offset_( 0 )
  , is_sliced_( false )
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
  std::sort( parts_.begin(), parts_.end(), primitive_sort_op );
}

NodeCollectionComposite::NodeCollectionComposite( const NodeCollectionComposite& composite,
  size_t start,
  size_t end,
  size_t step )
  : parts_( composite.parts_ )
  , size_( ( end - start - 1 ) / step + 1 )
  , step_( step )
  , start_part_( 0 )
  , start_offset_( 0 )
  , end_part_( composite.parts_.size() )
  , end_offset_( 0 )
  , is_sliced_( true )
{
  if ( end - start < 1 )
  {
    throw BadProperty( "Cannot create an empty composite NodeCollection." );
  }
  if ( start > composite.size() or end > composite.size() )
  {
    throw BadProperty( "Index out of range." );
  }

  if ( composite.is_sliced_ )
  {
    assert( composite.step_ > 1 or composite.end_part_ != 0 or composite.end_offset_ != 0 );
    // The NodeCollection is sliced
    if ( size_ > 1 )
    {
      // Creating a sliced NC with more than one node ID from a sliced NC is impossible.
      throw BadProperty( "Cannot slice a sliced composite NodeCollection." );
    }
    // we have a single single node ID, must just find where it is.
    const const_iterator it = composite.begin() + start;
    it.get_current_part_offset( start_part_, start_offset_ );
    end_part_ = start_part_;
    end_offset_ = start_offset_ + 1;
  }
  else
  {
    // The NodeCollection is not sliced
    // Update start and stop positions.
    const const_iterator start_it = composite.begin() + start;
    start_it.get_current_part_offset( start_part_, start_offset_ );

    const const_iterator end_it = composite.begin() + end;
    end_it.get_current_part_offset( end_part_, end_offset_ );
  }
}

NodeCollectionPTR
NodeCollectionComposite::operator+( NodeCollectionPTR rhs ) const
{
  if ( rhs->empty() )
  {
    return std::make_shared< NodeCollectionComposite >( *this );
  }
  if ( get_metadata().get() and not( get_metadata() == rhs->get_metadata() ) )
  {
    throw BadProperty( "can only join NodeCollections with the same metadata" );
  }
  if ( not valid() or not rhs->valid() )
  {
    throw KernelException(
      "InvalidNodeCollection: note that ResetKernel invalidates all previously created NodeCollections." );
  }
  if ( is_sliced_ )
  {
    assert( step_ > 1 or end_part_ != 0 or end_offset_ != 0 );
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
    assert( rhs_ptr );
    if ( rhs_ptr->is_sliced_ )
    {
      assert( rhs_ptr->step_ > 1 or rhs_ptr->end_part_ != 0 or rhs_ptr->end_offset_ != 0 );
      throw BadProperty( "Cannot add NodeCollection to a sliced composite." );
    }

    // check overlap between the two composites
    const auto shortest_longest_nc = std::minmax( *this,
      *rhs_ptr,
      []( const NodeCollectionComposite& a, const NodeCollectionComposite& b ) { return a.size() < b.size(); } );
    const auto& shortest = shortest_longest_nc.first;
    const auto& longest = shortest_longest_nc.second;

    for ( auto short_it = shortest.begin(); short_it < shortest.end(); ++short_it )
    {
      if ( longest.contains( ( *short_it ).node_id ) )
      {
        throw BadProperty( "Cannot join overlapping NodeCollections." );
      }
    }

    auto new_parts = parts_;
    new_parts.reserve( new_parts.size() + rhs_ptr->parts_.size() );
    new_parts.insert( new_parts.end(), rhs_ptr->parts_.begin(), rhs_ptr->parts_.end() );
    std::sort( new_parts.begin(), new_parts.end(), primitive_sort_op );
    merge_parts_( new_parts );
    if ( new_parts.size() == 1 )
    {
      // If there is only a single primitive in the composite, we extract it.
      return std::make_shared< NodeCollectionPrimitive >( new_parts[ 0 ] );
    }
    else
    {
      return std::make_shared< NodeCollectionComposite >( new_parts );
    }
  }
}

NodeCollectionPTR
NodeCollectionComposite::operator+( const NodeCollectionPrimitive& rhs ) const
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
  std::sort( new_parts.begin(), new_parts.end(), primitive_sort_op );
  merge_parts_( new_parts );
  if ( new_parts.size() == 1 )
  {
    return std::make_shared< NodeCollectionPrimitive >( new_parts[ 0 ] );
  }
  else
  {
    return std::make_shared< NodeCollectionComposite >( new_parts );
  }
}

size_t
NodeCollectionComposite::operator[]( const size_t i ) const
{
  if ( is_sliced_ )
  {
    assert( step_ > 1 or start_part_ > 0 or start_offset_ > 0 or end_part_ != parts_.size() or end_offset_ > 0 );
    // Composite is sliced, we use iterator arithmetic.
    return ( *( begin() + i ) ).node_id;
  }
  else
  {
    // Composite is unsliced, we can do a more efficient search.
    size_t tot_prev_node_ids = 0;
    for ( const auto& part : parts_ ) // iterate over NodeCollections
    {
      if ( tot_prev_node_ids + part.size() > i ) // is i in current NodeCollection?
      {
        size_t local_i = i - tot_prev_node_ids; // get local i
        return part[ local_i ];
      }
      else // i is not in current NodeCollection
      {
        tot_prev_node_ids += part.size();
      }
    }
    // throw exception if outside of NodeCollection
    throw std::out_of_range( "pos points outside of the NodeCollection" );
  }
}

bool
NodeCollectionComposite::operator==( NodeCollectionPTR rhs ) const
{
  auto const* const rhs_ptr = dynamic_cast< NodeCollectionComposite const* >( rhs.get() );

  // Checking if rhs_ptr is invalid first, to avoid segfaults. If rhs is a NodeCollectionPrimitive,
  // rhs_ptr will be a null pointer.
  if ( not rhs_ptr or size_ != rhs_ptr->size() or parts_.size() != rhs_ptr->parts_.size() )
  {
    return false;
  }
  auto rhs_nc = rhs_ptr->parts_.begin();
  for ( auto lhs_nc = parts_.begin(); lhs_nc != parts_.end(); ++lhs_nc, ++rhs_nc ) // iterate over NodeCollections
  {
    if ( not( ( *lhs_nc ) == ( *rhs_nc ) ) )
    {
      return false;
    }
  }
  return true;
}

NodeCollectionComposite::const_iterator
NodeCollectionComposite::local_begin( NodeCollectionPTR cp ) const
{
  const size_t num_vps = kernel().vp_manager.get_num_virtual_processes();
  const size_t current_vp = kernel().vp_manager.thread_to_vp( kernel().vp_manager.get_thread_id() );
  const size_t vp_first_node = kernel().vp_manager.node_id_to_vp( operator[]( 0 ) );

  return local_begin_( cp, num_vps, current_vp, vp_first_node );
}

NodeCollectionComposite::const_iterator
NodeCollectionComposite::MPI_local_begin( NodeCollectionPTR cp ) const
{
  const size_t num_processes = kernel().mpi_manager.get_num_processes();
  const size_t rank = kernel().mpi_manager.get_rank();
  const size_t rank_first_node =
    kernel().mpi_manager.get_process_id_of_vp( kernel().vp_manager.node_id_to_vp( operator[]( 0 ) ) );

  return local_begin_( cp, num_processes, rank, rank_first_node );
}


NodeCollectionComposite::const_iterator
NodeCollectionComposite::local_begin_( const NodeCollectionPTR cp,
  const size_t num_vp_elements,
  const size_t current_vp_element,
  const size_t vp_element_first_node ) const
{
  const size_t offset = ( current_vp_element - vp_element_first_node ) % num_vp_elements;

  if ( ( current_vp_element - vp_element_first_node ) % step_ != 0 )
  { // There are no local nodes in the NodeCollection.
    return end( cp );
  }

  size_t current_part = start_part_;
  size_t current_offset = start_offset_;
  if ( offset )
  {
    // First create an iterator at the start position.
    auto tmp_it = const_iterator( cp, *this, start_part_, start_offset_, step_ );
    tmp_it += offset; // Go forward to the offset.
    // Get current position.
    tmp_it.get_current_part_offset( current_part, current_offset );
  }

  return const_iterator( cp, *this, current_part, current_offset, num_vp_elements * step_ );
}

ArrayDatum
NodeCollectionComposite::to_array() const
{
  ArrayDatum node_ids;
  node_ids.reserve( size() );
  for ( auto it = begin(); it < end(); ++it )
  {
    node_ids.push_back( ( *it ).node_id );
  }
  return node_ids;
}

NodeCollectionPTR
NodeCollectionComposite::slice( size_t start, size_t end, size_t step ) const
{
  if ( not( start < end ) )
  {
    throw BadParameter( "start < stop required." );
  }
  if ( not( end <= size() ) )
  {
    throw BadParameter( "end <= size() required." );
  }
  if ( not valid() )
  {
    throw KernelException(
      "InvalidNodeCollection: note that ResetKernel invalidates all previously created NodeCollections." );
  }

  const auto new_composite = NodeCollectionComposite( *this, start, end, step );

  if ( step == 1 and new_composite.start_part_ == new_composite.end_part_ )
  {
    // Return only the primitive
    return new_composite.parts_[ new_composite.start_part_ ].slice(
      new_composite.start_offset_, new_composite.end_offset_ );
  }
  return std::make_shared< NodeCollectionComposite >( new_composite );
}

void
NodeCollectionComposite::merge_parts_( std::vector< NodeCollectionPrimitive >& parts ) const
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
NodeCollectionComposite::contains( const size_t node_id ) const
{
  return get_lid( node_id ) != -1;
}

long
NodeCollectionComposite::get_lid( const size_t node_id ) const
{
  const auto add_size_op = []( const long a, const NodeCollectionPrimitive& b ) { return a + b.size(); };

  long lower = 0;
  long upper = parts_.size() - 1;
  while ( lower <= upper )
  {
    const size_t middle = ( lower + upper ) / 2;

    if ( parts_[ middle ][ parts_[ middle ].size() - 1 ] < node_id )
    {
      lower = middle + 1;
    }
    else if ( node_id < ( parts_[ middle ][ 0 ] ) )
    {
      upper = middle - 1;
    }
    else
    {
      // At this point we know that node_id is in parts_[middle].
      if ( is_sliced_ )
      {
        assert( start_offset_ != 0 or start_part_ != 0 or end_part_ != 0 or end_offset_ != 0 or step_ > 1 );

        if ( middle < start_part_ or end_part_ < middle )
        {
          // middle is outside of the sliced area
          return -1;
        }
        // Need to find number of nodes in previous parts to know if the the step hits the node_id.
        const auto num_prev_nodes =
          std::accumulate( parts_.begin(), parts_.begin() + middle, static_cast< size_t >( 0 ), add_size_op );
        const auto absolute_pos = num_prev_nodes + parts_[ middle ].get_lid( node_id );

        // The first or the last node can be somewhere in the middle part.
        const auto absolute_part_start = start_part_ == middle ? start_offset_ : 0;
        const auto absolute_part_end = end_part_ == middle ? end_offset_ : parts_[ middle ].size();

        // Is node_id in the sliced NC?
        const auto node_id_before_start = node_id < parts_[ middle ][ absolute_part_start ];
        const auto node_id_after_end = parts_[ middle ][ absolute_part_end - 1 ] < node_id;
        const auto node_id_missed_by_step = ( ( absolute_pos - start_offset_ ) % step_ ) != 0;
        if ( node_id_before_start or node_id_after_end or node_id_missed_by_step )
        {
          return -1;
        }

        // Return the calculated local ID of node_id.
        return ( absolute_pos - start_offset_ ) / step_;
      }
      else
      {
        // Since NC is not sliced, we can just calculate and return the local ID.
        const auto sum_pre =
          std::accumulate( parts_.begin(), parts_.begin() + middle, static_cast< size_t >( 0 ), add_size_op );
        return sum_pre + parts_[ middle ].get_lid( node_id );
      }
    }
  }
  return -1;
}

bool
NodeCollectionComposite::has_proxies() const
{
  return std::all_of(
    parts_.begin(), parts_.end(), []( const NodeCollectionPrimitive& prim ) { return prim.has_proxies(); } );
}

void
NodeCollectionComposite::print_me( std::ostream& out ) const
{
  std::string metadata = parts_[ 0 ].get_metadata().get() ? parts_[ 0 ].get_metadata()->get_type() : "None";
  std::string nc = "NodeCollection(";
  std::string space( nc.size(), ' ' );

  if ( is_sliced_ )
  {
    assert( step_ > 1 or ( end_part_ != 0 or end_offset_ != 0 ) );
    // Sliced composite NodeCollection

    size_t current_part = 0;
    size_t current_offset = 0;
    size_t previous_part = std::numeric_limits< size_t >::infinity();
    size_t primitive_last = 0;

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
          out << "\n" + space
              << "model=" << kernel().model_manager.get_node_model( first_in_primitive.model_id )->get_name()
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
    out << "\n" + space << "model=" << kernel().model_manager.get_node_model( first_in_primitive.model_id )->get_name()
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
