/*
 *  gid_collection.h
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

#ifndef GID_COLLECTION_H
#define GID_COLLECTION_H

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
class GIDCollection;
class GIDCollectionPrimitive;
class GIDCollectionComposite;
class GIDCollectionMetadata;

using GIDCollectionPTR = std::shared_ptr< GIDCollection >;
using GIDCollectionMetadataPTR = std::shared_ptr< GIDCollectionMetadata >;

/**
 * Class for Metadata attached to GIDCollection.
 *
 * NEST modules that want to add metadata to GIDCollections they
 * create need to implement their own concrete subclass.
 */
class GIDCollectionMetadata
{
public:
  GIDCollectionMetadata() = default;
  virtual ~GIDCollectionMetadata() = default;

  virtual void set_status( const DictionaryDatum&, bool ) = 0;
  virtual void get_status( DictionaryDatum& ) const = 0;

  virtual void set_first_gid( index ) = 0;
  virtual index get_first_gid() const = 0;
  virtual std::string get_type() const = 0;
};

class GIDTriple
{
public:
  index gid{ 0 };
  index model_id{ 0 };
  size_t lid{ 0 };
  GIDTriple() = default;
};

/**
 * Iterator for GIDCollections.
 *
 * This iterator can iterate over primitive and composite GIDCollections.
 * Behavior is determined by the constructor used to create the iterator.
 */
class gc_const_iterator
{
  friend class GIDCollectionPrimitive;
  friend class GIDCollectionComposite;

private:
  GIDCollectionPTR coll_ptr_; //!< holds pointer reference in safe iterators
  size_t element_idx_;        //!< index into (current) primitive gid collection
  size_t part_idx_;           //!< index into parts vector of composite collection
  size_t step_;               //!< step for skipping due to e.g. slicing

  /**
   * Pointer to primitive collection to iterate over.
   * Zero if iterator is for composite collection.
   */
  GIDCollectionPrimitive const* const primitive_collection_;

  /**
   * Pointer to composite collection to iterate over.
   * Zero if iterator is for primitive collection.
   */
  GIDCollectionComposite const* const composite_collection_;

  /**
   * Create safe iterator for GIDCollectionPrimitive.
   * @param collection_ptr smart pointer to collection to keep collection alive
   * @param collection  Collection to iterate over
   * @param offset  Index of collection element iterator points to
   * @param step    Step for skipping due to e.g. slicing
   */
  explicit gc_const_iterator( GIDCollectionPTR collection_ptr,
    const GIDCollectionPrimitive& collection,
    size_t offset,
    size_t step = 1 );

  /**
   * Create safe iterator for GIDCollectionComposite.
   * @param collection_ptr smart pointer to collection to keep collection alive
   * @param collection  Collection to iterate over
   * @param part    Index of part of collection iterator points to
   * @param offset  Index of element in GC part that iterator points to
   * @param step    Step for skipping due to e.g. slicing
   */
  explicit gc_const_iterator( GIDCollectionPTR collection_ptr,
    const GIDCollectionComposite& collection,
    size_t part,
    size_t offset,
    size_t step = 1 );

public:
  gc_const_iterator( const gc_const_iterator& gci ) = default;
  void get_current_part_offset( size_t&, size_t& );

  GIDTriple operator*() const;
  bool operator!=( const gc_const_iterator& rhs ) const;
  bool operator<( const gc_const_iterator& rhs ) const;
  bool operator<=( const gc_const_iterator& rhs ) const;

  gc_const_iterator& operator++();
  gc_const_iterator& operator+=( const size_t );
  gc_const_iterator operator+( const size_t ) const;

  void print_me( std::ostream& ) const;
};

/**
 * Superclass for GIDCollections.
 *
 * The superclass acts as an interface to the primitive and composite
 * GIDCollection types. It contains methods, mostly virtual, for the subclasses,
 * and also create()-methods to be interfaced externally.
 *
 * The superclass also contains handling of the fingerprint, a unique identity
 * the GIDCollection gets from the kernel on creation, which ensures that the
 * GIDCollection is not used after the kernel is reset.
 */
class GIDCollection
{
  friend class gc_const_iterator;

public:
  using const_iterator = gc_const_iterator;

  /**
   * Initializer gets current fingerprint from the kernel.
   */
  GIDCollection();

  virtual ~GIDCollection() = default;

  /**
   * Create a GIDCollection from a vector of GIDs. Results in a primitive if the
   * GIDs are homogeneous and contiguous, or a composite otherwise.
   *
   * @param gids Vector of GIDs from which to create the GIDCollection
   * @return a GIDCollection pointer to the created GIDCollection
   */
  static GIDCollectionPTR create( const IntVectorDatum& gids );

  /**
   * Create a GIDCollection from an array of GIDs. Results in a primitive if the
   * GIDs are homogeneous and contiguous, or a composite otherwise.
   *
   * @param gids Array of GIDs from which to create the GIDCollection
   * @return a GIDCollection pointer to the created GIDCollection
   */
  static GIDCollectionPTR create( const TokenArray& gids );

  /**
   * Check to see if the fingerprint of the GIDCollection matches that of the
   * kernel.
   *
   * @return true if the fingerprint matches that of the kernel, false otherwise
   */
  bool valid() const;

  /**
   * Print out the contents of the GIDCollection in a pretty and informative
   * way.
   */
  virtual void print_me( std::ostream& ) const = 0;

  /**
   * Get the GID in the specified index in the GIDCollection.
   *
   * @param idx Index in the GIDCollection
   * @return a GID
   */
  virtual index operator[]( size_t ) const = 0;

  /**
   * Join two GIDCollections. May return a primitive or composite, depending on
   * the input.
   *
   * @param rhs GIDCollection pointer to the GIDCollection to be added
   * @return a GIDCollection pointer
   */
  virtual GIDCollectionPTR operator+( GIDCollectionPTR ) const = 0;
  virtual bool operator==( GIDCollectionPTR ) const = 0;

  /**
   * Check if two GIDCollections are equal.
   *
   * @param rhs GIDCollection pointer to the GIDCollection to be checked against
   * @return true if they are equal, false otherwise
   */
  virtual bool operator!=( GIDCollectionPTR ) const;

  /**
   * Method to get an iterator representing the beginning of the GIDCollection.
   *
   * @return an iterator representing the beginning of the GIDCollection
   */
  virtual const_iterator begin( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const = 0;

  /**
   * Method to get an iterator representing the beginning of the GIDCollection.
   *
   * @return an iterator representing the beginning of the GIDCollection, in a
   * parallel context.
   */
  virtual const_iterator local_begin( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const = 0;

  /**
   * Method to get an iterator representing the beginning of the GIDCollection.
   *
   * @return an iterator representing the beginning of the GIDCollection, in an
   * MPI-parallel context.
   */
  virtual const_iterator MPI_local_begin( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const = 0;

  /**
   * Method to get an iterator representing the end of the GIDCollection.
   *
   * @param offset Index of element GC that iterator points to
   *
   * @return an iterator representing the end of the GIDCollection, taking
   * offset into account
   */
  virtual const_iterator end( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const = 0;

  /**
   * Method that creates an ArrayDatum filled with GIDs from the GIDCollection.
   *
   * @return an ArrayDatum containing GIDs
   */
  virtual ArrayDatum to_array() const = 0;

  /**
   * Get the size of the GIDCollection.
   *
   * @return number of GIDs in the GIDCollection
   */
  virtual size_t size() const = 0;

  /**
   * Check if the GIDCollection contains a specified GID
   *
   * @param gid GID to see if exists in the GIDCollection
   * @return true if the GIDCollection contains the GID, false otherwise
   */
  virtual bool contains( index gid ) const = 0;

  /**
   * Slices the GIDCollection to the boundaries, with an optional step
   * parameter. Note that the boundaries being specified are inclusive.
   *
   * @param start Index of the GIDCollection to start at
   * @param stop Index of the GIDCollection to stop at
   * @param step Number of places between GIDs to skip. Defaults to 1
   * @return a GIDCollection pointer to the new, sliced GIDCollection.
   */
  virtual GIDCollectionPTR slice( size_t start, size_t stop, size_t step ) const = 0;

  /**
   * Sets the metadata of the GIDCollection.
   *
   * @param meta A Metadata pointer
   */
  virtual void set_metadata( GIDCollectionMetadataPTR ) = 0;

  /**
   * Gets the metadata of the GIDCollection.
   *
   * @return A Metadata pointer
   */
  virtual GIDCollectionMetadataPTR get_metadata() const = 0;

  virtual bool is_range() const = 0;

  /**
   * Returns index of node with given GID in GIDCollection.
   *
   * @return Index of node with given GID; -1 if node not in GIDCollection.
   */
  virtual long find( const index ) const = 0;

private:
  unsigned long fingerprint_; //!< Unique identity of the kernel that created the //!< GIDCollection
  static GIDCollectionPTR create_();
  static GIDCollectionPTR create_( const std::vector< index >& );
};

/**
 * Subclass for the primitive GIDCollection type.
 *
 * The primitive type contains only homogeneous and contiguous GIDs. It also
 * contains model ID and metadata of the GIDs.
 */
class GIDCollectionPrimitive : public GIDCollection
{
  friend class gc_const_iterator;

private:
  index first_;                       //!< The first GID in the primitive
  index last_;                        //!< The last GID in the primitive
  index model_id_;                    //!< Model ID of the GIDs
  GIDCollectionMetadataPTR metadata_; //!< Pointer to the metadata of the GIDs

public:
  using const_iterator = gc_const_iterator;

  /**
   * Create a primitive from a range of GIDs, with provided model ID and
   * metadata pointer.
   *
   * @param first The first GID in the primitive
   * @param last  The last GID in the primitive
   * @param model_id Model ID of the GIDs
   * @param meta Metadata pointer of the GIDs
   */
  GIDCollectionPrimitive( index first, index last, index model_id, GIDCollectionMetadataPTR );

  /**
   * Create a primitive from a range of GIDs, with provided model ID.
   *
   * @param first The first GID in the primitive
   * @param last  The last GID in the primitive
   * @param model_id Model ID of the GIDs
   */
  GIDCollectionPrimitive( index first, index last, index model_id );

  /**
   * Create a primitive from a range of GIDs. The model ID has to be found by
   * the constructor.
   *
   * @param first The first GID in the primitive
   * @param last  The last GID in the primitive
   */
  GIDCollectionPrimitive( index first, index last );

  /**
   * Primitive copy constructor.
   *
   * @param rhs Primitive to copy
   */
  GIDCollectionPrimitive( const GIDCollectionPrimitive& );

  /**
   * Create empty GIDCollection.
   *
   * @note This is only for use by SPBuilder.
   */
  GIDCollectionPrimitive();

  void print_me( std::ostream& ) const override;
  void print_primitive( std::ostream& ) const;

  index operator[]( const size_t ) const override;
  GIDCollectionPTR operator+( GIDCollectionPTR rhs ) const override;
  bool operator==( const GIDCollectionPTR rhs ) const override;
  bool operator==( const GIDCollectionPrimitive& rhs ) const;

  const_iterator begin( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const override;
  const_iterator local_begin( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const override;
  const_iterator MPI_local_begin( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const override;
  const_iterator end( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const override;

  //! Returns an ArrayDatum filled with GIDs from the primitive.
  ArrayDatum to_array() const override;

  //! Returns total number of GIDs in the primitive.
  size_t size() const override;

  bool contains( index gid ) const override;
  GIDCollectionPTR slice( size_t start, size_t stop, size_t step = 1 ) const override;

  void set_metadata( GIDCollectionMetadataPTR ) override;

  GIDCollectionMetadataPTR get_metadata() const override;

  bool is_range() const override;

  long find( const index ) const override;

  /**
   * Checks if GIDs in another primitive is a continuation of GIDs in this
   * primitive.
   *
   * @param other Primitive to check for continuity
   * @return True if the first element in the other primitive is the next after
   * the last element in this primitive, and they both have the same model ID.
   * Otherwise false.
   */
  bool is_contiguous_ascending( GIDCollectionPrimitive& other );

  /**
   * Checks if GIDs of another primitive is overlapping GIDs of this primitive
   *
   * @param rhs Primitive to be checked.
   * @return True if the other primitive overlaps, false otherwise.
   */
  bool overlapping( const GIDCollectionPrimitive& rhs ) const;
};

GIDCollectionPTR operator+( GIDCollectionPTR lhs, GIDCollectionPTR rhs );

/**
 * Subclass for the composite GIDCollection type.
 *
 * The composite type contains a collection of primitives which are not
 * contiguous and homogeneous with each other. If the composite is sliced, it
 * also holds information about what index to start at and which to end at, and
 * the step.
 */
class GIDCollectionComposite : public GIDCollection
{
  friend class gc_const_iterator;

private:
  std::vector< GIDCollectionPrimitive > parts_; //!< Vector of primitives
  size_t size_;                                 //!< Total number of GIDs
  size_t step_;                                 //!< Step length, set when slicing.
  size_t start_part_;                           //!< Primitive to start at, set when slicing
  size_t start_offset_;                         //!< Element to start at, set when slicing
  size_t stop_part_;                            //!< Primitive to stop at, set when slicing
  size_t stop_offset_;                          //!< Element to stop at, set when slicing

  /**
   * Goes through the vector of primitives, merging as much as possible.
   *
   * @param parts Vector of primitives to be merged.
   */
  void merge_parts( std::vector< GIDCollectionPrimitive >& parts ) const;

public:
  /**
   * Create a composite from a primitive, with boundaries and step length.
   *
   * @param primitive Primitive to be converted
   * @param start Offset in the primitive to begin at.
   * @param stop Offset in the primtive to stop at.
   * @param step Length to step in the primitive.
   */
  GIDCollectionComposite( const GIDCollectionPrimitive&, size_t, size_t, size_t );

  /**
     * Composite copy constructor.
     *
     * @param comp Composite to be copied.
     */
  GIDCollectionComposite( const GIDCollectionComposite& );

  /**
     * Creates a new composite from another, with boundaries and step length.
     * This constructor is used only when slicing.
     *
     * @param composite Composite to slice.
     * @param start Index in the composite to begin at.
     * @param stop Index in the composite to stop at.
     * @param step Length to step in the composite.
     */
  GIDCollectionComposite( const GIDCollectionComposite&, size_t, size_t, size_t );

  /**
   * Create a composite from a vector of primitives.
   *
   * @param parts Vector of primitives.
   */
  GIDCollectionComposite( const std::vector< GIDCollectionPrimitive >& );

  void print_me( std::ostream& ) const override;

  index operator[]( const size_t ) const override;

  /**
   * Addition operator.
   *
   * Joins this composite with another GIDCollection. The resulting
   * GIDCollection is sorted and merged, and converted to a primitive if
   * possible.
   *
   * @param rhs GIDCollection to add to this composite
   * @return a GIDCollection pointer to either a primitive or a composite.
   */
  GIDCollectionPTR operator+( GIDCollectionPTR rhs ) const override;
  GIDCollectionPTR operator+( const GIDCollectionPrimitive& rhs ) const;
  bool operator==( const GIDCollectionPTR rhs ) const override;

  const_iterator begin( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const override;
  const_iterator local_begin( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const override;
  const_iterator MPI_local_begin( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const override;
  const_iterator end( GIDCollectionPTR = GIDCollectionPTR( nullptr ) ) const override;

  //! Returns an ArrayDatum filled with GIDs from the composite.
  ArrayDatum to_array() const override;

  //! Returns total number of GIDs in the composite.
  size_t size() const override;

  bool contains( index gid ) const override;
  GIDCollectionPTR slice( size_t start, size_t stop, size_t step = 1 ) const override;

  void set_metadata( GIDCollectionMetadataPTR ) override;

  GIDCollectionMetadataPTR get_metadata() const override;

  bool is_range() const override;

  long find( const index ) const override;
};

inline bool GIDCollection::operator!=( GIDCollectionPTR rhs ) const
{
  return not( *this == rhs );
}

inline void GIDCollection::set_metadata( GIDCollectionMetadataPTR )
{
  throw KernelException( "Cannot set Metadata on this type of GIDCollection." );
}

inline GIDTriple gc_const_iterator::operator*() const
{
  GIDTriple gt;
  if ( primitive_collection_ )
  {
    gt.gid = primitive_collection_->first_ + element_idx_;
    if ( gt.gid > primitive_collection_->last_ )
    {
      throw KernelException( "Invalid GIDCollection iterator (primitive element beyond last element)" );
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
        throw KernelException( "Invalid GIDCollection iterator (composite element beyond specified stop element)" );
      }
    }
    else if ( part_idx_ >= composite_collection_->parts_.size()
      or element_idx_ >= composite_collection_->parts_[ part_idx_ ].size() )
    {
      throw KernelException( "Invalid GIDCollection iterator (composite element beyond last composite element)" );
    }

    // Add to local placement from GIDCollectionPrimitives that comes before the
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

    gt.gid = composite_collection_->parts_[ part_idx_ ][ element_idx_ ];
    gt.model_id = composite_collection_->parts_[ part_idx_ ].model_id_;
    gt.lid += element_idx_;
  }
  return gt;
}

inline gc_const_iterator& gc_const_iterator::operator++()
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

inline gc_const_iterator& gc_const_iterator::operator+=( const size_t n )
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

inline gc_const_iterator gc_const_iterator::operator+( const size_t n ) const
{
  gc_const_iterator it = *this;
  return it += n;
}

inline bool gc_const_iterator::operator!=( const gc_const_iterator& rhs ) const
{
  return not( part_idx_ == rhs.part_idx_ and element_idx_ == rhs.element_idx_ );
}

inline bool gc_const_iterator::operator<( const gc_const_iterator& rhs ) const
{
  return ( part_idx_ < rhs.part_idx_ or ( part_idx_ == rhs.part_idx_ and element_idx_ < rhs.element_idx_ ) );
}

inline bool gc_const_iterator::operator<=( const gc_const_iterator& rhs ) const
{
  return ( part_idx_ < rhs.part_idx_ or ( part_idx_ == rhs.part_idx_ and element_idx_ <= rhs.element_idx_ ) );
}

inline void
gc_const_iterator::get_current_part_offset( size_t& part, size_t& offset )
{
  part = part_idx_;
  offset = element_idx_;
}

inline index GIDCollectionPrimitive::operator[]( const size_t idx ) const
{
  // throw exception if outside of GIDCollection
  if ( first_ + idx > last_ )
  {
    throw std::out_of_range( "pos points outside of the GIDCollection" );
  }
  return first_ + idx;
}

inline bool GIDCollectionPrimitive::operator==( GIDCollectionPTR rhs ) const
{
  auto const* const rhs_ptr = dynamic_cast< GIDCollectionPrimitive const* >( rhs.get() );

  return first_ == rhs_ptr->first_ and last_ == rhs_ptr->last_ and model_id_ == rhs_ptr->model_id_
    and metadata_ == rhs_ptr->metadata_;
}

inline bool GIDCollectionPrimitive::operator==( const GIDCollectionPrimitive& rhs ) const
{
  return first_ == rhs.first_ and last_ == rhs.last_ and model_id_ == rhs.model_id_ and metadata_ == rhs.metadata_;
}

inline GIDCollectionPrimitive::const_iterator
GIDCollectionPrimitive::begin( GIDCollectionPTR cp ) const
{
  return const_iterator( cp, *this, 0 );
}

inline GIDCollectionPrimitive::const_iterator
GIDCollectionPrimitive::end( GIDCollectionPTR cp ) const
{
  return const_iterator( cp, *this, size() );
}

inline size_t
GIDCollectionPrimitive::size() const
{
  // empty GC has first_ == last_ == 0, need to handle that special
  return std::min( last_, last_ - first_ + 1 );
}

inline bool
GIDCollectionPrimitive::contains( index gid ) const
{
  return first_ <= gid and gid <= last_;
}

inline void
GIDCollectionPrimitive::set_metadata( GIDCollectionMetadataPTR meta )
{
  metadata_ = meta;
}

inline GIDCollectionMetadataPTR
GIDCollectionPrimitive::get_metadata() const
{
  return metadata_;
}

inline bool
GIDCollectionPrimitive::is_range() const
{
  return true;
}

inline long
GIDCollectionPrimitive::find( const index neuron_id ) const
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

inline index GIDCollectionComposite::operator[]( const size_t i ) const
{
  if ( step_ > 1 or start_part_ > 0 or start_offset_ > 0 or stop_part_ != parts_.size() or stop_offset_ > 0 )
  {
    // Composite is sliced, we use iterator arithmetic.
    return ( *( begin() + i ) ).gid;
  }
  else
  {
    // Composite is unsliced, we can do a more efficient search.
    size_t tot_prev_gids = 0;
    for ( const auto& part : parts_ ) // iterate over GIDCollections
    {
      if ( tot_prev_gids + part.size() > i ) // is i in current GIDCollection?
      {
        size_t local_i = i - tot_prev_gids; // get local i
        return part[ local_i ];
      }
      else // i is not in current GIDCollection
      {
        tot_prev_gids += part.size();
      }
    }
    // throw exception if outside of GIDCollection
    throw std::out_of_range( "pos points outside of the GIDCollection" );
  }
}


inline bool GIDCollectionComposite::operator==( GIDCollectionPTR rhs ) const
{
  auto const* const rhs_ptr = dynamic_cast< GIDCollectionComposite const* >( rhs.get() );

  if ( size_ != rhs_ptr->size() || parts_.size() != rhs_ptr->parts_.size() )
  {
    return false;
  }
  auto rhs_gc = rhs_ptr->parts_.begin();
  for ( auto lhs_gc = parts_.begin(); lhs_gc != parts_.end(); ++lhs_gc, ++rhs_gc ) // iterate over GIDCollections
  {
    if ( not( ( *lhs_gc ) == ( *rhs_gc ) ) )
    {
      return false;
    }
  }
  return true;
}

inline GIDCollectionComposite::const_iterator
GIDCollectionComposite::begin( GIDCollectionPTR cp ) const
{
  return const_iterator( cp, *this, start_part_, start_offset_, step_ );
}

inline GIDCollectionComposite::const_iterator
GIDCollectionComposite::end( GIDCollectionPTR cp ) const
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
GIDCollectionComposite::size() const
{
  return size_;
}

inline void
GIDCollectionComposite::set_metadata( GIDCollectionMetadataPTR meta )
{
  for ( auto& part : parts_ )
  {
    part.set_metadata( meta );
  }
}

inline GIDCollectionMetadataPTR
GIDCollectionComposite::get_metadata() const
{
  return parts_[ 0 ].get_metadata();
}

inline bool
GIDCollectionComposite::is_range() const
{
  return false;
}
} // namespace nest

#endif /* #ifndef GID_COLLECTION_H */
