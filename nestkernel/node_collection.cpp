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

// Includes from libnestutil
#include "numerics.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "model_manager.h"
#include "modelrange_manager.h"
#include "node.h"

// C++ includes:
#include "numeric"   // accumulate
#include <algorithm> // copy
#include <cmath>     // lcm


namespace nest
{

/**
 * Functor for sorting a vector of NodeCollectionPrimitives.
 *
 * Since primitives are contiguous, sort by GID of first element.
 */
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
  size_t stride,
  NCIteratorKind kind )
  : coll_ptr_( collection_ptr )
  , element_idx_( offset )
  , part_idx_( 0 )
  , step_( kind == NCIteratorKind::RANK_LOCAL ? std::lcm( stride, kernel::manager< MPIManager >.get_num_processes() )
                                              : ( kind == NCIteratorKind::THREAD_LOCAL ? std::lcm( stride,
                                                    kernel::manager< VPManager >.get_num_virtual_processes() )
                                                                                       : stride ) )
  , kind_( kind )
  , rank_or_vp_( kind == NCIteratorKind::RANK_LOCAL
        ? kernel::manager< MPIManager >.get_rank()
        : ( kind == NCIteratorKind::THREAD_LOCAL ? kernel::manager< VPManager >.get_vp() : invalid_thread ) )
  , primitive_collection_( &collection )
  , composite_collection_( nullptr )
{
  assert( not collection_ptr.get() or collection_ptr.get() == &collection );
  assert( element_idx_ <= collection.size() ); // allow == for end()

  FULL_LOGGING_ONLY( kernel::manager< KernelManager >.write_to_dump(
    String::compose( "NCIT Prim ctor rk %1, thr %2, pix %3, eix %4, step %5, kind %6, rvp %7",
      kernel::manager< MPIManager >.get_rank(),
      kernel::manager< VPManager >.get_thread_id(),
      part_idx_,
      element_idx_,
      step_,
      static_cast< int >( kind_ ),
      rank_or_vp_ ) ); )
}

nc_const_iterator::nc_const_iterator( NodeCollectionPTR collection_ptr,
  const NodeCollectionComposite& collection,
  size_t part,
  size_t offset,
  size_t stride,
  NCIteratorKind kind )
  : coll_ptr_( collection_ptr )
  , element_idx_( offset )
  , part_idx_( part )
  , step_( kind == NCIteratorKind::RANK_LOCAL ? std::lcm( stride, kernel::manager< MPIManager >.get_num_processes() )
                                              : ( kind == NCIteratorKind::THREAD_LOCAL ? std::lcm( stride,
                                                    kernel::manager< VPManager >.get_num_virtual_processes() )
                                                                                       : stride ) )
  , kind_( kind )
  , rank_or_vp_( kind == NCIteratorKind::RANK_LOCAL
        ? kernel::manager< MPIManager >.get_rank()
        : ( kind == NCIteratorKind::THREAD_LOCAL ? kernel::manager< VPManager >.get_vp() : invalid_thread ) )
  , primitive_collection_( nullptr )
  , composite_collection_( &collection )
{
  assert( not collection_ptr.get() or collection_ptr.get() == &collection );

  // Allow <= for end iterator
  assert( ( part < collection.parts_.size() and offset <= collection.parts_[ part ].size() ) );

  FULL_LOGGING_ONLY( kernel::manager< KernelManager >.write_to_dump(
    String::compose( "NCIT Comp ctor rk %1, thr %2, pix %3, eix %4, step %5, kind %6, rvp %7",
      kernel::manager< MPIManager >.get_rank(),
      kernel::manager< VPManager >.get_thread_id(),
      part_idx_,
      element_idx_,
      step_,
      static_cast< int >( kind_ ),
      rank_or_vp_ ) ); )
}

size_t
nc_const_iterator::find_next_within_part_( size_t n ) const
{
  const size_t new_element_idx = element_idx_ + n * step_;

  if ( primitive_collection_ )
  {
    // Avoid running over end of collection; primitive_collection_->size() is end marker
    return std::min( new_element_idx, primitive_collection_->size() );
  }

  if ( new_element_idx < composite_collection_->parts_[ part_idx_ ].size() )
  {
    if ( composite_collection_->valid_idx_( part_idx_, new_element_idx ) )
    {
      // We have found an element in the part
      return new_element_idx;
    }
    else
    {
      // We have reached the end of the node collection, return index for end iterator
      assert( part_idx_ == composite_collection_->last_part_ );
      return composite_collection_->last_elem_ + 1;
    }
  }

  // No new element found in this part and collection not exhausted
  return element_idx_;
}

void
nc_const_iterator::advance_global_iter_to_new_part_( size_t n )
{
  if ( part_idx_ == composite_collection_->last_part_ )
  {
    // No more parts, set to end()
    part_idx_ = composite_collection_->last_part_;
    element_idx_ = composite_collection_->last_elem_ + 1;
    return;
  }

  // Find new position counting from beginning of node collection
  const auto part_abs_begin = part_idx_ == 0 ? 0 : composite_collection_->cumul_abs_size_[ part_idx_ - 1 ];
  const auto new_abs_idx = part_abs_begin + element_idx_ + n * composite_collection_->stride_;

  // Confirm that new position is in a new part
  assert( new_abs_idx >= composite_collection_->cumul_abs_size_[ part_idx_ ] );

  // Move to part that contains new position
  do
  {
    ++part_idx_;
  } while ( part_idx_ <= composite_collection_->last_part_
    and composite_collection_->cumul_abs_size_[ part_idx_ ] <= new_abs_idx );

  // If there is another element, it must have this index
  element_idx_ = new_abs_idx - composite_collection_->cumul_abs_size_[ part_idx_ - 1 ];

  if ( not composite_collection_->valid_idx_( part_idx_, element_idx_ ) )
  {
    // Node collection exhausted
    part_idx_ = composite_collection_->last_part_;
    element_idx_ = composite_collection_->last_elem_ + 1;
  }
}

void
nc_const_iterator::advance_local_iter_to_new_part_( size_t n )
{
  // We know that we need to look in another part
  if ( part_idx_ == composite_collection_->last_part_ )
  {
    // No more parts, set to end()
    part_idx_ = composite_collection_->last_part_;
    element_idx_ = composite_collection_->last_elem_ + 1;
    return;
  }

  // {RANK,THREAD}_LOCAL iterators require phase adjustment
  // which is feasible only for single steps, so unroll
  for ( size_t k = 0; k < n; ++k )
  {
    // Find next part that has element in underlying GLOBAL stride
    do
    {
      ++part_idx_;
    } while ( part_idx_ <= composite_collection_->last_part_
      and composite_collection_->first_in_part_[ part_idx_ ] == invalid_index );

    if ( part_idx_ <= composite_collection_->last_part_ )
    {
      // We have a candidate part and a first valid element in it, so we perform phase adjustment

      assert( composite_collection_->first_in_part_[ part_idx_ ] != invalid_index );
      element_idx_ = composite_collection_->first_in_part_[ part_idx_ ];

      // Now perform phase adjustment
      switch ( kind_ )
      {
      case NCIteratorKind::RANK_LOCAL:
      {
        const size_t num_ranks = kernel::manager< MPIManager >.get_num_processes();
        const size_t current_rank = kernel::manager< MPIManager >.get_rank();

        std::tie( part_idx_, element_idx_ ) = composite_collection_->specific_local_begin_(
          num_ranks, current_rank, part_idx_, element_idx_, NodeCollectionComposite::gid_to_rank_ );

        FULL_LOGGING_ONLY( kernel::manager< KernelManager >.write_to_dump( String::compose(
          "ACIL rk %1, pix %2, eix %3", kernel::manager< MPIManager >.get_rank(), part_idx_, element_idx_ ) ); )
        break;
      }
      case NCIteratorKind::THREAD_LOCAL:
      {
        const size_t num_vps = kernel::manager< VPManager >.get_num_virtual_processes();
        const size_t current_vp =
          kernel::manager< VPManager >.thread_to_vp( kernel::manager< VPManager >.get_thread_id() );

        std::tie( part_idx_, element_idx_ ) = composite_collection_->specific_local_begin_(
          num_vps, current_vp, part_idx_, element_idx_, NodeCollectionComposite::gid_to_vp_ );

        break;
      }
      default:
        assert( false ); // should not be here, otherwise kind_ is inconsistent
        break;
      }
    }
    else
    {
      break; // no more parts to search
    }
  }

  // In case we did not find a solution in phase adjustment, set to end()
  if ( part_idx_ == invalid_index or not composite_collection_->valid_idx_( part_idx_, element_idx_ ) )
  {
    // Node collection exhausted, set to end()
    part_idx_ = composite_collection_->last_part_;
    element_idx_ = composite_collection_->last_elem_ + 1;
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
    gt.nc_index = element_idx_;
  }
  else
  {
    if ( not composite_collection_->valid_idx_( part_idx_, element_idx_ ) )
    {
      FULL_LOGGING_ONLY( kernel::manager< KernelManager >.write_to_dump(
        String::compose( "nci::op* comp err rk %1, lp %2, le %3, pix %4, eix %5, end_pix %6, end_eix %7",
          kernel::manager< MPIManager >.get_rank(),
          composite_collection_->last_part_,
          composite_collection_->last_elem_,
          part_idx_,
          element_idx_,
          composite_collection_->end().part_idx_,
          composite_collection_->end().element_idx_ ) ); )
      assert( false );
      throw KernelException( "Invalid NodeCollection iterator for composite collection)" );
    }

    const auto part_begin_idx = part_idx_ == 0 ? 0 : composite_collection_->cumul_abs_size_[ part_idx_ - 1 ];
    gt.nc_index = part_begin_idx + element_idx_;
    gt.node_id = composite_collection_->parts_[ part_idx_ ][ element_idx_ ];
    gt.model_id = composite_collection_->parts_[ part_idx_ ].model_id_;
  }

  return gt;
}

NodeCollection::NodeCollection()
  : fingerprint_( kernel::manager< KernelManager >.get_fingerprint() )
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
  size_t current_model = kernel::manager< ModelRangeManager >.get_model_id( node_ids[ 0 ] );

  std::vector< NodeCollectionPrimitive > parts;

  size_t old_node_id = current_first;
  for ( auto node_id = std::next( node_ids.begin() ); node_id < node_ids.end(); ++node_id )
  {
    if ( *node_id == old_node_id )
    {
      throw BadProperty( "All node IDs in a NodeCollection have to be unique" );
    }
    old_node_id = *node_id;

    const size_t next_model = kernel::manager< ModelRangeManager >.get_model_id( *node_id );

    if ( next_model == current_model and *node_id == ( current_last + 1 ) )
    {
      // node goes in Primitive
      ++current_last;
    }
    else
    {
      // store completed Primitive; node goes in new Primitive
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
  return fingerprint_ == kernel::manager< KernelManager >.get_fingerprint();
}

void
NodeCollection::get_metadata_status( DictionaryDatum& d ) const
{
  NodeCollectionMetadataPTR meta = get_metadata();
  if ( not meta )
  {
    return;
  }
  meta->get_status( d, this );
}

NodeCollectionPrimitive::NodeCollectionPrimitive( size_t first,
  size_t last,
  size_t model_id,
  NodeCollectionMetadataPTR meta )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( meta )
  , nodes_have_no_proxies_( not kernel::manager< ModelManager >.get_node_model( model_id_ )->has_proxies() )
{
  assert( first_ <= last_ );
  assert_consistent_model_ids_( model_id_ );
}

NodeCollectionPrimitive::NodeCollectionPrimitive( size_t first, size_t last, size_t model_id )
  : first_( first )
  , last_( last )
  , model_id_( model_id )
  , metadata_( nullptr )
  , nodes_have_no_proxies_( not kernel::manager< ModelManager >.get_node_model( model_id_ )->has_proxies() )
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
  const auto first_model_id = kernel::manager< ModelRangeManager >.get_model_id( first );
  const auto init_index = first + 1;
  for ( size_t node_id = init_index; node_id <= last; ++node_id )
  {
    const auto model_id = kernel::manager< ModelRangeManager >.get_model_id( node_id );
    if ( model_id != first_model_id )
    {
      throw BadProperty( "model ids does not match" );
    }
  }
  model_id_ = first_model_id;
  nodes_have_no_proxies_ = not kernel::manager< ModelManager >.get_node_model( model_id_ )->has_proxies();
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
NodeCollection::to_array( const std::string& selection ) const
{
  ArrayDatum node_ids;

  if ( selection == "thread" )
  {
    // We must do the folloing on the corresponding threads, but one at
    // a time to fill properly. Thread beginnings are marked by 0 thread 0
#pragma omp parallel
    {
#pragma omp critical
      {
        // We need to defined zero explicitly here, otherwise push_back() does strange things
        const size_t zero = 0;
        node_ids.push_back( zero );
        node_ids.push_back( kernel::manager< VPManager >.get_thread_id() );
        node_ids.push_back( zero );

        const auto end_it = end();
        for ( auto it = thread_local_begin(); it < end_it; ++it )
        {
          node_ids.push_back( ( *it ).node_id );
        }
      } // end critical
    }   // end parallel
  }
  else
  {
    // Slightly repetitive code but nc_const_iterator does not have
    // no-argument constructor nor copy constructor and this is a debug function only.
    if ( selection == "all" )
    {
      for ( const auto& val : *this )
      {
        node_ids.push_back( val.node_id );
      }
    }
    else if ( selection == "rank" )
    {
      const auto end_it = end();
      for ( auto it = rank_local_begin(); it < end_it; ++it )
      {
        node_ids.push_back( ( *it ).node_id );
      }
    }
    else
    {
      throw BadParameter(
        String::compose( "to_array() accepts only 'all', 'rank', 'thread', but got '%1'.", selection ) );
    }
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
    {
      // contiguous and homogeneous, lhs before rhs
      return std::make_shared< NodeCollectionPrimitive >( first_, rhs_ptr->last_, model_id_, metadata_ );
    }
    else if ( ( rhs_ptr->last_ + 1 ) == first_ and model_id_ == rhs_ptr->model_id_ )
    {
      // contiguous and homogeneous, rhs before lhs
      return std::make_shared< NodeCollectionPrimitive >( rhs_ptr->first_, last_, model_id_, metadata_ );
    }
    else
    {
      // not contiguous and homogeneous
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

NodeCollection::const_iterator
NodeCollectionPrimitive::rank_local_begin( NodeCollectionPTR cp ) const
{
  const size_t num_processes = kernel::manager< MPIManager >.get_num_processes();
  const size_t rank = kernel::manager< MPIManager >.get_rank();
  const size_t first_elem_rank =
    kernel::manager< MPIManager >.get_process_id_of_vp( kernel::manager< VPManager >.node_id_to_vp( first_ ) );
  const size_t elem_idx = ( rank - first_elem_rank + num_processes ) % num_processes;

  if ( elem_idx > size() ) // Too few node IDs to be shared among all MPI processes.
  {
    return const_iterator( cp, *this, size(), 1, nc_const_iterator::NCIteratorKind::END ); // end iterator
  }
  else
  {
    return const_iterator( cp, *this, elem_idx, num_processes, nc_const_iterator::NCIteratorKind::RANK_LOCAL );
  }
}

NodeCollection::const_iterator
NodeCollectionPrimitive::thread_local_begin( NodeCollectionPTR cp ) const
{
  const size_t num_vps = kernel::manager< VPManager >.get_num_virtual_processes();
  const size_t current_vp = kernel::manager< VPManager >.thread_to_vp( kernel::manager< VPManager >.get_thread_id() );
  const size_t vp_first_node = kernel::manager< VPManager >.node_id_to_vp( first_ );
  const size_t offset = ( current_vp - vp_first_node + num_vps ) % num_vps;

  if ( offset >= size() ) // Too few node IDs to be shared among all vps.
  {
    return nc_const_iterator( cp, *this, size(), 1, nc_const_iterator::NCIteratorKind::END ); // end iterator
  }
  else
  {
    return nc_const_iterator( cp, *this, offset, num_vps, nc_const_iterator::NCIteratorKind::THREAD_LOCAL );
  }
}

NodeCollectionPTR
NodeCollectionPrimitive::slice( size_t start, size_t end, size_t stride ) const
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
  if ( stride == 1 and not metadata_ )
  {
    // Create primitive NodeCollection passing node IDs.
    // Subtract 1 because "end" is one past last element to take while constructor expects ID of last node.
    sliced_nc = std::make_shared< NodeCollectionPrimitive >( first_ + start, first_ + end - 1, model_id_ );
  }
  else
  {
    // This is the "slicing" constructor, so we use slicing logic and pass end as it is
    sliced_nc = std::make_shared< NodeCollectionComposite >( *this, start, end, stride );
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
    model_id_ != invalid_index ? kernel::manager< ModelManager >.get_node_model( model_id_ )->get_name() : "none";

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
    const auto model_id = kernel::manager< ModelRangeManager >.get_model_id( node_id );
    if ( model_id != expected_model_id )
    {
      const auto node_model = kernel::manager< ModelRangeManager >.get_model_of_node_id( model_id )->get_name();
      const auto expected_model =
        kernel::manager< ModelRangeManager >.get_model_of_node_id( expected_model_id )->get_name();
      const auto message = "All nodes must have the same model (node with ID " + std::to_string( node_id )
        + " has model " + node_model + ", expected " + expected_model + ")";
      throw BadProperty( message );
    }
  }
}

NodeCollectionComposite::NodeCollectionComposite( const NodeCollectionPrimitive& primitive,
  size_t start,
  size_t end,
  size_t stride )
  : parts_( { primitive } )
  , size_( 1 + ( end - start - 1 ) / stride ) // see comment on constructor
  , stride_( stride )
  , first_part_( 0 )
  , first_elem_( start )
  , last_part_( 0 )
  , last_elem_( end - 1 )
  , is_sliced_( start != 0 or end != primitive.size() or stride > 1 )
  , cumul_abs_size_( { primitive.size() } )
  , first_in_part_( { first_elem_ } )
{
  assert( end > 0 );
  assert( first_elem_ <= last_elem_ );
}

NodeCollectionComposite::NodeCollectionComposite( const std::vector< NodeCollectionPrimitive >& parts )
  : parts_()
  , size_( 0 )
  , stride_( 1 )
  , first_part_( 0 )
  , first_elem_( 0 )
  , last_part_( 0 )
  , last_elem_( 0 )
  , is_sliced_( false )
  , cumul_abs_size_()
  , first_in_part_()
{
  if ( parts.size() < 1 )
  {
    throw BadProperty( "Cannot create an empty composite NodeCollection" );
  }

  NodeCollectionMetadataPTR meta = parts[ 0 ].get_metadata();

  for ( const auto& part : parts )
  {
    if ( meta.get() and not( meta == part.get_metadata() ) )
    {
      throw BadProperty( "all metadata in a NodeCollection must be the same" );
    }

    if ( not part.empty() )
    {
      parts_.push_back( part );
      size_ += part.size();
    }
  }

  const auto n_parts = parts_.size();
  if ( parts_.size() == 0 )
  {
    throw BadProperty( "Cannot create composite NodeCollection from only empty parts" );
  }

  std::sort( parts_.begin(), parts_.end(), primitive_sort_op );

  // Only after sorting can we set up the remaining fields
  last_part_ = n_parts - 1;
  last_elem_ = parts_[ last_part_ ].size() - 1; // well defined because we allow no empty parts

  cumul_abs_size_.resize( n_parts );
  cumul_abs_size_[ 0 ] = parts_[ 0 ].size();
  for ( size_t pix = 1; pix < n_parts; ++pix )
  {
    cumul_abs_size_[ pix ] = cumul_abs_size_[ pix - 1 ] + parts_[ pix ].size();
  }

  // All parts start at beginning since no slicing
  std::vector< size_t >( n_parts, 0 ).swap( first_in_part_ );
}

NodeCollectionComposite::NodeCollectionComposite( const NodeCollectionComposite& composite,
  size_t start,
  size_t end,
  size_t stride )
  : parts_( composite.parts_ )
  , size_( 1 + ( end - start - 1 ) / stride ) // see comment on constructor
  , stride_( stride )
  , first_part_( 0 )
  , first_elem_( 0 )
  , last_part_( 0 )
  , last_elem_( 0 )
  , is_sliced_( true )
  , cumul_abs_size_( parts_.size(), 0 )
  , first_in_part_( parts_.size(), invalid_index )
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
    if ( size_ > 1 )
    {
      // Creating a sliced NC with more than one node ID from a sliced NC is impossible.
      throw BadProperty( "Cannot slice a sliced composite NodeCollection." );
    }

    // we have a single node ID, must just find where it is.
    const nc_const_iterator it = composite.begin() + start;
    std::tie( first_part_, first_elem_ ) = it.get_part_offset();
    last_part_ = first_part_;
    last_elem_ = first_elem_;

    cumul_abs_size_[ first_part_ ] = parts_[ first_part_ ].size(); // absolute size of the one valid part
    first_in_part_[ first_part_ ] = first_elem_;
  }
  else
  {
    // The NodeCollection is not sliced
    // Update start and stop positions.
    const nc_const_iterator first_it = composite.begin() + start;
    std::tie( first_part_, first_elem_ ) = first_it.get_part_offset();

    const nc_const_iterator last_it = composite.begin() + ( end - 1 );
    std::tie( last_part_, last_elem_ ) = last_it.get_part_offset();

    // Fill cumulative size/first_in data structures beginning with first_part_
    // All entries have been initialized with 0 or invalid_index, respectively
    cumul_abs_size_[ first_part_ ] = parts_[ first_part_ ].size();
    first_in_part_[ first_part_ ] = first_elem_;

    for ( size_t pix = first_part_ + 1; pix <= last_part_; ++pix )
    {
      const auto prev_cas = cumul_abs_size_[ pix - 1 ];
      cumul_abs_size_[ pix ] = prev_cas + parts_[ pix ].size();

      // Compute absolute index from beginning of first_part_ for first element beyond part j-1
      const auto prev_num_elems = 1 + ( ( prev_cas - 1 - first_elem_ ) / stride_ );
      const auto next_elem_abs_idx = first_elem_ + prev_num_elems * stride_;
      assert( next_elem_abs_idx >= prev_cas );
      const auto next_elem_loc_idx = next_elem_abs_idx - prev_cas;

      // We have a next element if it is in the part; if we are in last_part_, we must not have passed last_elem
      if ( next_elem_abs_idx < cumul_abs_size_[ pix ] and ( pix < last_part_ or next_elem_loc_idx <= last_elem_ ) )
      {
        first_in_part_[ pix ] = next_elem_loc_idx;
      }
      else
      {
        first_in_part_[ pix ] = invalid_index;
      }
    }
  }

  // For consistency, fill size values of remaining entries
  for ( size_t pix = last_part_ + 1; pix < parts_.size(); ++pix )
  {
    cumul_abs_size_[ pix ] = cumul_abs_size_[ last_part_ ];
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
    assert( stride_ > 1 or last_part_ != 0 or last_elem_ != 0 );
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
      assert( rhs_ptr->stride_ > 1 or rhs_ptr->last_part_ != 0 or rhs_ptr->last_elem_ != 0 );
      throw BadProperty( "Cannot add NodeCollection to a sliced composite." );
    }

    // check overlap between the two composites
    const auto shortest_longest_nc = std::minmax( *this,
      *rhs_ptr,
      []( const NodeCollectionComposite& a, const NodeCollectionComposite& b ) { return a.size() < b.size(); } );
    const auto& shortest = shortest_longest_nc.first;
    const auto& longest = shortest_longest_nc.second;

    for ( const auto& short_elem : shortest )
    {
      if ( longest.contains( short_elem.node_id ) )
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
    // Composite is sliced, we use iterator arithmetic.
    return ( *( begin() + i ) ).node_id;
  }
  else
  {
    // Composite is not sliced, we can do a more efficient search.
    // TODO: Is this actually more efficient?
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
    throw std::out_of_range( String::compose( "pos %1 points outside of the NodeCollection", i ) );
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
  for ( auto lhs_nc = parts_.begin(); lhs_nc < parts_.end(); ++lhs_nc, ++rhs_nc ) // iterate over NodeCollections
  {
    if ( not( ( *lhs_nc ) == ( *rhs_nc ) ) )
    {
      return false;
    }
  }
  return true;
}


std::pair< size_t, size_t >
NodeCollectionComposite::specific_local_begin_( size_t period,
  size_t phase,
  size_t first_part,
  size_t first_elem,
  gid_to_phase_fcn_ gid_to_phase ) const
{
  assert( first_elem < parts_[ first_part ].size() );

  size_t pix = first_part;
  do
  {
    const size_t phase_first_node = gid_to_phase( parts_[ pix ][ first_elem ] );

    size_t elem_idx = first_index( period, phase_first_node, stride_, phase );
    /* elem_idx can now be
     * - elem_idx < part.size() : we have a solution
     * - invalid_index : equation not solvable in existing part (eg even thread and nc has only odd gids), must search
     * in remaining parts
     * - elem_idx >= part.size() : there would be a solution if the part had been larger with same structure
     */

    // Add starting point only if valid offset, otherwise we would invalidate invalid_index marker
    if ( elem_idx != invalid_index )
    {
      elem_idx += first_elem;
    }

    FULL_LOGGING_ONLY( kernel::manager< KernelManager >.write_to_dump(
      String::compose( "SPLB rk %1, thr %2, phase_first %3, offs %4, stp %5, sto %6,"
                       " pix %7, lp %8, le %9, primsz %10, nprts: %11, this: %12",
        kernel::manager< MPIManager >.get_rank(),
        kernel::manager< VPManager >.get_thread_id(),
        phase_first_node,
        offset,
        first_part,
        first_elem,
        pix,
        last_part_,
        last_elem_,
        parts_[ pix ].size(),
        parts_.size(),
        this ) ); )

    if ( elem_idx != invalid_index and elem_idx < parts_[ pix ].size()
      and ( pix < last_part_ or elem_idx <= last_elem_ ) )
    {
      assert( gid_to_phase( parts_[ pix ][ elem_idx ] ) == phase );
      return { pix, elem_idx };
    }
    else
    {
      // find next part with at least one element in stride
      do
      {
        ++pix;
      } while ( pix <= last_part_ and first_in_part_[ pix ] == invalid_index );

      if ( pix > last_part_ )
      {
        // node collection exhausted
        return { invalid_index, invalid_index };
      }
      else
      {
        first_elem = first_in_part_[ pix ];
      }
    }
  } while ( pix <= last_part_ );

  return { invalid_index, invalid_index };
}

size_t
NodeCollectionComposite::gid_to_vp_( size_t gid )
{
  return kernel::manager< VPManager >.node_id_to_vp( gid );
}

size_t
NodeCollectionComposite::gid_to_rank_( size_t gid )
{
  return kernel::manager< MPIManager >.get_process_id_of_vp( kernel::manager< VPManager >.node_id_to_vp( gid ) );
}

NodeCollection::const_iterator
NodeCollectionComposite::rank_local_begin( NodeCollectionPTR cp ) const
{
  const size_t num_ranks = kernel::manager< MPIManager >.get_num_processes();
  const size_t current_rank = kernel::manager< MPIManager >.get_rank();

  const auto [ part_index, part_offset ] =
    specific_local_begin_( num_ranks, current_rank, first_part_, first_elem_, gid_to_rank_ );
  if ( part_index != invalid_index and part_offset != invalid_index )
  {
    return nc_const_iterator( cp,
      *this,
      part_index,
      part_offset,
      std::lcm( stride_, num_ranks ),
      nc_const_iterator::NCIteratorKind::RANK_LOCAL );
  }
  else
  {
    return end( cp );
  }
}

NodeCollection::const_iterator
NodeCollectionComposite::thread_local_begin( NodeCollectionPTR cp ) const
{
  const size_t num_vps = kernel::manager< VPManager >.get_num_virtual_processes();
  const size_t current_vp = kernel::manager< VPManager >.thread_to_vp( kernel::manager< VPManager >.get_thread_id() );

  const auto [ part_index, part_offset ] =
    specific_local_begin_( num_vps, current_vp, first_part_, first_elem_, gid_to_vp_ );

  if ( part_index != invalid_index and part_offset != invalid_index )
  {
    return nc_const_iterator( cp,
      *this,
      part_index,
      part_offset,
      std::lcm( stride_, num_vps ),
      nc_const_iterator::NCIteratorKind::THREAD_LOCAL );
  }
  else
  {
    return end( cp );
  }
}

NodeCollectionPTR
NodeCollectionComposite::slice( size_t start, size_t end, size_t stride ) const
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

  FULL_LOGGING_ONLY( kernel::manager< KernelManager >.write_to_dump( "Calling NCC from slice()" ); )
  const auto new_composite = NodeCollectionComposite( *this, start, end, stride );
  FULL_LOGGING_ONLY( kernel::manager< KernelManager >.write_to_dump( "Calling NCC from slice() --- DONE" ); )

  if ( stride == 1 and new_composite.first_part_ == new_composite.last_part_ )
  {
    // Return only the primitive; pass last_elem_+1 because slice() expects end argument
    return new_composite.parts_[ new_composite.first_part_ ].slice(
      new_composite.first_elem_, new_composite.last_elem_ + 1 );
  }

  FULL_LOGGING_ONLY( kernel::manager< KernelManager >.write_to_dump(
    String::compose( "NewComposite: fp %1, fe %2, lp %3, le %4, sz %5, strd %6",
      new_composite.first_part_,
      new_composite.first_elem_,
      new_composite.last_part_,
      new_composite.last_elem_,
      new_composite.size_,
      new_composite.stride_ ) ); )

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

long
NodeCollectionComposite::get_nc_index( const size_t node_id ) const
{
  // Check if node is in node collection
  if ( node_id < parts_[ first_part_ ][ first_elem_ ] or parts_[ last_part_ ][ last_elem_ ] < node_id )
  {
    return -1;
  }

  // Find part to which node belongs
  size_t lower = first_part_;
  size_t upper = last_part_;
  while ( lower < upper )
  {
    // Because lower < upper, we are guaranteed that mid < upper
    const size_t mid = ( lower + upper ) / 2;

    // Because mid < upper <=> mid < last_part_, we do not need to worry about last_elem_
    if ( parts_[ mid ][ parts_[ mid ].size() - 1 ] < node_id )
    {
      lower = mid + 1;
    }
    // mid == first_part_ is possible, but if node_id is before start_elem_,
    // we handled that at the beginning, so here we just check if the node_id
    // comes before the mid part
    else if ( node_id < parts_[ mid ][ 0 ] )
    {
      upper = mid - 1;
    }
    else
    {
      lower = upper = mid;
    }
  }

  // If node_id is not in the NodeCollection, lower may pass upper in the loop above
  // See test_regression_issue-3213.py for an example case.
  assert( lower >= upper );

  if ( lower > upper or node_id < parts_[ lower ][ 0 ] or parts_[ lower ][ parts_[ lower ].size() - 1 ] < node_id )
  {
    // node_id is in a gap of nc
    return -1;
  }

  // We now know that lower == upper and that if the node is in this part
  // if it is in the node collection. We do not need to check for first/last,
  // since we did that above.
  const auto part_begin_idx = lower == 0 ? 0 : cumul_abs_size_[ lower - 1 ];
  const auto node_idx = part_begin_idx + parts_[ lower ].get_nc_index( node_id );

  if ( not is_sliced_ )
  {
    // Since NC is not sliced, node_idx is the desired index
    assert( this->operator[]( node_idx ) == node_id );
    return node_idx;
  }
  else
  {
    // We need to take stride into account
    const auto distance_from_first = node_idx - first_elem_;

    // Exploit that same stride applies to all parts
    if ( distance_from_first % stride_ == 0 )
    {
      const auto sliced_node_idx = distance_from_first / stride_;
      assert( this->operator[]( sliced_node_idx ) == node_id );
      return sliced_node_idx;
    }
    else
    {
      return -1;
    }
  }
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
    size_t current_part = 0;
    size_t current_offset = 0;
    size_t previous_part = std::numeric_limits< size_t >::infinity();
    size_t primitive_last = 0;

    size_t primitive_size = 0;
    NodeIDTriple first_in_primitive = *begin();

    std::vector< std::string > string_vector;

    out << nc << "metadata=" << metadata << ",";

    const auto end_it = end();
    for ( nc_const_iterator it = begin(); it < end_it; ++it )
    {
      std::tie( current_part, current_offset ) = it.get_part_offset();
      if ( current_part != previous_part ) // New primitive
      {
        if ( it > begin() )
        {
          // Need to count the primitive, so can't start at begin()
          out << "\n" + space
              << "model=" << kernel::manager< ModelManager >.get_node_model( first_in_primitive.model_id )->get_name()
              << ", size=" << primitive_size << ", ";
          if ( primitive_size == 1 )
          {
            out << "first=" << first_in_primitive.node_id << ", last=" << first_in_primitive.node_id << ";";
          }
          else
          {
            out << "first=" << first_in_primitive.node_id << ", last=";
            out << primitive_last;
            if ( stride_ > 1 )
            {
              out << ", step=" << stride_ << ";";
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
    out << "\n" + space
        << "model=" << kernel::manager< ModelManager >.get_node_model( first_in_primitive.model_id )->get_name()
        << ", size=" << primitive_size << ", ";
    if ( primitive_size == 1 )
    {
      out << "first=" << first_in_primitive.node_id << ", last=" << first_in_primitive.node_id;
    }
    else
    {
      out << "first=" << first_in_primitive.node_id << ", last=";
      out << primitive_last;
      if ( stride_ > 1 )
      {
        out << ", step=" << stride_;
      }
    }
  }
  else
  {
    // Unsliced Composite NodeCollection
    out << nc << "metadata=" << metadata << ",";
    for ( auto it = parts_.begin(); it < parts_.end(); ++it )
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

bool
NodeCollectionComposite::valid_idx_( const size_t part_idx, const size_t element_idx ) const
{

  return part_idx < last_part_ or ( part_idx == last_part_ and element_idx <= last_elem_ );
}

bool
NodeCollectionComposite::contains( const size_t node_id ) const
{

  return get_nc_index( node_id ) != -1;
}

bool
NodeCollectionComposite::empty() const
{

  // Composite NodeCollections can never be empty.
  return false;
}

bool
NodeCollectionComposite::is_range() const
{

  return false;
}

NodeCollectionMetadataPTR
NodeCollectionComposite::get_metadata() const
{

  return parts_[ 0 ].get_metadata();
}

void
NodeCollectionComposite::set_metadata( NodeCollectionMetadataPTR meta )
{

  for ( auto& part : parts_ )
  {
    part.set_metadata( meta );
  }
}

size_t
NodeCollectionComposite::stride() const
{

  return stride_;
}

size_t
NodeCollectionComposite::size() const
{

  return size_;
}

NodeCollection::const_iterator
NodeCollectionComposite::end( NodeCollectionPTR cp ) const
{

  // The unique end() element of a composite NC is given by one past the last element
  // This is the (potentially non-existing) next element irrespective of stride and step
  return nc_const_iterator(
    cp, *this, last_part_, last_elem_ + 1, /* stride */ 1, nc_const_iterator::NCIteratorKind::END );
}

NodeCollection::const_iterator
NodeCollectionComposite::begin( NodeCollectionPTR cp ) const
{

  return nc_const_iterator( cp, *this, first_part_, first_elem_, stride_ );
}

bool
NodeCollectionPrimitive::has_proxies() const
{

  return not nodes_have_no_proxies_;
}

long
NodeCollectionPrimitive::get_nc_index( const size_t neuron_id ) const
{

  if ( neuron_id < first_ or last_ < neuron_id )
  {
    return -1;
  }
  else
  {
    return neuron_id - first_;
  }
}

bool
NodeCollectionPrimitive::empty() const
{

  return last_ == 0;
}

bool
NodeCollectionPrimitive::is_range() const
{

  return true;
}

NodeCollectionMetadataPTR
NodeCollectionPrimitive::get_metadata() const
{

  return metadata_;
}

void
NodeCollectionPrimitive::set_metadata( NodeCollectionMetadataPTR meta )
{

  metadata_ = meta;
}

bool
NodeCollectionPrimitive::contains( const size_t node_id ) const
{

  return first_ <= node_id and node_id <= last_;
}

size_t
NodeCollectionPrimitive::stride() const
{

  return 1;
}

size_t
NodeCollectionPrimitive::size() const
{

  // empty NC has first_ == last_ == 0, need to handle that special
  return std::min( last_, last_ - first_ + 1 );
}

NodeCollection::const_iterator
NodeCollectionPrimitive::end( NodeCollectionPTR cp ) const
{

  // The unique end() element of a primitive NC is given by (part 0, element size()) )
  return nc_const_iterator( cp, *this, /* offset */ size(), /* stride */ 1, nc_const_iterator::NCIteratorKind::END );
}

NodeCollection::const_iterator
NodeCollectionPrimitive::begin( NodeCollectionPTR cp ) const
{

  return nc_const_iterator( cp, *this, /* offset */ 0, /* stride */ 1 );
}

bool
NodeCollectionPrimitive::operator==( const NodeCollectionPrimitive& rhs ) const
{

  // Not dereferencing rhs_ptr->metadata_ in the equality comparison because we want to avoid overloading
  // operator==() of *metadata_, and to let it handle typechecking.
  const bool eq_metadata =
    ( not metadata_ and not rhs.metadata_ ) or ( metadata_ and rhs.metadata_ and *metadata_ == rhs.metadata_ );

  return first_ == rhs.first_ and last_ == rhs.last_ and model_id_ == rhs.model_id_ and eq_metadata;
}

bool
NodeCollectionPrimitive::operator==( NodeCollectionPTR rhs ) const
{

  auto const* const rhs_ptr = dynamic_cast< NodeCollectionPrimitive const* >( rhs.get() );
  // Checking that rhs_ptr is valid first, to avoid segfaults. If rhs is a NodeCollectionComposite,
  // rhs_ptr will be a null pointer.
  if ( not rhs_ptr )
  {
    return false;
  }

  // We know we have a primitive collection, so forward
  return *this == *rhs_ptr;
}

size_t
NodeCollectionPrimitive::operator[]( const size_t idx ) const
{

  // throw exception if outside of NodeCollection
  if ( first_ + idx > last_ )
  {
    throw std::out_of_range( String::compose( "pos %1 points outside of the NodeCollection", idx ) );
  }
  return first_ + idx;
}

size_t
nc_const_iterator::get_step_size() const
{

  return step_;
}

std::pair< size_t, size_t >
nc_const_iterator::get_part_offset() const
{

  return { part_idx_, element_idx_ };
}

bool
nc_const_iterator::operator>=( const nc_const_iterator& rhs ) const
{

  return not( *this < rhs );
}

bool
nc_const_iterator::operator>( const nc_const_iterator& rhs ) const
{

  return not( *this <= rhs );
}

bool
nc_const_iterator::operator<=( const nc_const_iterator& rhs ) const
{

  return ( *this < rhs or *this == rhs );
}

bool
nc_const_iterator::operator<( const nc_const_iterator& rhs ) const
{

  return ( part_idx_ < rhs.part_idx_ or ( part_idx_ == rhs.part_idx_ and element_idx_ < rhs.element_idx_ ) );
}

bool
nc_const_iterator::operator!=( const nc_const_iterator& rhs ) const
{

  return not( *this == rhs );
}

bool
nc_const_iterator::operator==( const nc_const_iterator& rhs ) const
{

  return part_idx_ == rhs.part_idx_ and element_idx_ == rhs.element_idx_;
}

nc_const_iterator
nc_const_iterator::operator++( int )
{

  nc_const_iterator tmp = *this;
  ++( *this );
  return tmp;
}

nc_const_iterator&
nc_const_iterator::operator++()
{

  ( *this ) += 1;
  return *this;
}

nc_const_iterator
nc_const_iterator::operator+( const size_t n ) const
{

  nc_const_iterator it = *this;
  return it += n;
}

nc_const_iterator&
nc_const_iterator::operator+=( const size_t n )
{

  assert( kind_ != NCIteratorKind::END );

  if ( n == 0 )
  {
    return *this;
  }

  const auto new_element_idx = find_next_within_part_( n );

  // For a primitive collection, we either have a new element or are at the end
  // For a composite collection, we may need to search through further parts,
  // which is signalled by new_element_idx == element_idx_
  if ( primitive_collection_ or new_element_idx != element_idx_ )
  {
    element_idx_ = new_element_idx;
  }
  else
  {
    // We did not find a new element in the current part and have not exhausted the collection
    if ( kind_ == NCIteratorKind::GLOBAL )
    {
      advance_global_iter_to_new_part_( n );
    }
    else
    {
      advance_local_iter_to_new_part_( n );
    }
  }

  return *this;
}

size_t
NodeCollection::get_last() const
{

  assert( size() > 0 );
  return ( *( begin() + ( size() - 1 ) ) ).node_id;
}

size_t
NodeCollection::get_first() const
{

  return ( *begin() ).node_id;
}

void
NodeCollection::set_metadata( NodeCollectionMetadataPTR )
{

  throw KernelException( "Cannot set Metadata on this type of NodeCollection." );
}

bool
NodeCollection::operator!=( NodeCollectionPTR rhs ) const
{

  return not( *this == rhs );
}

NodeCollectionPTR
operator+( NodeCollectionPTR lhs, NodeCollectionPTR rhs )
{
  return lhs->operator+( rhs );
}

} // namespace nest
