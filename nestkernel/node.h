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
#include "event.h"
#include "histentry.h"
#include "nest_names.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node_collection.h"

#include "deprecation_warning.h"

// Includes from sli:
#include "dictdatum.h"

/** @file node.h
 * Declarations for base class Node
 */

namespace nest
{
class Model;
class Archiving_Node;


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

class Node
{
  friend class NodeManager;
  friend class ModelManager;
  friend class proxynode;
  friend class Synapse;
  friend class Model;
  friend class SimulationManager;

  Node& operator=( const Node& ); //!< not implemented

public:
  Node();
  Node( Node const& );
  virtual ~Node();

  /**
   * Virtual copy constructor.
   * This function should create a new object by
   * calling the derived class' copy constructor and
   * return its pointer.
   */
  virtual Node*
  clone() const
  {
    return 0;
  }

  /**
   * Returns true if the node has proxies on remote threads. This is
   * used to discriminate between different types of nodes, when adding
   * new nodes to the network.
   */
  virtual bool has_proxies() const;

  /**
   * Returns true if the node only receives events from nodes/devices
   * on the same thread.
   */
  virtual bool local_receiver() const;

  /**
   * Returns true if the node exists only once per process, but does
   * not have proxies on remote threads. This is used to
   * discriminate between different types of nodes, when adding new
   * nodes to the network.
   *
   * TODO: Is this true for *any* model at all? Maybe MUSIC related?
   */
  virtual bool one_node_per_process() const;

  /**
   * Returns true if the node if it sends/receives -grid events This is
   * used to discriminate between different types of nodes, when adding
   * new nodes to the network.
   */
  virtual bool is_off_grid() const;

  /**
   * Returns true if the node is a proxy node. This is implemented because
   * the use of RTTI is rather expensive.
   */
  virtual bool is_proxy() const;

  /**
   * Return class name.
   * Returns name of node model (e.g. "iaf_psc_alpha") as string.
   * This name is identical to the name that is used to identify
   * the model in the interpreter's model dictionary.
   */
  std::string get_name() const;

  /**
   * Return the element type of the node.
   * The returned Name is a free label describing the class of network
   * elements a node belongs to. Currently used values are "neuron",
   * "recorder", "stimulator", and "other", which are all defined as
   * static Name objects in the names namespace.
   * This function is overwritten with a corresponding value in the
   * derived classes
   */
  virtual Name get_element_type() const;

  /**
   * Return global Network ID.
   * Returns the global network ID of the Node.
   * Each node has a unique network ID which can be used to access
   * the Node comparable to a pointer.
   *
   * The smallest valid node ID is 1.
   */
  index get_node_id() const;

  /**
   * Return lockpointer to the NodeCollection that created this node.
   */
  NodeCollectionPTR get_nc() const;

  /**
   * Return model ID of the node.
   * Returns the model ID of the model for this node.
   * Model IDs start with 0.
   * @note The model ID is not stored in the model prototype instance.
   *       It is only set when actual nodes are created from a prototype.
   */
  int get_model_id() const;

  /**
   * Prints out one line of the tree view of the network.
   */
  virtual std::string
  print_network( int, int, std::string = "" )
  {
    return std::string();
  }

  /**
   * Returns true if node is frozen, i.e., shall not be updated.
   */
  bool is_frozen() const;

  /**
   * Returns true if the node uses the waveform relaxation method
   */
  bool node_uses_wfr() const;

  /**
   * Sets node_uses_wfr_ member variable
   * (to be able to set it to "true" for any class derived from Node)
   */
  void set_node_uses_wfr( const bool );

  /**
   * Set state variables to the default values for the model.
   * Dynamic variables are all observable state variables of a node
   * that change during Node::update().
   * After calling init_state(), the state variables
   * should have the same values that they had after the node was
   * created. In practice, they will be initialized to the values
   * of the prototype node (model).
   * @note If the parameters of the model have been changed since the node
   *       was created, the node will be initialized to the present values
   *       set in the model.
   * @note This function is the public interface to the private function
   *       Node::init_state_(const Node&) that must be implemented by
   *       derived classes.
   */
  void init_state();

  /**
   * Initialize buffers of a node.
   * This function initializes the Buffers of a Node, e.g., ring buffers
   * for incoming events, buffers for logging potentials.
   * This function is called before Simulate is called for the first time
   * on a node, but not upon resumption of a simulation.
   * This is a wrapper function, which calls the overloaded
   * Node::init_buffers_() worker only if the buffers of the node have not been
   * initialized yet.
   */
  void init_buffers();

  /**
   * Re-calculate dependent parameters of the node.
   * This function is called each time a simulation is begun/resumed.
   * It must re-calculate all internal Variables of the node required
   * for spike handling or updating the node.
   *
   */
  virtual void calibrate() = 0;

  /**
   * Cleanup node after Run. Override this function if a node needs to
   * "wrap up" things after a call to Run, i.e., before
   * SimulationManager::run() returns. Typical use-cases are devices
   * that need to flush buffers.
   */
  virtual void
  post_run_cleanup()
  {
  }

  /**
   * Finalize node.
   * Override this function if a node needs to "wrap up" things after a
   * full simulation, i.e., a cycle of Prepare, Run, Cleanup. Typical
   * use-cases are devices that need to close files.
   */
  virtual void
  finalize()
  {
  }

  /**
   * Bring the node from state $t$ to $t+n*dt$.
   *
   * n->update(T, from, to) performs the update steps beginning
   * at T+from .. T+to-1, ie, emitting events with time stamps
   * T+from+1 .. T+to.
   *
   * @param Time   network time at beginning of time slice.
   * @param long initial step inside time slice
   * @param long post-final step inside time slice
   *
   */
  virtual void update( Time const&, const long, const long ) = 0;

  /**
   * Bring the node from state $t$ to $t+n*dt$, sends SecondaryEvents
   * (e.g. GapJunctionEvent) and resets state variables to values at $t$.
   *
   * n->wfr_update(T, from, to) performs the update steps beginning
   * at T+from .. T+to-1.
   *
   * Does not emit spikes, does not log state variables.
   *
   * throws UnexpectedEvent if not reimplemented in derived class
   *
   * @param Time   network time at beginning of time slice.
   * @param long initial step inside time slice
   * @param long post-final step inside time slice
   *
   */
  virtual bool wfr_update( Time const&, const long, const long );

  /**
   * @defgroup status_interface Configuration interface.
   * Functions and infrastructure, responsible for the configuration
   * of Nodes from the SLI Interpreter level.
   *
   * Each node can be configured from the SLI level through a named
   * parameter interface. In order to change parameters, the user
   * can specify name value pairs for each parameter. These pairs
   * are stored in a data structure which is called Dictionary.
   * Likewise, the user can query the configuration of any node by
   * requesting a dictionary with name value pairs.
   *
   * The configuration interface consists of four functions which
   * implement storage and retrieval of named parameter sets.
   */

  /**
   * Change properties of the node according to the
   * entries in the dictionary.
   * @param d Dictionary with named parameter settings.
   * @ingroup status_interface
   */
  virtual void set_status( const DictionaryDatum& ) = 0;

  /**
   * Export properties of the node by setting
   * entries in the status dictionary.
   * @param d Dictionary.
   * @ingroup status_interface
   */
  virtual void get_status( DictionaryDatum& ) const = 0;

public:
  /**
   * @defgroup event_interface Communication.
   * Functions and infrastructure, responsible for communication
   * between Nodes.
   *
   * Nodes communicate by sending an receiving events. The
   * communication interface consists of two parts:
   * -# Functions to handle incoming events.
   * -# Functions to check if a connection between nodes is possible.
   *
   * @see Event
   */

  /**
   * Send an event to the receiving_node passed as an argument.
   * This is required during the connection handshaking to test,
   * if the receiving_node can handle the event type and receptor_type sent
   * by the source node.
   *
   * If dummy_target is true, this indicates that receiving_node is derived from
   * ConnTestDummyNodeBase and used in the first call to send_test_event().
   * This can be ignored in most cases, but Nodes sending DS*Events to their
   * own event hooks and then *Events to their proper targets must send
   * DS*Events when called with the dummy target, and *Events when called with
   * the real target, see #478.
   */
  virtual port send_test_event( Node& receiving_node, rport receptor_type, synindex syn_id, bool dummy_target );

  /**
   * Check if the node can handle a particular event and receptor type.
   * This function is called upon connection setup by send_test_event().
   *
   * handles_test_event() function is used to verify that the receiver
   * can handle the event. It can also be used by the receiver to
   * return information to the sender in form of the returned port.
   * The default implementation throws an IllegalConnection
   * exception.  Any node class should define handles_test_event()
   * functions for all those event types it can handle.
   *
   * See Kunkel et al, Front Neuroinform 8:78 (2014), Sec 3.
   *
   * @note The semantics of all other handles_test_event() functions is
   * identical.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual port handles_test_event( SpikeEvent&, rport receptor_type );
  virtual port handles_test_event( WeightRecorderEvent&, rport receptor_type );
  virtual port handles_test_event( RateEvent&, rport receptor_type );
  virtual port handles_test_event( DataLoggingRequest&, rport receptor_type );
  virtual port handles_test_event( CurrentEvent&, rport receptor_type );
  virtual port handles_test_event( ConductanceEvent&, rport receptor_type );
  virtual port handles_test_event( DoubleDataEvent&, rport receptor_type );
  virtual port handles_test_event( DSSpikeEvent&, rport receptor_type );
  virtual port handles_test_event( DSCurrentEvent&, rport receptor_type );
  virtual port handles_test_event( GapJunctionEvent&, rport receptor_type );
  virtual port handles_test_event( InstantaneousRateConnectionEvent&, rport receptor_type );
  virtual port handles_test_event( DiffusionConnectionEvent&, rport receptor_type );
  virtual port handles_test_event( DelayedRateConnectionEvent&, rport receptor_type );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( GapJunctionEvent& ge );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( InstantaneousRateConnectionEvent& re );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( DiffusionConnectionEvent& de );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( DelayedRateConnectionEvent& re );

  /**
   * Register a STDP connection
   *
   * @throws IllegalConnection
   *
   */
  virtual void register_stdp_connection( double, double );

  /**
   * Handle incoming spike events.
   * @param thrd Id of the calling thread.
   * @param e Event object.
   *
   * This handler has to be implemented if a Node should
   * accept spike events.
   * @see class SpikeEvent
   * @ingroup event_interface
   */
  virtual void handle( SpikeEvent& e );

  /**
   * Handle incoming weight recording events.
   * @param thrd Id of the calling thread.
   * @param e Event object.
   *
   * This handler has to be implemented if a Node should
   * accept weight recording events.
   * @see class WeightRecordingEvent
   * @ingroup event_interface
   */
  virtual void handle( WeightRecorderEvent& e );

  /**
   * Handler for rate events.
   * @see handle(SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( RateEvent& e );

  /**
   * Handler for universal data logging request.
   * @see handle(SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DataLoggingRequest& e );

  /**
   * Handler for universal data logging request.
   * @see handle(SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   * @note There is no connect_sender() for DataLoggingReply, since
   *       this event is only used as "back channel" for DataLoggingRequest.
   */
  virtual void handle( DataLoggingReply& e );

  /**
   * Handler for current events.
   * @see handle(thread, SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( CurrentEvent& e );

  /**
   * Handler for conductance events.
   * @see handle(thread, SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( ConductanceEvent& e );

  /**
   * Handler for DoubleData events.
   * @see handle(thread, SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DoubleDataEvent& e );

  /**
   * Handler for gap junction events.
   * @see handle(thread, GapJunctionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( GapJunctionEvent& e );

  /**
   * Handler for rate neuron events.
   * @see handle(thread, InstantaneousRateConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( InstantaneousRateConnectionEvent& e );

  /**
   * Handler for rate neuron events.
   * @see handle(thread, InstantaneousRateConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DiffusionConnectionEvent& e );

  /**
   * Handler for delay rate neuron events.
   * @see handle(thread, DelayedRateConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DelayedRateConnectionEvent& e );

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
  virtual double
  get_Ca_minus() const
  {
    return 0.0;
  }

  /**
   * Get the number of synaptic element for the current Node at Ca_t which
   * corresponds to the time of the last spike.
   * Return 0.0 if not overridden
   * @ingroup SP_functions
   */
  virtual double get_synaptic_elements( Name ) const
  {
    return 0.0;
  }

  /**
   * Get the number of vacant synaptic element for the current Node
   * Return 0 if not overridden
   * @ingroup SP_functions
   */
  virtual int get_synaptic_elements_vacant( Name ) const
  {
    return 0;
  }

  /**
   * Get the number of connected synaptic element for the current Node
   * Return 0 if not overridden
   * @ingroup SP_functions
   */
  virtual int get_synaptic_elements_connected( Name ) const
  {
    return 0;
  }

  /**
   * Get the number of all synaptic elements for the current Node at time t
   * Return an empty map if not overridden
   * @ingroup SP_functions
   */
  virtual std::map< Name, double >
  get_synaptic_elements() const
  {
    return std::map< Name, double >();
  }

  /**
   * Triggers the update of all SynapticElements
   * stored in the synaptic_element_map_. It also updates the calcium
   * concentration.
   * @param t double time when the update is being performed
   * @ingroup SP_functions
   */
  virtual void update_synaptic_elements( double ){};

  /**
   * Is used to reduce the number of synaptic elements in the node through
   * time. This amount is defined by tau_vacant.
   * @ingroup SP_functions
   */
  virtual void decay_synaptic_elements_vacant(){};

  /**
   * Is used to update the number of connected
   * synaptic elements (SynapticElement::z_connected_) when a synapse
   * is formed or deleted.
   * @param type Name, name of the synaptic element to connect
   * @param n int number of new connections of the given type
   * @ingroup SP_functions
   */
  virtual void connect_synaptic_element( Name, int ){};

  /**
   * return the Kminus value at t (in ms).
   * @throws UnexpectedEvent
   */
  virtual double get_K_value( double t );

  virtual double get_LTD_value( double t );

  /**
   * write the Kminus, nearest_neighbor_Kminus, and triplet_Kminus
   * values at t (in ms) to the provided locations.
   * @throws UnexpectedEvent
   */
  virtual void get_K_values( double t, double& Kminus, double& nearest_neighbor_Kminus, double& triplet_Kminus );

  /**
  * return the spike history for (t1,t2].
  * @throws UnexpectedEvent
  */
  virtual void get_history( double t1,
    double t2,
    std::deque< histentry >::iterator* start,
    std::deque< histentry >::iterator* finish );

  virtual void get_LTP_history( double t1,
    double t2,
    std::deque< histentry_cl >::iterator* start,
    std::deque< histentry_cl >::iterator* finish );

  /**
   * Modify Event object parameters during event delivery.
   * Some Nodes want to perform a function on an event for each
   * of their targets. An example is the poisson_generator which
   * needs to draw a random number for each target. The DSSpikeEvent,
   * DirectSendingSpikeEvent, calls sender->event_hook(thread, *this)
   * in its operator() function instead of calling target->handle().
   * The default implementation of Node::event_hook() just calls
   * target->handle(DSSpikeEvent&). Any reimplementation must also
   * execute this call. Otherwise the event will not be delivered.
   * If needed, target->handle(DSSpikeEvent) may be called more than
   * once.
   */
  virtual void event_hook( DSSpikeEvent& );

  virtual void event_hook( DSCurrentEvent& );

  /**
   * Store the number of the thread to which the node is assigned.
   * The assignment is done after node creation by the Network class.
   * @see: NodeManager::add_node().
   */
  void set_thread( thread );

  /**
   * Retrieve the number of the thread to which the node is assigned.
   */
  thread get_thread() const;

  /**
   * Store the number of the virtual process to which the node is assigned.
   * This is assigned to the node in NodeManager::add_node().
   */
  void set_vp( thread );

  /**
   * Retrieve the number of the virtual process to which the node is assigned.
   */
  thread get_vp() const;

  /** Set the model id.
   * This method is called by NodeManager::add_node() when a node is created.
   * @see get_model_id()
   */
  void set_model_id( int );

  /** Execute post-initialization actions in node models.
   * This method is called by NodeManager::add_node() on a node once
   * is fully initialized, i.e. after node ID, nc, model_id, thread, vp is
   * set.
   */
  void set_initialized();

  /**
   * @returns type of signal this node produces
   * used in check_connection to only connect neurons which send / receive
   * compatible information
   */
  virtual SignalType
  sends_signal() const
  {
    return SPIKE;
  }

  /**
   * @returns type of signal this node consumes
   * used in check_connection to only connect neurons which send / receive
   * compatible information
   */
  virtual SignalType
  receives_signal() const
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
  DictionaryDatum get_status_base();

  /**
   * Set status dictionary of a node.
   *
   * Forwards to set_status() of the derived class.
   * @internal
   */
  void set_status_base( const DictionaryDatum& );

  /**
   * Returns true if node is model prototype.
   */
  bool is_model_prototype() const;

  /**
   * set thread local index

   */
  void set_thread_lid( const index );

  /**
   * get thread local index
   */
  index get_thread_lid() const;

  //! True if buffers have been initialized.
  bool
  buffers_initialized() const
  {
    return buffers_initialized_;
  }

  void
  set_buffers_initialized( bool initialized )
  {
    buffers_initialized_ = initialized;
  }

  /**
   * Sets the local device id.
   * Throws an error if used on a non-device node.
   * @see get_local_device_id
   */
  virtual void set_local_device_id( const index lsdid );

  /**
   * Gets the local device id.
   * Throws an error if used on a non-device node.
   * @see set_local_device_id
   */
  virtual index get_local_device_id() const;

  /**
   * Member of DeprecationWarning class to be used by models if parameters are
   * deprecated.
   */
  DeprecationWarning deprecation_warning;

private:
  void set_node_id_( index ); //!< Set global node id

  void set_nc_( NodeCollectionPTR );

  /** Return a new dictionary datum .
   *
   * This function is called by get_status_base() and returns a new
   * empty dictionary by default.  Some nodes may contain a
   * permanent status dictionary which is then returned by
   * get_status_dict_().
   */
  virtual DictionaryDatum get_status_dict_();

protected:
  /**
   * Private function to initialize the state of a node to model defaults.
   * This function, which must be overloaded by all derived classes, provides
   * the implementation for initializing the state of a node to the model
   * defaults; the state is the set of observable dynamic variables.
   * @param Reference to model prototype object.
   * @see Node::init_state()
   * @note To provide a reasonable behavior during the transition to the new
   *       scheme, init_state_() has a default implementation calling
   *       init_dynamic_state_().
   */
  virtual void init_state_( Node const& );

  /**
   * Private function to initialize the buffers of a node.
   * This function, which must be overloaded by all derived classes, provides
   * the implementation for initializing the buffers of a node.
   * @see Node::init_buffers()
   */
  virtual void init_buffers_();

  virtual void set_initialized_();

  Model& get_model_() const;

  //! Mark node as frozen.
  void
  set_frozen_( bool frozen )
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
  const ConcreteNode& downcast( const Node& );

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

  thread thread_;            //!< thread node is assigned to
  thread vp_;                //!< virtual process node is assigned to
  bool frozen_;              //!< node shall not be updated if true
  bool buffers_initialized_; //!< Buffers have been initialized
  bool node_uses_wfr_;       //!< node uses waveform relaxation method
  bool initialized_;         //!< set true once a node is fully initialized

  NodeCollectionPTR nc_ptr_;
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
  return vp_ == invalid_thread_;
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
Node::downcast( const Node& n )
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
