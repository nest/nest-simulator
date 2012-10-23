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
#include "dictutils.h"
#include "connector.h"
#include "nest_time.h"
#include "nest_timeconverter.h"
#include "compound.h"
#include "arraydatum.h"

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
  typedef std::vector< Connector* > tVConnector;
  typedef std::vector< tVConnector > tVVConnector;
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
  void set_connector_status(Node& node, index syn_id, thread tid, const DictionaryDatum& d);
  
  ArrayDatum find_connections(DictionaryDatum params);
  void find_connections(ArrayDatum& connectome, thread t, index source, index syn_id, DictionaryDatum params);

  // aka CopyModel for synapse models
  index copy_synapse_prototype(index old_id, std::string new_name);

  bool has_user_prototypes() const;
  
  size_t get_num_connections() const;

  const Time get_min_delay() const;
  const Time get_max_delay() const;

  /**
   * Connect is used to establish a connection between a sender and
   * receiving node.
   * \param s A reference to the sending Node.
   * \param r A reference to the receiving Node.
   * \param t The thread of the target node.
   * \param syn The synapse model to use.
   * \returns The receiver port number for the new connection
   */ 
  void connect(Node& s, Node& r, thread tid, index syn);
  void connect(Node& s, Node& r, thread tid, double_t w, double_t d, index syn);
  void connect(Node& s, Node& r, thread tid, DictionaryDatum& p, index syn);
  
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

  std::vector<ConnectorModel*> pristine_prototypes_;   //!< The list of clean synapse prototypes
  std::vector<ConnectorModel*> prototypes_;            //!< The list of available synapse prototypes

  Network& net_;                               //!< The reference to the network
  Dictionary* synapsedict_;                    //!< The synapsedict (owned by the network)

  /**
   * A 3-dim structure to hold the Connector objects which in turn hold the connection
   * information.
   * - First dim: A std::vector for each local thread
   * - Second dim: A std::vectoir for each node on each thread
   * - Third dim: A std::vector for each synapse prototype, holding the Connector objects
   */
  tVVVConnector connections_;
  
  void init_();
  void delete_connections_();
  void clear_prototypes_();
  
  void validate_connector(thread tid, index gid, index syn_id);

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

} // namespace

#endif /* #ifndef CONNECTION_MANAGER_H */
