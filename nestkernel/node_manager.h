/*
 *  node_manager.h
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

#ifndef NODE_MANAGER_H
#define NODE_MANAGER_H

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"
#include "stopwatch.h"

// Includes from nestkernel:
#include "conn_builder.h"
#include "node_collection.h"
#include "nest_types.h"
#include "sparse_node_array.h"

// Includes from sli:
#include "arraydatum.h"
#include "dictdatum.h"

namespace nest
{

class Node;
class Model;

class NodeManager : public ManagerInterface
{
public:
  NodeManager();
  ~NodeManager();

  virtual void initialize();
  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  /**
   * Get properties of a node. The specified node must exist.
   * @throws nest::UnknownNode       Target does not exist in the network.
   */
  DictionaryDatum get_status( index );

  /**
   * Set properties of a Node. The specified node must exist.
   * @throws nest::UnknownNode Target does not exist in the network.
   * @throws nest::UnaccessedDictionaryEntry  Non-proxy target did not read dict
   *                                          entry.
   * @throws TypeMismatch   Array is not a flat & homogeneous array of integers.
   */
  void set_status( index, const DictionaryDatum& );

  /**
   * Add a number of nodes to the network.
   * This function creates n Node objects of Model m and adds them
   * to the Network at the current position.
   * @param m valid Model ID.
   * @param n Number of Nodes to be created. Defaults to 1 if not
   * specified.
   * @returns NodeCollection as lock pointer
   * @throws nest::UnknownModelID
   */
  NodeCollectionPTR add_node( index m, long n = 1 );

  /**
   * Get node ID's of all nodes with the given properties.
   *
   * Only node ID's of nodes matching the properties given in the dictionary
   * exactly will be returned. If the dictionary is empty, all nodes will be
   * returned. If the local_only bool is true, only node IDs of nodes simulated on
   * the local MPI process will be returned.
   *
   * @param dict parameter dictionary of selection properties
   * @param local_only bool indicating whether all nodes, or just mpi local nodes
   * should be returned.
   *
   * @returns NodeCollection as lock pointer
   */
  NodeCollectionPTR get_nodes( const DictionaryDatum& dict, const bool local_only );

  /**
   * Set the state (observable dynamic variables) of a node to model defaults.
   * @see Node::init_state()
   */
  void init_state( index );

  /**
   * Return total number of network nodes.
   */
  index size() const;

  /**
   * Returns the maximal number of nodes per virtual process.
   */
  index get_max_num_local_nodes() const;

  /**
   * Returns the number of devices per thread.
   */
  index get_num_thread_local_devices( thread t ) const;

  /**
   * Print network information.
   */
  void print( std::ostream& ) const;

  /**
   * Return true, if the given Node is on the local machine
   */
  bool is_local_node( Node* ) const;

  /**
   * Return true, if the given node ID is on the local machine
   */
  bool is_local_node_id( index node_id ) const;

  /**
   * Return pointer to the specified Node. The function expects that
   * the given node ID and thread are valid. If they are not, an assertion
   * will fail. In case the given Node does not exist on the fiven
   * thread, a proxy is returned instead.
   *
   * @param node_id index of the Node
   * @param tid local thread index of the Node
   *
   * @ingroup net_access
   */
  Node* get_node_or_proxy( index node_id, thread tid );

  /**
   * Return pointer of the specified Node.
   * @param i Index of the specified Node.
   */
  Node* get_node_or_proxy( index );

  /*
   * Return pointer of Node on the thread we are on.
   *
   * If the node has proxies, it returns the node on the first thread (used by
   * recorders).
   *
   * @params node_id Index of the Node.
   */
  Node* get_mpi_local_node_or_device_head( index );

  /**
   * Return a vector that contains the thread siblings.
   * @param i Index of the specified Node.
   *
   * @throws nest::NoThreadSiblingsAvailable Node does not have thread siblings.
   *
   * @ingroup net_access
   */
  std::vector< Node* > get_thread_siblings( index n ) const;

  /**
   * Ensure that all nodes in the network have valid thread-local IDs.
   * Create up-to-date vector of local nodes, nodes_vec_.
   * This method also sets the thread-local ID on all local nodes.
   */
  void ensure_valid_thread_local_ids();

  Node* thread_lid_to_node( thread t, targetindex thread_local_id ) const;

  /**
   * Get list of nodes on given thread.
   */
  const std::vector< Node* >& get_wfr_nodes_on_thread( thread ) const;

  /**
   * Prepare nodes for simulation and register nodes in node_list.
   * Calls prepare_node_() for each pertaining Node.
   * @see prepare_node_()
   */
  void prepare_nodes();

  /**
   * Get the number of nodes created by last prepare_nodes() call
   * @see prepare_nodes()
   * @return number of active nodes
   */
  size_t
  get_num_active_nodes()
  {
    return num_active_nodes_;
  };

  /**
   * Invoke post_run_cleanup() on all nodes.
   */
  void post_run_cleanup();

  /**
   * Invoke finalize() on all nodes.
   */
  void finalize_nodes();

  /**
   * Returns whether any node uses waveform relaxation
   */
  bool wfr_is_used() const;

  /**
   * Checks whether waveform relaxation is used by any node
   */
  void check_wfr_use();

  /**
   * Return a reference to the thread-local nodes of thread t.
   */
  const SparseNodeArray& get_local_nodes( thread ) const;

  bool have_nodes_changed() const;
  void set_have_nodes_changed( const bool changed );

private:
  /**
   * Initialize the network data structures.
   * init_() is used by the constructor and by reset().
   * @see reset()
   */
  void init_();
  void destruct_nodes_();

  /**
   * Helper function to set properties on single node.
   * @param node to set properties for
   * @param dictionary containing properties
   * @param if true (default), access flags are called before
   *        each call so Node::set_status_()
   * @throws UnaccessedDictionaryEntry
   */
  void set_status_single_node_( Node&, const DictionaryDatum&, bool clear_flags = true );

  /**
   * Initialized buffers, register in list of nodes to update/finalize.
   * @see prepare_nodes_()
   */
  void prepare_node_( Node* );

  /**
   * Add normal neurons.
   *
   * Each neuron is added to exactly one virtual process. On all other
   * VPs, it is represented by a proxy.
   *
   * @param model Model of neuron to create.
   * @param min_node_id node ID of first neuron to create.
   * @param max_node_id node ID of last neuron to create (inclusive).
   */
  void add_neurons_( Model& model, index min_node_id, index max_node_id, NodeCollectionPTR nc_ptr );

  /**
   * Add device nodes.
   *
   * For device nodes, a clone of the node is added to every virtual process.
   *
   * @param model Model of neuron to create.
   * @param min_node_id node ID of first neuron to create.
   * @param max_node_id node ID of last neuron to create (inclusive).
   */
  void add_devices_( Model& model, index min_node_id, index max_node_id, NodeCollectionPTR nc_ptr );

  /**
   * Add MUSIC nodes.
   *
   * Nodes for MUSIC communication are added once per MPI process and are
   * always placed on thread 0.
   *
   * @param model Model of neuron to create.
   * @param min_node_id node ID of first neuron to create.
   * @param max_node_id node ID of last neuron to create (inclusive).
   */
  void add_music_nodes_( Model& model, index min_node_id, index max_node_id, NodeCollectionPTR nc_ptr );

private:
  /**
   * The network as sparse array of local nodes. One entry per thread,
   * which contains only the thread-local nodes.
  */
  std::vector< SparseNodeArray > local_nodes_;

  std::vector< std::vector< Node* > > wfr_nodes_vec_; //!< Nodelists for unfrozen nodes that
                                                      //!< use the waveform relaxation method
  bool wfr_is_used_;                                  //!< there is at least one node that uses
                                                      //!< waveform relaxation
  //! Network size when wfr_nodes_vec_ was last updated
  index wfr_network_size_;
  size_t num_active_nodes_; //!< number of nodes created by prepare_nodes

  std::vector< index > num_thread_local_devices_; //!< stores number of thread local devices

  bool have_nodes_changed_; //!< true if new nodes have been created
                            //!< since startup or last call to simulate

  //! Store exceptions raised in thread-parallel sections for later handling
  std::vector< std::shared_ptr< WrappedThreadException > > exceptions_raised_;

  // private stop watch for benchmarking purposes
  Stopwatch sw_construction_create_;
};

inline index
NodeManager::size() const
{
  return local_nodes_[ 0 ].get_max_node_id();
}

inline Node*
NodeManager::thread_lid_to_node( thread t, targetindex thread_local_id ) const
{
  return local_nodes_[ t ].get_node_by_index( thread_local_id );
}

inline const std::vector< Node* >&
NodeManager::get_wfr_nodes_on_thread( thread t ) const
{
  return wfr_nodes_vec_.at( t );
}

inline bool
NodeManager::wfr_is_used() const
{
  return wfr_is_used_;
}

inline const SparseNodeArray&
NodeManager::get_local_nodes( thread t ) const
{
  return local_nodes_[ t ];
}

inline bool
NodeManager::have_nodes_changed() const
{
  return have_nodes_changed_;
}

inline void
NodeManager::set_have_nodes_changed( const bool changed )
{
  have_nodes_changed_ = changed;
}

} // namespace

#endif /* NODE_MANAGER_H */
