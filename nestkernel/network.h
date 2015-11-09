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

#include <dirent.h>
#include <errno.h>

#include <vector>
#include <string>
#include <typeinfo>
#include <ostream>
#include <cmath>

#include "config.h"
#include "nest_types.h"
#include "nest_time.h"
#include "model.h"
#include "dict.h"
#include "dictdatum.h"


#ifdef M_ERROR
#undef M_ERROR
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

/**
 * @file network.h
 * Declarations for class Network.
 */
class SLIInterpreter;

namespace nest
{

class VPManager;
class NodeManager;
class sli_neuron; // needs a handle to the interpreter_

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
  friend class sli_neuron;

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
   * Reset number of threads to one, reset device prefix to the
   * empty string and call reset().
   */
  void reset_kernel();

  /**
   * @defgroup net_access Network access
   * Functions to access network nodes.
   */


  /**
   * Get properties of a node. The specified node must exist.
   * @throws nest::UnknownNode       Target does not exist in the network.
   */
  DictionaryDatum get_status( index );


private:
  SLIInterpreter& interpreter_;
};

inline Network&
Network::get_network()
{
  assert( created_network_instance_ );
  return *network_instance_;
}

/****** former Scheduler functions ******/

} // namespace

#endif
