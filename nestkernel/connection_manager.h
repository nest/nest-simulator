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
#include "stopwatch_impl.h"

// Includes from nestkernel:
#include "conn_builder_factory.h"
#include "connection_id.h"
#include "connector_base.h"
#include "nest_time.h"
#include "nest_types.h"
#include "node_collection.h"
#include "per_thread_bool_indicator.h"
#include "send_buffer_position.h"
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
class GenericBipartiteConnBuilderFactory;
class GenericThirdConnBuilderFactory;
class Node;
class Event;
class SecondaryEvent;
class DelayChecker;
class GrowthCurve;
class SpikeData;
class BipartiteConnBuilder;
class ThirdOutBuilder;
class ThirdInBuilder;
class Time;
class TimeConverter;

class ConnectionManager : public ManagerInterface
{
  friend class SimulationManager; // update_delay_extrema_
public:
  enum ConnectionType
  {
    CONNECT,
    CONNECT_FROM_DEVICE,
    CONNECT_TO_DEVICE,
    NO_CONNECTION
  };

  ConnectionManager();

  ~ConnectionManager() override;

  void initialize( const bool ) override;

  void finalize( const bool ) override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) override;

  bool valid_connection_rule( std::string );

  void compute_target_data_buffer_size();

  void compute_compressed_secondary_recv_buffer_positions( const size_t tid );

  void collect_compressed_spike_data( const size_t tid );

  void clear_compressed_spike_data_map();

  /**
   * Add a connectivity rule, i.e. the respective ConnBuilderFactory.
   */
  template < typename ConnBuilder >
  void register_conn_builder( const std::string& name );

  /**
   * Add a connectivity rule, i.e. the respective ConnBuilderFactory.
   */
  template < typename ThirdConnBuilder >
  void register_third_conn_builder( const std::string& name );

  //! Obtain builder for bipartite connections
  BipartiteConnBuilder* get_conn_builder( const std::string& name,
    NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    ThirdOutBuilder* third_out,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );

  //! Obtain builder for bipartite connections
  ThirdOutBuilder* get_third_conn_builder( const std::string& name,
    NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    ThirdInBuilder* third_in,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );

  /**
   * Create connections.
   */
  void connect( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    const DictionaryDatum& conn_spec,
    const std::vector< DictionaryDatum >& syn_specs );

  /**
   * Connect two nodes.
   *
   * The source node is defined by its global ID.
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
  void connect( const size_t snode_id,
    Node* target,
    size_t target_thread,
    const synindex syn_id,
    const DictionaryDatum& params,
    const double delay = numerics::nan,
    const double weight = numerics::nan );

  /**
   * Connect two nodes.
   *
   * The source and target nodes are defined by their
   * global ID. The connection is established on the thread/process that owns
   * the target node.
   *
   * \param snode_id node ID of the sending Node.
   * \param target node ID of the target Node.
   * \param params Parameter dictionary to configure the synapse.
   * \param syn_id The synapse model to use.
   */
  bool connect( const size_t snode_id, const size_t target, const DictionaryDatum& params, const synindex syn_id );

  void connect_arrays( long* sources,
    long* targets,
    double* weights,
    double* delays,
    std::vector< std::string >& p_keys,
    double* p_values,
    size_t n,
    std::string syn_model );

  /**
   * @brief Connect nodes from SONATA specification.
   *
   * This function instantiates the `SonataConnector` class and calls the class member
   * function `connect`.
   *
   * @param graph_specs Specification dictionary, see PyNEST `SonataNetwork._create_graph_specs` for details.
   * @param hyberslab_size Size of the hyperslab to read in one read operation, applies to all HDF5 datasets.
   */
  void connect_sonata( const DictionaryDatum& graph_specs, const long hyberslab_size );

  /**
   * @brief Create tripartite connections
   *
   * @note `synapse_specs` is dictionary `{"primary": <syn_spec>, "third_in": <syn_spec>, "third_out": <syn_spec>}`; all
   * keys are optional
   */
  void connect_tripartite( NodeCollectionPTR sources,
    NodeCollectionPTR targets,
    NodeCollectionPTR third,
    const DictionaryDatum& connectivity,
    const DictionaryDatum& third_connectivity,
    const std::map< Name, std::vector< DictionaryDatum > >& synapse_specs );

  /**
   * Find first non-disabled thread-local connection of given synapse type with given source and target node.
   *
   * @returns Local connection id (lcid) or `invalid_index`
   */
  size_t find_connection( const size_t tid, const synindex syn_id, const size_t snode_id, const size_t tnode_id );

  void disconnect( const size_t tid, const synindex syn_id, const size_t snode_id, const size_t tnode_id );

  /**
   * Check whether a connection between the given source and target
   * nodes can be established on the given thread with id tid.
   *
   * \returns The type of connection as ConnectionType if the connection should
   * be made, ConnectionType::NO_CONNECTION otherwise.
   */
  ConnectionType connection_required( Node*& source, Node*& target, size_t tid );

  // aka conndatum GetStatus
  DictionaryDatum get_synapse_status( const size_t source_node_id,
    const size_t target_node_id,
    const size_t tid,
    const synindex syn_id,
    const size_t lcid ) const;

  // aka conndatum SetStatus
  void set_synapse_status( const size_t source_node_id,
    const size_t target_node_id,
    const size_t tid,
    const synindex syn_id,
    const size_t lcid,
    const DictionaryDatum& dict );

  /**
   * Return connections between pairs of neurons.
   *
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

  void get_sources( const std::vector< size_t >& targets,
    const size_t syn_id,
    std::vector< std::vector< size_t > >& sources );

  void get_targets( const std::vector< size_t >& sources,
    const size_t syn_id,
    const std::string& post_synaptic_element,
    std::vector< std::vector< size_t > >& targets );

  const std::vector< Target >& get_remote_targets_of_local_node( const size_t tid, const size_t lid ) const;

  size_t get_target_node_id( const size_t tid, const synindex syn_id, const size_t lcid ) const;

  bool get_device_connected( size_t tid, size_t lcid ) const;

  /**
   * Triggered by volume transmitter in update.
   *
   * Triggeres updates for all connectors of dopamine synapses that
   * are registered with the volume transmitter with node_id vt_node_id.
   */
  void
  trigger_update_weight( const long vt_node_id, const std::vector< spikecounter >& dopa_spikes, const double t_trig );

  /**
   * Return minimal connection delay, which is precomputed by
   * update_delay_extrema_().
   */
  long get_min_delay() const;

  /**
   * Return maximal connection delay, which is precomputed by
   * update_delay_extrema_().
   */
  long get_max_delay() const;

  bool get_user_set_delay_extrema() const;

  void send( const size_t tid,
    const synindex syn_id,
    const size_t lcid,
    const std::vector< ConnectorModel* >& cm,
    Event& e );

  /**
   * Send event e to all device targets of source source_node_id
   */
  void send_to_devices( const size_t tid, const size_t source_node_id, Event& e );

  void send_to_devices( const size_t tid, const size_t source_node_id, SecondaryEvent& e );

  /**
   * Send event e to all targets of source device ldid (local device id)
   */
  void send_from_device( const size_t tid, const size_t ldid, Event& e );

  /**
   * Send event e to all targets of node source on thread t
   */
  void send_local( size_t t, Node& source, Event& e );

  /**
   * Resize the structures for the Connector objects if necessary.
   *
   * This function should be called after number of threads, min_delay,
   * max_delay, and time representation have been changed in the scheduler.
   * The TimeConverter is used to convert times from the old to the new
   * representation. It is also forwarding the calibration request to all
   * ConnectorModel objects.
   */
  void calibrate( const TimeConverter& );

  //! Returns the delay checker for the current thread.
  DelayChecker& get_delay_checker();

  //! Removes processed entries from source table
  void clean_source_table( const size_t tid );

  //! Clears all entries in source table
  void clear_source_table( const size_t tid );

  //! Returns true if source table is kept after building network
  bool get_keep_source_table() const;

  //! Returns true if source table was cleared
  bool is_source_table_cleared() const;

  void prepare_target_table( const size_t tid );

  void resize_target_table_devices_to_number_of_neurons();

  void resize_target_table_devices_to_number_of_synapse_types();

  bool get_next_target_data( const size_t tid,
    const size_t rank_start,
    const size_t rank_end,
    size_t& target_rank,
    TargetData& next_target_data );

  bool fill_target_buffer( const size_t tid,
    const size_t rank_start,
    const size_t rank_end,
    std::vector< TargetData >& send_buffer_target_data,
    TargetSendBufferPosition& send_buffer_position );

  void reject_last_target_data( const size_t tid );

  void save_source_table_entry_point( const size_t tid );

  void reset_source_table_entry_point( const size_t tid );

  void restore_source_table_entry_point( const size_t tid );

  void add_target( const size_t tid, const size_t target_rank, const TargetData& target_data );

  /**
   * Returns whether spikes should be compressed.
   *
   * Implies that connections will be sorted by source.
   */
  bool use_compressed_spikes() const;

  /**
   * Sorts connections in the presynaptic infrastructure by increasing
   * source node ID.
   */
  void sort_connections( const size_t tid );

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
   * SourceTable.
   *
   * This function must be called if connections are
   * created after connections have been communicated previously. It
   * basically restores the connection infrastructure to a state where
   * all information only exists on the postsynaptic side.
   */
  void restructure_connection_tables( const size_t tid );

  void
  set_source_has_more_targets( const size_t tid, const synindex syn_id, const size_t lcid, const bool more_targets );

  void no_targets_to_process( const size_t tid );

  const std::vector< size_t >&
  get_secondary_send_buffer_positions( const size_t tid, const size_t lid, const synindex syn_id ) const;

  /**
   * Returns read position in MPI receive buffer for secondary connections.
   */
  size_t get_secondary_recv_buffer_position( const size_t tid, const synindex syn_id, const size_t lcid ) const;

  bool deliver_secondary_events( const size_t tid,
    const bool called_from_wfr_update,
    std::vector< unsigned int >& recv_buffer );

  void compress_secondary_send_buffer_pos( const size_t tid );

  void resize_connections();

  void sync_has_primary_connections();

  void check_secondary_connections_exist();

  bool has_primary_connections() const;

  bool secondary_connections_exist() const;

  size_t get_source_node_id( const size_t tid, const synindex syn_id, const size_t lcid );

  double get_stdp_eps() const;

  void set_stdp_eps( const double stdp_eps );

  // public stop watch for benchmarking purposes
  // start and stop in high-level connect functions in nestmodule.cpp and nest.cpp
  Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::MasterOnly > sw_construction_connect;

  const std::vector< SpikeData >& get_compressed_spike_data( const synindex syn_id, const size_t idx );

  //! Set iteration_state_ entries for all threads to beginning of compressed_spike_data_map_.
  void initialize_iteration_state();

private:
  size_t get_num_target_data( const size_t tid ) const;

  size_t get_num_connections_( const size_t tid, const synindex syn_id ) const;

  //! See get_connections()
  void get_connections_( const size_t tid,
    std::deque< ConnectionID >& connectome,
    NodeCollectionPTR source,
    NodeCollectionPTR target,
    synindex syn_id,
    long synapse_label ) const;

  void get_connections_to_targets_( const size_t tid,
    std::deque< ConnectionID >& connectome,
    NodeCollectionPTR source,
    NodeCollectionPTR target,
    synindex syn_id,
    long synapse_label ) const;

  void get_connections_from_sources_( const size_t tid,
    std::deque< ConnectionID >& connectome,
    NodeCollectionPTR source,
    NodeCollectionPTR target,
    synindex syn_id,
    long synapse_label ) const;

  void get_source_node_ids_( const size_t tid,
    const synindex syn_id,
    const size_t tnode_id,
    std::vector< size_t >& sources );

  /**
   * Removes disabled connections (of structural plasticity)
   */
  void remove_disabled_connections_( const size_t tid );

  /**
   * Splits a TokenArray of node IDs to two vectors containing node IDs of neurons and
   * node IDs of devices.
   */
  void split_to_neuron_device_vectors_( const size_t tid,
    NodeCollectionPTR nodecollection,
    std::vector< size_t >& neuron_node_ids,
    std::vector< size_t >& device_node_ids ) const;

  /**
   * Update delay extrema to current values.
   *
   * @note This entails MPI communication.
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
    const size_t s_node_id,
    const size_t tid,
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
    const size_t s_node_id,
    const size_t tid,
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
    const size_t tid,
    const synindex syn_id,
    const DictionaryDatum& params,
    const double delay = NAN,
    const double weight = NAN );

  /**
   * Increases the connection count.
   */
  void increase_connection_count( const size_t tid, const synindex syn_id );

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
   * structure: synapses|sources|target_threads
   */
  std::vector< std::vector< std::vector< SpikeData > > > compressed_spike_data_;

  /**
   * Stores absolute position in receive buffer of secondary events.
   * structure: threads|synapses|position
   */
  std::vector< std::vector< std::vector< size_t > > > secondary_recv_buffer_pos_;

  std::map< size_t, size_t > buffer_pos_of_source_node_id_syn_id_;

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
  std::vector< GenericBipartiteConnBuilderFactory* > connbuilder_factories_;

  DictionaryDatum thirdconnruledict_; //!< Dictionary for third-factor connection rules.

  //! Third-factor ConnBuilder factories, indexed by thirdconnruledict_ elements.
  std::vector< GenericThirdConnBuilderFactory* > thirdconnbuilder_factories_;

  long min_delay_; //!< Value of the smallest delay in the network.

  long max_delay_; //!< Value of the largest delay in the network in steps.

  //! Whether to keep source table after connection setup is complete.
  bool keep_source_table_;

  //! True if new connections have been created since startup or last call to
  //! simulate.
  bool connections_have_changed_;

  //! true if GetConnections has been called.
  bool get_connections_has_been_called_;

  /**
   *  Whether to use spike compression; if a neuron has targets on
   *  multiple threads of a process, this switch makes sure that only
   *  a single packet is sent to the process instead of one packet per
   *  target thread; implies sort_connections_by_source_ = true; for
   *  more details see the discussion and sketch in
   *  https://github.com/nest/nest-simulator/pull/1338
   */
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

  //! For each thread, store (syn_id, compressed_spike_data_map_::iterator) pair for next iteration while filling target
  //! buffers
  std::vector< std::pair< size_t, std::map< size_t, CSDMapEntry >::const_iterator > > iteration_state_;
};

} // namespace nest

#endif /* CONNECTION_MANAGER_H */
