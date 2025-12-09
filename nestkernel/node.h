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
#include <string>
#include <vector>

// Includes from nestkernel:
#include "common_synapse_properties.h"
#include "deprecation_warning.h"
#include "event.h"
#include "histentry.h"
#include "nest_time.h"
#include "nest_types.h"
#include "secondary_event_impl.h"
#include "weight_optimizer.h"

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
 *
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
  friend class Model;
  friend class SimulationManager;

  Node& operator=( const Node& ); //!< not implemented

public:
  Node();
  Node( Node const& );
  virtual ~Node();

  /**
   * This function creates a new object by calling the derived class' copy constructor and
   * returning its pointer.
   */
  virtual Node*
  clone() const
  {
    return nullptr;
  }

  /**
   * Returns true if the node has proxies on remote threads.
   *
   * This is used to discriminate between different types of nodes, when adding
   * new nodes to the network.
   */
  virtual bool has_proxies() const;

  /**
   * Returns true if the node supports the Urbanczik-Senn plasticity rule
   */
  virtual bool supports_urbanczik_archiving() const;

  /**
   * Returns true if the node only receives events from nodes/devices
   * on the same thread.
   */
  virtual bool local_receiver() const;

  /**
   * Returns true if the node exists only once per process, but does
   * not have proxies on remote threads.
   *
   * This is used to discriminate between different types of nodes, when adding new
   * nodes to the network. As of now, this function is only true for MUSIC related proxies?
   */
  virtual bool one_node_per_process() const;

  /**
   * Returns true if the node sends/receives off-grid events.
   *
   * This is used to discriminate between different types of nodes when adding
   * new nodes to the network.
   */
  virtual bool is_off_grid() const;

  /**
   * Returns true if the node is a proxy node.
   *
   * This is implemented because the use of RTTI is rather expensive.
   */
  virtual bool is_proxy() const;

  /**
   * Return class name.
   *
   * Returns name of node model (e.g. "iaf_psc_alpha") as string.
   * This name is identical to the name that is used to identify
   * the model in the interpreter's model dictionary.
   */
  std::string get_name() const;

  /**
   * Return the element type of the node.
   *
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
   *
   * Returns the global network ID of the Node.
   * Each node has a unique network ID which can be used to access
   * the Node comparable to a pointer.
   *
   * The smallest valid node ID is 1.
   */
  size_t get_node_id() const;


  /**
   * Return model ID of the node.
   *
   * Returns the model ID of the model for this node.
   * Model IDs start with 0.
   * @note The model ID is not stored in the model prototype instance.
   *       It is only set when actual nodes are created from a prototype.
   */
  int get_model_id() const;

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
   * Initialize node prior to first simulation after node has been created.
   *
   * init() allows the node to configure internal data structures prior to
   * being simulated. The method has an effect only the first time it is
   * called on a given node, otherwise it returns immediately. init() calls
   * virtual functions init_state_() and init_buffers_().
   */
  void init();

  /**
   * Re-calculate dependent parameters of the node.
   *
   * This function is called each time a simulation is begun/resumed.
   * It must re-calculate all internal Variables of the node required
   * for spike handling or updating the node.
   *
   */
  virtual void pre_run_hook() = 0;

  /**
   * Re-calculate time-based properties of the node.
   * This function is called after a change in resolution.
   */
  virtual void calibrate_time( const TimeConverter& );

  /**
   * Cleanup node after Run.
   *
   * Override this function if a node needs to
   * "wrap up" things after a call to Run, i.e., before
   * SimulationManager::run() returns. Typical use-cases are devices
   * that need to flush buffers.
   */
  virtual void post_run_cleanup();

  /**
   * Finalize node.
   *
   * Override this function if a node needs to "wrap up" things after a
   * full simulation, i.e., a cycle of Prepare, Run, Cleanup. Typical
   * use-cases are devices that need to close files.
   */
  virtual void finalize();

  /**
   * Advance the state of the node in time through the given interval.
   *
   * This method advances the state of the node through the interval
   * ``(origin+from, origin+to]``, which is at most ``min_delay`` long.
   *
   * - Precondition: State of the node corresponds to the time ``origin+from``.
   * - Postcondition: State of the node corresponds to the time ``origin+to``.
   *
   * Each step between ``from`` and ``to`` corresponds to one simulation timestep (``nest.resolution``).
   *
   * If events are emitted, they have time stamps in the interval
   * ``T+from+1 .. T+to``.
   *
   * @param origin network time at beginning of time slice
   * @param from initial step inside time slice
   * @param to post-final step inside time slice
   *
   */
  virtual void update( Time const&, const long, const long ) = 0;

  /**
   * Advance the state of the node in time through the given interval (see
   * Node::update() for more details).
   *
   * Does not emit spikes, does not log state variables.
   *
   * throws UnexpectedEvent if not reimplemented in derived class
   *
   * @param origin network time at beginning of time slice
   * @param from initial step inside time slice
   * @param to post-final step inside time slice
   *
   */
  virtual bool wfr_update( Time const&, const long, const long );

  /**
   * @defgroup status_interface Configuration interface.
   *
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
   *
   * @param d Dictionary with named parameter settings.
   * @ingroup status_interface
   */
  virtual void set_status( const DictionaryDatum& ) = 0;

  /**
   * Export properties of the node by setting
   * entries in the status dictionary.
   *
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
   *
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
  virtual size_t send_test_event( Node& receiving_node, size_t receptor_type, synindex syn_id, bool dummy_target );

  /**
   * Check if the node can handle a particular event and receptor type.
   *
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
  virtual size_t handles_test_event( SpikeEvent&, size_t receptor_type );
  virtual size_t handles_test_event( WeightRecorderEvent&, size_t receptor_type );
  virtual size_t handles_test_event( RateEvent&, size_t receptor_type );
  virtual size_t handles_test_event( DataLoggingRequest&, size_t receptor_type );
  virtual size_t handles_test_event( CurrentEvent&, size_t receptor_type );
  virtual size_t handles_test_event( ConductanceEvent&, size_t receptor_type );
  virtual size_t handles_test_event( DoubleDataEvent&, size_t receptor_type );
  virtual size_t handles_test_event( DSSpikeEvent&, size_t receptor_type );
  virtual size_t handles_test_event( DSCurrentEvent&, size_t receptor_type );
  virtual size_t handles_test_event( GapJunctionEvent&, size_t receptor_type );
  virtual size_t handles_test_event( InstantaneousRateConnectionEvent&, size_t receptor_type );
  virtual size_t handles_test_event( DiffusionConnectionEvent&, size_t receptor_type );
  virtual size_t handles_test_event( DelayedRateConnectionEvent&, size_t receptor_type );
  virtual size_t handles_test_event( LearningSignalConnectionEvent&, size_t receptor_type );
  virtual size_t handles_test_event( SICEvent&, size_t receptor_type );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   *
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( GapJunctionEvent& ge );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   *
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( InstantaneousRateConnectionEvent& re );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   *
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( DiffusionConnectionEvent& de );

  /**
   * Required to check, if source neuron may send a SecondaryEvent.
   *
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( DelayedRateConnectionEvent& re );

  /**
   * Required to check if source node may send a LearningSignalConnectionEvent.
   *
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( LearningSignalConnectionEvent& re );

  /**
   * Required to check if source node may send a SICEvent.
   *
   * This base class implementation throws IllegalConnection
   * and needs to be overwritten in the derived class.
   * @ingroup event_interface
   * @throws IllegalConnection
   */
  virtual void sends_secondary_event( SICEvent& sic );

  /**
   * Register a STDP connection
   *
   * @throws IllegalConnection
   *
   */
  virtual void register_stdp_connection( double, double );

  /**
   * @brief Registers an eprop synapse and initializes the update history.
   *
   * The time for the first entry of the update history is set to the neuron specific shift for `bsshslm_2020`
   * models and to the negative transmission delay from the recurrent to the output layer otherwise.
   *
   * @throws IllegalConnection
   */
  virtual void register_eprop_connection();

  /**
   * @brief Retrieves the temporal shift of the signal.
   *
   * Retrieves the number of steps the time-point of the signal has to be shifted to
   * place it at the correct location in the e-prop-related histories.
   *
   * @note Unlike the original e-prop, where signals arise instantaneously, NEST
   * considers connection delays. Thus, to reproduce the original results, we
   * compensate for the delays and synchronize the signals by shifting the
   * history.
   *
   * @return The number of time steps to shift.
   *
   * @throws IllegalConnection
   */
  virtual long get_shift() const;

  /**
   *  Registers the current update in the update history and deregisters the previous update.
   *
   * @param t_previous_update The time step of the previous update.
   * @param t_current_update The time step of the current update.
   * @param eprop_isi_trace_cutoff The cutoff value for the eprop inter-spike interval trace (optional, default: 0).
   *
   * @throws IllegalConnection
   */
  virtual void write_update_to_history( const long t_previous_update,
    const long t_current_update,
    const long eprop_isi_trace_cutoff = 0 );

  /**
   * Retrieves the maximum number of time steps integrated between two consecutive spikes.
   *
   * @return The cutoff value for the inter-spike interval eprop trace.
   *
   * @throws IllegalConnection
   */
  virtual long get_eprop_isi_trace_cutoff() const;

  /**
   * Checks if the node is part of the recurrent network and thus not a readout neuron.
   *
   * @note The e-prop synapse calls this function of the target node. If true,
   * it skips weight updates within the first interval step of the update
   * interval.
   *
   * @return true if the node is an eprop recurrent node, false otherwise.
   *
   * @throws IllegalConnection
   */
  virtual bool is_eprop_recurrent_node() const;

  /**
   * Handle incoming spike events.
   *
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
   *
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
   *
   * @see handle(SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( RateEvent& e );

  /**
   * Handler for universal data logging request.
   *
   * @see handle(SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DataLoggingRequest& e );

  /**
   * Handler for universal data logging request.
   *
   * @see handle(SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   * @note There is no connect_sender() for DataLoggingReply, since
   *       this event is only used as "back channel" for DataLoggingRequest.
   */
  virtual void handle( DataLoggingReply& e );

  /**
   * Handler for current events.
   *
   * @see handle(thread, SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( CurrentEvent& e );

  /**
   * Handler for conductance events.
   *
   * @see handle(thread, SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( ConductanceEvent& e );

  /**
   * Handler for DoubleData events.
   *
   * @see handle(thread, SpikeEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DoubleDataEvent& e );

  /**
   * Handler for gap junction events.
   *
   * @see handle(thread, GapJunctionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( GapJunctionEvent& e );

  /**
   * Handler for rate neuron events.
   *
   * @see handle(thread, InstantaneousRateConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( InstantaneousRateConnectionEvent& e );

  /**
   * Handler for rate neuron events.
   *
   * @see handle(thread, InstantaneousRateConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DiffusionConnectionEvent& e );

  /**
   * Handler for delay rate neuron events.
   *
   * @see handle(thread, DelayedRateConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( DelayedRateConnectionEvent& e );

  /**
   * Handler for learning signal connection events.
   *
   * @see handle(thread, LearningSignalConnectionEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( LearningSignalConnectionEvent& e );

  /**
   * Handler for slow inward current events (SICEvents).
   *
   * @see handle(thread,SICEvent&)
   * @ingroup event_interface
   * @throws UnexpectedEvent
   */
  virtual void handle( SICEvent& e );

  /**
   * @defgroup SP_functions Structural Plasticity in NEST.
   *
   * Functions related to accessibility and setup of variables required for
   * the implementation of a model of Structural Plasticity in NEST.
   *
   */

  /**
   * Return the Ca_minus value at time Ca_t which corresponds to the time of
   * the last update in Calcium concentration which is performed each time
   * a Node spikes.
   *
   * Return 0.0 if not overridden
   * @ingroup SP_functions
   */
  virtual double get_Ca_minus() const;

  /**
   * Get the number of synaptic element for the current Node at Ca_t which
   * corresponds to the time of the last spike.
   *
   * Return 0.0 if not overridden
   * @ingroup SP_functions
   */
  virtual double get_synaptic_elements( Name ) const;

  /**
   * Get the number of vacant synaptic element for the current Node
   * Return 0 if not overridden
   * @ingroup SP_functions
   */
  virtual int get_synaptic_elements_vacant( Name ) const;

  /**
   * Get the number of connected synaptic element for the current Node
   *
   * Return 0 if not overridden
   * @ingroup SP_functions
   */
  virtual int get_synaptic_elements_connected( Name ) const;

  /**
   * Get the number of all synaptic elements for the current Node at time t
   *
   * Return an empty map if not overridden
   * @ingroup SP_functions
   */
  virtual std::map< Name, double > get_synaptic_elements() const;

  /**
   * Triggers the update of all SynapticElements
   * stored in the synaptic_element_map_.
   *
   * It also updates the calcium concentration.
   *
   * @param t double time when the update is being performed
   * @ingroup SP_functions
   */
  virtual void update_synaptic_elements( double ) {};

  /**
   * Is used to reduce the number of synaptic elements in the node through
   * time.
   *
   * This amount is defined by tau_vacant.
   * @ingroup SP_functions
   */
  virtual void decay_synaptic_elements_vacant() {};

  /**
   * Is used to update the number of connected
   * synaptic elements (SynapticElement::z_connected_) when a synapse
   * is formed or deleted.
   *
   * @param type Name, name of the synaptic element to connect
   * @param n int number of new connections of the given type
   * @ingroup SP_functions
   */
  virtual void connect_synaptic_element( Name, int ) {};

  /**
   * return the Kminus value at t (in ms).
   * @throws UnexpectedEvent
   */
  virtual double get_K_value( double t );

  virtual double get_LTD_value( double t );

  /**
   * write the Kminus, nearest_neighbor_Kminus, and Kminus_triplet
   * values at t (in ms) to the provided locations.
   *
   * @throws UnexpectedEvent
   */
  virtual void get_K_values( double t, double& Kminus, double& nearest_neighbor_Kminus, double& Kminus_triplet );

  /**
   * return the spike history for (t1,t2].
   * @throws UnexpectedEvent
   */
  virtual void get_history( double t1,
    double t2,
    std::deque< histentry >::iterator* start,
    std::deque< histentry >::iterator* finish );

  // for Clopath synapse
  virtual void get_LTP_history( double t1,
    double t2,
    std::deque< histentry_extended >::iterator* start,
    std::deque< histentry_extended >::iterator* finish );
  // for Urbanczik synapse
  virtual void get_urbanczik_history( double t1,
    double t2,
    std::deque< histentry_extended >::iterator* start,
    std::deque< histentry_extended >::iterator* finish,
    int );
  // make neuron parameters accessible in Urbanczik synapse
  virtual double get_C_m( int comp );
  virtual double get_g_L( int comp );
  virtual double get_tau_L( int comp );
  virtual double get_tau_s( int comp );
  virtual double get_tau_syn_ex( int comp );
  virtual double get_tau_syn_in( int comp );

  /**
   * Compute gradient change for eprop synapses.
   *
   * This method is called from an eprop synapse on the eprop target neuron. It updates various parameters related to
   * e-prop plasticity according to Bellec et al. (2020) with additional biological features described in Korcsak-Gorzo,
   * Stapmanns, and Espinoza Valverde et al. (in preparation).
   *
   * @param t_spike [in] Time of the current spike.
   * @param t_spike_previous [in] Time of the previous spike.
   * @param z_previous_buffer [in, out] Value of presynaptic spiking variable from previous time step.
   * @param z_bar [in, out] Filtered presynaptic spiking variable.
   * @param e_bar [in, out] Filtered eligibility trace.
   * @param e_bar_reg [in, out] Filtered eligibility trace for firing rate regularization.
   * @param epsilon [out] Component of eligibility vector corresponding to the adaptive firing threshold variable.
   * @param weight [in, out] Synaptic weight.
   * @param cp [in] Common properties for synapses.
   * @param optimizer [in] Instance of weight optimizer.
   *
   */
  virtual void compute_gradient( const long t_spike,
    const long t_spike_previous,
    double& z_previous_buffer,
    double& z_bar,
    double& e_bar,
    double& e_bar_reg,
    double& epsilon,
    double& weight,
    const CommonSynapseProperties& cp,
    WeightOptimizer* optimizer );

  /**
   * Compute gradient change for eprop synapses.
   *
   * This method is called from an eprop synapse on the eprop target neuron. It updates various parameters related to
   * e-prop plasticity according to Bellec et al. (2020).
   *
   * @param presyn_isis [in, out] Vector of inter-spike intervals.
   * @param t_previous_update [in] Time of the last update.
   * @param t_previous_trigger_spike [in] Time of the last trigger spike.
   * @param kappa [in] Decay factor for the eligibility trace.
   * @param average_gradient [in] Boolean flag determining whether to compute an average of the gradients over the given
   * period.
   *
   * @return Returns the computed gradient value.
   */
  virtual double compute_gradient( std::vector< long >& presyn_isis,
    const long t_previous_update,
    const long t_previous_trigger_spike,
    const double kappa,
    const bool average_gradient );

  /**
   * Modify Event object parameters during event delivery.
   *
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
   *
   * The assignment is done after node creation by the Network class.
   * @see: NodeManager::add_node().
   */
  void set_thread( size_t );

  /**
   * Retrieve the number of the thread to which the node is assigned.
   */
  size_t get_thread() const;

  /**
   * Store the number of the virtual process to which the node is assigned.
   *
   * This is assigned to the node in NodeManager::add_node().
   */
  void set_vp( size_t );

  /**
   * Retrieve the number of the virtual process to which the node is assigned.
   */
  size_t get_vp() const;

  /**
   * Set the model id.
   *
   * This method is called by NodeManager::add_node() when a node is created.
   * @see get_model_id()
   */
  void set_model_id( int );

  /**
   * Execute post-initialization actions in node models.
   *
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
  virtual SignalType sends_signal() const;

  /**
   * @returns type of signal this node consumes
   * used in check_connection to only connect neurons which send / receive
   * compatible information
   */
  virtual SignalType receives_signal() const;

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
  void set_thread_lid( const size_t );

  /**
   * get thread local index
   */
  size_t get_thread_lid() const;

  /**
   * Sets the local device id.
   *
   * Throws an error if used on a non-device node.
   * @see get_local_device_id
   */
  virtual void set_local_device_id( const size_t lsdid );

  /**
   * Gets the local device id.
   *
   * Throws an error if used on a non-device node.
   * @see set_local_device_id
   */
  virtual size_t get_local_device_id() const;

  /**
   * Member of DeprecationWarning class to be used by models if parameters are
   * deprecated.
   */
  DeprecationWarning deprecation_warning;

  /**
   * Set index in node collection; required by ThirdOutBuilder.
   */
  void set_tmp_nc_index( size_t index );

  /**
   * Return and invalidate index in node collection; required by ThirdOutBuilder.
   *
   * @note Not const since it invalidates index in node object.
   */
  size_t get_tmp_nc_index();


private:
  void set_node_id_( size_t ); //!< Set global node id

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
   * Configure state variables depending on runtime information.
   *
   * Overload this method if the node needs to adapt state variables prior to
   * first simulation to runtime information, e.g., the number of incoming
   * connections.
   */
  virtual void init_state_();

  /**
   * Configure persistent internal data structures.
   *
   * Let node configure persistent internal data structures, such as input
   * buffers or ODE solvers, to runtime information prior to first simulation.
   */
  virtual void init_buffers_();

  virtual void set_initialized_();

  Model& get_model_() const;

  //! Mark node as frozen.
  void set_frozen_( bool frozen );

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
  size_t node_id_;

  /**
   * Local id of this node in the thread-local vector of nodes.
   */
  size_t thread_lid_;

  /**
   * Model ID.
   *
   * It is only set for actual node instances, not for instances of class Node
   * representing model prototypes. Model prototypes always have model_id_==-1.
   * @see get_model_id(), set_model_id()
   */
  int model_id_;

  size_t thread_;      //!< thread node is assigned to
  size_t vp_;          //!< virtual process node is assigned to
  bool frozen_;        //!< node shall not be updated if true
  bool initialized_;   //!< state and buffers have been initialized
  bool node_uses_wfr_; //!< node uses waveform relaxation method

  /**
   * Store index in NodeCollection.
   *
   * @note This is only here so that the primary connection builder can inform the ThirdOutBuilder
   * about the index of the target neuron in the targets node collection. This is required for block-based
   * builders.
   *
   * @note Set by set_tmp_nc_index() and invalidated by get_tmp_nc_index().
   */
  size_t tmp_nc_index_;
};

template < typename ConcreteNode >
const ConcreteNode&
Node::downcast( const Node& n )
{

  ConcreteNode const* tp = dynamic_cast< ConcreteNode const* >( &n );
  assert( tp != 0 );
  return *tp;
}

} // namespace

#endif
