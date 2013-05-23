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
#include <bitset>
#include <string>
#include <sstream>
#include <vector>
#include <deque>
#include <utility>
#include "nest.h"
#include "nest_time.h"
#include "nest_names.h"
#include "dictdatum.h"
#include "histentry.h"
#include "event.h"

/** @file node.h
 * Declarations for base class Node
 */

namespace nest {

  class Scheduler;
  class Model;

  class Subnet;
  class Network;
  class Archiving_Node;
  class histentry;
  class Connector;
  class Connection;

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
   * to direcly subclass from base class Node.
   *
   * @see class Event
   * @see Subnet
   * @ingroup user_interface
   */

  /* BeginDocumentation
     Name: Node - General properties of all nodes.
     Parameters:
     frozen     booltype    - Whether the node is updated during simulation
     global_id  integertype - The global id of the node (cf. local_id)
     local      booltype    - Whether the node is available on the local process
     local_id   integertype - The id of the node in the current  (cf. global_id)
     model      literaltype - The model type the node was created from 
     parent     integertype - The global id of the parent subnet
     state      integertype - The state of the node (see the help on elementstates for details)
     thread     integertype - The id of the thread the node is assigned to (valid locally)
     vp         integertype - The id of the virtual process the node is assigned to (valid globally)
     SeeAlso: GetStatus, SetStatus, elementstates
   */
  
  
  class Node
  {
    friend class Network;
    friend class Scheduler;
    friend class Subnet;
    friend class proxynode;
    friend class Synapse;
    friend class Model;
    
    Node& operator=(const Node&); //!< not implemented

  public:

    /**
     * Enumeration for status flags.
     * @see test(flag)
     * @see set(flag)
     * @see unset(flag)
     * @see flip(flag)
     * @todo Review set of states and transition rules, as well as use.
     */
    enum flag
      {
        valid=0,    //!< default element state
        busy,       //!< element is not fully updated
        updated,    //!< update was completed successfully
        suspended,  //!< update was suspended
        frozen,     //!< element or branch is "frozen"
        buffers_initialized, //!< set if buffers are initialized
        err,        //!< some error has occoured
        n_flags
      };

    Node();


    /**
     * The copy constructor assumes that information from
     * the original object is needed to complete the construction.
     * Thus, we merely copy all fields and just set the state flag
     * to "incomplete", to indicate that construction is not
     * ready yet.
     */
    Node(Node const &);
    virtual ~Node();

    /**
     * Virtual copy constructor.
     * This function should create a new object by
     * calling the derived class' copy constructor and
     * return its pointer.
     */
    virtual Node *clone() const{ return 0;}

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
     * Returns name of node model (e.g. "iaf_neuron") as string.
     * This name is identical to the name that is used to identify
     * the model in the interpreter's model dictionary.
     */
    std::string get_name() const;

    virtual 
      void register_connector(nest::Connector&) {}
     

    /**
     * Return global Network ID.
     * Returns the global network ID of the Node.
     * Each node has a unique network ID which can be used to access
     * the Node comparable to a pointer. By definition, the top-level
     * subnet has ID=0.
     */
    index get_gid() const;

    /**
     * Return local node ID.
     * Returns the ID of the node within the parent subject.
     * Local IDs start with 0.
     */
    index get_lid() const;

    /**
     * Return the index to the node in the node array of the parent subnet.
     * @note Since subnets no longer store non-local nodes, LIDs are no
     *       longer identical to these indices.
     */
    index get_subnet_index() const;

    /**
     * Return model ID of the node.
     * Returns the model ID of the model for this node.
     * Model IDs start with 0, Subnet always having ID 0.
     * @note The model ID is not stored in the model prototype instance. 
     *       It is only set when actual nodes are created from a prototype.
     */
    int get_model_id() const;

    /**
     * Return pointer to parent subnet.
     * Each node is member of a subnet whose pointer can be accessed
     * through this function.
     * This pointer must be non NULL for all Nodes which are not the
     * top-level subnet. Only the top-level subnet returns NULL.
     */
    Subnet* get_parent() const;

    /************************************************
     * Functions to modify and test state flags
     */

    /**
     * Return status flag of node.
     * @see enum flag
     */
    index get_status_flag() const;

    /**
     * Clear all status flags of node.
     * @see enum flag
     */
    void reset_status_flag();

    /**
     * Prints out one line of the tree view of the network.
     */
    virtual
    std::string print_network(int , int, std::string = "") {return std::string();}

    /**
     * Flip one or more state flags.
     * @param flag Bit mask, representing the flags to be flipped.
     */
    void flip(flag);

    /**
     * Set one or more state flags.
     * @param flag Bit mask, representing the flags to be set.
     */
    void set(flag);

    /**
     * Unset one or more state flags.
     * @param flag Bit mask, representing the flags to be cleared.
     */
    void unset(flag);

    /**
     * Test one or more state flags.
     * @param flag Bit mask, representing the flags to be tested.
     * @return Returns true, if all flags in the mask are set and false
     * otherwise.
     */
    bool test(flag) const;

    /**
     * Returns true if the node has been updated during the current time-slice.
     * Each simulation step brings all nodes from state $t$ to state $t+dt$.
     * At the beginning of a time slice, the system clock matches the states
     * of all nodes in the network. In this state, is_updated() will return false for
     * all nodes.
     * Each node which is updated has advanced to state $t+dt$. Its local state is ahead of the
     * ststem clock.
     * is_updated() will return true if the node is in state $t+dt$.
     * is_updated() will return false if the node is still in state $t$.
     * @node The state for the updated_ flag is only indirectly coupled to the node's
     * is_updated state.
     */
    bool is_updated() const;

    /**
     * Returns true if frozen flag is true.
     */
    bool is_frozen() const;

    /**
     * Return pointer to network driver class.
     * @todo This member should return a reference, not a pointer.
     */
    static Network* network();

    /**
     * Returns true if the node is allocated in the local process.
     */
    bool is_local() const;

    /**
     * Set state variables to the default values for the model.
     * Dynamic variables are all observable state variables of a node 
     * that change during Node::update().
     * After calling init_state(), the state variables 
     * should have the same values that they had after the node was
     * created. In practice, they will be initialized to the values
     * of the prototype node (model).
     * @note If the parameters of the model have been changes since the node
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
     * This is a wrapper function, which calls the overloaded Node::init_buffers_()
     * worker only if the buffers of the node have not been initialized yet. It
     * then sets the buffers_initialized flag.
     */
     void init_buffers();

    /**
     * Re-calculate dependent parameters of the node.
     * This function is called each time a simulation is begun/resumed.
     * It must re-calculate all internal Variables of the node required
     * for spike handling or updating the node. 
     * 
     */
    virtual void calibrate()=0;
  
    /**
     * Finalize node.
     * Override this function if a node needs to "wrap up" things after a simulation,
     * i.e., before Scheduler::resume() returns. Typical use-cases are devices
     * that need to flush buffers or disconnect from external files or pipes.
     */
    virtual void finalize() {}
    
    /**
     * Bring the node from state $t$ to $t+n*dt$.
     *
     * n->update(T, from, to) performs the update steps beginning
     * at T+from .. T+to-1, ie, emitting events with time stamps
     * T+from+1 .. T+to.
     *
     * @param Time   network time at beginning of time slice.
     * @param long_t initial step inside time slice
     * @param long_t post-final step inside time slice
     *
     */
    virtual 
    void update(Time const &, const long_t, const long_t)=0;


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
    virtual 
    void set_status(const DictionaryDatum&)=0;

    /**
     * Export properties of the node by setting
     * entries in the status dictionary.
     * @param d Dictionary.
     * @ingroup status_interface
     */
    virtual 
    void get_status(DictionaryDatum&) const=0;

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
     * This function checks if the receiver accepts the connection by creating an
     * instance of the event type it sends in its update() function and passing it
     * to connect_sender() of the receiver. Afterwards it has to return the event
     * so that the Connection object can check if it supports the type of event.
     */
    virtual
    port check_connection(Connection& c, port receptor);

    /**
     * Register a STDP connection
     *
     * @throws IllegalConnection
     *
     */
    virtual
    void register_stdp_connection(double_t);

    /**
     * Unregister a STDP connection
     *
     * @throws IllegalConnection
     *
     */
    virtual
    void unregister_stdp_connection(double_t);



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
    virtual
    void handle(SpikeEvent& e);

    /**
     * Request a connection with the receiver.
     * @param e Fully initialised event object.
     * @retval Receiver port number, or 0 if unused.
     * A negative receiver port is a request to the sender
     * to cancel the connection without issueing an error.
     *
     * @throws IllegalConnection
     *
     * connect_sender() function is used to verify that the receiver
     * can handle the event. It can also be used by the receiver to
     * store information about an event sender.
     * The default definition  throws an IllegalConnection
     * exception.  Any node class should define empty connect_sender
     * functions for all those event types it can handle.
     *
     * connect_sender() is called by the register_connection() method
     * of the respective connector object.
     *
     * @note The semantics of all other connect_sender() functions is identical.
     * @ingroup event_interface
     * @throws IllegalConnection
     */
    virtual
    port connect_sender(SpikeEvent& e, port receptor);

    /**
     * Handler for rate events.
     * @see handle(SpikeEvent&)
     * @ingroup event_interface
     * @throws UnexpectedEvent
     */
    virtual
    void handle(RateEvent& e);

    /**
     * Establish connection for rate events (receiver).
     * @see connect_sender(SpikeEvent&)
     * @ingroup event_interface
     */
    virtual
    port connect_sender(RateEvent& e, port receptor);

    /**
     * Handler for universal data logging request.
     * @see handle(SpikeEvent&)
     * @ingroup event_interface
     * @throws UnexpectedEvent
     */
    virtual
    void handle(DataLoggingRequest& e);

    /**
     * Establish connection for data logging request (receiver).
     * @see connect_sender(SpikeEvent&, port)
     * @ingroup event_interface
     * @throws IllegalConnection
     */
    virtual
    port connect_sender(DataLoggingRequest&, port);

    /**
     * Handler for universal data logging request.
     * @see handle(SpikeEvent&)
     * @ingroup event_interface
     * @throws UnexpectedEvent
     * @note There is no connect_sender() for DataLoggingReply, since
     *       this event is only used as "back channel" for DataLoggingRequest.
     */
    virtual
    void handle(DataLoggingReply& e);

    /**
     * Handler for current events.
     * @see handle(thread, SpikeEvent&)
     * @ingroup event_interface
     * @throws UnexpectedEvent
     */
    virtual
    void handle(CurrentEvent& e);

    /**
     * Establish connection for current events (receiver).
     * @see connect_sender(SpikeEvent&)
     * @ingroup event_interface
     * @throws IllegalConnection
     */
    virtual
    port connect_sender(CurrentEvent& e, port receptor);

    /**
     * Handler for conductance events.
     * @see handle(thread, SpikeEvent&)
     * @ingroup event_interface
     * @throws UnexpectedEvent
     */
    virtual
    void handle(ConductanceEvent& e);

    /**
     * Establish connection for conductance events (receiver).
     * @see connect_sender(SpikeEvent&)
     * @ingroup event_interface
     * @throws IllegalConnection
     */
    virtual
    port connect_sender(ConductanceEvent& e, port receptor);

    /**
     * Handler for DoubleData events.
     * @see handle(thread, SpikeEvent&)
     * @ingroup event_interface
     * @throws UnexpectedEvent
     */
    virtual
    void handle(DoubleDataEvent& e);

    /**
     * Establish connection for DoubleData events (receiver).
     * @see connect_sender(SpikeEvent&)
     * @ingroup event_interface
     * @throws IllegalConnection
     */
    virtual
    port connect_sender(DoubleDataEvent& e, port receptor);

    /**
     * return the Kminus value at t (in ms).
     * @throws UnexpectedEvent
     */
     virtual
     double_t get_K_value(double_t t); 

    /**
     * write the Kminus and triplet_Kminus values at t (in ms) to
     * the provided locations.
     * @throws UnexpectedEvent
     */
     virtual
     void get_K_values(double_t t, double_t& Kminus, double_t& triplet_Kminus); 

     /**
     * return the spike history for (t1,t2].
     * @throws UnexpectedEvent
     */
     virtual
     void get_history(double_t t1, double_t t2, 
                   std::deque<histentry>::iterator* start,
  		   std::deque<histentry>::iterator* finish);

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
    virtual
    void event_hook(DSSpikeEvent&);

    virtual
    void event_hook(DSCurrentEvent&);

    /**
     * Store the number of the thread to which the node is assigned.
     * The assignment is done after node creation by the Network class.
     * @see: Network::add_node().
     */
    void set_thread(thread);

    /**
     * Retrieve the number of the thread to which the node is assigned.
     */
    thread get_thread() const;

    /**
     * Store the number of the virtual process to which the node is assigned.
     * This is assigned to the node in network::add_node().
     */
    void set_vp(thread);

    /**
     * Retrieve the number of the virtual process to which the node is assigned.
     */
    thread get_vp() const;

    /** Set the model id.
     * This method is called by Network::add_node() when a node is created.
     * @see get_model_id()
     */
    void  set_model_id(int);

    /**
     * @returns true if node can be entered with the NestModule::ChangeSubnet() 
     *          commands (only true for Subnets).
     */
    virtual bool allow_entry() const;

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
      * set_status_base() manages the frozen flag of the node itself
      * and then invokes the set_status() of the derived class.
      * @internal
      */
     void set_status_base(const DictionaryDatum&);

     /**
      * Returns true if node is model prototype.
      */
     bool is_model_prototype() const;

  private:

    void  set_lid_(index);         //!< Set local id, relative to the parent subnet
    void  set_parent_(Subnet *);   //!< Set pointer to parent subnet.
    void  set_gid_(index);         //!< Set global node id
    void  set_subnet_index_(index);//!< Index into node array in subnet

    /** Return a new dictionary datum .
     *   
     * This function is called by get_status_base() and returns a new
     * empty dictionary by default.  Some nodes may contain a
     * permanent status dictionary which is then returned by
     * get_status_dict_().
     */ 
    virtual
    DictionaryDatum get_status_dict_();

  protected:

    /**
     * Return the number of thread siblings in SiblingContainer.
     *
     * This method is meaningful only for SiblingContainer, for which it
     * returns the number of siblings in the container.
     * For all other models (including Subnet), it returns 0, which is not
     * wrong. By defining the method in this way, we avoid many dynamic casts.
     */
    virtual
    size_t num_thread_siblings_() const { return 0;}

    /**
     * Return the specified member of a SiblingContainer.
     *
     * This method is meaningful only for SiblingContainer, for which it
     * returns the pointer to the indexed node in the container.
     * For all other models (including Subnet), it returns a null pointer
     * and throws and assertion.By defining the method in this way, we avoid
     * many dynamic casts.
     */
    virtual
    Node* get_thread_sibling_(index) const { assert(false); return 0; }

    /**
     * Return specified member of a SiblingContainer, with access control.
     */
    virtual
    Node* get_thread_sibling_safe_(index) const { assert(false); return 0; }

     /**
      * Private function to initialize the state of a node to model defaults.
      * This function, which must be overloaded by all derived classes, provides
      * the implementation for initializing the state of a node to the model defaults;
      * the state is the set of observable dynamic variables. 
      * @param Reference to model prototype object.
      * @see Node::init_state()
      * @note To provide a reasonable behavior during the transition to the new scheme,
      *       init_state_() has a default implementation calling init_dynamic_state_().
      */
     virtual void init_state_(Node const&) =0;
     
     /**
      * Private function to initialize the buffers of a node.
      * This function, which must be overloaded by all derived classes, provides
      * the implementation for initializing the buffers of a node.
      * @see Node::init_buffers()
      */
     virtual void init_buffers_() =0;

    Model & get_model_() const;

    /**
     * Auxiliary function to downcast a Node to a concrete class derived from Node.
     * @note This function is used to convert generic Node references to specific
     *       ones when intializing parameters or state from a prototype.
     */
    template <typename ConcreteNode>
    const ConcreteNode& downcast(const Node&);

   private:
    index    gid_;           //!< Global element id (within network).
    index    lid_;           //!< Local element id (within parent).
    index    subnet_index_;  //!< Index of node in parent's node array
    
    /**
     * Model ID.
     * @see get_model_id(), set_model_id()
     */
    int      model_id_;      
    Subnet *parent_;              //!< Pointer to parent.
    std::bitset<n_flags> stat_;   //!< enum flag as bit mask.

    thread   thread_;        //!< thread node is assigned to
    thread   vp_;            //!< virtual process node is assigned to
      
  protected:
    static Network* net_;    //!< Pointer to global network driver.
  };

  
  inline
  index Node::get_status_flag() const
  {
    return stat_.to_ulong();
  }

  inline
  void Node::reset_status_flag()
  {
    stat_.reset();
  }
  
  inline
  bool Node::test(enum flag f) const
  {
    return stat_.test(f);
  }

  inline
  void Node::set(enum flag f)
  {
    stat_.set(f);
  }

  inline
  void Node::unset(enum flag f)
  {
    stat_.reset(f);
  }

  inline
  void Node::flip(enum flag f)
  {
    stat_[f].flip();
  }

  inline
  bool Node::is_frozen() const
  {
    return stat_.test(frozen);
  }

  inline
  bool Node::has_proxies() const
  {
    return true;
  }

  inline
  bool Node::local_receiver() const
  {
    return false;
  }

  inline
  bool Node::one_node_per_process() const
  {
    return false;
  }

  inline
  bool Node::is_off_grid() const
  {
    return false;
  }

  inline
  bool Node::is_proxy() const
  {
    return false;
  }

  inline
  index Node::get_lid() const
  {
    return lid_;
  }

  inline
  index Node::get_gid() const
  {
    return gid_;
  }

  inline
  index Node::get_subnet_index() const
  {
    return subnet_index_;
  }

  inline
  void Node::set_gid_(index i)
  {
    gid_=i;
  }

  inline
  void Node::set_lid_(index i)
  {
    lid_=i;
  }

  inline
  void Node::set_subnet_index_(index i)
  {
    subnet_index_ = i;
  }

  inline
  int Node::get_model_id() const
  {
    return model_id_;
  }

  inline
  void Node::set_model_id(int i)
  {
    model_id_ = i;
  }

  inline
  bool Node::is_model_prototype() const
  {
    return vp_ == invalid_thread_;
  }

  inline
  Subnet * Node::get_parent() const
  {
    return parent_;
  }

  inline
  void Node::set_parent_(Subnet *c)
  {
    parent_=c;
  }

  inline
  Network* Node::network()
  {
    return net_;
  }

  inline
  void Node::set_thread(thread t)
  {
    thread_ = t;
  }

  inline
  thread Node::get_thread() const
  {
    return thread_;
  }

  inline
  void Node::set_vp(thread vp)
  {
    vp_ = vp;
  }

  inline
  thread Node::get_vp() const
  {
    return vp_;
  }

  template <typename ConcreteNode>
  const ConcreteNode& Node::downcast(const Node& n)
  {
    ConcreteNode const* tp = dynamic_cast<ConcreteNode const*>(&n);
    assert(tp != 0);
    return *tp;
  }
  
} // namespace

#endif
