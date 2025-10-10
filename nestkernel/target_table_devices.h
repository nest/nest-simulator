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

// Includes from nestkernel:
#include "connection_id.h"
#include "connector_base.h"
#include "event.h"
#include "nest_types.h"

// Includes from SLI:
#include "dictdatum.h"

namespace nest
{
class Node;
class ConnectorModel;

/**
 * This data structure stores the connections between local neurons
 * and devices. The core structure is a two dimensional vector, which
 * is arranged as follows:
 * - first dim: threads
 * - second dim: local nodes/neurons
 */
class TargetTableDevices
{
private:
  //! 3d structure storing connections from neurons to devices
  std::vector< std::vector< std::vector< ConnectorBase* > > > target_to_devices_;

  //! 3d structure storing connections from devices to neurons
  std::vector< std::vector< std::vector< ConnectorBase* > > > target_from_devices_;

  //! 3d structure storing node IDs of sending devices (necessary for
  //! get_connections)
  std::vector< std::vector< size_t > > sending_devices_node_ids_;

public:
  TargetTableDevices();
  ~TargetTableDevices();

  /**
   * Initialize data structures.
   */
  void initialize();

  /**
   * Delete data structure.
   */
  void finalize();

  /**
   * Adds a connection from the neuron source to the device target.
   */
  void add_connection_to_device( Node& source,
    Node& target,
    const size_t s_node_id,
    const size_t tid,
    const synindex syn_id,
    const DictionaryDatum& p,
    const double d,
    const double w );

  /**
   * Adds a connection from the device source to the neuron target.
   */
  void add_connection_from_device( Node& source,
    Node& target,
    const size_t tid,
    const synindex syn_id,
    const DictionaryDatum& p,
    const double d,
    const double w );

  /**
   * Sends a spike event to all targets of the source neuron.
   */
  void send_to_device( const size_t tid, const size_t s_node_id, Event& e, const std::vector< ConnectorModel* >& cm );
  void send_to_device( const size_t tid,
    const size_t s_node_id,
    SecondaryEvent& e,
    const std::vector< ConnectorModel* >& cm );

  /**
   * Sends a spike event to all targets of the source device.
   */
  void send_from_device( const size_t tid, const size_t ldid, Event& e, const std::vector< ConnectorModel* >& cm );

  /**
   * Resizes vectors according to number of local nodes.
   */
  void resize_to_number_of_neurons();

  /**
   * Resizes vectors according to number of available synapse types.
   */
  void resize_to_number_of_synapse_types();

  /**
   * Returns all connections from neurons to devices.
   */
  void get_connections_to_devices_( const size_t requested_source_node_id,
    const size_t requested_target_node_id,
    const size_t tid,
    const synindex synapse_id,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const;

  /**
   * Returns all connections from particular neuron to devices.
   */
  void get_connections_to_device_for_lid_( const size_t lid,
    const size_t requested_target_node_id,
    const size_t tid,
    const synindex syn_id,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const;

  /**
   * Returns all connections from devices to neurons.
   */
  void get_connections_from_devices_( const size_t requested_source_node_id,
    const size_t requested_target_node_id,
    const size_t tid,
    const synindex synapse_id,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const;

  /**
   * Returns all connections between neurons and devices.
   */
  void get_connections( const size_t requested_source_node_id,
    const size_t requested_target_node_id,
    const size_t tid,
    const synindex synapse_id,
    const long synapse_label,
    std::deque< ConnectionID >& conns ) const;

  /**
   * Returns synapse status of connection from neuron to device.
   */
  void get_synapse_status_to_device( const size_t tid,
    const size_t source_node_id,
    const synindex syn_id,
    DictionaryDatum& dict,
    const size_t lcid ) const;

  /**
   * Returns synapse status of connection from device to neuron.
   */
  void get_synapse_status_from_device( const size_t tid,
    const size_t ldid,
    const synindex syn_id,
    DictionaryDatum& dict,
    const size_t lcid ) const;

  /**
   * Sets synapse status of connection from neuron to device.
   */
  void set_synapse_status_to_device( const size_t tid,
    const size_t source_node_id,
    const synindex syn_id,
    ConnectorModel& cm,
    const DictionaryDatum& dict,
    const size_t lcid );

  /**
   * Sets synapse status of connection from device to neuron.
   */
  void set_synapse_status_from_device( const size_t tid,
    const size_t ldid,
    const synindex syn_id,
    ConnectorModel& cm,
    const DictionaryDatum& dict,
    const size_t lcid );

  /**
   * Checks if the device has any connections in this thread
   */
  bool is_device_connected( size_t tid, size_t lcid ) const;
};

} // namespace nest

#endif
