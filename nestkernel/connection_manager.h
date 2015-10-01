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

#include "nest_types.h"
#include "model.h"
#include "dictutils.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "arraydatum.h"
#include "sparsetable.h"

#include <cmath>

namespace nest
{
class ConnectorBase;
class ConnectorModel;
class spikecounter;
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
  ConnectionManager();
  ~ConnectionManager();

  void init();
  void reset();

  /**
   * Add ConnectionManager specific stuff to the root status dictionary
   */
  void get_status( DictionaryDatum& d ) const;

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
   * The function then iterates all entries in source and collects the connection IDs to all neurons
   * in target.
   */
  ArrayDatum get_connections( DictionaryDatum params ) const;

  void get_connections( ArrayDatum& connectome,
    TokenArray const* source,
    TokenArray const* target,
    size_t syn_id ) const;

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
    double_t d = NAN,
    double_t w = NAN );
  void connect( Node& s,
    Node& r,
    index s_gid,
    thread tid,
    index syn,
    DictionaryDatum& p,
    double_t d = NAN,
    double_t w = NAN );


  /**
   * Experimental bulk connector. See documentation in network.h
   */
  bool connect( ArrayDatum& d );

  void trigger_update_weight( const long_t vt_gid,
    const std::vector< spikecounter >& dopa_spikes,
    const double_t t_trig );

  void send( thread t, index sgid, Event& e );

private:

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

  ConnectorBase* validate_source_entry( thread tid, index s_gid, synindex syn_id );

};

} // namespace


#endif /* #ifndef CONNECTION_MANAGER_H */
