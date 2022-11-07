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

// Includes from libnestutil:
#include "manager_interface.h"
#include "stopwatch.h"

// Includes from nestkernel:
#include "conn_builder.h"
#include "connection_id.h"
#include "connector_base.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "node_collection.h"
#include "per_thread_bool_indicator.h"
#include "source_table.h"
#include "spike_data.h"
#include "target_table.h"
#include "target_table_devices.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictdatum.h"

namespace nest
{
class GenericConnBuilderFactory;
class spikecounter;
class Node;
class Event;
class SecondaryEvent;
class DelayChecker;
class GrowthCurve;
class SpikeData;

class ConnectionManager : public ManagerInterface
{
  friend class SimulationManager; // update_delay_extrema_
public:
  /**
   * Connection type.
   */
  enum ConnectionType
  {
    CONNECT,
    CONNECT_FROM_DEVICE,
    CONNECT_TO_DEVICE,
    NO_CONNECTION
  };

  ConnectionManager();
  ~ConnectionManager() override;

  void initialize() override;
  void finalize() override;
  void change_number_of_threads() override;
  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) override;

  bool valid_connection_rule( std::string );

  void compute_target_data_buffer_size();
  void compute_compressed_secondary_recv_buffer_positions( const thread tid );
  void collect_compressed_spike_data( const thread tid );
  void clear_compressed_spike_data_map( const thread tid );

  /**
   * Add a connectivity rule, i.e. the respective ConnBuilderFactory.
   */
  template < typename ConnBuilder >
  void register_conn_builder( const std::string& name );

  ConnBuilder* get_conn_builder( const std::string& name,
    NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );

  /**
   * Create connections.
   */
  void connect( NodeCollectionPTR, NodeCollectionPTR, const DictionaryDatum&, const std::vector< DictionaryDatum >& );

  void connect( TokenArray, TokenArray, const DictionaryDatum& );

  /**
   * Connect two nodes. The source node is defined by its global ID.
   * The target node is defined by the node. The connection is
   * established on the thread/process that owns the target node.
   *
   * The parameters delay and weight have the default value numerics::nan.
   * numerics::nan is a special value, which describes double values that
   * are not a number. If delay or weight is omitted in a connect call,
   * numerics::nan indicates this and weight/delay are set only, if they are
   * valid.
   *
   * \param snode_id node ID of the sending Node.
   * \param target Pointer to target Node.
   * \param target_thread Thread that hosts the target node.
   * \param syn_id The synapse model to use.
   * \param params Parameter dictionary to configure the synapse.
   * \param delay Delay of the connection (in ms).
   * \param weight Weight of the connection.
   */
  void connect( const index snode_id,
    Node* target,
    thread target_thread,
    const synindex syn_id,
    const DictionaryDatum& params,
    const double delay = numerics::nan,
    const double weight = numerics::nan );

  /**
   * Connect two nodes. The source and target nodes are defined by their
   * global ID. The connection is established on the thread/process that owns
   * the target node.
   *
   * \param snode_id node ID of the sending Node.
   * \param target node ID of the target Node.
   * \param params Parameter dictionary to configure the synapse.
   * \param syn_id The synapse model to use.
   */
  bool connect( const index snode_id, const index target, const DictionaryDatum& params, const synindex syn_id );

  void connect_arrays( long* sources,
    long* targets,
    double* weights,
    double* delays,
    std::vector< std::string >& p_keys,
    double* p_values,
    size_t n,
    std::string syn_model );

  index find_connection( const thread tid, const synindex syn_id, const index snode_id, const index tnode_id );

  void disconnect( const thread tid, const synindex syn_id, const index snode_id, const index tnode_id );

  /**
   * Check whether a connection between the given source and target
   * nodes can be established on the given thread with id tid.
   *
   * \returns The type of connection as ConnectionType if the connection should
   * be made, ConnectionType::NO_CONNECTION otherwise.
   */
  ConnectionType connection_required( Node*& source, Node*& target, thread tid );

  // aka conndatum GetStatus
  DictionaryDatum get_synapse_status( const index source_node_id,
    const index target_node_id,
    const thread tid,
    const synindex syn_id,
    const index lcid ) const;

  // aka conndatum SetStatus
  void set_synapse_status( const index source_node_id,
    const index target_node_id,
    const thread tid,
    const synindex syn_id,
    const index lcid,
    const DictionaryDatum& dict );

  /**
   * Return connections between pairs of neurons.
   * The params dictionary can have the following entries:
   * 'source' a token array with node IDs of source neurons.
   * 'target' a token array with node IDs of target neuron.
   * If either of these does not exist, all neuron are used for the respective
   * entry.
   * 'synapse_model' name of the synapse model, or all synapse models are
   * searched.
   * 'synapse_label' label (long) of the synapse, or all synapses are
   * searched.
   * The function then iterates all entries in source and collects the
   * connection IDs to all neurons in target.
   */
  ArrayDatum get_connections( const DictionaryDatum& params );

  void get_connections( std::deque< ConnectionID >& connectome,
    NodeCollectionPTR source,
    NodeCollectionPTR target,
    synindex syn_id,
    long synapse_label ) const;

  /**
   * Returns the number of connections in the network.
   */
  size_t get_num_connections() const;

  /**
   * Returns the number of connections of this synapse type.
   */
  size_t get_num_connections( const synindex syn_id ) const;

  void
  get_sources( const std::vector< index >& targets, const index syn_id, std::vector< std::vector< index > >& sources );

  void get_targets( const std::vector< index >& sources,
    const index syn_id,
    const std::string& post_synaptic_element,
    std::vector< std::vector< index > >& targets );

  const std::vector< Target >& get_remote_targets_of_local_node( const thread tid, const index lid ) const;

  index get_target_node_id( const thread tid, const synindex syn_id, const index lcid ) const;

  bool get_device_connected( thread tid, index lcid ) const;
  /**
   * Triggered by volume transmitter in update.
   * Triggeres updates for all connectors of dopamine synapses that
   * are registered with the volume transmitter with node_id vt_node_id.
   */
  void
  trigger_update_weight( const long vt_node_id, const std::vector< spikecounter >& dopa_spikes, const double t_trig );

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

  void
  send( const thread tid, const synindex syn_id, const index lcid, const std::vector< ConnectorModel* >& cm, Event& e );

  /**
   * Send event e to all device targets of source source_node_id
   */
  void send_to_devices( const thread tid, const index source_node_id, Event& e );
  void send_to_devices( const thread tid, const index source_node_id, SecondaryEvent& e );

  /**
   * Send event e to all targets of source device ldid (local device id)
   */
  void send_from_device( const thread tid, const index ldid, Event& e );

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

  //! Removes processed entries from source table
  void clean_source_table( const thread tid );

  //! Clears all entries in source table
  void clear_source_table( const thread tid );

  //! Returns true if source table is kept after building network
  bool get_keep_source_table() const;

  //! Returns true if source table was cleared
  bool is_source_table_cleared() const;

  void prepare_target_table( const thread tid );

  void resize_target_table_devices_to_number_of_neurons();

  void resize_target_table_devices_to_number_of_synapse_types();

  bool get_next_target_data( const thread tid,
    const thread rank_start,
    const thread rank_end,
    thread& target_rank,
    TargetData& next_target_data );

  void reject_last_target_data( const thread tid );

  void save_source_table_entry_point( const thread tid );

  void reset_source_table_entry_point( const thread tid );

  void restore_source_table_entry_point( const thread tid );

  void add_target( const thread tid, const thread target_rank, const TargetData& target_data );

  /**
   * Return sort_connections_by_source_, which indicates whether
   * connections_ and source_table_ should be sorted according to
   * source node ID.
   */
  bool get_sort_connections_by_source() const;

  bool use_compressed_spikes() const;

  /**
   * Sorts connections in the presynaptic infrastructure by increasing
   * source node ID.
   */
  void sort_connections( const thread tid );

  /**
   * Removes disabled connections (of structural plasticity)
   */
  void remove_disabled_connections( const thread tid );

  /**
   * Returns true if connection information needs to be
   * communicated. False otherwise.
   */
  bool connections_have_changed() const;

  /**
   * Sets flag indicating whether connection information needs to be
   * communicated to true.
   */
  void set_connections_have_changed();

  /**
   * Sets flag indicating whether connection information needs to be
   * communicated to false.
   */
  void unset_connections_have_changed();

  /**
   * Deletes TargetTable and resets processed flags of
   * SourceTable. This function must be called if connections are
   * created after connections have been communicated previously. It
   * basically restores the connection infrastructure to a state where
   * all information only exists on the postsynaptic side.
   */
  void restructure_connection_tables( const thread tid );

  void
  set_source_has_more_targets( const thread tid, const synindex syn_id, const index lcid, const bool more_targets );

  void no_targets_to_process( const thread tid );

  const std::vector< size_t >&
  get_secondary_send_buffer_positions( const thread tid, const index lid, const synindex syn_id ) const;

  /**
   * Returns read position in MPI receive buffer for secondary connections.
   */
  size_t get_secondary_recv_buffer_position( const thread tid, const synindex syn_id, const index lcid ) const;

  bool deliver_secondary_events( const thread tid,
    const bool called_from_wfr_update,
    std::vector< unsigned int >& recv_buffer );

  void compress_secondary_send_buffer_pos( const thread tid );

  void resize_connections();

  void sync_has_primary_connections();

  void check_secondary_connections_exist();

  bool has_primary_connections() const;

  bool secondary_connections_exist() const;

  index get_source_node_id( const thread tid, const synindex syn_id, const index lcid );

  double get_stdp_eps() const;

  void set_stdp_eps( const double stdp_eps );

  // public stop watch for benchmarking purposes
  // start and stop in high-level connect functions in nestmodule.cpp and nest.cpp
  Stopwatch sw_construction_connect;

  const std::vector< SpikeData >& get_compressed_spike_data( const synindex syn_id, const index idx );

private:
  size_t get_num_target_data( const thread tid ) const;

  size_t get_num_connections_( const thread tid, const synindex syn_id ) const;

  void
  get_source_node_ids_( const thread tid, const synindex syn_id, const index tnode_id, std::vector< index >& sources );

  /**
   * Splits a TokenArray of node IDs to two vectors containing node IDs of neurons and
   * node IDs of devices.
   */
  void split_to_neuron_device_vectors_( const thread tid,
    NodeCollectionPTR nodecollection,
    std::vector< index >& neuron_node_ids,
    std::vector< index >& device_node_ids ) const;

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
   * This method queries and finds the maximum delay
   * of all local connections
   */
  const Time get_max_delay_time_() const;

  /**
   * Deletes all connections.
   */
  void delete_connections_();

  /**
   * connect_ is used to establish a connection between a sender and
   * receiving node which both have proxies.
   *
   * The parameters delay and weight have the default value numerics::nan.
   * numerics::nan is a special value, which describes double values that
   * are not a number. If delay or weight is omitted in an connect call,
   * numerics::nan indicates this and weight/delay are set only, if they are
   * valid.
   *
   * \param source A reference to the sending Node.
   * \param target A reference to the receiving Node.
   * \param s_node_id The node ID of the sending Node.
   * \param tid The thread of the target node.
   * \param syn_id The synapse model to use.
   * \param params The parameters for the connection.
   * \param delay The delay of the connection (optional).
   * \param weight The weight of the connection (optional).
   */
  void connect_( Node& source,
    Node& target,
    const index s_node_id,
    const thread tid,
    const synindex syn_id,
    const DictionaryDatum& params,
    const double delay = numerics::nan,
    const double weight = numerics::nan );

  /**
   * connect_to_device_ is used to establish a connection between a sender and
   * receiving node if the sender has proxies, and the receiver does not.
   *
   * The parameters delay and weight have the default value NAN.
   * NAN is a special value in cmath, which describes double values that
   * are not a number. If delay or weight is omitted in an connect call,
   * NAN indicates this and weight/delay are set only, if they are valid.
   *
   * \param source A reference to the sending Node.
   * \param target A reference to the receiving Node.
   * \param s_node_id The node ID of the sending Node.
   * \param tid The thread of the target node.
   * \param syn_id The synapse model to use.
   * \param params The parameters for the connection.
   * \param delay The delay of the connection (optional).
   * \param weight The weight of the connection (optional).
   */
  void connect_to_device_( Node& source,
    Node& target,
    const index s_node_id,
    const thread tid,
    const synindex syn_id,
    const DictionaryDatum& params,
    const double delay = NAN,
    const double weight = NAN );

  /**
   * connect_from_device_ is used to establish a connection between a sender and
   * receiving node if the sender does not have proxies.
   *
   * The parameters delay and weight have the default value NAN.
   * NAN is a special value in cmath, which describes double values that
   * are not a number. If delay or weight is omitted in an connect call,
   * NAN indicates this and weight/delay are set only, if they are valid.
   *
   * \param source A reference to the sending Node.
   * \param target A reference to the receiving Node.
   * \param s_node_id The node ID of the sending Node.
   * \param tid The thread of the target node.
   * \param syn_id The synapse model to use.
   * \param params The parameters for the connection.
   * \param delay The delay of the connection (optional).
   * \param weight The weight of the connection (optional).
   */
  void connect_from_device_( Node& source,
    Node& target,
    const thread tid,
    const synindex syn_id,
    const DictionaryDatum& params,
    const double delay = NAN,
    const double weight = NAN );

  /**
   * Increases the connection count.
   */
  void increase_connection_count( const thread tid, const synindex syn_id );

  /**
   * A structure to hold the Connector objects which in turn hold the
   * connection information. Corresponds to a three dimensional
   * structure: threads|synapses|connections
   */
  std::vector< std::vector< ConnectorBase* > > connections_;

  /**
   * A structure to hold the node IDs of presynaptic neurons during
   * postsynaptic connection creation, before the connection
   * information has been transferred to the presynaptic side.
   * Internally arranged in a 3d structure: threads|synapses|node IDs
   */
  SourceTable source_table_;

  /**
   * A structure to hold "unpacked" spikes on the postsynaptic side if
   * spike compression is enabled. Internally arranged in a 3d
   * structure: synapses|sources|spike data
   */
  std::vector< std::vector< std::vector< SpikeData > > > compressed_spike_data_;

  /**
   * Stores absolute position in receive buffer of secondary events.
   * structure: threads|synapses|position
   */
  std::vector< std::vector< std::vector< size_t > > > secondary_recv_buffer_pos_;

  std::map< index, size_t > buffer_pos_of_source_node_id_syn_id_;

  /**
   * A structure to hold the information about targets for each
   * neuron on the presynaptic side. Internally arranged in a 3d
   * structure: threads|localnodes|targets
   */
  TargetTable target_table_;

  TargetTableDevices target_table_devices_;

  std::vector< DelayChecker > delay_checkers_;

  /**
   * A structure to count the number of synapses of a specific
   * type. Arranged in a 2d structure: threads|synapsetypes.
   */
  std::vector< std::vector< size_t > > num_connections_;

  DictionaryDatum connruledict_; //!< Dictionary for connection rules.

  //! ConnBuilder factories, indexed by connruledict_ elements.
  std::vector< GenericConnBuilderFactory* > connbuilder_factories_;

  delay min_delay_; //!< Value of the smallest delay in the network.

  delay max_delay_; //!< Value of the largest delay in the network in steps.

  //! Whether to keep source table after connection setup is complete.
  bool keep_source_table_;

  //! True if new connections have been created since startup or last call to
  //! simulate.
  bool connections_have_changed_;

  //! true if GetConnections has been called.
  bool get_connections_has_been_called_;

  //! Whether to sort connections by source node ID.
  bool sort_connections_by_source_;

  //! Whether to use spike compression; if a neuron has targets on
  //! multiple threads of a process, this switch makes sure that only
  //! a single packet is sent to the process instead of one packet per
  //! target thread; requires sort_connections_by_source_ = true; for
  //! more details see the discussion and sketch in
  //! https://github.com/nest/nest-simulator/pull/1338
  bool use_compressed_spikes_;

  //! Whether primary connections (spikes) exist.
  bool has_primary_connections_;

  //! Check for primary connections (spikes) on each thread.
  PerThreadBoolIndicator check_primary_connections_;

  //! Whether secondary connections (e.g., gap junctions) exist.
  bool secondary_connections_exist_;

  //! Check for secondary connections (e.g., gap junctions) on each thread.
  PerThreadBoolIndicator check_secondary_connections_;

  //! Maximum distance between (double) spike times in STDP that is
  //! still considered 0. See issue #894
  double stdp_eps_;
};

inline bool
ConnectionManager::valid_connection_rule( std::string rule_name )
{
  return connruledict_->known( rule_name );
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

inline void
ConnectionManager::clean_source_table( const thread tid )
{
  if ( not keep_source_table_ )
  {
    source_table_.clean( tid );
  }
}

inline void
ConnectionManager::clear_source_table( const thread tid )
{
  if ( not keep_source_table_ )
  {
    source_table_.clear( tid );
  }
}

inline bool
ConnectionManager::get_keep_source_table() const
{
  return keep_source_table_;
}

inline bool
ConnectionManager::is_source_table_cleared() const
{
  return source_table_.is_cleared();
}

inline void
ConnectionManager::resize_target_table_devices_to_number_of_neurons()
{
  target_table_devices_.resize_to_number_of_neurons();
}

inline void
ConnectionManager::resize_target_table_devices_to_number_of_synapse_types()
{
  target_table_devices_.resize_to_number_of_synapse_types();
}

inline void
ConnectionManager::reject_last_target_data( const thread tid )
{
  source_table_.reject_last_target_data( tid );
}

inline void
ConnectionManager::save_source_table_entry_point( const thread tid )
{
  source_table_.save_entry_point( tid );
}

inline void
ConnectionManager::no_targets_to_process( const thread tid )
{
  source_table_.no_targets_to_process( tid );
}

inline void
ConnectionManager::reset_source_table_entry_point( const thread tid )
{
  source_table_.reset_entry_point( tid );
}

inline void
ConnectionManager::restore_source_table_entry_point( const thread tid )
{
  source_table_.restore_entry_point( tid );
}

inline void
ConnectionManager::prepare_target_table( const thread tid )
{
  target_table_.prepare( tid );
}

inline const std::vector< Target >&
ConnectionManager::get_remote_targets_of_local_node( const thread tid, const index lid ) const
{
  return target_table_.get_targets( tid, lid );
}

inline bool
ConnectionManager::connections_have_changed() const
{
  return connections_have_changed_;
}

inline void
ConnectionManager::add_target( const thread tid, const thread target_rank, const TargetData& target_data )
{
  target_table_.add_target( tid, target_rank, target_data );
}

inline bool
ConnectionManager::get_next_target_data( const thread tid,
  const thread rank_start,
  const thread rank_end,
  thread& target_rank,
  TargetData& next_target_data )
{
  return source_table_.get_next_target_data( tid, rank_start, rank_end, target_rank, next_target_data );
}

inline const std::vector< size_t >&
ConnectionManager::get_secondary_send_buffer_positions( const thread tid, const index lid, const synindex syn_id ) const
{
  return target_table_.get_secondary_send_buffer_positions( tid, lid, syn_id );
}

inline size_t
ConnectionManager::get_secondary_recv_buffer_position( const thread tid, const synindex syn_id, const index lcid ) const
{
  return secondary_recv_buffer_pos_[ tid ][ syn_id ][ lcid ];
}

inline size_t
ConnectionManager::get_num_connections_( const thread tid, const synindex syn_id ) const
{
  return connections_[ tid ][ syn_id ]->size();
}

inline index
ConnectionManager::get_source_node_id( const thread tid, const synindex syn_index, const index lcid )
{
  return source_table_.get_node_id( tid, syn_index, lcid );
}

inline bool
ConnectionManager::has_primary_connections() const
{
  return has_primary_connections_;
}

inline bool
ConnectionManager::secondary_connections_exist() const
{
  return secondary_connections_exist_;
}

inline bool
ConnectionManager::get_sort_connections_by_source() const
{
  return sort_connections_by_source_;
}

inline bool
ConnectionManager::use_compressed_spikes() const
{
  return use_compressed_spikes_;
}

inline double
ConnectionManager::get_stdp_eps() const
{
  return stdp_eps_;
}

inline index
ConnectionManager::get_target_node_id( const thread tid, const synindex syn_id, const index lcid ) const
{
  return connections_[ tid ][ syn_id ]->get_target_node_id( tid, lcid );
}

inline bool
ConnectionManager::get_device_connected( const thread tid, const index lcid ) const
{
  return target_table_devices_.is_device_connected( tid, lcid );
}

inline void
ConnectionManager::send( const thread tid,
  const synindex syn_id,
  const index lcid,
  const std::vector< ConnectorModel* >& cm,
  Event& e )
{
  connections_[ tid ][ syn_id ]->send( tid, lcid, cm, e );
}

inline void
ConnectionManager::restructure_connection_tables( const thread tid )
{
  assert( not source_table_.is_cleared() );
  target_table_.clear( tid );
  source_table_.reset_processed_flags( tid );
}

inline void
ConnectionManager::set_source_has_more_targets( const thread tid,
  const synindex syn_id,
  const index lcid,
  const bool more_targets )
{
  connections_[ tid ][ syn_id ]->set_source_has_more_targets( lcid, more_targets );
}

inline const std::vector< SpikeData >&
ConnectionManager::get_compressed_spike_data( const synindex syn_id, const index idx )
{
  return compressed_spike_data_.at( syn_id ).at( idx );
}

inline void
ConnectionManager::clear_compressed_spike_data_map( const thread tid )
{
  source_table_.clear_compressed_spike_data_map( tid );
}

} // namespace nest

#endif /* CONNECTION_MANAGER_H */
