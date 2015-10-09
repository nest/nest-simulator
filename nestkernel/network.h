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
#include "event.h"
#include "compose.hpp"
#include "dictdatum.h"
#include <ostream>
#include <cmath>

#include "dirent.h"
#include "errno.h"

#include "sparse_node_array.h"

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
class NodeManager;

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

class Network
{
  friend class VPManager;
  friend class SimulationManager;
  friend class ConnectionBuilderManager;
  friend class EventDeliveryManager;
  friend class MPIManager;
  friend class NodeManager;

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
   * Return the model id for a given model name.
   */
  int get_model_id( const char[] ) const;

  /**
   * Return the Model for a given model ID.
   */
  Model* get_model( index ) const;

  DictionaryDatum get_connector_defaults( index sc );
  void set_connector_defaults( const index sc, const DictionaryDatum& d );


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


  /**
   * @defgroup net_access Network access
   * Functions to access network nodes.
   */

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

private:
  void init_();

  void clear_models_( bool called_from_destructor = false );

  SLIInterpreter& interpreter_;
  ConnectionManager connection_manager_;

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
  Dictionary* modeldict_;        //!< Dictionary for models.
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

  bool dict_miss_is_error_; //!< whether to throw exception on missed dictionary entries

  bool model_defaults_modified_; //!< whether any model defaults have been modified


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

} // namespace

#endif
