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


#include <cmath>

namespace nest
{

class ConnectorModel;
class Network;

/**
 * Manages the available connection prototypes and connections. It provides
 * the interface to establish and modify connections between nodes.
 */
class ConnectionManager
{
  friend class ConnectionBuilderManager;

public:
  ConnectionManager();
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

  // aka CopyModel for synapse models
  synindex copy_synapse_prototype( synindex old_id, std::string new_name );

  bool has_user_prototypes() const;

private:
  std::vector< ConnectorModel* > pristine_prototypes_; //!< The list of clean synapse prototypes
  std::vector< std::vector< ConnectorModel* > > prototypes_; //!< The list of available synapse
                                                             //!< prototypes: first dimension one
                                                             //!< entry per thread, second dimension
                                                             //!< for each synapse type

  Dictionary* synapsedict_; //!< The synapsedict (owned by the network)


  void init_();

  void clear_prototypes_();


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
