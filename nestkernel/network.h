/*
 *  network.h
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

#ifndef NETWORK_H
#define NETWORK_H
#include "config.h"
#include <vector>
#include <string>
#include <typeinfo>
#include "nest.h"
#include "model.h"
#include "scheduler.h"
#include "exceptions.h"
#include "proxynode.h"
#include "connection_manager.h"
#include "event.h"
#include "modelrangemanager.h"
#include "compose.hpp"
#include "dictdatum.h"
#include "numerics.h"
#include <ostream>
#include <cmath>

#include "dirent.h"
#include "errno.h"

#include "sparse_node_array.h"

#include "growth_curve_factory.h"

#ifdef M_ERROR
#undef M_ERROR
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef HAVE_MUSIC
#include "music_event_handler.h"
#endif

/**
 * @file network.h
 * Declarations for class Network.
 */
class TokenArray;
class SLIInterpreter;

namespace nest
{
class Subnet;
class SiblingContainer;
class Event;
class Node;
class GenericConnBuilderFactory;
class GenericGrowthCurveFactory;
class GIDCollection;

/**
 * @defgroup network Network access and administration
 * @brief Network administration and scheduling.
 * This module contains all classes which are involved in the
 * administration of the Network and the scheduling during
 * simulation.
 */

/**
 * Main administrative interface to the network.
 * Class Network is responsible for
 * -# Administration of Model objects.
 * -# Administration of network Nodes.
 * -# Administration of the simulation time.
 * -# Update and scheduling during simulation.
 * -# Memory cleanup at exit.
 *
 * @see Node
 * @see Model
 * @ingroup user_interface
 * @ingroup network
 */

/* BeginDocumentation
Name: kernel - Global properties of the simulation kernel.

Description:
Global properties of the simulation kernel.

Parameters:
  The following parameters are available in the kernel status dictionary.

  Time and resolution
  resolution               doubletype  - The resolution of the simulation (in ms)
  time                     doubletype  - The current simulation time
  to_do                    integertype - The number of steps yet to be simulated (read only)
  max_delay                doubletype  - The maximum delay in the network
  min_delay                doubletype  - The minimum delay in the network
  ms_per_tic               doubletype  - The number of milliseconds per tic
  tics_per_ms              doubletype  - The number of tics per millisecond
  tics_per_step            integertype - The number of tics per simulation time step
  T_max                    doubletype  - The largest representable time value (read only)
  T_min                    doubletype  - The smallest representable time value (read only)

  Parallel processing
  total_num_virtual_procs  integertype - The total number of virtual processes
  local_num_threads        integertype - The local number of threads
  num_processes            integertype - The number of MPI processes (read only)
  num_rec_processes        integertype - The number of MPI processes reserved for recording spikes
  num_sim_processes        integertype - The number of MPI processes reserved for simulating neurons
  off_grid_spiking         booltype    - Whether to transmit precise spike times in MPI
                                         communication (read only)

  Random number generators
  grng_seed                integertype - Seed for global random number generator used
                                         synchronously by all virtual processes to
                                         create, e.g., fixed fan-out connections
                                         (write only).
  rng_seeds                arraytype   - Seeds for the per-virtual-process random
                                         number generators used for most purposes.
                                         Array with one integer per virtual process,
                                         all must be unique and differ from
                                         grng_seed (write only).

  Output
  data_path                stringtype  - A path, where all data is written to
                                         (default is the current directory)
  data_prefix              stringtype  - A common prefix for all data files
  overwrite_files          booltype    - Whether to overwrite existing data files
  print_time               booltype    - Whether to print progress information during the simulation

  Network information
  network_size             integertype - The number of nodes in the network (read only)
  num_connections          integertype - The number of connections in the network
                                         (read only, local only)

  Miscellaneous
  dict_miss_is_error       booltype    - Whether missed dictionary entries are treated as errors
  prelim_tol		   doubletype  - Tolerance of prelim iterations
  prelim_interpolation_order integertype - Interpolation order of polynomial used in prelim
                                           iterations

SeeAlso: Simulate, Node
*/

class Network
{
  friend class Scheduler;

public:
  Network( SLIInterpreter& );
  ~Network();

  /**
   * Reset deletes all nodes and reallocates all memory pools for
   * nodes.
   */
  void reset();

  /**
   * Reset number of threads to one, reset device prefix to the
   * empty string and call reset().
   */
  void reset_kernel();

  /**
   * Reset the network to the state at T = 0.
   */
  void reset_network();

  /**
   * Registers a fundamental model for use with the network.
   * @param   m     Model object.
   * @param   private_model  If true, model is not entered in modeldict.
   * @return void
   * @note The Network calls the Model object's destructor at exit.
   * @see register_model
   */
  void register_basis_model( Model& m, bool private_model = false );

  /**
   * Register a built-in model for use with the network.
   * Also enters the model in modeldict, unless private_model is true.
   * @param   m     Model object.
   * @param   private_model  If true, model is not entered in modeldict.
   * @return Model ID assigned by network
   * @note The Network calls the Model object's destructor at exit.
   */
  index register_model( Model& m, bool private_model = false );

  /**
   * Copy an existing model and register it as a new model.
   * This function allows users to create their own, cloned models.
   * @param old_id The id of the existing model.
   * @param new_name The name of the new model.
   * @retval Index, identifying the new Model object.
   * @see copy_synapse_prototype()
   */
  index copy_model( index old_id, std::string new_name );

  /**
   * Register a synapse prototype at the connection manager.
   */
  synindex register_synapse_prototype( ConnectorModel* cf );

  /**
   * Register a synapse prototype for a secondary synapse at the connection manager.
   */
  synindex register_secondary_synapse_prototype( ConnectorModel* cf );

  /**
   * Copy an existing synapse type.
   * @see copy_model(), ConnectionManager::copy_synapse_prototype()
   */
  int copy_synapse_prototype( index sc, std::string );

  /**
   * Add a connectivity rule, i.e. the respective ConnBuilderFactory.
   */
  template < typename ConnBuilder >
  void register_conn_builder( const std::string& name );

  /**
   * Add a growth curve for MSP
   */
  template < typename GrowthCurve >
  void register_growth_curve( const std::string& name );

  /**
   * Create a new Growth Curve object using the GrowthCurve Factory
   * @param name which defines the type of GC to be created
   * @return a new Growth Curve object of the type indicated by name
   */
  GrowthCurve* new_growth_curve( Name name );

  /**
   * Return the model id for a given model name.
   */
  int get_model_id( const char[] ) const;

  /**
   * Return the Model for a given model ID.
   */
  Model* get_model( index ) const;

  /**
   * Return the Model for a given GID.
   */
  Model* get_model_of_gid( index );

  /**
   * Return the Model ID for a given GID.
   */
  index get_model_id_of_gid( index );

  /**
   * Return the contiguous range of ids of nodes with the same model
   * than the node with the given GID.
   */
  const modelrange& get_contiguous_gid_range( index gid ) const;

  /**
   * Add a number of nodes to the network.
   * This function creates n Node objects of Model m and adds them
   * to the Network at the current position.
   * @param m valid Model ID.
   * @param n Number of Nodes to be created. Defaults to 1 if not
   * specified.
   * @throws nest::UnknownModelID
   */
  index add_node( index m, long_t n = 1 );

  /**
   * Restore nodes from an array of status dictionaries.
   * The following entries must be present in each dictionary:
   * /model - with the name or index of a neuron mode.
   *
   * The following entries are optional:
   * /parent - the node is created in the parent subnet
   *
   * Restore nodes uses the current working node as root. Thus, all
   * GIDs in the status dictionaties are offset by the GID of the current
   * working node. This allows entire subnetworks to be copied.
   */
  void restore_nodes( ArrayDatum& );

  /**
   * Set the state (observable dynamic variables) of a node to model defaults.
   * @see Node::init_state()
   */
  void init_state( index );

  /**
   * Return total number of network nodes.
   * The size also includes all Subnet objects.
   */
  index size() const;

  /**
   * Connect two nodes. The source node is defined by its global ID.
   * The target node is defined by the node. The connection is
   * established on the thread/process that owns the target node.
   *
   * The parameters delay and weight have the default value NAN.
   * NAN is a special value in cmath, which describes double values that
   * are not a number. If delay or weight is omitted in a connect call,
   * NAN indicates this and weight/delay are set only, if they are valid.
   *
   * \param s GID of the sending Node.
   * \param target Pointer to target Node.
   * \param target_thread Thread that hosts the target node.
   * \param syn The synapse model to use.
   * \param d Delay of the connection (in ms).
   * \param w Weight of the connection.
   */
  void connect( index s,
    Node* target,
    thread target_thread,
    index syn,
    double_t d = numerics::nan,
    double_t w = numerics::nan );

  /**
   * Connect two nodes. The source node is defined by its global ID.
   * The target node is defined by the node. The connection is
   * established on the thread/process that owns the target node.
   *
   * The parameters delay and weight have the default value NAN.
   * NAN is a special value in cmath, which describes double values that
   * are not a number. If delay or weight is omitted in an connect call,
   * NAN indicates this and weight/delay are set only, if they are valid.
   *
   * \param s GID of the sending Node.
   * \param target Pointer to target Node.
   * \param target_thread Thread that hosts the target node.
   * \param syn The synapse model to use.
   * \param params parameter dict to configure the synapse
   * \param d Delay of the connection (in ms).
   * \param w Weight of the connection.
   */
  void connect( index s,
    Node* target,
    thread target_thread,
    index syn,
    DictionaryDatum& params,
    double_t d = numerics::nan,
    double_t w = numerics::nan );

  /**
   * Connect two nodes. The source node is defined by its global ID.
   * The target node is defined by the node. The connection is
   * established on the thread/process that owns the target node.
   *
   * \param s GID of the sending Node.
   * \param target pointer to target Node.
   * \param target_thread thread that hosts the target node
   * \param params parameter dict to configure the synapse
   * \param syn The synapse model to use.
   */
  bool connect( index s, index r, DictionaryDatum& params, index syn );

  void subnet_connect( Subnet&, Subnet&, int, index syn );

  /**
   * Connect from an array of dictionaries.
   */
  void connect( ArrayDatum& connectome );

  void divergent_connect( index s,
    const TokenArray r,
    const TokenArray weights,
    const TokenArray delays,
    index syn );
  /**
   * Connect one source node with many targets.
   * The dictionary d contains arrays for all the connections of type syn.
   */

  void divergent_connect( index s, DictionaryDatum d, index syn );

  void random_divergent_connect( index s,
    const TokenArray r,
    index n,
    const TokenArray w,
    const TokenArray d,
    bool,
    bool,
    index syn );

  void convergent_connect( const TokenArray s,
    index r,
    const TokenArray weights,
    const TokenArray delays,
    index syn );

  /**
   * Specialized version of convegent_connect
   * called by random_convergent_connect threaded
   */
  void convergent_connect( const std::vector< index >& s_id,
    index r,
    const TokenArray& weight,
    const TokenArray& delays,
    index syn );

  void random_convergent_connect( const TokenArray s,
    index t,
    index n,
    const TokenArray w,
    const TokenArray d,
    bool,
    bool,
    index syn );

  /**
   * Use openmp threaded parallelization to speed up connection.
   * Parallelize over target list.
   */
  void random_convergent_connect( TokenArray s,
    TokenArray t,
    TokenArray n,
    TokenArray w,
    TokenArray d,
    bool,
    bool,
    index syn );

  /**
   * Create connections.
   */
  void connect( const GIDCollection&,
    const GIDCollection&,
    const DictionaryDatum&,
    const DictionaryDatum& );

  DictionaryDatum get_connector_defaults( index sc );
  void set_connector_defaults( index sc, DictionaryDatum& d );

  DictionaryDatum get_synapse_status( index gid, index syn, port p, thread tid );
  void set_synapse_status( index gid, index syn, port p, thread tid, DictionaryDatum& d );

  ArrayDatum get_connections( DictionaryDatum dict );

  Subnet* get_root() const; ///< return root subnet.
  Subnet* get_cwn() const;  ///< current working node.

  /**
   * Change current working node. The specified node must
   * exist and be a subnet.
   * @throws nest::IllegalOperation Target is no subnet.
   */
  void go_to( index );

  void simulate( Time const& );
  /**
   * Resume the simulation after it was terminated.
   */
  void resume();

  /**
   * Terminate the simulation after the time-slice is finished.
   */
  void terminate();

  /**
   * Return true if NEST will be quit because of an error, false otherwise.
   */
  bool quit_by_error() const;

  /**
   * Return the exitcode that would be returned to the calling shell
   * if NEST would quit now.
   */
  int get_exitcode() const;

  void memory_info();

  void print( index, int );

  /**
   * Triggered by volume transmitter in update.
   * Triggeres updates for all connectors of dopamine synapses that
   * are registered with the volume transmitter with gid vt_gid.
   */
  void trigger_update_weight( const long_t vt_gid,
    const vector< spikecounter >& dopa_spikes,
    const double_t t_trig );

  /**
   * Standard routine for sending events. This method decides if
   * the event has to be delivered locally or globally. It exists
   * to keep a clean and unitary interface for the event sending
   * mechanism.
   * @note Only specialization for SpikeEvent does remote sending.
   *       Specialized for DSSpikeEvent to avoid that these events
   *       are sent to remote processes.
   * \see send_local()
   */
  template < class EventT >
  void send( Node& source, EventT& e, const long_t lag = 0 );


  /**
   * Send a secondary event.
   */
  void send_secondary( Node& source, SecondaryEvent& e );


  /**
   * Send event e to all targets of node source on thread t
   */
  void send_local( thread t, Node& source, Event& e );

  /**
   * Send event e directly to its target node. This should be
   * used only where necessary, e.g. if a node wants to reply
   * to a *RequestEvent immediately.
   */
  void send_to_node( Event& e );

  /**
   * Return minimal connection delay.
   */
  delay get_min_delay() const;

  /**
   * Return maximal connection delay.
   */
  delay get_max_delay() const;

  size_t get_prelim_interpolation_order() const;
  double_t get_prelim_tol() const;

  /**
   * Get the time at the beginning of the current time slice.
   */
  Time const& get_slice_origin() const;

  /**
   * Get the time at the beginning of the previous time slice.
   */
  Time get_previous_slice_origin() const;

  /**
   * Get the current simulation time.
   * Defined only while no simulation in progress.
   */
  Time const get_time() const;

  /**
   * Get random number client of a thread.
   * Defaults to thread 0 to allow use in non-threaded
   * context.  One may consider to introduce an additional
   * RNG just for the non-threaded context.
   */
  librandom::RngPtr get_rng( thread thrd = 0 ) const;

  /**
   * Get global random number client.
   * This grng must be used synchronized from all threads.
   */
  librandom::RngPtr get_grng() const;

  /**
   * Get number of threads.
   * This function returns the total number of threads per process.
   */
  thread get_num_threads() const;

  /**
   * Suggest a VP for a given global node ID
   */
  thread suggest_vp( index ) const;

  /**
   * Suggest a VP for a given global recording node ID
   */
  thread suggest_rec_vp( index ) const;

  /**
   * Convert a given VP ID to the corresponding thread ID
   */
  thread vp_to_thread( thread vp ) const;

  /**
   * Convert a given thread ID to the corresponding VP ID
   */
  thread thread_to_vp( thread t ) const;

  /**
   * Get number of processes.
   */
  thread get_num_processes() const;

  /**
   * Get number of recording processes.
   */
  thread get_num_rec_processes() const;

  /**
   * Get number of simulating processes.
   */
  thread get_num_sim_processes() const;

  /**
   * Set number of recording processes.
   */
  void set_num_rec_processes( int nrp );

  /**
   * Return true, if the given Node is on the local machine
   */
  bool is_local_node( Node* ) const;

  /**
   * Return true, if the given gid is on the local machine
   */
  bool is_local_gid( index gid ) const;

  /**
   * Return true, if the given VP is on the local machine
   */
  bool is_local_vp( thread ) const;

  /**
   * See Scheduler::get_simulated()
   */
  bool get_simulated() const;

  /**
   * @defgroup net_access Network access
   * Functions to access network nodes.
   */

  /**
   * Return pointer of the specified Node.
   * @param i Index of the specified Node.
   * @param thr global thread index of the Node.
   *
   * @throws nest::UnknownNode       Target does not exist in the network.
   *
   * @ingroup net_access
   */
  Node* get_node( index, thread thr = 0 );

  /**
   * Return the Subnet that contains the thread siblings.
   * @param i Index of the specified Node.
   *
   * @throws nest::NoThreadSiblingsAvailable     Node does not have thread siblings.
   *
   * @ingroup net_access
   */
  const SiblingContainer* get_thread_siblings( index n ) const;

  /**
   * Check, if there are instances of a given model.
   * @param i index of the model to check for
   * @return true, if model is instantiated at least once.
   */
  bool model_in_use( index i );

  /**
   * The prefix for files written by devices.
   * The prefix must not contain any part of a path.
   * @see get_data_dir(), overwrite_files()
   */
  const std::string& get_data_prefix() const;

  /**
   * The path for files written by devices.
   * It may be the empty string (use current directory).
   * @see get_data_prefix(), overwrite_files()
   */
  const std::string& get_data_path() const;

  /**
   * Indicate if existing data files should be overwritten.
   * @return true if existing data files should be overwritten by devices. Default: false.
   */
  bool overwrite_files() const;

  /**
   * return current communication style.
   * A result of true means off_grid, false means on_grid communication.
   */
  bool get_off_grid_communication() const;

  /**
   * Set properties of a Node. The specified node must exist.
   * @throws nest::UnknownNode       Target does not exist in the network.
   * @throws nest::UnaccessedDictionaryEntry  Non-proxy target did not read dict entry.
   * @throws TypeMismatch            Array is not a flat & homogeneous array of integers.
   */
  void set_status( index, const DictionaryDatum& );

  /**
   * Get properties of a node. The specified node must exist.
   * @throws nest::UnknownNode       Target does not exist in the network.
   */
  DictionaryDatum get_status( index );

  /**
   * Execute a SLI command in the neuron's namespace.
   */
  int execute_sli_protected( DictionaryDatum, Name );

  /**
   * Return a reference to the model dictionary.
   */
  const Dictionary& get_modeldict();

  /**
   * Return the synapse dictionary
   */
  const Dictionary& get_synapsedict() const;

  /**
   * Recalibrate scheduler clock.
   */
  void calibrate_clock();

  /**
   * Return 0 for even, 1 for odd time slices.
   *
   * This is useful for buffers that need to be written alternatingly
   * by time slice. The value is given by Scheduler::get_slice_() % 2.
   * @see read_toggle
   */
  size_t write_toggle() const;

  /**
   * Return 1 - write_toggle().
   *
   * This is useful for buffers that need to be read alternatingly
   * by slice. The value is given by 1-write_toggle().
   * @see write_toggle
   */
  size_t read_toggle() const;

  /**
   * Does the network contain copies of models created using CopyModel?
   */
  bool has_user_models() const;

  /**
   * Ensure that all nodes in the network have valid thread-local IDs.
   */
  void
  ensure_valid_thread_local_ids()
  {
    scheduler_.ensure_valid_thread_local_ids();
  }

  /** Display a message. This function displays a message at a
   *  specific error level. Messages with an error level above
   *  M_ERROR will be written to std::cerr in addition to
   *  std::cout.
   *  \n
   *  \n
   *  The message will ony be displayed if the current verbosity level
   *  is greater than or equal to the input level.
   *
   *  @ingroup SLIMessaging
   */
  void message( int level, const char from[], const char text[] );
  void message( int level, const std::string& loc, const std::string& msg );

  /**
   * Returns true if unread dictionary items should be treated as error.
   */
  bool dict_miss_is_error() const;

#ifdef HAVE_MUSIC
public:
  /**
   * Register a MUSIC input port (portname) with the port list.
   * This will increment the counter of the respective entry in the
   * music_in_portlist.
   *
   * The argument pristine should be set to true when a model
   * registers the initial port name. This typically happens when the
   * copy constructor of the model registers a port, as in
   * models/music_event_in_proxy.cpp. Setting pristine = true causes
   * the port to be also added to pristine_music_in_portlist.  See
   * also comment above Network::pristine_music_in_portlist_.
   */
  void register_music_in_port( std::string portname, bool pristine = false );

  /**
   * Unregister a MUSIC input port (portname) from the port list.
   * This will decrement the counter of the respective entry in the
   * music_in_portlist and remove the entry if the counter is 0
   * after decrementing it.
   */
  void unregister_music_in_port( std::string portname );

  /**
   * Register a node (of type music_input_proxy) with a given MUSIC
   * port (portname) and a specific channel. The proxy will be
   * notified, if a MUSIC event is being received on the respective
   * channel and port.
   */
  void register_music_event_in_proxy( std::string portname, int channel, nest::Node* mp );

  /**
   * Set the acceptable latency (latency) for a music input port (portname).
   */
  void set_music_in_port_acceptable_latency( std::string portname, double_t latency );
  void set_music_in_port_max_buffered( std::string portname, int_t maxbuffered );
  /**
   * Data structure to hold variables and parameters associated with a port.
   */
  struct MusicPortData
  {
    MusicPortData( size_t n, double_t latency, int_t m )
      : n_input_proxies( n )
      , acceptable_latency( latency )
      , max_buffered( m )
    {
    }
    MusicPortData()
    {
    }
    size_t n_input_proxies; // Counter for number of music_input proxies
                            // connected to this port
    double_t acceptable_latency;
    int_t max_buffered;
  };

  /**
   * The mapping between MUSIC input ports identified by portname
   * and the corresponding port variables and parameters.
   * @see register_music_in_port()
   * @see unregister_music_in_port()
   */
  std::map< std::string, MusicPortData > music_in_portlist_;

  /**
   * A copy of music_in_portlist_ at the pristine state.
   *
   * This is used to reset music_in_portlist_ to its pristine state in
   * Network::init_ (a default state). Pristine here refers to the
   * initial state of music_in_portlist_ after the loading of the
   * pristine_models_.
   */
  std::map< std::string, MusicPortData > pristine_music_in_portlist_;

  /**
   * The mapping between MUSIC input ports identified by portname
   * and the corresponding MUSIC event handler.
   */
  std::map< std::string, MusicEventHandler > music_in_portmap_;

  /**
   * Publish all MUSIC input ports that were registered using
   * Network::register_music_event_in_proxy().
   */
  void publish_music_in_ports_();

  /**
   * Call update() for each of the registered MUSIC event handlers
   * to deliver all queued events to the target music_in_proxies.
   */
  void update_music_event_handlers_( Time const&, const long_t, const long_t );
#endif

  /**
   * Gets ID of local thread.
   * Returns thread ID if OPENMP is installed
   * and zero otherwise.
   */
  int get_thread_id() const;

  void
  set_model_defaults_modified()
  {
    model_defaults_modified_ = true;
  }
  bool
  model_defaults_modified() const
  {
    return model_defaults_modified_;
  }

  Node* thread_lid_to_node( thread t, targetindex thread_local_id ) const;

private:
  /**
   * Initialize the network data structures.
   * init_() is used by the constructor and by reset().
   * @see reset()
   */
  void init_();
  void destruct_nodes_();
  void clear_models_( bool called_from_destructor = false );

  /**
   * Helper function to set properties on single node.
   * @param node to set properties for
   * @param dictionary containing properties
   * @param if true (default), access flags are called before
   *        each call so Node::set_status_()
   * @throws UnaccessedDictionaryEntry
   */
  void set_status_single_node_( Node&, const DictionaryDatum&, bool clear_flags = true );

  //! Helper function to set device data path and prefix.
  void set_data_path_prefix_( const DictionaryDatum& d );

  SLIInterpreter& interpreter_;
  SparseNodeArray local_nodes_; //!< The network as sparse array of local nodes
  Scheduler scheduler_;
  ConnectionManager connection_manager_;

  Subnet* root_;    //!< Root node.
  Subnet* current_; //!< Current working node (for insertion).

  /* BeginDocumentation
     Name: synapsedict - Dictionary containing all synapse models.
     Description:
     'synapsedict info' shows the contents of the dictionary
     Synapse model names ending with '_hpc' provide minimal memory requirements by using
     thread-local target neuron IDs and fixing the `rport` to 0.
     Synapse model names ending with '_lbl' allow to assign an individual integer label
     (`synapse_label`) to created synapses at the cost of increased memory requirements.
     FirstVersion: October 2005
     Author: Jochen Martin Eppler
     SeeAlso: info
  */
  Dictionary* synapsedict_; //!< Dictionary for synapse models.

  /* BeginDocumentation
     Name: modeldict - dictionary containing all devices and models of NEST
     Description:
     'modeldict info' shows the contents of the dictionary
     SeeAlso: info, Device, RecordingDevice, iaf_neuron, subnet
  */
  Dictionary* modeldict_; //!< Dictionary for models.

  /* BeginDocumentation
     Name: connruledict - dictionary containing all connectivity rules
     Description:
     This dictionary provides the connection rules that can be used
     in Connect.
     'connruledict info' shows the contents of the dictionary.
     SeeAlso: Connect
  */
  Dictionary* connruledict_; //!< Dictionary for connection rules.

  /* BeginDocumentation
     Name: growthcurvedict - growth curves for Model of Structural Plasticity
     Description:
     This dictionary provides indexes for the growth curve factory
  */
  Dictionary* growthcurvedict_; //!< Dictionary for growth rules.

  Model* siblingcontainer_model; //!< The model for the SiblingContainer class

  std::string data_path_;   //!< Path for all files written by devices
  std::string data_prefix_; //!< Prefix for all files written by devices
  bool overwrite_files_;    //!< If true, overwrite existing data files.

  /**
   * The list of clean models. The first component of the pair is a
   * pointer to the actual Model, the second is a flag indicating if
   * the model is private. Private models are not entered into the
   * modeldict.
   */
  std::vector< std::pair< Model*, bool > > pristine_models_;

  std::vector< Model* > models_; //!< The list of available models
  std::vector< std::vector< Node* > >
    proxy_nodes_; //!< Placeholders for remote nodes, one per thread
  std::vector< Node* >
    dummy_spike_sources_; //!< Placeholders for spiking remote nodes, one per thread

  std::vector< GenericConnBuilderFactory* >
    connbuilder_factories_; //! ConnBuilder factories, indexed by connruledict_ elements.

  std::vector< GenericGrowthCurveFactory* >
    growthcurve_factories_; //! GrowthCurve factories, indexed by growthcurvedict_ elements.

  Modelrangemanager node_model_ids_; //!< Records the model id of each neuron in the network

  bool dict_miss_is_error_; //!< whether to throw exception on missed dictionary entries

  bool model_defaults_modified_; //!< whether any model defaults have been modified
};

inline void
Network::terminate()
{
  scheduler_.terminate();
}

inline bool
Network::quit_by_error() const
{
  Token t = interpreter_.baselookup( Name( "systemdict" ) );
  DictionaryDatum systemdict = getValue< DictionaryDatum >( t );
  t = systemdict->lookup( Name( "errordict" ) );
  DictionaryDatum errordict = getValue< DictionaryDatum >( t );
  return getValue< bool >( errordict, "quitbyerror" );
}

inline int
Network::get_exitcode() const
{
  Token t = interpreter_.baselookup( Name( "statusdict" ) );
  DictionaryDatum statusdict = getValue< DictionaryDatum >( t );
  return getValue< long >( statusdict, "exitcode" );
}

inline index
Network::size() const
{
  return local_nodes_.get_max_gid() + 1;
}

inline Node*
Network::thread_lid_to_node( thread t, targetindex thread_local_id ) const
{
  return scheduler_.thread_lid_to_node( t, thread_local_id );
}

inline void
Network::connect( ArrayDatum& connectome )
{
  connection_manager_.connect( connectome );
}

inline DictionaryDatum
Network::get_synapse_status( index gid, index syn, port p, thread tid )
{
  return connection_manager_.get_synapse_status( gid, syn, p, tid );
}

inline void
Network::set_synapse_status( index gid, index syn, port p, thread tid, DictionaryDatum& d )
{
  connection_manager_.set_synapse_status( gid, syn, p, tid, d );
}

inline ArrayDatum
Network::get_connections( DictionaryDatum params )
{
  return connection_manager_.get_connections( params );
}

inline void
Network::set_connector_defaults( index sc, DictionaryDatum& d )
{
  connection_manager_.set_prototype_status( sc, d );
}

inline DictionaryDatum
Network::get_connector_defaults( index sc )
{
  return connection_manager_.get_prototype_status( sc );
}

inline synindex
Network::register_synapse_prototype( ConnectorModel* cm )
{
  return connection_manager_.register_synapse_prototype( cm );
}

inline synindex
Network::register_secondary_synapse_prototype( ConnectorModel* cm )
{
  synindex synid = connection_manager_.register_synapse_prototype( cm );
  // call function in scheduler to register secondary synapse type
  // and create corresponding event
  scheduler_.register_secondary_synapse_prototype( cm, synid );
  return synid;
}

inline int
Network::copy_synapse_prototype( index sc, std::string name )
{
  return connection_manager_.copy_synapse_prototype( sc, name );
}

inline Time const&
Network::get_slice_origin() const
{
  return scheduler_.get_slice_origin();
}

inline Time
Network::get_previous_slice_origin() const
{
  return scheduler_.get_previous_slice_origin();
}

inline Time const
Network::get_time() const
{
  return scheduler_.get_time();
}

inline Subnet*
Network::get_root() const
{
  return root_;
}

inline Subnet*
Network::get_cwn( void ) const
{
  return current_;
}

inline thread
Network::get_num_threads() const
{
  return scheduler_.get_num_threads();
}

inline thread
Network::get_num_processes() const
{
  return scheduler_.get_num_processes();
}

inline thread
Network::get_num_rec_processes() const
{
  return scheduler_.get_num_rec_processes();
}

inline thread
Network::get_num_sim_processes() const
{
  return scheduler_.get_num_sim_processes();
}

inline void
Network::set_num_rec_processes( int nrp )
{
  scheduler_.set_num_rec_processes( nrp );
}

inline bool
Network::is_local_node( Node* n ) const
{
  return !( n->is_proxy() );
}

inline bool
Network::is_local_gid( index gid ) const
{
  return local_nodes_.get_node_by_gid( gid ) != 0;
}

inline bool
Network::is_local_vp( thread t ) const
{
  return scheduler_.is_local_vp( t );
}

inline int
Network::suggest_vp( index gid ) const
{
  return scheduler_.suggest_vp( gid );
}

inline int
Network::suggest_rec_vp( index gid ) const
{
  return scheduler_.suggest_rec_vp( gid );
}

inline thread
Network::vp_to_thread( thread vp ) const
{
  return scheduler_.vp_to_thread( vp );
}

inline thread
Network::thread_to_vp( thread t ) const
{
  return scheduler_.thread_to_vp( t );
}

inline bool
Network::get_simulated() const
{
  return scheduler_.get_simulated();
}

inline delay
Network::get_min_delay() const
{
  return scheduler_.get_min_delay();
}

inline delay
Network::get_max_delay() const
{
  return scheduler_.get_max_delay();
}

inline size_t
Network::get_prelim_interpolation_order() const
{
  return scheduler_.get_prelim_interpolation_order();
}

inline double_t
Network::get_prelim_tol() const
{
  return scheduler_.get_prelim_tol();
}

inline void
Network::trigger_update_weight( const long_t vt_gid,
  const vector< spikecounter >& dopa_spikes,
  const double_t t_trig )
{
  connection_manager_.trigger_update_weight( vt_gid, dopa_spikes, t_trig );
}

template < class EventT >
inline void
Network::send( Node& source, EventT& e, const long_t lag )
{
  e.set_stamp( get_slice_origin() + Time::step( lag + 1 ) );
  e.set_sender( source );
  thread t = source.get_thread();
  index gid = source.get_gid();

  assert( !source.has_proxies() );
  connection_manager_.send( t, gid, e );
}


inline void
Network::send_secondary( Node& source, SecondaryEvent& e )
{
  e.set_stamp( get_slice_origin() + Time::step( 1 ) );
  e.set_sender( source );
  e.set_sender_gid( source.get_gid() );
  thread t = source.get_thread();
  scheduler_.send_remote( t, e );
}


template <>
inline void
Network::send< SpikeEvent >( Node& source, SpikeEvent& e, const long_t lag )
{
  e.set_stamp( get_slice_origin() + Time::step( lag + 1 ) );
  e.set_sender( source );
  thread t = source.get_thread();

  if ( source.has_proxies() )
  {
    if ( source.is_off_grid() )
      scheduler_.send_offgrid_remote( t, e, lag );
    else
      scheduler_.send_remote( t, e, lag );
  }
  else
    send_local( t, source, e );
}

template <>
inline void
Network::send< DSSpikeEvent >( Node& source, DSSpikeEvent& e, const long_t lag )
{

  e.set_stamp( get_slice_origin() + Time::step( lag + 1 ) );
  e.set_sender( source );
  thread t = source.get_thread();

  assert( !source.has_proxies() );
  send_local( t, source, e );
}

inline void
Network::send_local( thread t, Node& source, Event& e )
{
  index sgid = source.get_gid();
  e.set_sender_gid( sgid );
  connection_manager_.send( t, sgid, e );
}

inline void
Network::send_to_node( Event& e )
{
  e();
}

inline void
Network::calibrate_clock()
{
  scheduler_.calibrate_clock();
}

inline size_t
Network::write_toggle() const
{
  return scheduler_.get_slice() % 2;
}

inline size_t
Network::read_toggle() const
{
  // define in terms of write_toggle() to ensure consistency
  return 1 - write_toggle();
}

inline librandom::RngPtr
Network::get_rng( thread t ) const
{
  return scheduler_.get_rng( t );
}

inline librandom::RngPtr
Network::get_grng() const
{
  return scheduler_.get_grng();
}

inline Model*
Network::get_model( index m ) const
{
  if ( m >= models_.size() || models_[ m ] == 0 )
    throw UnknownModelID( m );

  return models_[ m ];
}

inline Model*
Network::get_model_of_gid( index gid )
{
  return models_[ get_model_id_of_gid( gid ) ];
}

inline index
Network::get_model_id_of_gid( index gid )
{
  if ( not node_model_ids_.is_in_range( gid ) )
    throw UnknownNode( gid );

  return node_model_ids_.get_model_id( gid );
}

inline const modelrange&
Network::get_contiguous_gid_range( index gid ) const
{
  return node_model_ids_.get_range( gid );
}

inline const std::string&
Network::get_data_path() const
{
  return data_path_;
}

inline const std::string&
Network::get_data_prefix() const
{
  return data_prefix_;
}

inline bool
Network::overwrite_files() const
{
  return overwrite_files_;
}

inline bool
Network::get_off_grid_communication() const
{
  return scheduler_.get_off_grid_communication();
}

inline const Dictionary&
Network::get_modeldict()
{
  assert( modeldict_ != 0 );
  return *modeldict_;
}

inline const Dictionary&
Network::get_synapsedict() const
{
  assert( synapsedict_ != 0 );
  return *synapsedict_;
}

inline bool
Network::has_user_models() const
{
  return models_.size() > pristine_models_.size();
}

inline bool
Network::dict_miss_is_error() const
{
  return dict_miss_is_error_;
}

typedef lockPTR< Network > NetPtr;

//!< Functor to compare Models by their name.
class ModelComp : public std::binary_function< int, int, bool >
{
  const std::vector< Model* >& models;

public:
  ModelComp( const vector< Model* >& nmodels )
    : models( nmodels )
  {
  }
  bool operator()( int a, int b )
  {
    return models[ a ]->get_name() < models[ b ]->get_name();
  }
};

inline int
Network::get_thread_id() const
{
#ifdef _OPENMP
  return omp_get_thread_num();
#else
  return 0;
#endif
}

inline GrowthCurve*
Network::new_growth_curve( Name name )
{
  const long gc_id = ( *growthcurvedict_ )[ name ];
  return growthcurve_factories_.at( gc_id )->create();
}

} // namespace

#endif
