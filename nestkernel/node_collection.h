/*
 *  node_collection.h
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

#ifndef NODE_COLLECTION_H
#define NODE_COLLECTION_H

// C++ includes:
#include <ctime>
#include <ostream>
#include <stdexcept> // out_of_range
#include <vector>
#include <memory>

// Includes from libnestuil:
#include "lockptr.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"

namespace nest
{
class NodeCollection;
class NodeCollectionPrimitive;
class NodeCollectionComposite;
class NodeCollectionMetadata;

using NodeCollectionPTR = std::shared_ptr< NodeCollection >;
using NodeCollectionMetadataPTR = std::shared_ptr< NodeCollectionMetadata >;

/**
 * Class for Metadata attached to NodeCollection.
 *
 * NEST modules that want to add metadata to NodeCollections they
 * create need to implement their own concrete subclass.
 */
class NodeCollectionMetadata
{
public:
  NodeCollectionMetadata() = default;
  virtual ~NodeCollectionMetadata() = default;

  virtual void set_status( const DictionaryDatum&, bool ) = 0;
  virtual void get_status( DictionaryDatum& ) const = 0;

  virtual void set_first_node_id( index ) = 0;
  virtual index get_first_node_id() const = 0;
  virtual std::string get_type() const = 0;
};

class NodeIDTriple
{
public:
  index node_id{ 0 };
  index model_id{ 0 };
  size_t lid{ 0 };
  NodeIDTriple() = default;
};

/**
 * Iterator for NodeCollections.
 *
 * This iterator can iterate over primitive and composite NodeCollections.
 * Behavior is determined by the constructor used to create the iterator.
 */
class nc_const_iterator
{
  friend class NodeCollectionPrimitive;
  friend class NodeCollectionComposite;

private:
  NodeCollectionPTR coll_ptr_; //!< holds pointer reference in safe iterators
  size_t element_idx_;         //!< index into (current) primitive node collection
  size_t part_idx_;            //!< index into parts vector of composite collection
  size_t step_;                //!< step for skipping due to e.g. slicing

  /**
   * Pointer to primitive collection to iterate over.
   * Zero if iterator is for composite collection.
   */
  NodeCollectionPrimitive const* const primitive_collection_;

  /**
   * Pointer to composite collection to iterate over.
   * Zero if iterator is for primitive collection.
   */
  NodeCollectionComposite const* const composite_collection_;

  /**
   * Create safe iterator for NodeCollectionPrimitive.
   * @param collection_ptr smart pointer to collection to keep collection alive
   * @param collection  Collection to iterate over
   * @param offset  Index of collection element iterator points to
   * @param step    Step for skipping due to e.g. slicing
   */
  explicit nc_const_iterator( NodeCollectionPTR collection_ptr,
    const NodeCollectionPrimitive& collection,
    size_t offset,
    size_t step = 1 );

  /**
   * Create safe iterator for NodeCollectionComposite.
   * @param collection_ptr smart pointer to collection to keep collection alive
   * @param collection  Collection to iterate over
   * @param part    Index of part of collection iterator points to
   * @param offset  Index of element in NC part that iterator points to
   * @param step    Step for skipping due to e.g. slicing
   */
  explicit nc_const_iterator( NodeCollectionPTR collection_ptr,
    const NodeCollectionComposite& collection,
    size_t part,
    size_t offset,
    size_t step = 1 );

public:
  nc_const_iterator( const nc_const_iterator& nci ) = default;
  void get_current_part_offset( size_t&, size_t& );

  NodeIDTriple operator*() const;
  bool operator!=( const nc_const_iterator& rhs ) const;
  bool operator<( const nc_const_iterator& rhs ) const;
  bool operator<=( const nc_const_iterator& rhs ) const;

  nc_const_iterator& operator++();
  nc_const_iterator& operator+=( const size_t );
  nc_const_iterator operator+( const size_t ) const;

  void print_me( std::ostream& ) const;
};

/**
 * Superclass for NodeCollections.
 *
 * The superclass acts as an interface to the primitive and composite
 * NodeCollection types. It contains methods, mostly virtual, for the subclasses,
 * and also create()-methods to be interfaced externally.
 *
 * The superclass also contains handling of the fingerprint, a unique identity
 * the NodeCollection gets from the kernel on creation, which ensures that the
 * NodeCollection is not used after the kernel is reset.
 */
class NodeCollection
{
  friend class nc_const_iterator;

public:
  using const_iterator = nc_const_iterator;

  /**
   * Initializer gets current fingerprint from the kernel.
   */
  NodeCollection();

  virtual ~NodeCollection() = default;

  /**
   * Create a NodeCollection from a vector of node IDs. Results in a primitive if the
   * node IDs are homogeneous and contiguous, or a composite otherwise.
   *
   * @param node_ids Vector of node IDs from which to create the NodeCollection
   * @return a NodeCollection pointer to the created NodeCollection
   */
  static NodeCollectionPTR create( const IntVectorDatum& node_ids );

  /**
   * Create a NodeCollection from an array of node IDs. Results in a primitive if the
   * node IDs are homogeneous and contiguous, or a composite otherwise.
   *
   * @param node_ids Array of node IDs from which to create the NodeCollection
   * @return a NodeCollection pointer to the created NodeCollection
   */
  static NodeCollectionPTR create( const TokenArray& node_ids );

  /**
   * Check to see if the fingerprint of the NodeCollection matches that of the
   * kernel.
   *
   * @return true if the fingerprint matches that of the kernel, false otherwise
   */
  bool valid() const;

  /**
   * Print out the contents of the NodeCollection in a pretty and informative
   * way.
   */
  virtual void print_me( std::ostream& ) const = 0;

  /**
   * Get the node ID in the specified index in the NodeCollection.
   *
   * @param idx Index in the NodeCollection
   * @return a node ID
   */
  virtual index operator[]( size_t ) const = 0;

  /**
   * Join two NodeCollections. May return a primitive or composite, depending on
   * the input.
   *
   * @param rhs NodeCollection pointer to the NodeCollection to be added
   * @return a NodeCollection pointer
   */
  virtual NodeCollectionPTR operator+( NodeCollectionPTR ) const = 0;
  virtual bool operator==( NodeCollectionPTR ) const = 0;

  /**
   * Check if two NodeCollections are equal.
   *
   * @param rhs NodeCollection pointer to the NodeCollection to be checked against
   * @return true if they are equal, false otherwise
   */
  virtual bool operator!=( NodeCollectionPTR ) const;

  /**
   * Method to get an iterator representing the beginning of the NodeCollection.
   *
   * @return an iterator representing the beginning of the NodeCollection
   */
  virtual const_iterator begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const = 0;

  /**
   * Method to get an iterator representing the beginning of the NodeCollection.
   *
   * @return an iterator representing the beginning of the NodeCollection, in a
   * parallel context.
   */
  virtual const_iterator local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const = 0;

  /**
   * Method to get an iterator representing the beginning of the NodeCollection.
   *
   * @return an iterator representing the beginning of the NodeCollection, in an
   * MPI-parallel context.
   */
  virtual const_iterator MPI_local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const = 0;

  /**
   * Method to get an iterator representing the end of the NodeCollection.
   *
   * @param offset Index of element NC that iterator points to
   *
   * @return an iterator representing the end of the NodeCollection, taking
   * offset into account
   */
  virtual const_iterator end( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const = 0;

  /**
   * Method that creates an ArrayDatum filled with node IDs from the NodeCollection.
   *
   * @return an ArrayDatum containing node IDs
   */
  virtual ArrayDatum to_array() const = 0;

  /**
   * Get the size of the NodeCollection.
   *
   * @return number of node IDs in the NodeCollection
   */
  virtual size_t size() const = 0;

  /**
   * Check if the NodeCollection contains a specified node ID
   *
   * @param node_id node ID to see if exists in the NodeCollection
   * @return true if the NodeCollection contains the node ID, false otherwise
   */
  virtual bool contains( index node_id ) const = 0;

  /**
   * Slices the NodeCollection to the boundaries, with an optional step
   * parameter. Note that the boundaries being specified are inclusive.
   *
   * @param start Index of the NodeCollection to start at
   * @param stop Index of the NodeCollection to stop at
   * @param step Number of places between node IDs to skip. Defaults to 1
   * @return a NodeCollection pointer to the new, sliced NodeCollection.
   */
  virtual NodeCollectionPTR slice( size_t start, size_t stop, size_t step ) const = 0;

  /**
   * Sets the metadata of the NodeCollection.
   *
   * @param meta A Metadata pointer
   */
  virtual void set_metadata( NodeCollectionMetadataPTR ) = 0;

  /**
   * Gets the metadata of the NodeCollection.
   *
   * @return A Metadata pointer
   */
  virtual NodeCollectionMetadataPTR get_metadata() const = 0;

  virtual bool is_range() const = 0;

  /**
   * Returns index of node with given node ID in NodeCollection.
   *
   * @return Index of node with given node ID; -1 if node not in NodeCollection.
   */
  virtual long find( const index ) const = 0;

private:
  unsigned long fingerprint_; //!< Unique identity of the kernel that created the //!< NodeCollection
  static NodeCollectionPTR create_();
  static NodeCollectionPTR create_( const std::vector< index >& );
};

/**
 * Subclass for the primitive NodeCollection type.
 *
 * The primitive type contains only homogeneous and contiguous node IDs. It also
 * contains model ID and metadata of the node IDs.
 */
class NodeCollectionPrimitive : public NodeCollection
{
  friend class nc_const_iterator;

private:
  index first_;                        //!< The first node ID in the primitive
  index last_;                         //!< The last node ID in the primitive
  index model_id_;                     //!< Model ID of the node IDs
  NodeCollectionMetadataPTR metadata_; //!< Pointer to the metadata of the node IDs

public:
  using const_iterator = nc_const_iterator;

  /**
   * Create a primitive from a range of node IDs, with provided model ID and
   * metadata pointer.
   *
   * @param first The first node ID in the primitive
   * @param last  The last node ID in the primitive
   * @param model_id Model ID of the node IDs
   * @param meta Metadata pointer of the node IDs
   */
  NodeCollectionPrimitive( index first, index last, index model_id, NodeCollectionMetadataPTR );

  /**
   * Create a primitive from a range of node IDs, with provided model ID.
   *
   * @param first The first node ID in the primitive
   * @param last  The last node ID in the primitive
   * @param model_id Model ID of the node IDs
   */
  NodeCollectionPrimitive( index first, index last, index model_id );

  /**
   * Create a primitive from a range of node IDs. The model ID has to be found by
   * the constructor.
   *
   * @param first The first node ID in the primitive
   * @param last  The last node ID in the primitive
   */
  NodeCollectionPrimitive( index first, index last );

  /**
   * Primitive copy constructor.
   *
   * @param rhs Primitive to copy
   */
  NodeCollectionPrimitive( const NodeCollectionPrimitive& );

  /**
   * Create empty NodeCollection.
   *
   * @note This is only for use by SPBuilder.
   */
  NodeCollectionPrimitive();

  void print_me( std::ostream& ) const override;
  void print_primitive( std::ostream& ) const;

  index operator[]( const size_t ) const override;
  NodeCollectionPTR operator+( NodeCollectionPTR rhs ) const override;
  bool operator==( const NodeCollectionPTR rhs ) const override;
  bool operator==( const NodeCollectionPrimitive& rhs ) const;

  const_iterator begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator MPI_local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator end( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;

  //! Returns an ArrayDatum filled with node IDs from the primitive.
  ArrayDatum to_array() const override;

  //! Returns total number of node IDs in the primitive.
  size_t size() const override;

  bool contains( index node_id ) const override;
  NodeCollectionPTR slice( size_t start, size_t stop, size_t step = 1 ) const override;

  void set_metadata( NodeCollectionMetadataPTR ) override;

  NodeCollectionMetadataPTR get_metadata() const override;

  bool is_range() const override;

  long find( const index ) const override;

  /**
   * Checks if node IDs in another primitive is a continuation of node IDs in this
   * primitive.
   *
   * @param other Primitive to check for continuity
   * @return True if the first element in the other primitive is the next after
   * the last element in this primitive, and they both have the same model ID.
   * Otherwise false.
   */
  bool is_contiguous_ascending( NodeCollectionPrimitive& other );

  /**
   * Checks if node IDs of another primitive is overlapping node IDs of this primitive
   *
   * @param rhs Primitive to be checked.
   * @return True if the other primitive overlaps, false otherwise.
   */
  bool overlapping( const NodeCollectionPrimitive& rhs ) const;
};

NodeCollectionPTR operator+( NodeCollectionPTR lhs, NodeCollectionPTR rhs );

/**
 * Subclass for the composite NodeCollection type.
 *
 * The composite type contains a collection of primitives which are not
 * contiguous and homogeneous with each other. If the composite is sliced, it
 * also holds information about what index to start at and which to end at, and
 * the step.
 */
class NodeCollectionComposite : public NodeCollection
{
  friend class nc_const_iterator;

private:
  std::vector< NodeCollectionPrimitive > parts_; //!< Vector of primitives
  size_t size_;                                  //!< Total number of node IDs
  size_t step_;                                  //!< Step length, set when slicing.
  size_t start_part_;                            //!< Primitive to start at, set when slicing
  size_t start_offset_;                          //!< Element to start at, set when slicing
  size_t stop_part_;                             //!< Primitive to stop at, set when slicing
  size_t stop_offset_;                           //!< Element to stop at, set when slicing

  /**
   * Goes through the vector of primitives, merging as much as possible.
   *
   * @param parts Vector of primitives to be merged.
   */
  void merge_parts( std::vector< NodeCollectionPrimitive >& parts ) const;

public:
  /**
   * Create a composite from a primitive, with boundaries and step length.
   *
   * @param primitive Primitive to be converted
   * @param start Offset in the primitive to begin at.
   * @param stop Offset in the primtive to stop at.
   * @param step Length to step in the primitive.
   */
  NodeCollectionComposite( const NodeCollectionPrimitive&, size_t, size_t, size_t );

  /**
     * Composite copy constructor.
     *
     * @param comp Composite to be copied.
     */
  NodeCollectionComposite( const NodeCollectionComposite& );

  /**
     * Creates a new composite from another, with boundaries and step length.
     * This constructor is used only when slicing.
     *
     * @param composite Composite to slice.
     * @param start Index in the composite to begin at.
     * @param stop Index in the composite to stop at.
     * @param step Length to step in the composite.
     */
  NodeCollectionComposite( const NodeCollectionComposite&, size_t, size_t, size_t );

  /**
   * Create a composite from a vector of primitives.
   *
   * @param parts Vector of primitives.
   */
  NodeCollectionComposite( const std::vector< NodeCollectionPrimitive >& );

  void print_me( std::ostream& ) const override;

  index operator[]( const size_t ) const override;

  /**
   * Addition operator.
   *
   * Joins this composite with another NodeCollection. The resulting
   * NodeCollection is sorted and merged, and converted to a primitive if
   * possible.
   *
   * @param rhs NodeCollection to add to this composite
   * @return a NodeCollection pointer to either a primitive or a composite.
   */
  NodeCollectionPTR operator+( NodeCollectionPTR rhs ) const override;
  NodeCollectionPTR operator+( const NodeCollectionPrimitive& rhs ) const;
  bool operator==( const NodeCollectionPTR rhs ) const override;

  const_iterator begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator MPI_local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator end( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;

  //! Returns an ArrayDatum filled with node IDs from the composite.
  ArrayDatum to_array() const override;

  //! Returns total number of node IDs in the composite.
  size_t size() const override;

  bool contains( index node_id ) const override;
  NodeCollectionPTR slice( size_t start, size_t stop, size_t step = 1 ) const override;

  void set_metadata( NodeCollectionMetadataPTR ) override;

  NodeCollectionMetadataPTR get_metadata() const override;

  bool is_range() const override;

  long find( const index ) const override;
};

inline bool NodeCollection::operator!=( NodeCollectionPTR rhs ) const
{
  return not( *this == rhs );
}

inline void NodeCollection::set_metadata( NodeCollectionMetadataPTR )
{
  throw KernelException( "Cannot set Metadata on this type of NodeCollection." );
}

inline NodeIDTriple nc_const_iterator::operator*() const
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
    if ( composite_collection_->stop_offset_ != 0 or composite_collection_->stop_part_ != 0 )
    {
      if ( not( part_idx_ < composite_collection_->stop_part_
             or ( part_idx_ == composite_collection_->stop_part_
                  and element_idx_ < composite_collection_->stop_offset_ ) ) )
      {
        throw KernelException( "Invalid NodeCollection iterator (composite element beyond specified stop element)" );
      }
    }
    else if ( part_idx_ >= composite_collection_->parts_.size()
      or element_idx_ >= composite_collection_->parts_[ part_idx_ ].size() )
    {
      throw KernelException( "Invalid NodeCollection iterator (composite element beyond last composite element)" );
    }

    // Add to local placement from NodeCollectionPrimitives that comes before the
    // current one.
    gt.lid = 0;
    for ( const auto& part : composite_collection_->parts_ )
    {
      if ( part == composite_collection_->parts_[ part_idx_ ] )
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

inline nc_const_iterator& nc_const_iterator::operator++()
{
  if ( primitive_collection_ )
  {
    element_idx_ += step_;
    if ( element_idx_ >= primitive_collection_->size() )
    {
      element_idx_ = primitive_collection_->size();
    }
  }
  else
  {
    element_idx_ += step_;
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
    if ( composite_collection_->stop_offset_ != 0 or composite_collection_->stop_part_ != 0 )
    {
      if ( part_idx_ >= composite_collection_->stop_part_ and element_idx_ >= composite_collection_->stop_offset_ )
      {
        part_idx_ = composite_collection_->stop_part_;
        element_idx_ = composite_collection_->stop_offset_;
      }
    }
    else if ( part_idx_ >= composite_collection_->parts_.size() )
    {
      auto end_of_composite = composite_collection_->end();
      part_idx_ = end_of_composite.part_idx_;
      element_idx_ = end_of_composite.element_idx_;
    }
  }
  return *this;
}

inline nc_const_iterator& nc_const_iterator::operator+=( const size_t n )
{
  if ( primitive_collection_ )
  {
    element_idx_ += n * step_;
  }
  else
  {
    for ( size_t i = 0; i < n; ++i )
    {
      operator++();
    }
  }
  return *this;
}

inline nc_const_iterator nc_const_iterator::operator+( const size_t n ) const
{
  nc_const_iterator it = *this;
  return it += n;
}

inline bool nc_const_iterator::operator!=( const nc_const_iterator& rhs ) const
{
  return not( part_idx_ == rhs.part_idx_ and element_idx_ == rhs.element_idx_ );
}

inline bool nc_const_iterator::operator<( const nc_const_iterator& rhs ) const
{
  return ( part_idx_ < rhs.part_idx_ or ( part_idx_ == rhs.part_idx_ and element_idx_ < rhs.element_idx_ ) );
}

inline bool nc_const_iterator::operator<=( const nc_const_iterator& rhs ) const
{
  return ( part_idx_ < rhs.part_idx_ or ( part_idx_ == rhs.part_idx_ and element_idx_ <= rhs.element_idx_ ) );
}

inline void
nc_const_iterator::get_current_part_offset( size_t& part, size_t& offset )
{
  part = part_idx_;
  offset = element_idx_;
}

inline index NodeCollectionPrimitive::operator[]( const size_t idx ) const
{
  // throw exception if outside of NodeCollection
  if ( first_ + idx > last_ )
  {
    throw std::out_of_range( "pos points outside of the NodeCollection" );
  }
  return first_ + idx;
}

inline bool NodeCollectionPrimitive::operator==( NodeCollectionPTR rhs ) const
{
  auto const* const rhs_ptr = dynamic_cast< NodeCollectionPrimitive const* >( rhs.get() );

  return first_ == rhs_ptr->first_ and last_ == rhs_ptr->last_ and model_id_ == rhs_ptr->model_id_
    and metadata_ == rhs_ptr->metadata_;
}

inline bool NodeCollectionPrimitive::operator==( const NodeCollectionPrimitive& rhs ) const
{
  return first_ == rhs.first_ and last_ == rhs.last_ and model_id_ == rhs.model_id_ and metadata_ == rhs.metadata_;
}

inline NodeCollectionPrimitive::const_iterator
NodeCollectionPrimitive::begin( NodeCollectionPTR cp ) const
{
  return const_iterator( cp, *this, 0 );
}

inline NodeCollectionPrimitive::const_iterator
NodeCollectionPrimitive::end( NodeCollectionPTR cp ) const
{
  return const_iterator( cp, *this, size() );
}

inline size_t
NodeCollectionPrimitive::size() const
{
  // empty NC has first_ == last_ == 0, need to handle that special
  return std::min( last_, last_ - first_ + 1 );
}

inline bool
NodeCollectionPrimitive::contains( index node_id ) const
{
  return first_ <= node_id and node_id <= last_;
}

inline void
NodeCollectionPrimitive::set_metadata( NodeCollectionMetadataPTR meta )
{
  metadata_ = meta;
}

inline NodeCollectionMetadataPTR
NodeCollectionPrimitive::get_metadata() const
{
  return metadata_;
}

inline bool
NodeCollectionPrimitive::is_range() const
{
  return true;
}

inline long
NodeCollectionPrimitive::find( const index neuron_id ) const
{
  if ( neuron_id > last_ )
  {
    return -1;
  }
  else
  {
    return neuron_id - first_;
  }
}

inline index NodeCollectionComposite::operator[]( const size_t i ) const
{
  if ( step_ > 1 or start_part_ > 0 or start_offset_ > 0 or stop_part_ != parts_.size() or stop_offset_ > 0 )
  {
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


inline bool NodeCollectionComposite::operator==( NodeCollectionPTR rhs ) const
{
  auto const* const rhs_ptr = dynamic_cast< NodeCollectionComposite const* >( rhs.get() );

  if ( size_ != rhs_ptr->size() || parts_.size() != rhs_ptr->parts_.size() )
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

inline NodeCollectionComposite::const_iterator
NodeCollectionComposite::begin( NodeCollectionPTR cp ) const
{
  return const_iterator( cp, *this, start_part_, start_offset_, step_ );
}

inline NodeCollectionComposite::const_iterator
NodeCollectionComposite::end( NodeCollectionPTR cp ) const
{
  if ( stop_part_ != 0 or stop_offset_ != 0 )
  {
    return const_iterator( cp, *this, stop_part_, stop_offset_, step_ );
  }
  else
  {
    return const_iterator( cp, *this, parts_.size(), 0 );
  }
}

inline size_t
NodeCollectionComposite::size() const
{
  return size_;
}

inline void
NodeCollectionComposite::set_metadata( NodeCollectionMetadataPTR meta )
{
  for ( auto& part : parts_ )
  {
    part.set_metadata( meta );
  }
}

inline NodeCollectionMetadataPTR
NodeCollectionComposite::get_metadata() const
{
  return parts_[ 0 ].get_metadata();
}

inline bool
NodeCollectionComposite::is_range() const
{
  return false;
}
} // namespace nest

#endif /* #ifndef NODE_COLLECTION_H */
