/*
 *  target_table_devices.cpp
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

// Includes from nestkernel:
#include "connector_base.h"
#include "kernel_manager.h"
#include "target_table_devices_impl.h"
#include "vp_manager_impl.h"

nest::TargetTableDevices::TargetTableDevices()
{
}

nest::TargetTableDevices::~TargetTableDevices()
{
}

void
nest::TargetTableDevices::initialize()
{
  const thread num_threads = kernel().vp_manager.get_num_threads();
  target_to_devices_.resize( num_threads );
  target_from_devices_.resize( num_threads );
  sending_devices_node_ids_.resize( num_threads );
}

void
nest::TargetTableDevices::finalize()
{
#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    for ( std::vector< std::vector< ConnectorBase* > >::iterator iit = target_to_devices_[ tid ].begin();
          iit != target_to_devices_[ tid ].end();
          ++iit )
    {
      for ( std::vector< ConnectorBase* >::iterator iiit = iit->begin(); iiit != iit->end(); ++iiit )
      {
        delete *iiit;
      }
    }

    for ( std::vector< std::vector< ConnectorBase* > >::iterator iit = target_from_devices_[ tid ].begin();
          iit != target_from_devices_[ tid ].end();
          ++iit )
    {
      for ( std::vector< ConnectorBase* >::iterator iiit = iit->begin(); iiit != iit->end(); ++iiit )
      {
        delete *iiit;
      }
    }
  } // end omp parallel

  std::vector< std::vector< std::vector< ConnectorBase* > > >().swap( target_to_devices_ );
  std::vector< std::vector< std::vector< ConnectorBase* > > >().swap( target_from_devices_ );
  std::vector< std::vector< index > >().swap( sending_devices_node_ids_ );
}

void
nest::TargetTableDevices::resize_to_number_of_neurons()
{
#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    target_to_devices_[ tid ].resize( kernel().node_manager.get_max_num_local_nodes() + 1 );
    target_from_devices_[ tid ].resize( kernel().node_manager.get_num_local_devices() + 1 );
    sending_devices_node_ids_[ tid ].resize( kernel().node_manager.get_num_local_devices() + 1 );
  } // end omp parallel
}

void
nest::TargetTableDevices::resize_to_number_of_synapse_types()
{
#pragma omp parallel
  {
    const thread tid = kernel().vp_manager.get_thread_id();
    for ( index lid = 0; lid < target_to_devices_[ tid ].size(); ++lid )
    {
      // make sure this device has support for all synapse types
      target_to_devices_[ tid ][ lid ].resize( kernel().model_manager.get_num_synapse_prototypes(), NULL );
    }
    for ( index ldid = 0; ldid < target_from_devices_[ tid ].size(); ++ldid )
    {
      // make sure this device has support for all synapse types
      target_from_devices_[ tid ][ ldid ].resize( kernel().model_manager.get_num_synapse_prototypes(), NULL );
    }
  } // end omp parallel
}

void
nest::TargetTableDevices::get_connections_to_devices_( const index requested_source_node_id,
  const index requested_target_node_id,
  const thread tid,
  const synindex syn_id,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  if ( requested_source_node_id != 0 )
  {
    const index lid = kernel().vp_manager.node_id_to_lid( requested_source_node_id );
    get_connections_to_device_for_lid_( lid, requested_target_node_id, tid, syn_id, synapse_label, conns );
  }
  else
  {
    for ( index lid = 0; lid < target_to_devices_[ tid ].size(); ++lid )
    {
      get_connections_to_device_for_lid_( lid, requested_target_node_id, tid, syn_id, synapse_label, conns );
    }
  }
}

void
nest::TargetTableDevices::get_connections_to_device_for_lid_( const index lid,
  const index requested_target_node_id,
  const thread tid,
  const synindex syn_id,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  if ( target_to_devices_[ tid ][ lid ].size() > 0 )
  {
    const index source_node_id = kernel().vp_manager.lid_to_node_id( lid );
    // not the root subnet and valid connector
    if ( source_node_id > 0 and target_to_devices_[ tid ][ lid ][ syn_id ] != NULL )
    {
      target_to_devices_[ tid ][ lid ][ syn_id ]->get_all_connections(
        source_node_id, requested_target_node_id, tid, synapse_label, conns );
    }
  }
}

void
nest::TargetTableDevices::get_connections_from_devices_( const index requested_source_node_id,
  const index requested_target_node_id,
  const thread tid,
  const synindex syn_id,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  for ( std::vector< index >::const_iterator it = sending_devices_node_ids_[ tid ].begin();
        it != sending_devices_node_ids_[ tid ].end();
        ++it )
  {
    const index source_node_id = *it;
    if ( source_node_id > 0 and ( requested_source_node_id == source_node_id or requested_source_node_id == 0 ) )
    {
      const Node* source = kernel().node_manager.get_node_or_proxy( source_node_id, tid );
      const index ldid = source->get_local_device_id();

      if ( target_from_devices_[ tid ][ ldid ].size() > 0 )
      {
        // not the root subnet and valid connector
        if ( target_from_devices_[ tid ][ ldid ][ syn_id ] != NULL )
        {
          target_from_devices_[ tid ][ ldid ][ syn_id ]->get_all_connections(
            source_node_id, requested_target_node_id, tid, synapse_label, conns );
        }
      }
    }
  }
}

void
nest::TargetTableDevices::get_connections( const index requested_source_node_id,
  const index requested_target_node_id,
  const thread tid,
  const synindex syn_id,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  // collect all connections from neurons to devices
  get_connections_to_devices_( requested_source_node_id, requested_target_node_id, tid, syn_id, synapse_label, conns );

  // collect all connections from devices
  get_connections_from_devices_(
    requested_source_node_id, requested_target_node_id, tid, syn_id, synapse_label, conns );
}
