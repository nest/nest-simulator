/*
 *  target_table_devices_impl.h
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

#ifndef TARGET_TABLE_DEVICES_IMPL_H
#define TARGET_TABLE_DEVICES_IMPL_H

// Includes from nestkernel:
#include "connector_base.h"
#include "kernel_manager.h"
#include "model_manager.h"
#include "node.h"
#include "target_table_devices.h"
#include "vp_manager_impl.h"

inline void
nest::TargetTableDevices::add_connection_to_device( Node& source,
  Node& target,
  const index source_node_id,
  const thread tid,
  const synindex syn_id,
  const DictionaryDatum& p,
  const double d,
  const double w )
{
  const index lid = kernel().vp_manager.node_id_to_lid( source_node_id );
  assert( lid < target_to_devices_[ tid ].size() );
  assert( syn_id < target_to_devices_[ tid ][ lid ].size() );

  kernel()
    .model_manager.get_synapse_prototype( syn_id, tid )
    .add_connection( source, target, target_to_devices_[ tid ][ lid ], syn_id, p, d, w );
}

inline void
nest::TargetTableDevices::add_connection_from_device( Node& source,
  Node& target,
  const thread tid,
  const synindex syn_id,
  const DictionaryDatum& p,
  const double d,
  const double w )
{
  const index ldid = source.get_local_device_id();
  assert( ldid != invalid_index );
  assert( ldid < target_from_devices_[ tid ].size() );
  assert( syn_id < target_from_devices_[ tid ][ ldid ].size() );

  kernel()
    .model_manager.get_synapse_prototype( syn_id, tid )
    .add_connection( source, target, target_from_devices_[ tid ][ ldid ], syn_id, p, d, w );

  // store node ID of sending device
  sending_devices_node_ids_[ tid ][ ldid ] = source.get_node_id();
}

inline void
nest::TargetTableDevices::send_to_device( const thread tid,
  const index source_node_id,
  Event& e,
  const std::vector< ConnectorModel* >& cm )
{
  const index lid = kernel().vp_manager.node_id_to_lid( source_node_id );
  for ( std::vector< ConnectorBase* >::iterator it = target_to_devices_[ tid ][ lid ].begin();
        it != target_to_devices_[ tid ][ lid ].end();
        ++it )
  {
    if ( *it != NULL )
    {
      ( *it )->send_to_all( tid, cm, e );
    }
  }
}

inline void
nest::TargetTableDevices::send_to_device( const thread tid,
  const index source_node_id,
  SecondaryEvent& e,
  const std::vector< ConnectorModel* >& cm )
{
  const index lid = kernel().vp_manager.node_id_to_lid( source_node_id );
  const std::vector< synindex >& supported_syn_ids = e.get_supported_syn_ids();
  for ( std::vector< synindex >::const_iterator cit = supported_syn_ids.begin(); cit != supported_syn_ids.end(); ++cit )
  {
    if ( target_to_devices_[ tid ][ lid ][ *cit ] != NULL )
    {
      target_to_devices_[ tid ][ lid ][ *cit ]->send_to_all( tid, cm, e );
    }
  }
}

inline void
nest::TargetTableDevices::get_synapse_status_to_device( const thread tid,
  const index source_node_id,
  const synindex syn_id,
  DictionaryDatum& dict,
  const index lcid ) const
{
  const index lid = kernel().vp_manager.node_id_to_lid( source_node_id );
  if ( target_to_devices_[ tid ][ lid ][ syn_id ] != NULL )
  {
    target_to_devices_[ tid ][ lid ][ syn_id ]->get_synapse_status( tid, lcid, dict );
  }
}

inline void
nest::TargetTableDevices::set_synapse_status_to_device( const thread tid,
  const index source_node_id,
  const synindex syn_id,
  ConnectorModel& cm,
  const DictionaryDatum& dict,
  const index lcid )
{
  const index lid = kernel().vp_manager.node_id_to_lid( source_node_id );
  if ( target_to_devices_[ tid ][ lid ][ syn_id ] != NULL )
  {
    target_to_devices_[ tid ][ lid ][ syn_id ]->set_synapse_status( lcid, dict, cm );
  }
}

#endif /* TARGET_TABLE_DEVICES_IMPL_H */
