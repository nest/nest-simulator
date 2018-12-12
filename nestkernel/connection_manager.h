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

// Includes from nestkernel:
#include "conn_builder.h"
#include "connection_id.h"
#include "connector_base.h"
#include "gid_collection.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "source_table.h"
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
class Subnet;
class Event;
class SecondaryEvent;
class DelayChecker;
class GrowthCurve;
class SpikeData;

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

  void compute_target_data_buffer_size();
  void compute_compressed_secondary_recv_buffer_positions( const thread tid );

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
   * valid.
   *
   * \param sgid GID of the sending Node.
   * \param target Pointer to target Node.
   * \param target_thread Thread that hosts the target node.
   * \param syn_id The synapse model to use.
   * \param params Parameter dictionary to configure the synapse.
   * \param delay Delay of the connection (in ms).
   * \param weight Weight of the connection.
   */
  void connect( const index sgid,
    Node* target,
    thread target_thread,
    const synindex syn_id,
    const DictionaryDatum& params,
    const double_t delay = numerics::nan,
    const double_t weight = numerics::nan );

  /**
   * Connect two nodes. The source and target nodes are defined by their
   * global ID. The connection is established on the thread/process that owns
   * the target node.
   *
   * \param sgid GID of the sending Node.
   * \param target GID of the target Node.
   * \param params Parameter dictionary to configure the synapse.
   * \param syn_id The synapse model to use.
   */
  bool connect( const index sgid,
    const index target,
    const DictionaryDatum& params,
    const synindex syn_id );

  index find_connection( const thread tid,
    const synindex syn_id,
    const index sgid,
    const index tgid );

  void disconnect( const thread tid,
    const synindex syn_id,
    const index sgid,
    const index tgid );

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
   *
   * @note This method is used only by DataConnect.
   */
  bool data_connect_connectome( const ArrayDatum& connectome );

  /**
   * Connect one source node with many targets.
   * The dictionary d contains arrays for all the connections of type syn.
   * AKA DataConnect
   */
  void data_connect_single( const index source_id,
    DictionaryDatum d,
    const index syn );

  // aka conndatum GetStatus
  DictionaryDatum get_synapse_status( const index source_gid,
    const index target_gid,
    const thread tid,
    const synindex syn_id,
    const index lcid ) const;

  // aka conndatum SetStatus
  void set_synapse_status( const index source_gid,
    const index target_gid,
    const thread tid,
    const synindex syn_id,
    const index lcid,
    const DictionaryDatum& dict );

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
  ArrayDatum get_connections( const DictionaryDatum& params ) const;

  void get_connections( std::deque< ConnectionID >& connectome,
    TokenArray const* source,
    TokenArray const* target,
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

  void get_sources( const std::vector< index >& targets,
    const index syn_id,
    std::vector< std::vector< index > >& sources );

  void get_targets( const std::vector< index >& sources,
    const index syn_id,
    const std::string& post_synaptic_element,
    std::vector< std::vector< index > >& targets );

  const std::vector< Target >&
  get_remote_targets_of_local_node( const thread tid, const index lid ) const;

  index get_target_gid( const thread tid,
    const synindex syn_id,
    const index lcid ) const;

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

  void send( const thread tid,
    const synindex syn_id,
    const index lcid,
    const std::vector< ConnectorModel* >& cm,
    Event& e );

  /**
   * Send event e to all device targets of source source_gid
   */
  void send_to_devices( const thread tid, const index source_gid, Event& e );

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

  void add_target( const thread tid,
    const thread target_rank,
    const TargetData& target_data );

  /**
   * Return sort_connections_by_source_, which indicates whether
   * connections_ and source_table_ should be sorted according to
   * source gid.
   */
  bool get_sort_connections_by_source() const;

  /**
   * Sorts connections in the presynaptic infrastructure by increasing
   * source gid.
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
  bool have_connections_changed() const;

  /**
   * Sets flag indicating whether connection information needs to be
   * communicated.
   */
  void set_have_connections_changed( const bool changed );

  /**
   * Deletes TargetTable and resets processed flags of
   * SourceTable. This function must be called if connections are
   * created after connections have been communicated previously. It
   * basically restores the connection infrastructure to a state where
   * all information only exists on the postsynaptic side.
   */
  void restructure_connection_tables( const thread tid );

  void set_has_source_subsequent_targets( const thread tid,
    const synindex syn_id,
    const index lcid,
    const bool subsequent_targets );

  void no_targets_to_process( const thread tid );

  const std::vector< size_t >& get_secondary_send_buffer_positions(
    const thread tid,
    const index lid,
    const synindex syn_id ) const;

  /**
   * Returns read position in MPI receive buffer for secondary connections.
   */
  size_t get_secondary_recv_buffer_position( const thread tid,
    const synindex syn_id,
    const index lcid ) const;

  bool deliver_secondary_events( const thread tid,
    const bool called_from_wfr_update,
    std::vector< unsigned int >& recv_buffer );

  void compress_secondary_send_buffer_pos( const thread tid );

  void resize_connections();

  void sync_has_primary_connections();

  void check_secondary_connections_exist();

  bool has_primary_connections() const;

  bool secondary_connections_exist() const;

  index
  get_source_gid( const thread tid, const synindex syn_id, const index lcid );

  double get_stdp_eps() const;

  void set_stdp_eps( const double stdp_eps );

private:
  size_t get_num_target_data( const thread tid ) const;

  size_t get_num_connections_( const thread tid, const synindex syn_id ) const;

  void get_source_gids_( const thread tid,
    const synindex syn_id,
    const index tgid,
    std::vector< index >& sources );

  /**
   * Splits a TokenArray of GIDs to two vectors containing GIDs of neurons and
   * GIDs of devices.
   */
  void split_to_neuron_device_vectors_( const thread tid,
    TokenArray const* gid_token_array,
    std::vector< index >& neuron_gids,
    std::vector< index >& device_gids ) const;

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
   * \param s_gid The global id of the sending Node.
   * \param tid The thread of the target node.
   * \param syn_id The synapse model to use.
   * \param params The parameters for the connection.
   * \param delay The delay of the connection (optional).
   * \param weight The weight of the connection (optional).
   */
  void connect_( Node& source,
    Node& target,
    const index s_gid,
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
   * \param s_gid The global id of the sending Node.
   * \param tid The thread of the target node.
   * \param syn_id The synapse model to use.
   * \param params The parameters for the connection.
   * \param delay The delay of the connection (optional).
   * \param weight The weight of the connection (optional).
   */
  void connect_to_device_( Node& source,
    Node& target,
    const index s_gid,
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
   * \param s_gid The global id of the sending Node.
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
   * A structure to hold the global ids of presynaptic neurons during
   * postsynaptic connection creation, before the connection
   * information has been transferred to the presynaptic side.
   * Internally arranged in a 3d structure: threads|synapses|gids
   */
  SourceTable source_table_;

  /**
   * Stores absolute position in receive buffer of secondary events.
   * structure: threads|synapses|position
   */
  std::vector< std::vector< std::vector< size_t > > >
    secondary_recv_buffer_pos_;

  std::map< index, size_t > buffer_pos_of_source_gid_syn_id_;

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

  /**
   * @BeginDocumentation
   * Name: connruledict - dictionary containing all connectivity rules
   *
   * Description:
   * This dictionary provides the connection rules that can be used
   * in Connect.
   * 'connruledict info' shows the contents of the dictionary.
   *
   * SeeAlso: Connect
   */
  DictionaryDatum connruledict_; //!< Dictionary for connection rules.

  //! ConnBuilder factories, indexed by connruledict_ elements.
  std::vector< GenericConnBuilderFactory* > connbuilder_factories_;

  delay min_delay_; //!< Value of the smallest delay in the network.

  delay max_delay_; //!< Value of the largest delay in the network in steps.

  //! Whether to keep source table after connection setup is complete.
  bool keep_source_table_;

  //! True if new connections have been created since startup or last call to
  //! simulate.
  bool have_connections_changed_;

  //! Whether to sort connections by source gid.
  bool sort_connections_by_source_;

  //! Whether primary connections (spikes) exist.
  bool has_primary_connections_;

  //! Whether secondary connections (e.g., gap junctions) exist.
  bool secondary_connections_exist_;

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
ConnectionManager::get_remote_targets_of_local_node( const thread tid,
  const index lid ) const
{
  return target_table_.get_targets( tid, lid );
}

inline bool
ConnectionManager::have_connections_changed() const
{
  return have_connections_changed_;
}

inline void
ConnectionManager::set_have_connections_changed( const bool changed )
{
  have_connections_changed_ = changed;
}

inline void
ConnectionManager::add_target( const thread tid,
  const thread target_rank,
  const TargetData& target_data )
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
  return source_table_.get_next_target_data(
    tid, rank_start, rank_end, target_rank, next_target_data );
}

inline const std::vector< size_t >&
ConnectionManager::get_secondary_send_buffer_positions( const thread tid,
  const index lid,
  const synindex syn_id ) const
{
  return target_table_.get_secondary_send_buffer_positions( tid, lid, syn_id );
}

inline size_t
ConnectionManager::get_secondary_recv_buffer_position( const thread tid,
  const synindex syn_id,
  const index lcid ) const
{
  return secondary_recv_buffer_pos_[ tid ][ syn_id ][ lcid ];
}

inline size_t
ConnectionManager::get_num_connections_( const thread tid,
  const synindex syn_id ) const
{
  return connections_[ tid ][ syn_id ]->size();
}

inline index
ConnectionManager::get_source_gid( const thread tid,
  const synindex syn_index,
  const index lcid )
{
  return source_table_.get_gid( tid, syn_index, lcid );
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

inline double
ConnectionManager::get_stdp_eps() const
{
  return stdp_eps_;
}

inline index
ConnectionManager::get_target_gid( const thread tid,
  const synindex syn_id,
  const index lcid ) const
{
  return connections_[ tid ][ syn_id ]->get_target_gid( tid, lcid );
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
ConnectionManager::set_has_source_subsequent_targets( const thread tid,
  const synindex syn_id,
  const index lcid,
  const bool subsequent_targets )
{
  connections_[ tid ][ syn_id ]->set_has_source_subsequent_targets(
    lcid, subsequent_targets );
}

} // namespace nest

#endif /* CONNECTION_MANAGER_H */
