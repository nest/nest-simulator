/*
 *  vectorized_node.h
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


#ifndef VECTORIZED_NODE_H
#define VECTORIZED_NODE_H


// C++ includes:
#include "map"
#include "vector"
#include <algorithm>
#include <deque>

// Includes from nestkernel:
#include "jit_node.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node.h"
#include "synaptic_element.h"


// Includes from sli:
#include "dictdatum.h"
namespace nest
{
class JitNode;
class TimeConverter;
class VectorizedNode
{
  friend class JitNode;

public:
  VectorizedNode();

  // VectorizedNode( const VectorizedNode* );

  virtual std::shared_ptr< VectorizedNode >
  clone() const
  {
    return 0;
  };

  virtual ~VectorizedNode()
  {
    node_uses_wfr_.clear();
    frozen_.clear();
    initialized_.clear();
    global_ids.clear();
  }

  index
  get_thread()
  {
    return thread;
  }

  void
  set_thread( index t )
  {
    thread = t;
  }
  virtual std::map< std::string, const std::vector< double >& >

  get_recordables() const
  {
    return std::map< std::string, const std::vector< double >& >();
  }

  void reset();
  /**
   * Returns true if node is frozen, i.e., shall not be updated.
   */
  bool is_frozen( index ) const;

  /**
   * Returns true if the node uses the waveform relaxation method
   */
  bool node_uses_wfr( index ) const;

  index size() const;
  /**
   * Sets node_uses_wfr_ member variable
   * (to be able to set it to "true" for any class derived from Node)
   */
  void set_node_uses_wfr( const bool, index );

  index get_global_id( index ) const;

  void insert_global_id( index );

  void
  set_wrapper( Node* wrapper )
  {
    wrapper_ = wrapper;
  }

  Node* get_wrapper( index = -1, nest::index = 0 ) const;
  /**
   * Initialize node prior to first simulation after node has been created.
   *
   * init() allows the node to configure internal data structures prior to
   * being simulated. The method has an effect only the first time it is
   * called on a given node, otherwise it returns immediately. init() calls
   * virtual functions init_state_() and init_buffers_().
   */
  void init( index );

  /**
   * Re-calculate dependent parameters of the node.
   * This function is called each time a simulation is begun/resumed.
   * It must re-calculate all internal Variables of the node required
   * for spike handling or updating the node.
   *
   */
  virtual void calibrate( index ) = 0;

  /**
   * Re-calculate time-based properties of the node.
   * This function is called after a change in resolution.
   */
  virtual void
  calibrate_time( const TimeConverter&, index )
  {
  }

  /**
   * Cleanup node after Run. Override this function if a node needs to
   * "wrap up" things after a call to Run, i.e., before
   * SimulationManager::run() returns. Typical use-cases are devices
   * that need to flush buffers.
   */
  virtual void
  post_run_cleanup( index )
  {
  }
  /**
   * Finalize node.
   * Override this function if a node needs to "wrap up" things after a
   * full simulation, i.e., a cycle of Prepare, Run, Cleanup. Typical
   * use-cases are devices that need to close files.
   */
  virtual void
  finalize( index )
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
  virtual void update( Time const&, const long, const long, index ) = 0;

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
  virtual bool wfr_update( Time const&, const long, const long, index );


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
  virtual port send_test_event( Node& receiving_node, rport receptor_type, synindex syn_id, bool dummy_target, index );

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
  virtual port handles_test_event( SpikeEvent&, rport receptor_type, index );
  virtual port handles_test_event( WeightRecorderEvent&, rport receptor_type, index );
  virtual port handles_test_event( RateEvent&, rport receptor_type, index );
  virtual port handles_test_event( DataLoggingRequest&, rport receptor_type, index );
  virtual port handles_test_event( CurrentEvent&, rport receptor_type, index );
  virtual port handles_test_event( ConductanceEvent&, rport receptor_type, index );
  virtual port handles_test_event( DoubleDataEvent&, rport receptor_type, index );
  virtual port handles_test_event( DSSpikeEvent&, rport receptor_type, index );
  virtual port handles_test_event( DSCurrentEvent&, rport receptor_type, index );
  virtual port handles_test_event( GapJunctionEvent&, rport receptor_type, index );
  virtual port handles_test_event( InstantaneousRateConnectionEvent&, rport receptor_type, index );
  virtual port handles_test_event( DiffusionConnectionEvent&, rport receptor_type, index );
  virtual port handles_test_event( DelayedRateConnectionEvent&, rport receptor_type, index );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( GapJunctionEvent&, index );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( InstantaneousRateConnectionEvent&, index );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( DiffusionConnectionEvent&, index );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( DelayedRateConnectionEvent&, index );

  /**
   * Register a STDP connection
   *
   * @throws IllegalConnection
   *
   */
  virtual void register_stdp_connection( double, double, index );

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
  virtual void handle( SpikeEvent&, index );

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
  virtual void handle( WeightRecorderEvent&, index );

  /**
   * Handler for rate events.
   * @see handle(SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( RateEvent&, index );

  /**
   * Handler for universal data logging request.
   * @see handle(SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DataLoggingRequest&, index );

  /**
   * Handler for universal data logging request.
   * @see handle(SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   * @note There is no connect_sender() for DataLoggingReply, since
   *       this event is only used as "back channel" for DataLoggingRequest.
   */
  virtual void handle( DataLoggingReply&, index );

  /**
   * Handler for current events.
   * @see handle(thread, SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( CurrentEvent&, index );

  /**
   * Handler for conductance events.
   * @see handle(thread, SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( ConductanceEvent&, index );

  /**
   * Handler for DoubleData events.
   * @see handle(thread, SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DoubleDataEvent&, index );

  /**
   * Handler for gap junction events.
   * @see handle(thread, GapJunctionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( GapJunctionEvent&, index );

  /**
   * Handler for rate neuron events.
   * @see handle(thread, InstantaneousRateConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( InstantaneousRateConnectionEvent&, index );

  /**
   * Handler for rate neuron events.
   * @see handle(thread, InstantaneousRateConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DiffusionConnectionEvent&, index );

  /**
   * Handler for delay rate neuron events.
   * @see handle(thread, DelayedRateConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DelayedRateConnectionEvent&, index );

  /**
   * @defgroup SP_functions Structural Plasticity in NEST.
   * Functions related to accessibility and setup of variables required for
   * the implementation of a model of Structural Plasticity in NEST.
   *
   */


  virtual double
  get_Ca_minus( index ) const
  {
    return 0.0;
  }
  std::string
  get_name() const
  {
    return "Ayssar_testing_concept";
  }


  /**
   * return the Kminus value at t (in ms).
   * @throws UnexpectedEvent
   */
  virtual double get_K_value( double t, index );

  virtual double get_LTD_value( double t, index );

  /**
   * write the Kminus, nearest_neighbor_Kminus, and Kminus_triplet
   * values at t (in ms) to the provided locations.
   * @throws UnexpectedEvent
   */
  virtual void get_K_values( double t, double& Kminus, double& nearest_neighbor_Kminus, double& Kminus_triplet, index );

  virtual void get_history( double t1,
    double t2,
    std::deque< histentry >::iterator* start,
    std::deque< histentry >::iterator* finish,
    index );

  virtual void get_LTP_history( double t1,
    double t2,
    std::deque< histentry_extended >::iterator* start,
    std::deque< histentry_extended >::iterator* finish,
    index );

  virtual void get_urbanczik_history( double t1,
    double t2,
    std::deque< histentry_extended >::iterator* start,
    std::deque< histentry_extended >::iterator* finish,
    int,
    index );

  // make neuron parameters accessible in Urbanczik synapse
  virtual double get_C_m( int comp, index );
  virtual double get_g_L( int comp, index );
  virtual double get_tau_L( int comp, index );
  virtual double get_tau_s( int comp, index );
  virtual double get_tau_syn_ex( int comp, index );
  virtual double get_tau_syn_in( int comp, index );


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
  virtual void event_hook( DSSpikeEvent&, index );

  virtual void event_hook( DSCurrentEvent&, index );


  /**
   * @returns type of signal this node produces
   * used in check_connection to only connect neurons which send / receive
   * compatible information
   */
  virtual SignalType
  sends_signal( index ) const
  {
    return SPIKE;
  }

  /**
   * @returns type of signal this node consumes
   * used in check_connection to only connect neurons which send / receive
   * compatible information
   */
  virtual SignalType
  receives_signal( index ) const
  {
    return SPIKE;
  }

  /**
   * \fn double get_synaptic_elements(Name n)
   * get the number of synaptic element for the current Node
   * the number of synaptic elements is a double value but the number of
   * actual vacant and connected elements is an integer truncated from this
   * value
   * @param local_id position of node in the vector
   */
  virtual double
  get_synaptic_elements( Name, index ) const
  {
    return 0.0;
  };

  /**
   * \fn int get_synaptic_elements_vacant(Name n)
   * Get the number of synaptic elements of type n which are available
   * for new synapse creation
   * @param local_id position of node in the vector
   */
  virtual int
  get_synaptic_elements_vacant( Name, index ) const
  {
    return 0;
  }

  /**
   * \fn int get_synaptic_elements_connected(Name n)
   * get the number of synaptic element of type n which are currently
   * connected
   * @param local_id position of node in the vector
   */
  virtual int
  get_synaptic_elements_connected( Name, index ) const
  {
    return 0;
  }

  /**
   * \fn std::map<Name, double> get_synaptic_elements()
   * get the number of all synaptic elements for the current Node
   * @param local_id position of node in the vector
   */
  virtual std::map< Name, double >
  get_synaptic_elements( index ) const
  {
    return std::map< Name, double >();
  }

  /**
   * \fn void update_synaptic_elements()
   * Change the number of synaptic elements in the node depending on the
   * dynamics described by the corresponding growth curve
   * @param local_id position of node in the vector
   */
  virtual void update_synaptic_elements( double, index ) {};

  /**
   * \fn void decay_synaptic_elements_vacant()
   * Delete a certain portion of the vacant synaptic elements which are not
   * in use
   * @param local_id position of node in the vector
   */
  virtual void decay_synaptic_elements_vacant( index ) {};

  /**
   * \fn void connect_synaptic_element()
   * Change the number of connected synaptic elements by n
   * @param local_id position of node in the vector
   */
  virtual void connect_synaptic_element( Name, int, index ) {};

  virtual void get_status( DictionaryDatum&, index ) const {};
  virtual void set_status( const DictionaryDatum&, index ) {};
  virtual void resize( index, index = 0 );

  /**
   * retrieve the current value of tau_Ca which defines the exponential decay
   * constant of the intracellular calcium concentration
   * @param local_id position of node in the vector
   */
  virtual double get_tau_Ca( index ) const = 0;

protected:
  /**
   * Configure state variables depending on runtime information.
   *
   * Overload this method if the node needs to adapt state variables prior to
   * first simulation to runtime information, e.g., the number of incoming
   * connections.
   */
  virtual void init_state_( index ) {};

  virtual void set_frozen_( bool, index );

  /**
   * Configure persistent internal data structures.
   *
   * Let node configure persistent internal data structures, such as input
   * buffers or ODE solvers, to runtime information prior to first simulation.
   */
  virtual void init_buffers_( index ) {};

  virtual void set_initialized_( index );
  /**
   * \fn void set_spiketime(Time const & t_sp, double offset)
   * record spike history
   */
  virtual void set_spiketime( Time const&, index = -1, double = 0.0 ) {};

  /**
   * \fn void clear_history()
   * clear spike history
   */
  virtual void clear_history( index ) = 0;


private:
  std::vector< bool > node_uses_wfr_;
  std::vector< bool > frozen_;
  std::vector< bool > initialized_;
  std::vector< index > global_ids;
  index thread;
  Node* wrapper_;
};
inline bool
VectorizedNode::is_frozen( index local_id ) const
{
  return frozen_.at( local_id );
}
inline bool
VectorizedNode::node_uses_wfr( index local_id ) const
{
  return node_uses_wfr_.at( local_id );
}
inline void
VectorizedNode::set_node_uses_wfr( const bool value, index local_id )
{
  node_uses_wfr_.at( local_id ) = value;
}
inline void
VectorizedNode::init( index local_id )
{
  if ( initialized_.at( local_id ) )
  {
    return;
  }
  init_state_( local_id );
  init_buffers_( local_id );

  initialized_.at( local_id ) = true;
}
}
#endif // VECTORIZED_NODE_H
