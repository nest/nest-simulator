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

#include <vector>
#include <limits>

#include "nest.h"
#include "model.h"
#include "dictutils.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "arraydatum.h"
#include "sparsetable.h"
#include "numerics.h"
#include "../models/volume_transmitter.h"
#include <cmath>

namespace nest
{
class ConnectorBase;
class ConnectorModel;
class Network;

/**
 * Manages the available connection prototypes and connections. It provides
 * the interface to establish and modify connections between nodes.
 */
class ConnectionManager
{

  typedef google::sparsetable< ConnectorBase* > tSConnector; // for all neurons having targets
  typedef std::vector< tSConnector > tVSConnector;           // for all threads

public:
  ConnectionManager( Network& net );
  ~ConnectionManager();

  void init( Dictionary* );
  void reset();

  /**
   * Register a synapse type. This is called by Network::register_synapse_prototype.
   * Returns an id for the prototype.
   */
  synindex register_synapse_prototype( ConnectorModel* cf );

  /**
   * Checks, whether connections of the given type were created
   */
  bool synapse_prototype_in_use( synindex syn_id );

  /**
   * Add ConnectionManager specific stuff to the root status dictionary
   */
  void get_status( DictionaryDatum& d ) const;

  // aka SetDefaults for synapse models
  void set_prototype_status( synindex syn_id, const DictionaryDatum& d );
  // aka GetDefaults for synapse models
  DictionaryDatum get_prototype_status( synindex syn_id ) const;

  // aka conndatum GetStatus
  DictionaryDatum get_synapse_status( index gid, synindex syn_id, port p, thread tid );
  // aka conndatum SetStatus
  void
  set_synapse_status( index gid, synindex syn_id, port p, thread tid, const DictionaryDatum& d );

  /**
   * Return connections between pairs of neurons.
   * The params dictionary can have the following entries:
   * 'source' a token array with GIDs of source neurons.
   * 'target' a token array with GIDs of target neuron.
   * If either of these does not exist, all neuron are used for the respective entry.
   * 'synapse_model' name of the synapse model, or all synapse models are searched.
   * 'synapse_label' label (long_t) of the synapse, or all synapses are searched.
   * The function then iterates all entries in source and collects the connection IDs to all neurons
   * in target.
   */
  ArrayDatum get_connections( DictionaryDatum params ) const;

  void get_connections( ArrayDatum& connectome,
    TokenArray const* source,
    TokenArray const* target,
    size_t syn_id,
    long_t synapse_label ) const;

  // aka CopyModel for synapse models
  synindex copy_synapse_prototype( synindex old_id, std::string new_name );

  bool has_user_prototypes() const;

  bool get_user_set_delay_extrema() const;

  const Time get_min_delay() const;
  const Time get_max_delay() const;

  /**
   * Make sure that the connection counters are up-to-date and return
   * the total number of connections in the network.
   */
  size_t get_num_connections() const;

  /**
   * Connect is used to establish a connection between a sender and
   * receiving node.
   *
   * The parameters delay and weight have the default value NAN.
   * NAN is a special value in cmath, which describes double values that
   * are not a number. If delay or weight is omitted in an connect call,
   * NAN indicates this and weight/delay are set only, if they are valid.
   *
   * \param s A reference to the sending Node.
   * \param r A reference to the receiving Node.
   * \param t The thread of the target node.
   * \param syn The synapse model to use.
   * \returns The receiver port number for the new connection
   */
  void connect( Node& s,
    Node& r,
    index s_gid,
    thread tid,
    index syn,
    double_t d = numerics::nan,
    double_t w = numerics::nan );
  void connect( Node& s,
    Node& r,
    index s_gid,
    thread tid,
    index syn,
    DictionaryDatum& p,
    double_t d = numerics::nan,
    double_t w = numerics::nan );


  /**
   * Experimental bulk connector. See documentation in network.h
   */
  bool connect( ArrayDatum& d );

  void trigger_update_weight( const long_t vt_gid,
    const vector< spikecounter >& dopa_spikes,
    const double_t t_trig );

  void send( thread t, index sgid, Event& e );

  //! send secondary events, e.g. for gap junctions
  void send_secondary( thread t, SecondaryEvent& e );

  /**
   * Resize the structures for the Connector objects if necessary.
   * This function should be called after number of threads, min_delay, max_delay,
   * and time representation have been changed in the scheduler.
   * The TimeConverter is used to convert times from the old to the new representation.
   * It is also forwarding the calibration
   * request to all ConnectorModel objects.
   */
  void calibrate( const TimeConverter& );

  /**
   * Return pointer to protoype for given synapse id.
   * @throws UnknownSynapseType
   */
  const ConnectorModel& get_synapse_prototype( synindex syn_id, thread t = 0 ) const;

  /**
   * Asserts validity of synapse index, otherwise throws exception.
   * @throws UnknownSynapseType
   */
  void assert_valid_syn_id( synindex syn_id, thread t = 0 ) const;

private:
  std::vector< ConnectorModel* > pristine_prototypes_; //!< The list of clean synapse prototypes
  std::vector< std::vector< ConnectorModel* > > prototypes_; //!< The list of available synapse
                                                             //!< prototypes: first dimension one
                                                             //!< entry per thread, second dimension
                                                             //!< for each synapse type

  Network& net_;            //!< The reference to the network
  Dictionary* synapsedict_; //!< The synapsedict (owned by the network)

  /**
   * A 3-dim structure to hold the Connector objects which in turn hold the connection
   * information.
   * - First dim: A std::vector for each local thread
   * - Second dim: A std::vector for each node on each thread
   * - Third dim: A std::vector for each synapse prototype, holding the Connector objects
   */

  tVSConnector connections_;

  mutable size_t num_connections_; //!< The global counter for the number of synapses

  void init_();
  void delete_connections_();
  void clear_prototypes_();

  ConnectorBase* validate_source_entry( thread tid, index s_gid, synindex syn_id );
};

inline const ConnectorModel&
ConnectionManager::get_synapse_prototype( synindex syn_id, thread t ) const
{
  assert_valid_syn_id( syn_id );
  return *( prototypes_[ t ][ syn_id ] );
}

inline void
ConnectionManager::assert_valid_syn_id( synindex syn_id, thread t ) const
{
  if ( syn_id >= prototypes_[ t ].size() || prototypes_[ t ][ syn_id ] == 0 )
    throw UnknownSynapseType( syn_id );
}

inline bool
ConnectionManager::has_user_prototypes() const
{
  return prototypes_[ 0 ].size() > pristine_prototypes_.size();
}

} // namespace


#endif /* #ifndef CONNECTION_MANAGER_H */
