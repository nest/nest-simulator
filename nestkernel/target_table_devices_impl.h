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
#include "target_table_devices.h"
#include "kernel_manager.h"
#include "vp_manager_impl.h"
#include "model_manager.h"
#include "node.h"
#include "connector_base.h"

inline void
nest::TargetTableDevices::add_connection_to_device( Node& source, Node& target, index s_gid, thread tid, index syn, double_t d, double_t w )
{
  const index lid = kernel().vp_manager.gid_to_lid( s_gid );
  assert( lid < target_to_devices_[ tid ]->size() );
  kernel().model_manager.get_synapse_prototype( syn, tid ).add_connection_5g(
    source, target, (*target_to_devices_[ tid ])[ lid ], syn, d, w );
}

inline void
nest::TargetTableDevices::add_connection_from_device( Node& source, Node& target, index s_gid, thread tid, index syn, double_t d, double_t w )
{
  const index ldid = source.get_local_device_id();
  assert( ldid != invalid_index );
  assert( ldid < target_from_devices_[ tid ]->size() );
  kernel().model_manager.get_synapse_prototype( syn, tid ).add_connection_5g(
    source, target, (*target_from_devices_[ tid ])[ ldid ], syn, d, w );
}

inline void
nest::TargetTableDevices::send_to_device( thread tid, const index s_gid, Event& e, const std::vector< ConnectorModel* >& cm )
{
  const index lid = kernel().vp_manager.gid_to_lid( s_gid );
  (*target_to_devices_[ tid ])[ lid ]->send_to_all( e, tid, cm );
}

inline void
nest::TargetTableDevices::send_from_device( thread tid, const index ldid, Event& e, const std::vector< ConnectorModel* >& cm )
{
  (*target_from_devices_[ tid ])[ ldid ]->send_to_all( e, tid, cm );
}

#endif /* TARGET_TABLE_DEVICES_IMPL_H */
