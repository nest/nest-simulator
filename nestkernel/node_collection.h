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
#include <memory>
#include <ostream>
#include <stdexcept> // out_of_range
#include <vector>

// Includes from libnestuil:
#include "lockptr.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"

// Includes from thirdparty:
#include "compose.hpp"

namespace nest
{
class Node;
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

  /**
   * Retrieve status information sliced according to slicing of node collection
   *
   * @note If nullptr is passed for NodeCollection*, full metadata irrespective of any slicing is returned.
   *  This is used by NodeCollectionMetadata::operator==() which does not have access to the NodeCollection.
   */
  virtual void get_status( DictionaryDatum&, NodeCollection const* ) const = 0;

  virtual void set_first_node_id( size_t ) = 0;
  virtual size_t get_first_node_id() const = 0;
  virtual std::string get_type() const = 0;

  virtual bool operator==( const NodeCollectionMetadataPTR ) const = 0;
};

/**
 * Represent single node entry in node collection.
 *
 * These triples are not actually stored in the node collection, they are constructed when an iterator is dereferenced.
 */
class NodeIDTriple
{
public:
  size_t node_id { 0 };  //!< Global ID of neuron
  size_t model_id { 0 }; //!< ID of neuron model
  size_t nc_index { 0 }; //!< position with node collection
  NodeIDTriple() = default;
};

/**
 * Iterator for NodeCollections.
 *
 * This iterator can iterate over primitive and composite NodeCollections.
 * Behavior is determined by the constructor used to create the iterator.
 *
 * @note In addition to a raw pointer to either a primitive or composite node collection, which is used
 * for all actual work, the iterator also holds a NodeCollectionPTR to the NC it iterates over. This is necessary
 * so that anonymous node collections at the SLI/Python level are not auto-destroyed when they go out
 * of scope while the iterator lives on.
 *
 * @note We decided not to implement iterators for primitive and composite node collections or for
 * stepping through rank- og thread-local elements using subclasses to avoid vtable lookups. Whether
 * this indeed gives best performance should be checked at some point.
 *
 * In the following discussion, we use the following terms:
 *
 * - **global iterator** is an iterator that steps through all elements of a node collection irrespective of the VP they
 * belong to
 * - **{rank,thread}-local iterator** is an iterator that steps only through those elements that belong to the
 * rank/thread on which the iterator was created
 * - **stride** is the user-given stride through a node collection as in ``nc[::stride]``
 * - **period** is 1 for global iterators and the number of ranks/threads for {rank/thread}-local iterators
 * - **phase** is the placement of a given node within the period, thus always 0 for global iterators;
 *   for composite node collections, the phase needs to be determined independently for each part
 * - **step** is the number of elements of the underlying primitive node collection to advance by to move to the next
 *   element; for global iterators, it is equal to the stride, for {rank/thread}-local iterators it is given by
 * lcm(stride, period) and applies only *within* each part of a composite node collection
 *
 * Note further that
 * - `first_`, `first_part_`, `first_elem_` and `last_`, `last_part_`, `last_elem_` refer to the
 *   first and last elements belonging to the node collection; in particular "last" is *inclusive*
 * - For primitive NCs, the end iterator is given by `part_idx_ == 0, element_idx_ = last_ + 1`
 * - For and empty primitive NC, the end iterator is given by `part_idx_ == 0, element_idx_ = 0`
 * - For composite NCs, the end iterator is given by `part_idx_ == last_part_, element_idx_ == last_elem_ + 1`
 * - To check whether an iterator over a composite NC is valid, use `NodeCollectionComposite::valid_idx_()`
 * - Composite NCs can never be empty
 * - The `end()` iterator is the same independent of whether one iterates globally or locally
 * - Constructing the `end()` iterator is costly, especially for composite NCs. In classic for loops including `... ; it
 * < nc->end() ; ...`, the `end()` iterator is constructed anew for every iteration, even if `nc` is `const` (tested
 * with AppleClang 15 and GCC 13, `-std=c++17`). Therefore, either construct the `end()` iterator first
 *
 *         const auto end_it = nc->end();
 *         for ( auto it = nc->thread_local_begin() ; it < end_it ; ++it )
 *
 *   or use range iteration (global iteration only, uses `!=` to compare iterators)
 *
 *         for ( const auto& nc_elem : nc )
 *
 * ## Iteration over primitive collections
 *
 * For ``NodeCollectionPrimitive``, creating a rank/thread-local ``begin()`` iterator and stepping is straightforward:
 *
 * 1. Since `stride == 1` by definition for a primitive NC, we have `step == period`
 * 2. Find the `phase` of the first element of the NC
 * 3. Use modular arithmetic to find the first element in the NC belonging to the current rank/thread, if it exists.
 * 4. To move forward by `n` elements, add `n * step` and check that one is still `<= last_`
 * 5. If one stepped past `last_`, set `element_idx_ = last_ + 1` to ensure that `it != nc->end()` compares correctly
 *
 *
 * ## Constructing a composite node collection
 *
 * Upon construction of a `NodeCollectionComposite` instance, we need to determine its first and last entries.
 * We can distinguish three different cases, mapping to three different constructors.
 *
 * ### Case A: Slicing a primitive collection
 *
 * If `nc` is a primivtive collection and we slice as `nc[start:end:stride]`, we only have a single part and
 *
 * - `first_part_ = 0, first_elem_ = start`
 * - `last_part_ = 0, last_elem_ = end - 1`
 * - `stride_ = stride`
 * - `size_ =  1 + ( end - start - 1 ) / stride` (integer division, see c'tor doc for derivation)
 *
 * ### Case B: Joining multiple primitive collections
 *
 * We receive a list of parts, which are all primitive collections. The new collection consists of all elements of all
 * parts.
 *
 * 1. Collect all non-empty parts into the vector `parts_`
 * 2. Ensure parts do not overlap and sort in order of increasing GIDs
 *
 * The collection now begins with the first element of the first part and ends with the last element of the last part,
 * i.e.,
 *
 * - `first_part_ = 0, first_elem_ = 0`
 * - `last_part_ = 0, last_elem_ = parts_[last_elem_].size() - 1`
 * - `stride_ = 1`
 * - `size_ = sum_k parts_[k].size()`
 *
 * ### Case C: Slicing a composite node collection
 *
 * Here, we have two subcases:
 *
 * #### Case C.1: Single element of sliced composite
 *
 * If `nc` is already sliced, we can only pick a single element given by `start` and `end==start+1`.  We proceed as
 * follows:
 *
 * 1. Build iterator pointing to element as `it = nc.begin() + start`
 * 2. Extract from this iterator
 *   - `first_part_ = it.part_idx_, first_elem_ = it.elem_idx_`
 * 3. We further have
 *   - `last_part_ = first_part_, last_elem_ = last_elem_`
 *   - `stride_ = 1` (the constructor is called with stride==1 if we do `nc[1]`)
 *   - `size_ = 1` (but computed using equation above for consistency with C.2)
 *
 * #### Case C.2: Slicing of non-sliced composite
 *
 * We slice as `nc[start:end:stride]` but are guaranteed that all elements in `parts_` are in `nc` and all parts have
 * stride 1. We thus proceed as follows:
 * 1. Build iterator pointing to first element as `first_it = nc.begin() + start`
 * 2. Build iterator pointing to last element as `last_it = nc.begin() + (end - 1)`
 * 2. Extract from these iterators
 *   - `first_part_ = first_it.part_idx_, first_elem_ = first_it.elem_idx_`
 *   - `last_part_ = last_it.part_idx_, last_elem_ = last_it.elem_idx_`
 * 3. We further have
 *   - `stride_ = stride` (irrelevant in this case, but set thus for consistency with C.2)
 *   - `size_ = 1 + ( end - start - 1 ) / stride`
 *
 * ### Additional data structures
 *
 * We further construct a vector of the cumulative sizes of all parts and a vector containing for each part the
 * part-local index to the first element of each part that belongs to the node collection, or `invalid_index` if there
 * is no element in the part.
 * **Note:** The cumulative sizes include *all* elements of the parts, including elements before `first_elem_` or after
 * `last_elem_`, but they do *not* include elements in parts before `first_part_` or after `last_part_`.
 *
 * #### Case A
 *
 * - `cumul_abs_size_` has a single element equal to the size of the underlying primitive collection.
 * - `first_in_part_` has a single element equal to `first_elem_`
 *
 * #### Case B
 *
 * - `cumul_abs_size_` are straightforward cumulative sums of the sizes, beginning with `parts_[0].size()`
 * - `first_in_part_` has `parts_.size()` elements, all zero since `stride==1` so we start from the beginning of each
 * part
 *
 * #### Case C.1
 *
 * - `cumul_abs_size_`: zero before `first_part_`, for all subsequent parts `parts_[first_part_].size()`
 * - `first_in_part_`: for `first_part_`, it is `first_elem_`, for all others `invalid_index`
 *
 * #### Case C.2
 *
 * - `cumul_abs_size_`:
 *   - zero before `first_part_`
 *   - cumulative sums from `first_part_` on starting with `parts_[first_part_].size()`
 *   - from `last_part_` on all `cumul_abs_size_[last_part_]`
 * - `first_in_part_`:
 *   - for `first_part_`, it is `first_elem_`
 *   - for all subsequent parts `part_idx_ <= last_part_`:
 *     1. Compute number of elements in previous parts, taking stride into account (same equation as for composite size)
 *       `n_pe = 1 + ( cumul_abs_size_[ part_idx_ - 1 ] - 1 - first_elem_ ) / stride`
 *     2. Compute absolute index of next element from beginning of first_part_
 *       `next_abs_idx = first_elem_ + n_pe * stride_`
 *     3. Compute part-local index
 *       `next_loc_idx = next_abs_idx - cumul_abs_size_[ part_idx_ - 1 ]`
 *     4. If `next_loc_idx_` is valid index, store as `first_in_part_[part_idx_]`, otherwise store `invalid_index`
 *   - `invalid_index` for all parts before `first_part_` and after `last_part_`
 *
 *
 * ## Iteration over composite collections
 *
 *  - All underlying parts are primitive node collections and thus have `stride == 1`. One cannot join node collections
 * with different strides.
 * - Thus, if a composite NC is sliced, the same `stride` applies to all parts; by definition, also the same `period`
 * applies to all parts
 * - Therefore, the `step = lcm(stride, period)` is the same throughout
 * - When slicing a compositve node collection, we always retain the full set of parts and mark by `first_part_` and
 * `last_part_` (inclusive) which parts are relevant for the sliced collection.
 *
 * We now need to distinguish between global and local iteration.
 *
 * ### Global iteration
 *
 * For global iteration, we can ignore phase relations. We need to take into account slicing and possible gaps between
 * parts, as well as the possibility, for `stride > 1`, that parts contain no elements. Consider the following node
 * collection with several parts; vertical bars indicate borders between parts ( to construct such a node collection,
 * join collections with different neuron models). In the table, (PartIdx, ElemIdx) show the iterator values for
 * iterators pointing to the corresponding element in the collection, while PythonIdx is the index that applies for
 * slicing from the Python level. Different slices are shown in the final lines of the table, with asterisks marking the
 * elements belonging to the sliced collection. Note that the second slice does not contain any elements from the first
 * and third parts.
 *
 *      GID        1 2 | 3 4 5 | 6 7 8 | 9 10 11
 *      PartIdx    0 0 | 1 1 1 | 2 2 2 | 3  3  3
 *      ElemIdx    0 1 | 0 1 2 | 0 1 2 | 0  1  2
 *      ----------------------------------------
 *      PythonIdx  0 1 | 2 3 4 | 5 6 7 | 8  9 10
 *      ----------------------------------------
 *      [::3]      *   |   *   |   *   |    *
 *      [4::5]         |     * |       |    *
 *      [1:11:3]     * |     * |     * |
 *
 * #### Iterator initialization
 *
 * The `begin()` iterator is given by
 * - `part_idx_ = nc.first_part_, element_idx_ = nc.first_elem_`
 * - `step_ = nc.stride_`
 * - `kind_ = NCIterator::GLOBAL`
 *
 * #### Iterator stepping
 * - Assume we want to move `n` elements forward. In the `[::3]` example above, starting with `begin()` and stepping by
 *   `n=2` elements forward would take us from the element with GID 1 to the element with GID 7.
 * - Proceed like this
 *   1. Compute candidate element index `new_idx = element_idx_ + n * step_`
 *   2. If we have passed the end of the collection, we set `part_idx_, element_idx_` to the end-iterator values and
 * return
 *   3. Otherwise, if we are still in the current part, we set `element_idx_ = new_idx` and return
 *   4. Otherwise, we need to look in the next part.
 *   5. We first check if there is a next part, if not, we set to end()
 *   6. Otherwise, we move through remaining parts and use `cumul_abs_size_` to check if we have reached a part
 *     containing `new_idx`. If so, we also need to check if we ended up in `last_part_` and if so, if before
 * `last_elem_`.
 *   7. If we have found a valid element in a new part, we set `part_idx_, element_idx_`, otherwise, we set them to the
 * end() iterator.
 *
 * ### Local iteration
 *
 * Rank-local and thread-local iteration work in exactly the same way, just that the period and phase are in one case
 * given by the ranks and in the other by the threads. Thus, we discuss the algorithms only once.
 *
 * - For `period > 1`, the relation of `phase` to position in a given part can differ from part to part. Consider the
 * following node collection in a simulation with four threads (one MPI process). The table shows the GID of the neuron,
 * its thread (phase), the part_idx and the element_idx of a iterator pointing to the element. Finally, elements that
 * belong to thread 1 and 2 respectively, are marked with an asterisk (only 11 elements shown for brevity):
 *
 *         GID       1  2  3  4  5  6  7  8  9  10  11
 *         PartIdx   0  0  0  0  0  0  0  0  0   0   0
 *         ElemIdx   0  1  2  3  4  5  6  7  8   9  10
 *         Phase     1  2  3  0  1  2  3  0  1   2   3
 *         -------------------------------------------
 *         On thr 1  *           *           *
 *         On thr 2     *           *            *
 *
 *     The phase relation here is `phase == element_idx % period + 1`, where `1` is the phase of the node with GID 1.
 *
 *     Now consider a new node collection constructed by
 *
 *         nc2 = nc[:5] + nc[7:]
 *
 *     This yields a node collection with two parts, marked with the vertical line (note that nodes 6 and 7 are not
 * included). We consider it once it its entirety and once sliced as `nc2[::3]`. The `1stInPt` line marks the elements
 * indexed by the `first_in_part_` vector.
 *
 *         GID            1  2  3  4  5  |  8  9  10  11  12  13  14  15  16  17  18  19  20  21
 *         PartIdx        0  0  0  0  0  |  1  1   1   1   1   1   1   1   1   1   1   1   1   1
 *         ElemIdx        0  1  2  3  4  |  0  1   2   3   4   5   6   7   8   9  10  11  12  13
 *         Phase          1  2  3  0  1  |  0  1   2   3   0   1   2   3   0   1   2   3   0   1
 *         -------------------------------------------------------------------------------------
 *         All 1stInPt    #              |  #
 *         All thr 0               *     |  *              *               *               *
 *         All thr 1      *           *  |     *               *               *               *
 *         All thr 2         *           |         *               *               *
 *         All thr 3            *        |             *               *               *
 *         -------------------------------------------------------------------------------------
 *         [::3] 1stInPt  #              |     #
 *         [::3] thr 0             *     |                 *
 *         [::3] thr 1    *              |     *                                               *
 *         [::3] thr 2                   |                                         *
 *         [::3] thr 3                   |                             *
 *
 *     The phase relation is the same as above for the first part, but for the second part, the phase relation is
 *     `phase == element_idx % period + 0`, where `0` is the phase of the node with GID 8, the first node in the second
 * part.
 *
 *
 * #### Local iterator initialization for primitive collection
 *
 * 1. Obtain the `first_phase_`, i.e., the phase (rank/thread) of the `first_` element in the collection.
 * 2. Let `proc_phase_` be the phase of the rank or thread constructing the iterator (i.e, the number of the rank or
 * thread)
 * 3. Then set
 * - `part_idx_ = 0, element_idx_ = ( proc_phase_ - first_phase_ + period ) % period`
 * - `step_ = period`
 * - `kind_ = NCIterator::{RANK,THREAD}_LOCAL`
 *
 * #### Local iterator initialization for composite collection
 *
 * Here, we may need to move through several parts to find the first valid entry. We proceed as follows:
 *
 * 1. Set `part_idx = nc.first_part_, elem_idx = nc.first_elem_`
 * 2. With `elem_idx` as starting point, find index of first element in part `part_idx` that belongs to the current
 * rank/thread. This is done by `first_index()` provided by `libnestutil/numerics.h`, see documentation of algorithm
 * there. This step is the "phase adjustment" mentioned above.
 * 3. If Step 2 did not return a valid index, increase `part_idx` until we have found a part containing a valid entry,
 * i.e., one with `nc.first_in_part_[part_idx] != invalid_index` or until we have exhausted the collection
 * 4. If we have exhausted the collection, set iterator to `end()`, otherwise set `elem_idx =
 * nc.first_in_part_[part_idx]` with Step 2.
 *
 * We further have
 * - `step_ = lcm(nc.stride_, period)`
 * - `kind_ = NCIterator::{RANK, THREAD}_LOCAL`
 *
 * #### Stepping a local iterator
 *
 * - Assume we want to move `n` elements forward. In the `[::3]` example with four threads above,
 *   starting with `thread_local_begin()` on thread 0 abd stepping `n=1` element forward
 *   would take us from the element with GID 4 to the element with GID 16.
 * - Proceed like this
 *   1. Compute candidate element index `new_idx = element_idx_ + n * step_`
 *   2. If `new_idx` is in the current part, we set `element_idx_ = new_idx` and are done.
 *   3. Otherwise, if there are no more parts, set to the end iterator
 *   4. If more parts are available, we need to unroll the step by `n` into `n` steps of size 1, because otherwise
 *     we cannot handle the phase  adjustment on transition into new parts correctly.
 *   5. For each individual step we do the following
 *     a. Advance `part_idx_` until we have a valid `nc.first_in_part_[part_idx_]` or no more parts
 *     b. If there are no more parts, set to end iterator and return
 *     c. Otherwise, set `element_idx_ = nc.first_in_part_[part_idx_]` and then find the first element
 *       after that local the current thread/rank using the same algorithm as for the local iterator intialization
 *   6. Set to the end() iterator if we did not find a valid solution.
 */
class nc_const_iterator
{
  friend class NodeCollectionPrimitive;
  friend class NodeCollectionComposite;

public:
  //! Markers for kind of iterator, required by `composite_update_indices_()`.
  enum class NCIteratorKind
  {
    GLOBAL,       //!< iterate over all elements of node collection
    RANK_LOCAL,   //!< iterate only over elements on owning rank
    THREAD_LOCAL, //!< iterate only over elements on owning thread
    END           //!< end iterator, never increase
  };

private:
  NodeCollectionPTR coll_ptr_; //!< pointer to keep node collection alive, see note
  size_t element_idx_;         //!< index into (current) primitive node collection
  size_t part_idx_;            //!< index into parts vector of composite collection
  size_t step_;                //!< internal step also accounting for stepping over rank/thread
  const NCIteratorKind kind_;  //!< whether to iterate over all elements or rank/thread specific
  const size_t rank_or_vp_;    //!< rank or vp iterator is bound to

  //! Pointer to primitive collection to iterate over.  Zero if iterator is for composite collection.
  NodeCollectionPrimitive const* const primitive_collection_;

  //! Pointer to composite collection to iterate over. Zero if iterator is for primitive collection.
  NodeCollectionComposite const* const composite_collection_;

  /**
   * Create safe iterator for NodeCollectionPrimitive.
   *
   * @param collection_ptr smart pointer to collection to keep collection alive
   * @param collection  Collection to iterate over
   * @param offset  Index of collection element iterator points to
   * @param stride    Step for skipping due to e.g. slicing; does NOT include stepping over rank/thread
   */
  explicit nc_const_iterator( NodeCollectionPTR collection_ptr,
    const NodeCollectionPrimitive& collection,
    size_t offset,
    size_t stride,
    NCIteratorKind kind = NCIteratorKind::GLOBAL );

  /**
   * Create safe iterator for NodeCollectionComposite.
   *
   * @param collection_ptr smart pointer to collection to keep collection alive
   * @param collection  Collection to iterate over
   * @param part    Index of part of collection iterator points to
   * @param offset  Index of element in NC part that iterator points to
   * @param stride    Step for skipping due to e.g. slicing; does NOT include stepping over rank/thread
   */
  explicit nc_const_iterator( NodeCollectionPTR collection_ptr,
    const NodeCollectionComposite& collection,
    size_t part,
    size_t offset,
    size_t stride,
    NCIteratorKind kind = NCIteratorKind::GLOBAL );

  /**
   * Return element_idx_ for next element if within part. Returns current element_idx_ otherwise.
   */
  size_t find_next_within_part_( size_t n ) const;

  /**
   * Advance composite GLOBAL iterator by n elements, taking stride into account.
   */
  void advance_global_iter_to_new_part_( size_t n );

  /**
   * Advance composite {THREAD,RANK}_LOCAL iterator by n elements, taking stride into account.
   */
  void advance_local_iter_to_new_part_( size_t n );

public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = long;
  using value_type = NodeIDTriple;
  using pointer = NodeIDTriple*;
  using reference = NodeIDTriple&;

  nc_const_iterator( const nc_const_iterator& nci ) = default;
  std::pair< size_t, size_t > get_part_offset() const;

  NodeIDTriple operator*() const;
  bool operator==( const nc_const_iterator& rhs ) const;
  bool operator!=( const nc_const_iterator& rhs ) const;
  bool operator<( const nc_const_iterator& rhs ) const;
  bool operator<=( const nc_const_iterator& rhs ) const;
  bool operator>( const nc_const_iterator& rhs ) const;
  bool operator>=( const nc_const_iterator& rhs ) const;

  nc_const_iterator& operator++();
  nc_const_iterator operator++( int ); // postfix
  nc_const_iterator& operator+=( const size_t );
  nc_const_iterator operator+( const size_t ) const;

  /**
   * Return step size of iterator.
   *
   * For thread- and rank-local iterators, this takes into account stepping over all VPs / ranks.
   * For stepped node collections, this takes also stepping into account. Thus if we have a
   * thread-local iterator in a simulation with 4 VPs and a node-collection step of 3, then the
   * iterator's step is 12.
   */
  size_t get_step_size() const;

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
 *
 * There are two types of NodeCollections
 *
 *  - **Primitive NCs** contain a contiguous range of GIDs of the same neuron model and always have stride 1.
 *
 *    - Slicing a primitive node collection in the form ``nc[j:k]`` returns a new primitive node collection,
 *      *except* when ``nc`` has (spatial) metadata, in which case a composite node collection is returned.
 *      The reason for this is that we otherwise would have to create a copy of the position information.
 *
 *  - **Composite NCs** can contain
 *
 *    - A single primitive node collection with metadata created by ``nc[j:k]`` slicing; the composite NC
 *      then represents a view on the primitive node collection with window ``j:k``.
 *    - Any sequence of primitive node collections with the same or different neuron types; if the node collections
 *      contain metadata, all must contain the same metadata and all parts of the composite are separate views
 *    - A striding slice over a NC in the form ``nc[j:k:s]``, where ``j`` and ``k`` are optional. Here,
 *      ``nc`` must have stride 1. If ``nc`` has metadata, it must be a primitive node collection.
 *
 *  For any node collection, three types of iterators can be obtained by different ``begin()`` methods:
 *
 *   - ``begin()`` returns an iterator iterating over all elements of the node collection
 *   - ``rank_local_begin()`` returns an iterator iterating over the elements of the node collection which are local to
 * the rank on which it is called
 *   - ``thread_local_begin()`` returns an iterator iterating over the elements of the node collection which are local
 * to the thread (ie VP) on which it is called
 *
 *  There is only a single type of end iterator returned by ``end()``.
 *
 *  For more information on composite node collections, see the documentation for the derived class and
 * `nc_const_iterator`.
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
   * Create a NodeCollection from a vector of node IDs.
   *
   * Results in a primitive if the
   * node IDs are homogeneous and contiguous, or a composite otherwise.
   *
   * @param node_ids Vector of node IDs from which to create the NodeCollection
   * @return a NodeCollection pointer to the created NodeCollection
   */
  static NodeCollectionPTR create( const IntVectorDatum& node_ids );

  /**
   * Create a NodeCollection from an array of node IDs.
   *
   * Results in a primitive if the node IDs are homogeneous and
   * contiguous, or a composite otherwise.
   *
   * @param node_ids Array of node IDs from which to create the NodeCollection
   * @return a NodeCollection pointer to the created NodeCollection
   */
  static NodeCollectionPTR create( const TokenArray& node_ids );

  /**
   * Create a NodeCollection from a single node ID.
   *
   * Results in a primitive unconditionally.
   *
   * @param node_id Node ID from which to create the NodeCollection
   * @return a NodeCollection pointer to the created NodeCollection
   */
  static NodeCollectionPTR create( const size_t node_id );

  /**
   * Create a NodeCollection from a single node pointer.
   *
   * Results in a primitive unconditionally.
   *
   * @param node Node pointer from which to create the NodeCollection
   * @return a NodeCollection pointer to the created NodeCollection
   */
  static NodeCollectionPTR create( const Node* node );

  /**
   * Create a NodeCollection from an array of node IDs.
   *
   * Results in a primitive if the node IDs are homogeneous and
   * contiguous, or a composite otherwise.
   *
   * @param node_ids Array of node IDs from which to create the NodeCollection
   * @return a NodeCollection pointer to the created NodeCollection
   */
  static NodeCollectionPTR create( const std::vector< size_t >& node_ids );

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
  virtual size_t operator[]( size_t ) const = 0;

  /**
   * Join two NodeCollections.
   *
   * May return a primitive or composite, depending on
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
   * Return iterator stepping from first node on the thread it is called on over nodes on that thread.
   *
   * @return an iterator representing the beginning of the NodeCollection, in a
   * parallel context.
   */
  virtual const_iterator thread_local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const = 0;

  /**
   * Method to get an iterator representing the beginning of the NodeCollection.
   *
   * @return an iterator representing the beginning of the NodeCollection, in an
   * MPI-parallel context.
   */
  virtual const_iterator rank_local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const = 0;

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
   * Method that creates an ArrayDatum filled with node IDs from the NodeCollection; for debugging
   *
   * @param selection is "all", "rank" or "thread"
   *
   * @return an ArrayDatum containing node IDs ; if thread, separate thread sections by "0 thread# 0"
   */
  ArrayDatum to_array( const std::string& selection ) const;

  /**
   * Get the size of the NodeCollection.
   *
   * @return number of node IDs in the NodeCollection
   */
  virtual size_t size() const = 0;

  /**
   * Get the step of the NodeCollection.
   *
   * @return Stride between node IDs in the NodeCollection
   */
  virtual size_t stride() const = 0;

  /**
   * Check if the NodeCollection contains a specified node ID
   *
   * @param node_id node ID to see if exists in the NodeCollection
   * @return true if the NodeCollection contains the node ID, false otherwise
   */
  virtual bool contains( const size_t node_id ) const = 0;

  /**
   * Slices the NodeCollection to the boundaries, with an optional step
   * parameter.
   *
   * Note that the boundaries being specified are inclusive.
   *
   * @param start Index of the NodeCollection to start at
   * @param end One past the index of the NodeCollection to stop at
   * @param stride Number of places between node IDs to skip. Defaults to 1
   * @return a NodeCollection pointer to the new, sliced NodeCollection.
   */
  virtual NodeCollectionPTR slice( size_t start, size_t end, size_t stride ) const = 0;

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
   * Checks if the NodeCollection has no elements.
   *
   * @return true if the NodeCollection is empty, false otherwise
   */
  virtual bool empty() const = 0;

  /**
   * Returns index of node with given node ID in NodeCollection.
   *
   * Index here is into the sliced node collection, so that nc[ nc.get_nc_index( gid )].node_id == gid.
   *
   * @return Index of node with given node ID; -1 if node not in NodeCollection.
   */
  virtual long get_nc_index( const size_t ) const = 0;

  /**
   * Returns whether the NodeCollection contains any nodes with proxies or not.
   *
   * @return true if any nodes in the NodeCollection has proxies, false otherwise.
   */
  virtual bool has_proxies() const = 0;

  /**
   * Collect metadata into dictionary.
   */
  void get_metadata_status( DictionaryDatum& ) const;

  /**
   * return the first stored ID (i.e, ID at index zero) inside the NodeCollection
   */
  size_t get_first() const;

  /**
   * return the last stored ID inside the NodeCollection
   */
  size_t get_last() const;


private:
  unsigned long fingerprint_; //!< Unique identity of the kernel that created the NodeCollection
  static NodeCollectionPTR create_();
  static NodeCollectionPTR create_( const std::vector< size_t >& );
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
  // Even though all members are logically const, we cannot declare them const because
  // sorting or merging the parts_ array requires assignment.
  size_t first_;                       //!< The first node ID in the primitive
  size_t last_;                        //!< The last node ID in the primitive
  size_t model_id_;                    //!< Model ID of the node IDs
  NodeCollectionMetadataPTR metadata_; //!< Pointer to the metadata of the node IDs
  bool nodes_have_no_proxies_;         //!< Whether the primitive contains devices or not

  /**
   * Raise an error if the model IDs of all nodes in the primitive are not the same as the expected model id.
   *
   * @note  For use in the constructor only.
   *
   * @param model_id Expected model id.
   */
  void assert_consistent_model_ids_( const size_t ) const;

public:
  /**
   * Create a primitive from a range of node IDs, with provided model ID and
   * metadata pointer.
   *
   * @param first The first node ID in the primitive
   * @param last  The last node ID in the primitive
   * @param model_id Model ID of the node IDs
   * @param meta Metadata pointer of the node IDs
   */
  NodeCollectionPrimitive( size_t first, size_t last, size_t model_id, NodeCollectionMetadataPTR );

  /**
   * Create a primitive from a range of node IDs, with provided model ID.
   *
   * @param first The first node ID in the primitive
   * @param last  The last node ID in the primitive
   * @param model_id Model ID of the node IDs
   */
  NodeCollectionPrimitive( size_t first, size_t last, size_t model_id );

  /**
   * Create a primitive from a range of node IDs. The model ID has to be found by
   * the constructor.
   *
   * @param first The first node ID in the primitive
   * @param last  The last node ID in the primitive
   */
  NodeCollectionPrimitive( size_t first, size_t last );

  /**
   * Primitive copy constructor.
   *
   * @param rhs Primitive to copy
   */
  NodeCollectionPrimitive( const NodeCollectionPrimitive& ) = default;

  /**
   * Primitive assignment operator.
   *
   * @param rhs Primitive to assign
   */
  NodeCollectionPrimitive& operator=( const NodeCollectionPrimitive& ) = default;

  /**
   * Create empty NodeCollection.
   *
   * @note This is only for use by SPBuilder.
   */
  NodeCollectionPrimitive();

  void print_me( std::ostream& ) const override;
  void print_primitive( std::ostream& ) const;

  size_t operator[]( const size_t ) const override;
  NodeCollectionPTR operator+( NodeCollectionPTR rhs ) const override;
  bool operator==( const NodeCollectionPTR rhs ) const override;
  bool operator==( const NodeCollectionPrimitive& rhs ) const;

  const_iterator begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator thread_local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator rank_local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator end( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;

  //! Returns total number of node IDs in the primitive.
  size_t size() const override;

  //! Returns the stride between node IDs in the primitive (always 1).
  size_t stride() const override;

  bool contains( const size_t node_id ) const override;
  NodeCollectionPTR slice( size_t start, size_t end, size_t stride = 1 ) const override;

  void set_metadata( NodeCollectionMetadataPTR ) override;

  NodeCollectionMetadataPTR get_metadata() const override;

  bool is_range() const override;
  bool empty() const override;

  long get_nc_index( const size_t ) const override;

  bool has_proxies() const override;

  /**
   * Checks if node IDs in another primitive is a continuation of node IDs in this
   * primitive.
   *
   * @param other Primitive to check for continuity
   * @return True if the first element in the other primitive is the next after
   * the last element in this primitive, and they both have the same model ID.
   * Otherwise false.
   */
  bool is_contiguous_ascending( const NodeCollectionPrimitive& other ) const;

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
 * also holds information about what index to start at, one past the index to end at, and
 * the step. The endpoint is one past the last valid node.
 *
 * @note To avoid creating copies of Primitives (not sure that saves much), Composite keeps
 * primitives as they are. These are called parts. It then sets markers
 *
 * - ``first_part_``, ``first_elem_`` to the first node belonging to the slice
 * - ``last_part_``, ``last_elem_`` to the last node belongig to the slice
 *
 * @note
 * - Any part after ``first_part_`` but before ``last_part_`` will always be in the NC in its entirety.
 * - A composite node collection is never empty (in that case it would be replaced with a Primitive. Therefore,
 *   there is always at least one part with one element.
 */
class NodeCollectionComposite : public NodeCollection
{
  friend class nc_const_iterator;

private:
  std::vector< NodeCollectionPrimitive > parts_; //!< Primitives forming composite
  size_t size_;                                  //!< Total number of node IDs, takes into account slicing
  size_t stride_;                                //!< Step length, set when slicing.
  size_t first_part_;                            //!< Primitive to start at, set when slicing
  size_t first_elem_;                            //!< Element to start at, set when slicing
  size_t last_part_;                             //!< Last entry of parts_ belonging to sliced NC
  size_t last_elem_;                             //!< Last entry of parts_[last_part_] belonging to sliced NC
  bool is_sliced_;                               //!< Whether the NodeCollectionComposite is sliced
  std::vector< size_t > cumul_abs_size_;         //!< Cumulative size of parts
  std::vector< size_t >
    first_in_part_; //!< Local index to first element in each part when slicing is taken into account, or invalid_index

  /**
   * Goes through the vector of primitives, merging as much as possible.
   *
   * @param parts Vector of primitives to be merged.
   */
  void merge_parts_( std::vector< NodeCollectionPrimitive >& parts ) const;

  //! Type for lambda-helper function used by {rank, thread, specific}_local_begin
  typedef size_t ( *gid_to_phase_fcn_ )( size_t );

  /**
   * Abstraction of {rank, thread}_local_begin.
   *
   * @param period  number of ranks or virtual processes
   * @param phase calling rank or virtual process
   * @param start_part begin seach in this part of the collection
   * @param start_offset begin search from this offset in start_part
   * @param period_first_node  function converting gid to rank or thread
   * @returns { part_index, part_offset }  — values are `invalid_index` if no solution found
   */
  std::pair< size_t, size_t > specific_local_begin_( size_t period,
    size_t phase,
    size_t start_part,
    size_t start_offset,
    gid_to_phase_fcn_ period_first_node ) const;

  /**
   * Return true if part_idx/element_idx pair indicates element of collection
   */
  bool valid_idx_( const size_t part_idx, const size_t element_idx ) const;

  /**
   * Find next part and offset in it after moving beyond previous part, based on stride.
   *
   * @param part_idx Part for current iterator position
   * @param element_idx Element for current iterator position
   * @param n Number of node collection elements we advance by (ie argument that was passed to to `operator+(n)`)
   *
   * @return New part-offset tuple pointing into new part, or invalid_index tuple.
   */
  std::pair< size_t, size_t > find_next_part_( size_t part_idx, size_t element_idx, size_t n = 1 ) const;

  //! helper for thread_local_begin/compsite_update_indices
  static size_t gid_to_vp_( size_t gid );

  //! helper for rank_local_begin/compsite_update_indices
  static size_t gid_to_rank_( size_t gid );


public:
  /**
   * Create a composite from a primitive, with boundaries and step length.
   *
   * Let the slicing be given by b:e:s for brevity. Then the elements of the sliced composite will be given by
   *
   * b, b + s, ..., b + j s < e  <=>   b, b + s, ..., b + j s ≤ e - 1  <=>  j ≤ floor( ( e - 1 - b ) / s )
   *
   * Since j = 0 is included in the sequence above, the sliced node collection has 1 + floor( ( e - 1 - b ) / s )
   * elements. Flooring is implemented via integer division.
   *
   * @param primitive Primitive to be converted
   * @param start Offset in the primitive to begin at.
   * @param end Offset in the primitive, one past the node to end at.
   * @param step Length to step in the primitive.
   */
  NodeCollectionComposite( const NodeCollectionPrimitive&, size_t, size_t, size_t );

  /**
   * Creates a new composite from another, with boundaries and step length.
   * This constructor is used only when slicing.
   *
   * Since we do not allow slicing of sliced node collections with step > 1, the underlying node collections all
   * have step one and we can calculate the size of the sliced node collection as described in the constructor
   * taking a NodeCollectionPrimitive as argument.
   *
   * @param composite Composite to slice.
   * @param start Index in the composite to begin at.
   * @param end Index in the composite one past the node to end at.
   * @param step Length to step in the composite.
   */
  NodeCollectionComposite( const NodeCollectionComposite&, size_t, size_t, size_t );

  /**
   * Create a composite from a vector of primitives.
   *
   * Since primitives by definition contain contiguous elements, the size of the composite collection is the
   * sum of the size of its parts.
   *
   * @param parts Vector of primitives.
   */
  explicit NodeCollectionComposite( const std::vector< NodeCollectionPrimitive >& );

  /**
   * Composite copy constructor.
   *
   * @param comp Composite to be copied.
   */
  NodeCollectionComposite( const NodeCollectionComposite& ) = default;

  void print_me( std::ostream& ) const override;

  size_t operator[]( const size_t ) const override;

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
  const_iterator thread_local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator rank_local_begin( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;
  const_iterator end( NodeCollectionPTR = NodeCollectionPTR( nullptr ) ) const override;

  //! Returns total number of node IDs in the composite.
  size_t size() const override;

  //! Returns the stride between node IDs in the composite.
  size_t stride() const override;

  bool contains( const size_t node_id ) const override;
  NodeCollectionPTR slice( size_t start, size_t end, size_t step = 1 ) const override;

  void set_metadata( NodeCollectionMetadataPTR ) override;

  NodeCollectionMetadataPTR get_metadata() const override;

  bool is_range() const override;
  bool empty() const override;

  long get_nc_index( const size_t ) const override;

  bool has_proxies() const override;
};

} // namespace nest

#endif /* #ifndef NODE_COLLECTION_H */
