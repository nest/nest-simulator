/*
 *  connection_manager.h
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

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

// C++ includes:
#include <string>
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"
#include "sparsetable.h"

// Includes from nestkernel:
#include "connection_id.h"
#include "conn_builder.h"
#include "gid_collection.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "nest_types.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictdatum.h"

namespace nest
{
class ConnectorBase;
class GenericConnBuilderFactory;
class spikecounter;
class Node;
class Subnet;
class Event;
class SecondaryEvent;
class DelayChecker;
class GrowthCurve;

// for all neurons having targets
typedef google::sparsetable< ConnectorBase* > tSConnector;
typedef std::vector< tSConnector > tVSConnector; // for all threads

// each thread checks delays themselve
typedef std::vector< DelayChecker > tVDelayChecker;

typedef std::vector< size_t > tVCounter; // each synapse type has a counter
// and each threads counts for all its synapses
typedef std::vector< tVCounter > tVVCounter;

class ConnectionManager : public ManagerInterface
{
  friend class SimulationManager; // update_delay_extrema_
public:
  ConnectionManager();
  virtual ~ConnectionManager();

  virtual void initialize();
  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  DictionaryDatum& get_connruledict();

  /**
   * Add a connectivity rule, i.e. the respective ConnBuilderFactory.
   */
  template < typename ConnBuilder >
  void register_conn_builder( const std::string& name );

  ConnBuilder* get_conn_builder( const std::string& name,
    const GIDCollection& sources,
    const GIDCollection& targets,
    const DictionaryDatum& conn_spec,
    const DictionaryDatum& syn_spec );

  /**
   * Create connections.
   */
  void connect( const GIDCollection&,
    const GIDCollection&,
    const DictionaryDatum&,
    const DictionaryDatum& );

  /**
   * Connect two nodes. The source node is defined by its global ID.
   * The target node is defined by the node. The connection is
   * established on the thread/process that owns the target node.
   *
   * The parameters delay and weight have the default value numerics::nan.
   * numerics::nan is a special value, which describes double values that
   * are not a number. If delay or weight is omitted in a connect call,
   * numerics::nan indicates this and weight/delay are set only, if they are
   *valid.
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
    double d = numerics::nan,
    double w = numerics::nan );

  /**
   * Connect two nodes. The source node is defined by its global ID.
   * The target node is defined by the node. The connection is
   * established on the thread/process that owns the target node.
   *
   * The parameters delay and weight have the default value numerics::nan.
   * numerics::nan is a special value, which describes double values that
   * are not a number. If delay or weight is omitted in an connect call,
   * numerics::nan indicates this and weight/delay are set only, if they are
   *valid.
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
    double d = numerics::nan,
    double w = numerics::nan );

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

  void
  disconnect( Node& target, index sgid, thread target_thread, index syn_id );

  void subnet_connect( Subnet&, Subnet&, int, index syn );

  /**
   * Connect, using a dictionary with arrays.
   * The connection rule is based on the details of the dictionary entries
   * source and target.
   * If source and target are both either a GID or a list of GIDs with equal
   * size, then source and target are connected one-to-one.
   * If source is a gid and target is a list of GIDs then the sources is
   * connected to all targets.
   * If source is a list of GIDs and target is a GID, then all sources are
   * connected to the target.
   * At this stage, the task of connect is to separate the dictionary into one
   * for each thread and then to forward the connect call to the connectors who
   * can then deal with the details of the connection.
   */
  bool data_connect_connectome( const ArrayDatum& connectome );

  /**
   * Connect one source node with many targets.
   * The dictionary d contains arrays for all the outgoing connections of type
   * syn.
   */
  void data_connect_single( const index s, DictionaryDatum d, const index syn );

  // aka conndatum GetStatus
  DictionaryDatum
  get_synapse_status( index gid, synindex syn, port p, thread tid );
  // aka conndatum SetStatus
  void set_synapse_status( index gid,
    synindex syn,
    port p,
    thread tid,
    const DictionaryDatum& d );


  /**
   * Return connections between pairs of neurons.
   * The params dictionary can have the following entries:
   * 'source' a token array with GIDs of source neurons.
   * 'target' a token array with GIDs of target neuron.
   * If either of these does not exist, all neuron are used for the respective
   * entry.
   * 'synapse_model' name of the synapse model, or all synapse models are
   * searched.
   * 'synapse_label' label (long) of the synapse, or all synapses are
   * searched.
   * The function then iterates all entries in source and collects the
   * connection IDs to all neurons in target.
   */
  ArrayDatum get_connections( DictionaryDatum dict ) const;

  void get_connections( std::deque< ConnectionID >& connectome,
    TokenArray const* source,
    TokenArray const* target,
    size_t syn_id,
    long synapse_label ) const;

  /**
   * Returns the number of connections in the network.
   */
  size_t get_num_connections() const;

  /**
   * Returns the number of connections of this synapse type.
   */
  size_t get_num_connections( synindex syn_id ) const;

  void get_sources( std::vector< index > targets,
    std::vector< std::vector< index > >& sources,
    index synapse_model );

  void get_targets( const std::vector< index >& sources,
    std::vector< std::vector< index > >& targets,
    const index synapse_model,
    const std::string& post_synaptic_element );

  /**
   * Triggered by volume transmitter in update.
   * Triggeres updates for all connectors of dopamine synapses that
   * are registered with the volume transmitter with gid vt_gid.
   */
  void trigger_update_weight( const long vt_gid,
    const std::vector< spikecounter >& dopa_spikes,
    const double t_trig );

  /**
   * Return minimal connection delay, which is precomputed by
   * update_delay_extrema_().
   */
  delay get_min_delay() const;

  /**
   * Return maximal connection delay, which is precomputed by
   * update_delay_extrema_().
   */
  delay get_max_delay() const;

  bool get_user_set_delay_extrema() const;

  void send( thread t, index sgid, Event& e );

  void send_secondary( thread t, SecondaryEvent& e );

  /**
   * Send event e to all targets of node source on thread t
   */
  void send_local( thread t, Node& source, Event& e );

  /**
   * Resize the structures for the Connector objects if necessary.
   * This function should be called after number of threads, min_delay,
   * max_delay, and time representation have been changed in the scheduler.
   * The TimeConverter is used to convert times from the old to the new
   * representation. It is also forwarding the calibration request to all
   * ConnectorModel objects.
   */
  void calibrate( const TimeConverter& );

  /**
   * Returns the delay checker for the current thread.
   */
  DelayChecker& get_delay_checker();

  /**
   * Returns initial connector capacity.
   * When a connector is first created, it starts with this capacity
   * (if >= connector_cutoff).
   */
  size_t get_initial_connector_capacity() const;

  /**
   * Return large connector limit.
   * Capacity doubling is used up to this limit.
   */
  size_t get_large_connector_limit() const;

  /**
   * Returns large connector growth factor.
   * This capacity growth factor is used beyond the large connector limit.
   */
  double get_large_connector_growth_factor() const;

  double get_stdp_eps() const;

  void set_stdp_eps( const double stdp_eps );

private:
  /**
   * Update delay extrema to current values.
   *
   * Static since it only operates in static variables. This allows it to be
   * called from const-method get_status() as well.
   */
  void update_delay_extrema_();

  /**
   * This method queries and finds the minimum delay
   * of all local connections
   */
  const Time get_min_delay_time_() const;

  /**
   * This method queries and finds the minimum delay
   * of all local connections
   */
  const Time get_max_delay_time_() const;

  /**
   * Deletes all connections and also frees the PMA.
   */
  void delete_connections_();

  ConnectorBase*
  validate_source_entry_( thread tid, index s_gid, synindex syn_id );

  ConnectorBase* validate_source_entry_( thread tid, index s_gid );

  /**
   * Connect is used to establish a connection between a sender and
   * receiving node.
   *
   * The parameters delay and weight have the default value numerics::nan.
   * numerics::nan is a special value, which describes double values that
   * are not a number. If delay or weight is omitted in an connect call,
   * numerics::nan indicates this and weight/delay are set only, if they are
   *valid.
   *
   * \param s A reference to the sending Node.
   * \param r A reference to the receiving Node.
   * \param t The thread of the target node.
   * \param syn The synapse model to use.
   * \returns The receiver port number for the new connection
   */
  void connect_( Node& s,
    Node& r,
    index s_gid,
    thread tid,
    index syn,
    double d = numerics::nan,
    double w = numerics::nan );
  void connect_( Node& s,
    Node& r,
    index s_gid,
    thread tid,
    index syn,
    DictionaryDatum& p,
    double d = numerics::nan,
    double w = numerics::nan );

  /**
   * A 3-dim structure to hold the Connector objects which in turn hold the
   * connection information.
   * - First dim: A std::vector for each local thread
   * - Second dim: A std::vector for each node on each thread
   * - Third dim: A std::vector for each synapse prototype, holding the
   * Connector objects
   */
  tVSConnector connections_;

  tVDelayChecker delay_checkers_;

  tVVCounter vv_num_connections_;

  /**
   * BeginDocumentation
   * Name: connruledict - dictionary containing all connectivity rules
   * Description:
   * This dictionary provides the connection rules that can be used
   * in Connect.
   * 'connruledict info' shows the contents of the dictionary.
   * SeeAlso: Connect
   */
  DictionaryDatum connruledict_; //!< Dictionary for connection rules.

  //! ConnBuilder factories, indexed by connruledict_ elements.
  std::vector< GenericConnBuilderFactory* > connbuilder_factories_;

  delay min_delay_; //!< Value of the smallest delay in the network.

  delay max_delay_; //!< Value of the largest delay in the network in steps.

  /**
   * When a connector is first created, it starts with this capacity
   * (if >= connector_cutoff)
   */
  size_t initial_connector_capacity_;

  //! Capacity doubling is used up to this limit
  size_t large_connector_limit_;

  //! Capacity growth factor to use beyond the limit
  double large_connector_growth_factor_;

  //! Maximum distance between (double) spike times in STDP that is
  //! still considered 0. See issue #894
  double stdp_eps_;
};

inline DictionaryDatum&
ConnectionManager::get_connruledict()
{
  return connruledict_;
}

inline delay
ConnectionManager::get_min_delay() const
{
  return min_delay_;
}

inline delay
ConnectionManager::get_max_delay() const
{
  return max_delay_;
}

inline size_t
ConnectionManager::get_initial_connector_capacity() const
{
  return initial_connector_capacity_;
}

inline size_t
ConnectionManager::get_large_connector_limit() const
{
  return large_connector_limit_;
}

inline double
ConnectionManager::get_large_connector_growth_factor() const
{
  return large_connector_growth_factor_;
}

inline double
ConnectionManager::get_stdp_eps() const
{
  return stdp_eps_;
}

} // namespace nest

#endif /* CONNECTION_MANAGER_H */
