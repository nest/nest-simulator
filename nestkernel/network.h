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
#include "nest_types.h"
#include "nest_time.h"
#include "model.h"
#include "exceptions.h"
#include "proxynode.h"
#include "connection_manager.h"
#include "modelrange.h"
#include "event.h"
#include "compose.hpp"
#include "dictdatum.h"
#include <ostream>
#include <cmath>

#include "dirent.h"
#include "errno.h"

#include "sparse_node_array.h"
#include "randomgen.h"
#include "communicator.h"

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
class GIDCollection;
class VPManager;

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
(start here.)

Parameters:
  The following parameters can be set in the status dictionary.

  data_path                stringtype  - A path, where all data is written to (default is the
current directory)
  data_prefix              stringtype  - A common prefix for all data files
  dict_miss_is_error       booltype    - Whether missed dictionary entries are treated as errors
  local_num_threads        integertype - The local number of threads (cf. global_num_virt_procs)
  max_delay                doubletype  - The maximum delay in the network
  min_delay                doubletype  - The minimum delay in the network
  ms_per_tic               doubletype  - The number of miliseconds per tic (cf. tics_per_ms,
tics_per_step)
  network_size             integertype - The number of nodes in the network
  num_connections          integertype - The number of connections in the network
  off_grid_spiking         booltype    - Whether to transmit precise spike times in MPI communicatio
  print_time               booltype    - Whether to print progress information during the simulation
  resolution               doubletype  - The resolution of the simulation (in ms)
  tics_per_ms              doubletype  - The number of tics per milisecond (cf. ms_per_tic,
tics_per_step)
  tics_per_step            integertype - The number of tics per simulation time step (cf.
ms_per_tic, tics_per_ms)
  time                     doubletype  - The current simulation time
  total_num_virtual_procs  integertype - The total number of virtual processes (cf.
local_num_threads)
  to_do                    integertype - The number of steps yet to be simulated
  T_max                    doubletype  - The largest representable time value
  T_min                    doubletype  - The smallest representable time value
SeeAlso: Simulate, Node
*/

class Network
{
  friend class VPManager;
  friend class SimulationManager;
  friend class EventDeliveryManager;
  friend class MPIManager;

private:
  Network( SLIInterpreter& );
  static Network* network_instance_;
  static bool created_network_instance_;

  Network( Network const& );        // Don't Implement
  void operator=( Network const& ); // Don't Implement

public:
  /**
   * Create/destroy and access the Network singleton.
   */
  static void create_network( SLIInterpreter& );
  static void destroy_network();
  static Network& get_network();

  ~Network();

  /**
   * Reset deletes all nodes and reallocates all memory pools for
   * nodes.
   * @note Threading parameters as well as random number state
   * are not reset. This has to be done manually.
   */
  void reset();

  /**
   * Reset number of threads to one, reset device prefix to the
   * empty string and call reset().
   */
  void reset_kernel();

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
   * Return the model id for a given model name.
   */
  int get_model_id( const char[] ) const;

  /**
   * Return the Model for a given model ID.
   */
  Model* get_model( index ) const;

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
  void restore_nodes( const ArrayDatum& );

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
    double_t d = NAN,
    double_t w = NAN );

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
    double_t d = NAN,
    double_t w = NAN );

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
  void set_connector_defaults( const index sc, const DictionaryDatum& d );

  DictionaryDatum get_synapse_status( index gid, index syn, port p, thread tid );
  void set_synapse_status( index gid, index syn, port p, thread tid, const DictionaryDatum& d );

  ArrayDatum get_connections( DictionaryDatum dict );

  Subnet* get_root() const; ///< return root subnet.
  Subnet* get_cwn() const;  ///< current working node.

  /**
   * Change current working node. The specified node must
   * exist and be a subnet.
   * @throws nest::IllegalOperation Target is no subnet.
   */
  void go_to( index );

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
   * Return minimal connection delay.
   */
  delay get_min_delay() const;

  /**
   * Return maximal connection delay.
   */
  delay get_max_delay() const;

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
   * Return true, if the given Node is on the local machine
   */
  bool is_local_node( Node* ) const;

  /**
   * Return true, if the given gid is on the local machine
   */
  bool is_local_gid( index gid ) const;

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
   * Calibrate clock after resolution change.
   */
  void calibrate_clock();


  /**
   * Does the network contain copies of models created using CopyModel?
   */
  bool has_user_models() const;

  /**
   * Ensure that all nodes in the network have valid thread-local IDs.
   */
  void ensure_valid_thread_local_ids();


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
   */
  void register_music_in_port( std::string portname );

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

  SLIInterpreter& interpreter_;
  SparseNodeArray local_nodes_; //!< The network as sparse array of local nodes
  ConnectionManager connection_manager_;

  Subnet* root_;    //!< Root node.
  Subnet* current_; //!< Current working node (for insertion).

  /* BeginDocumentation
     Name: synapsedict - Dictionary containing all synapse models.
     Description:
     'synapsedict info' shows the contents of the dictionary
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

  Model* siblingcontainer_model; //!< The model for the SiblingContainer class

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

  bool dict_miss_is_error_; //!< whether to throw exception on missed dictionary entries

  bool model_defaults_modified_; //!< whether any model defaults have been modified

  /************ Previously Scheduler ***************/
public:

  /**
   * Return the process id for a given virtual process. The real process' id
   * of a virtual process is defined by the relation: p = (vp mod P), where
   * P is the total number of processes.
   */
  thread get_process_id( thread vp ) const;

  /**
   * set communication style to off_grid (true) or on_grid
   */
  void set_off_grid_communication( bool off_grid_spiking );

  /**
   * Get slice number. Increased by one for each slice. Can be used
   * to choose alternating buffers.
   */
  size_t get_slice() const;

  /**
   * Return (T+d) mod max_delay.
   */
  delay get_modulo( delay d );

  /**
   * Index to slice-based buffer.
   * Return ((T+d)/min_delay) % ceil(max_delay/min_delay).
   */
  delay get_slice_modulo( delay d );

private:
  /******** Member functions former owned by the scheduler ********/

  void init_scheduler_();

  /**
   * Prepare nodes for simulation and register nodes in node_list.
   * Calls prepare_node_() for each pertaining Node.
   * @see prepare_node_()
   */
  void prepare_nodes();

  /**
   * Initialized buffers, register in list of nodes to update/finalize.
   * @see prepare_nodes()
   */
  void prepare_node_( Node* );

  /**
   * Invoke finalize() on nodes registered for finalization.
   */
  void finalize_nodes();



  void create_rngs_( const bool ctor_call = false );
  void create_grng_( const bool ctor_call = false );

  /**
   * Update delay extrema to current values.
   *
   * Static since it only operates in static variables. This allows it to be
   * called from const-method get_status() as well.
   */
  void update_delay_extrema_();


  /**
   * Create up-to-date vector of local nodes, nodes_vec_.
   *
   * This method also sets the thread-local ID on all local nodes.
   */
  void update_nodes_vec_();


  /**
   * Increment total number of global spike detectors by 1
   */
  void increment_n_gsd();

  /**
   * Get total number of global spike detectors
   */
  index get_n_gsd();

private:
  /******** Member variables former owned by the scheduler ********/
  bool initialized_;
  bool simulating_; //!< true if simulation in progress

  index n_gsd_; //!< Total number of global spike detectors, used for distributing them over
                //!< recording processes

  vector< vector< Node* > > nodes_vec_; //!< Nodelists for unfrozen nodes
  index nodes_vec_network_size_;        //!< Network size when nodes_vec_ was last updated


  bool print_time_; //!< Indicates whether time should be printed during simulations (or not)

  std::vector< long_t > rng_seeds_; //!< The seeds of the local RNGs. These do not neccessarily
                                    //!< describe the state of the RNGs.

  long_t
    grng_seed_; //!< The seed of the global RNG, not neccessarily describing the state of the GRNG.

  delay min_delay_; //!< Value of the smallest delay in the network.

  delay max_delay_; //!< Value of the largest delay in the network in steps.

  /**
   * Vector of random number generators for threads.
   * There must be PRECISELY one rng per thread.
   */
  vector< librandom::RngPtr > rng_;

  /**
   * Global random number generator.
   * This rng must be synchronized on all threads
   */
  librandom::RngPtr grng_;
};

inline Network&
Network::get_network()
{
  assert( created_network_instance_ );
  return *network_instance_;
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
  return nodes_vec_[ t ][ thread_local_id ];
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
Network::set_synapse_status( index gid, index syn, port p, thread tid, const DictionaryDatum& d )
{
  connection_manager_.set_synapse_status( gid, syn, p, tid, d );
}

inline ArrayDatum
Network::get_connections( DictionaryDatum params )
{
  return connection_manager_.get_connections( params );
}

inline void
Network::set_connector_defaults( const index sc, const DictionaryDatum& d )
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

inline int
Network::copy_synapse_prototype( index sc, std::string name )
{
  return connection_manager_.copy_synapse_prototype( sc, name );
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

inline bool
Network::is_local_gid( index gid ) const
{
  return local_nodes_.get_node_by_gid( gid ) != 0;
}

inline delay
Network::get_min_delay() const
{
  return min_delay_;
}

inline delay
Network::get_max_delay() const
{
  return max_delay_;
}

inline void
Network::trigger_update_weight( const long_t vt_gid,
  const vector< spikecounter >& dopa_spikes,
  const double_t t_trig )
{
  connection_manager_.trigger_update_weight( vt_gid, dopa_spikes, t_trig );
}

inline librandom::RngPtr
Network::get_rng( thread t ) const
{
  assert( t < static_cast< thread >( rng_.size() ) );
  return rng_[ t ];
}

inline librandom::RngPtr
Network::get_grng() const
{
  return grng_;
}

inline Model*
Network::get_model( index m ) const
{
  if ( m >= models_.size() || models_[ m ] == 0 )
    throw UnknownModelID( m );

  return models_[ m ];
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

/****** former Scheduler functions ******/

inline void
Network::prepare_node_( Node* n )
{
  // Frozen nodes are initialized and calibrated, so that they
  // have ring buffers and can accept incoming spikes.
  n->init_buffers();
  n->calibrate();
}

inline void
Network::increment_n_gsd()
{
  ++n_gsd_;
}

inline index
Network::get_n_gsd()
{
  return n_gsd_;
}

inline void
Network::ensure_valid_thread_local_ids()
{
  update_nodes_vec_();
}

} // namespace

#endif
