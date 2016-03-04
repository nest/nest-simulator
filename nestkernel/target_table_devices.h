/*
 *  target_table_devices.h
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

#ifndef TARGET_TABLE_DEVICES_H
#define TARGET_TABLE_DEVICES_H

// C++ includes:
#include <vector>
#include <map>
#include <cassert>

// Includes from nestkernel:
#include "nest_types.h"
#include "event.h"

// Includes from SLI:
#include "dictdatum.h"

namespace nest
{
class Node;
class HetConnector;
class ConnectorModel;

/** This data structure stores the connections of the local neurons to
 * devices. The core structure is a two dimensional vector, which is
 * arranged as follows:
 * 1st dimension: threads
 * 2nd dimension: local nodes/neurons
 */
class TargetTableDevices
{
private:
  //! 3d structure storing connections
  std::vector< std::vector< HetConnector* >* > target_to_devices_;
  std::vector< std::vector< HetConnector* >* > target_from_devices_;
  
public:
  TargetTableDevices();
  ~TargetTableDevices();
  //! initialize data structures
  void initialize();
  //! delete data structure
  void finalize();
  //! add a connection from the neuron source to the device target
  void add_connection_to_device( Node& source, Node& target, index s_gid, thread tid, index syn, double_t d, double_t w );
  void add_connection_to_device( Node& source, Node& target, index s_gid, thread tid, index syn, DictionaryDatum& p, double_t d, double_t w );
  //! add a connection from the device source to the neuron target
  void add_connection_from_device( Node& source, Node& target, index s_gid, thread tid, index syn, double_t d, double_t w );
  void add_connection_from_device( Node& source, Node& target, index s_gid, thread tid, index syn, DictionaryDatum& p, double_t d, double_t w );
  //! send a spike event to all targets of the source neuron
  void send_to_device( thread tid, const index s_gid, Event& e, const std::vector< ConnectorModel* >& cm );
  //! send a spike event to all targets of the source device
  void send_from_device( thread tid, const index ldid, Event& e, const std::vector< ConnectorModel* >& cm );

  //! resize the target table according to number of local nodes
  void resize();
};

} // namespace nest

#endif
