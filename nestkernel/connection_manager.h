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
#include "connector.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "arraydatum.h"

#include "sparsetable.h"

namespace nest
{

class Network;
class ConnectorModel;

/**
 * Manages the available connection prototypes and connections. It provides
 * the interface to establish and modify connections between nodes.
 */
class ConnectionManager
{
  struct syn_id_connector
  {
    index syn_id;
    Connector* connector;
  };

  typedef std::vector< syn_id_connector > tVConnector;
  typedef google::sparsetable< tVConnector > tVVConnector;
  typedef std::vector< tVVConnector > tVVVConnector;

public:
  ConnectionManager(Network& net);
  ~ConnectionManager();

  void init(Dictionary*);
  void reset();
  
  /**
   * Register a synapse type. This is called by Network::register_synapse_prototype.
   * Returns an id, which is needed to unregister the prototype later.
   */
  index register_synapse_prototype(ConnectorModel * cf);

  /**
   * Checks, whether connections of the given type were created
   */
  bool synapse_prototype_in_use(index syn_id);

  /**
   * Unregister a previously registered synapse prototype.
   */
  void unregister_synapse_prototype(index syn_id);

  /**
   * Try, if it is possible to unregister synapse prototype.
   * Throw exception, if not possible.
   */
  void try_unregister_synapse_prototype(index syn_id);

  /** 
   * Add ConnectionManager specific stuff to the root status dictionary
   */
  void get_status(DictionaryDatum& d) const;

  // aka SetDefaults for synapse models
  void set_prototype_status(index syn_id, const DictionaryDatum& d);
  // aka GetDefaults for synapse models
  DictionaryDatum get_prototype_status(index syn_id) const;

  // aka conndatum GetStatus
  DictionaryDatum get_synapse_status(index gid, index syn_id, port p, thread tid);
  // aka conndatum SetStatus
  void set_synapse_status(index gid, index syn_id, port p, thread tid, const DictionaryDatum& d);

  DictionaryDatum get_connector_status(const Node& node, index syn_id);
  DictionaryDatum get_connector_status(index gid, index syn_id);
  void set_connector_status(Node& node, index syn_id, thread tid, const DictionaryDatum& d);
  
  ArrayDatum find_connections(DictionaryDatum params);
  void find_connections(ArrayDatum& connectome, thread t, index source, int syn_vec_index, index syn_id, DictionaryDatum params);

  /**
   * Return connections between pairs of neurons.
   * The params dictionary can have the following entries:
   * 'source' a token array with GIDs of source neurons.
   * 'target' a token array with GIDs of target neuron.
   * If either of these does not exist, all neuron are used for the respective entry.
   * 'synapse_model' name of the synapse model, or all synapse models are searched.
   * The function then iterates all entries in source and collects the connection IDs to all neurons in target.
   * get_connections will eventually replace find_connections.
   */
  ArrayDatum get_connections(DictionaryDatum params) const;

  void get_connections(ArrayDatum& connectome, TokenArray const *source, TokenArray const *target, size_t syn_id) const;

  /**
   * Return connections between source and any neuron on thread t.
   */
  void get_connections(ArrayDatum& connectome, index source, thread t,  index syn_id) const;

  // aka CopyModel for synapse models
  index copy_synapse_prototype(index old_id, std::string new_name);

  bool has_user_prototypes() const;

  bool get_user_set_delay_extrema() const;

  const Time get_min_delay() const;
  const Time get_max_delay() const;

  /**
   * Count the number of connections in all connectors and update the
   * global counter num_connections_ in ConnectionManager and the
   * local counters in the prototype objects.
   * @note This function must be const, because it is called by const
   * functions like get_num_connections() and get_status(). The number
   * of connections is stored in a mutable variable num_connections_.
   */
  void count_connections() const;

  /**
   * Make sure that the connection counters are up-to-date and return
   * the total number of connections in the network.
   */
  size_t get_num_connections() const;

  /**
   * Connect is used to establish a connection between a sender and
   * receiving node.
   * \param s A reference to the sending Node.
   * \param r A reference to the receiving Node.
   * \param t The thread of the target node.
   * \param syn The synapse model to use.
   * \returns The receiver port number for the new connection
   */
  void connect(Node& s, Node& r, index s_gid, thread tid, index syn);
  void connect(Node& s, Node& r, index s_gid, thread tid, double_t w, double_t d, index syn);
  void connect(Node& s, Node& r, index s_gid, thread tid, DictionaryDatum& p, index syn);

  /** 
   * Experimental bulk connector. See documentation in network.h
   */
  bool connect(ArrayDatum &d);

  void send(thread t, index sgid, Event& e);

  /**
   * Resize the structures for the Connector objects if necessary.
   * This function should be called after number of threads, min_delay, max_delay, 
   * and time representation have been changed in the scheduler. 
   * The TimeConverter is used to convert times from the old to the new representation.
   * It is also forwarding the calibration
   * request to all ConnectorModel objects.
   */  
  void calibrate(const TimeConverter &);

private:

  std::vector<ConnectorModel*> pristine_prototypes_; //!< The list of clean synapse prototypes
  std::vector<ConnectorModel*> prototypes_;          //!< The list of available synapse prototypes

  Network& net_;            //!< The reference to the network
  Dictionary* synapsedict_; //!< The synapsedict (owned by the network)

  /**
   * A 3-dim structure to hold the Connector objects which in turn hold the connection
   * information.
   * - First dim: A std::vector for each local thread
   * - Second dim: A std::vector for each node on each thread
   * - Third dim: A std::vector for each synapse prototype, holding the Connector objects
   */
  tVVVConnector connections_;
  
  mutable size_t num_connections_;              //!< The global counter for the number of synapses
  mutable bool num_conn_changed_since_counted_; //!< Did the number of synapses change since counting?

  void init_();
  void delete_connections_();
  void clear_prototypes_();
  
  index validate_connector(thread tid, index gid, index syn_id);

  /**
   * Return pointer to protoype for given synapse id.
   * @throws UnknownSynapseType
   */
  const ConnectorModel& get_synapse_prototype(index syn_id) const;

  /**
   * Asserts validity of synapse index, otherwise throws exception.
   * @throws UnknownSynapseType
   */
  void assert_valid_syn_id(index syn_id) const;

  /**
   * For a given thread, source gid and synapse id, return the correct
   * index (syn_vec_index) into the connection store, so that one can
   * access the corresponding connector using
   * \code
       connections_[tid][gid][syn_vec_index].
     \endcode
   * @returns the index of the Connector or -1 if it does not exist.
   */  
  int get_syn_vec_index(thread tid, index gid, index syn_id) const;
};

inline
const ConnectorModel& ConnectionManager::get_synapse_prototype(index syn_id) const
{
  assert_valid_syn_id(syn_id);
  return *(prototypes_[syn_id]);
}

inline
void ConnectionManager::assert_valid_syn_id(index syn_id) const
{
  if (syn_id >= prototypes_.size() || prototypes_[syn_id] == 0)
    throw UnknownSynapseType(syn_id);
}

inline
bool ConnectionManager::has_user_prototypes() const
{
  return prototypes_.size() > pristine_prototypes_.size();
}

inline
int ConnectionManager::get_syn_vec_index(thread tid, index gid, index syn_id) const
{
  if (static_cast<size_t>(tid) >= connections_.size() || gid >= connections_[tid].size() || connections_[tid][gid].size() == 0)
    return -1;

  index syn_vec_index = 0;
  while ( syn_vec_index < connections_[tid][gid].size() && connections_[tid][gid][syn_vec_index].syn_id != syn_id )
    syn_vec_index++;

  if (syn_vec_index == connections_[tid][gid].size())
    return -1;
  
  return syn_vec_index;  
}

} // namespace

#endif /* #ifndef CONNECTION_MANAGER_H */
