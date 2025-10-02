/*
 *  sparse_node_array.h
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

#ifndef SPARSE_NODE_ARRAY_H
#define SPARSE_NODE_ARRAY_H

// C++ includes:
#include <cassert>
#include <map>

// Includes from nestkernel:
#include "nest_types.h"

// Includes from libnestutil
#include "block_vector.h"


namespace nest
{
class Node;

/**
 * Sparse representation of local nodes.
 *
 * SparseNodeArray maps node IDs of thread-local nodes to Node*. It provides
 * a const iterator interface for iteration over all local nodes, lookup by
 * node ID and lookup by numeric index into local nodes. The latter is provided
 * to support HPC synapses using TargetIdentifierIndex representation.
 *
 * For efficient lookup, all normal nodes (with proxies) need to be created together
 * and all devices need to be created together. It does not matter which are created first. If nodes
 * and devices are created alternatingly, lookup performance may suffer significantly
 * if many devices are present. This mainly affects network connection.
 *
 * As nodes are added to an initially empty SparseNodeArray, the array tracks
 * whether nodes have proxies or not. Array lookup is split at the point of the first
 * proxy/no-proxy (or no-proxy/proxy) transition between nodes. In the no-proxy
 * region, where all nodes are represented locally, Node IDs are mapped directly
 * to array indices. In the proxy region, they are scaled by 1/n_vp.
 *
 * To reliably reject requests for node IDs beyond the globally maximal node ID, the
 * latter must be set explicitly. A SparseNodeArray is said to be in *consistent state*
 * if the global maximal node ID has been set. Once add_local_node() is called, the
 * array is not in consistent state until the global maximal node ID is set again. This
 * is indicated by setting the max_node_id_ == 0. Looking up nodes while the
 * array is not in a consistent state triggers an assertion.
 *
 * To also support cases in which users alternate creation of nodes with and
 * without proxies or use nodes with special behavior (e.g., MUSIC nodes), we
 * perform a linear search from the estimated location of the node in the array.
 *
 * The following invariants hold when the array is in consistent state:
 *
 * 1. Entries are sorted by strictly increasing node ID (nid).
 * 2. All entries with index i < split_idx_ belong to the left part of the array,
 *    all remaining entries to the right part.
 * 3. All entries with node ID nid < split_node_id_ belong to the left part of the array,
 *    all remaining entries to the right part.
 * 4. nodes_[0].get_node()->has_proxies() == nodes_[i].get_node()->has_proxies() for 0 <= i < lookup_split_idx_
 *
 * @note
 * - The last invariant simply means that all nodes in the left part of the array have the same value of has_proxies().
 *
 */
class SparseNodeArray
{
public:
  /**
   * Entry representing individual node.
   *
   * @note
   * - The Node ID is stored in the entry to ensure cache locality during lookup.
   * - Actual entries will always point to valid nodes.
   * - Must be public to allow iteration by SparseNodeArray-users.
   */
  class NodeEntry
  {
    friend class SparseNodeArray;

  public:
    /**
     * @note
     * This constructor is only provided to allow BlockVector to initialize
     * new blocks with "zero" values.
     */
    NodeEntry()
      : node_( nullptr )
      , node_id_( 0 )
    {
    }

    /**
     * @param Node to be represented
     * @param Index of node to be represented
     */
    NodeEntry( Node&, size_t );

    Node* get_node() const;     //!< return pointer to represented node
    size_t get_node_id() const; //!< return ID of represented node

  private:
    Node* node_;     //!< @note pointer to allow zero-entries for BlockVector compatibility
    size_t node_id_; //!< store node ID locally for faster searching
  };

  //! Iterator inherited from BlockVector
  typedef BlockVector< SparseNodeArray::NodeEntry >::const_iterator const_iterator;

  //! Create empty sparse node array
  SparseNodeArray();

  /**
   * Return size of container.
   *
   * This is the number of local nodes.
   *
   * @see get_max_node_id()
   */
  size_t size() const;

  //! Clear the array
  void clear();

  /**
   * Add single local node.
   */
  void add_local_node( Node& );

  /**
   * Set max node ID to maximum in network.
   *
   * This also sets split_node_id_ to max node ID + 1 as long as we have not split.
   *
   * @note
   * Must be called by any method adding nodes to the network at end of
   * each batch of nodes added.
   */
  void set_max_node_id( size_t );

  /**
   * Globally largest node ID.
   */
  size_t get_max_node_id() const;

  /**
   *  Return pointer to node or nullptr if node is not local.
   *
   *  @note
   *  The caller is responsible for providing proper
   *  proxy-node pointers for non-local nodes.
   */
  Node* get_node_by_node_id( size_t ) const;

  /**
   * Lookup node based on index into container.
   *
   * @note Required for target lookup by HPC synapses.
   */
  Node* get_node_by_index( size_t ) const;

  /**
   * Constant iterators for safe iteration of SparseNodeArray.
   */
  const_iterator begin() const;
  const_iterator end() const;

private:
  bool is_consistent_() const;

  BlockVector< NodeEntry > nodes_; //!< stores local node information
  size_t global_max_node_id_;      //!< globally largest node ID
  size_t local_min_node_id_;       //!< smallest local node ID
  size_t local_max_node_id_;       //!< largest local node ID

  double left_scale_;  //!< scale factor for left side of array
  double right_scale_; //!< scale factor for right side of array

  /**
   * Globally smallest node ID in right side of array.
   *
   * - Is updated by set_max_node_id()
   * - Is global_max_node_id_ + 1 as long as right side is empty.
   */
  size_t split_node_id_;

  /**
   * Array index of first element in right side of array.
   */
  size_t split_idx_;

  /**
   * Mark whether split has happened during network construction.
   *
   * False as long as only one kind of neuron has been added.
   */
  bool have_split_;

  /**
   * Proxy status of nodes on left side of array.
   */
  bool left_side_has_proxies_;
};

} // namespace nest


#endif /* SPARSE_NODE_ARRAY_H */
