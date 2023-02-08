/*
 *  node.h
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

#ifndef NODE_H
#define NODE_H

// C++ includes:
#include <bitset>
#include <deque>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Includes from nestkernel:
#include "deprecation_warning.h"
#include "event.h"
#include "histentry.h"
#include "nest_names.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node_collection.h"
#include "node_interface.h"
#include "secondary_event.h"

// Includes from sli:
#include "dictdatum.h"

/** @file node.h
 * Declarations for base class Node
 */

namespace nest
{
class Model;
class ArchivingNode;
class TimeConverter;


/**
 * @defgroup user_interface Model developer interface.
 * Functions and classes important for implementing new Node and
 * Model classes.
 */

/**
 * Base class for all NEST network objects.
 *
 * Class Node is the top of the simulation object hierarchy. It
 * defines the most general interface to a network element.
 *
 * Class Node provide the interface for
 * - updating the dynamic state of an object
 * - connecting nodes, using particular Events
 * - accepting connection requests
 * - handling incoming events
 * A new type of Node must be derived from this base class and
 * implement its interface.
 * In order to keep the inheritance hierarchy flat, it is encouraged
 * to directly subclass from base class Node.
 *
 * @see class Event
 * @ingroup user_interface
 */

/** @BeginDocumentation

   Name: Node - General properties of all nodes.

   Parameters:
   frozen     booltype    - Whether the node is updated during simulation
   global_id  integertype - The node ID of the node (cf. local_id)
   local      booltype    - Whether the node is available on the local process
   model      literaltype - The model type the node was created from
   state      integertype - The state of the node (see the help on elementstates
                            for details)
   thread     integertype - The id of the thread the node is assigned to (valid
                            locally)
   vp         integertype - The id of the virtual process the node is assigned
                            to (valid globally)

   SeeAlso: GetStatus, SetStatus, elementstates
 */

class Node : public NodeInterface
{
  friend class NodeManager;
  friend class ModelManager;
  friend class proxynode;
  friend class Synapse;
  friend class Model;
  friend class SimulationManager;

  NodeInterface& operator=( const NodeInterface& ); //!< not implemented

public:
  Node();
  Node( Node const& );
  virtual ~Node();

  /**
   * Returns true if the node has proxies on remote threads. This is
   * used to discriminate between different types of nodes, when adding
   * new nodes to the network.
   */
  bool has_proxies() const override;

  /**
   * Returns true if the node supports the Urbanczik-Senn plasticity rule
   */
  bool supports_urbanczik_archiving() const override;

  /**
   * Returns true if the node only receives events from nodes/devices
   * on the same thread.
   */
  bool local_receiver() const override;

  /**
   * Returns true if the node exists only once per process, but does
   * not have proxies on remote threads. This is used to
   * discriminate between different types of nodes, when adding new
   * nodes to the network.
   *
   * TODO: Is this true for *any* model at all? Maybe MUSIC related?
   */
  bool one_node_per_process() const override;

  /**
   * Returns true if the node sends/receives off-grid events. This is
   * used to discriminate between different types of nodes when adding
   * new nodes to the network.
   */
  bool is_off_grid() const override;

  /**
   * Returns true if the node is a proxy node. This is implemented because
   * the use of RTTI is rather expensive.
   */
  bool is_proxy() const override;

  /**
   * Return class name.
   * Returns name of node model (e.g. "iaf_psc_alpha") as string.
   * This name is identical to the name that is used to identify
   * the model in the interpreter's model dictionary.
   */
  std::string get_name() const override;

  /**
   * Return the element type of the node.
   * The returned Name is a free label describing the class of network
   * elements a node belongs to. Currently used values are "neuron",
   * "recorder", "stimulator", and "other", which are all defined as
   * static Name objects in the names namespace.
   * This function is overwritten with a corresponding value in the
   * derived classes
   */
  Name get_element_type() const override;

  /**
   * Return global Network ID.
   * Returns the global network ID of the Node.
   * Each node has a unique network ID which can be used to access
   * the Node comparable to a pointer.
   *
   * The smallest valid node ID is 1.
   */
  index get_node_id() const override;

  /**
   * Return lockpointer to the NodeCollection that created this node.
   */
  NodeCollectionPTR get_nc() const override;

  /**
   * Return model ID of the node.
   * Returns the model ID of the model for this node.
   * Model IDs start with 0.
   * @note The model ID is not stored in the model prototype instance.
   *       It is only set when actual nodes are created from a prototype.
   */
  int get_model_id() const override;

  /**
   * Returns true if node is frozen, i.e., shall not be updated.
   */
  bool is_frozen() const override;

  /**
   * Returns true if the node uses the waveform relaxation method
   */
  bool node_uses_wfr() const override;

  /**
   * Sets node_uses_wfr_ member variable
   * (to be able to set it to "true" for any class derived from Node)
   */
  void set_node_uses_wfr( const bool ) override;

  /**
   * Initialize node prior to first simulation after node has been created.
   *
   * init() allows the node to configure internal data structures prior to
   * being simulated. The method has an effect only the first time it is
   * called on a given node, otherwise it returns immediately. init() calls
   * virtual functions init_state_() and init_buffers_().
   */
  void init() override;


  /**
   * Re-calculate time-based properties of the node.
   * This function is called after a change in resolution.
   */
  void
  calibrate_time( const TimeConverter& ) override
  {
  }

  /**
   * Cleanup node after Run. Override this function if a node needs to
   * "wrap up" things after a call to Run, i.e., before
   * SimulationManager::run() returns. Typical use-cases are devices
   * that need to flush buffers.
   */
  void
  post_run_cleanup() override
  {
  }

  /**
   * Finalize node.
   * Override this function if a node needs to "wrap up" things after a
   * full simulation, i.e., a cycle of Prepare, Run, Cleanup. Typical
   * use-cases are devices that need to close files.
   */
  void
  finalize() override
  {
  }


public:
  /**
   * @defgroup SP_functions Structural Plasticity in NEST.
   * Functions related to accessibility and setup of variables required for
   * the implementation of a model of Structural Plasticity in NEST.
   *
   */

  /**
   * Return the Ca_minus value at time Ca_t which corresponds to the time of
   * the last update in Calcium concentration which is performed each time
   * a Node spikes.
   * Return 0.0 if not overridden
   * @ingroup SP_functions
   */
  double
  get_Ca_minus() const override
  {
    return 0.0;
  }

  /**
   * Get the number of synaptic element for the current Node at Ca_t which
   * corresponds to the time of the last spike.
   * Return 0.0 if not overridden
   * @ingroup SP_functions
   */
  double
  get_synaptic_elements( Name ) const override
  {
    return 0.0;
  }

  int
  get_synaptic_elements_vacant( Name ) const override
  {
    return 0;
  }


  int
  get_synaptic_elements_connected( Name ) const override
  {
    return 0;
  }


  std::map< Name, double >
  get_synaptic_elements() const override
  {
    return std::map< Name, double >();
  }


  void update_synaptic_elements( double ) override {};


  void decay_synaptic_elements_vacant() override {};


  void connect_synaptic_element( Name, int ) override {};


  void event_hook( DSSpikeEvent& ) override;

  void event_hook( DSCurrentEvent& ) override;


  void set_thread( thread ) override;

  thread get_thread() const override;

  void set_vp( thread ) override;

  thread get_vp() const override;


  void set_model_id( int ) override;

  void set_initialized() override;


  virtual SignalType
  sends_signal() const override
  {
    return SPIKE;
  }


  virtual SignalType
  receives_signal() const override
  {
    return SPIKE;
  }


  /**
   *  Return a dictionary with the node's properties.
   *
   *  get_status_base() first gets a dictionary with the basic
   *  information of an element, using get_status_dict_(). It then
   *  calls the custom function get_status(DictionaryDatum) with
   *  the created status dictionary as argument.
   */
  DictionaryDatum get_status_base() override;

  /**
   * Set status dictionary of a node.
   *
   * Forwards to set_status() of the derived class.
   * @internal
   */
  void set_status_base( const DictionaryDatum& ) override;

  /**
   * Returns true if node is model prototype.
   */
  bool is_model_prototype() const override;

  /**
   * set thread local index

   */
  void set_thread_lid( const index ) override;

  /**
   * get thread local index
   */
  index get_thread_lid() const override;

  virtual index get_local_device_id() const override;

  /**
   * Member of DeprecationWarning class to be used by models if parameters are
   * deprecated.
   */
  DeprecationWarning deprecation_warning;

protected:
  void set_node_id_( index ) override;

  void set_nc_( NodeCollectionPTR ) override;

  void init_state_() override;

  /**
   * Configure persistent internal data structures.
   *
   * Let node configure persistent internal data structures, such as input
   * buffers or ODE solvers, to runtime information prior to first simulation.
   */
  void init_buffers_() override;

  void set_initialized_() override;

  Model& get_model_() const override;

  //! Mark node as frozen.
  void
  set_frozen_( bool frozen ) override
  {
    frozen_ = frozen;
  }

  /**
   * Auxiliary function to downcast a Node to a concrete class derived from
   * Node.
   * @note This function is used to convert generic Node references to specific
   *       ones when intializing parameters or state from a prototype.
   */
  template < typename ConcreteNode >
  const ConcreteNode& downcast( const NodeInterface& );

private:
  /**
   * Global Element ID (node ID).
   *
   * The node ID is unique within the network. The smallest valid node ID is 1.
   */
  index node_id_;

  /**
   * Local id of this node in the thread-local vector of nodes.
   */
  index thread_lid_;

  /**
   * Model ID.
   * It is only set for actual node instances, not for instances of class Node
   * representing model prototypes. Model prototypes always have model_id_==-1.
   * @see get_model_id(), set_model_id()
   */
  int model_id_;

  thread thread_;      //!< thread node is assigned to
  thread vp_;          //!< virtual process node is assigned to
  bool frozen_;        //!< node shall not be updated if true
  bool initialized_;   //!< state and buffers have been initialized
  bool node_uses_wfr_; //!< node uses waveform relaxation method

  NodeCollectionPTR nc_ptr_; //!< Original NodeCollection of this node, used to extract node-specific metadata
};

inline bool
Node::is_frozen() const
{
  return frozen_;
}

inline bool
Node::node_uses_wfr() const
{
  return node_uses_wfr_;
}

inline bool
Node::supports_urbanczik_archiving() const
{
  return false;
}

inline void
Node::set_node_uses_wfr( const bool uwfr )
{
  node_uses_wfr_ = uwfr;
}

inline bool
Node::has_proxies() const
{
  return true;
}

inline bool
Node::local_receiver() const
{
  return false;
}

inline bool
Node::one_node_per_process() const
{
  return false;
}

inline bool
Node::is_off_grid() const
{
  return false;
}

inline bool
Node::is_proxy() const
{
  return false;
}

inline Name
Node::get_element_type() const
{
  return names::neuron;
}

inline index
Node::get_node_id() const
{
  return node_id_;
}

inline NodeCollectionPTR
Node::get_nc() const
{
  return nc_ptr_;
}

inline void
Node::set_node_id_( index i )
{
  node_id_ = i;
}


inline void
Node::set_nc_( NodeCollectionPTR nc_ptr )
{
  nc_ptr_ = nc_ptr;
}

inline int
Node::get_model_id() const
{
  return model_id_;
}

inline void
Node::set_model_id( int i )
{
  model_id_ = i;
}

inline bool
Node::is_model_prototype() const
{
  return vp_ == invalid_thread;
}

inline void
Node::set_thread( thread t )
{
  thread_ = t;
}

inline thread
Node::get_thread() const
{
  return thread_;
}

inline void
Node::set_vp( thread vp )
{
  vp_ = vp;
}

inline thread
Node::get_vp() const
{
  return vp_;
}

template < typename ConcreteNode >
const ConcreteNode&
Node::downcast( const NodeInterface& n )
{
  ConcreteNode const* tp = dynamic_cast< ConcreteNode const* >( &n );
  assert( tp != 0 );
  return *tp;
}

inline void
Node::set_thread_lid( const index tlid )
{
  thread_lid_ = tlid;
}

inline index
Node::get_thread_lid() const
{
  return thread_lid_;
}

} // namespace

#endif
