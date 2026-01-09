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
#include "target_table_devices.h"
#include "connector_base.h"
#include "connector_model_impl.h"
#include "kernel_manager.h"
#include "model_manager.h"

#include "node_manager.h"

namespace nest
{

TargetTableDevices::TargetTableDevices()
{
}

TargetTableDevices::~TargetTableDevices()
{
}

void
TargetTableDevices::initialize()
{
  const size_t num_threads = kernel::manager< VPManager >.get_num_threads();
  target_to_devices_.resize( num_threads );
  target_from_devices_.resize( num_threads );
  sending_devices_node_ids_.resize( num_threads );
}

void
TargetTableDevices::finalize()
{
  for ( size_t tid = 0; tid < target_to_devices_.size(); ++tid )
  {
    for ( auto iit = target_to_devices_[ tid ].begin(); iit != target_to_devices_[ tid ].end(); ++iit )
    {
      for ( std::vector< ConnectorBase* >::iterator iiit = iit->begin(); iiit != iit->end(); ++iiit )
      {
        delete *iiit;
      }
    }
  }

  for ( size_t tid = 0; tid < target_from_devices_.size(); ++tid )
  {
    for ( auto iit = target_from_devices_[ tid ].begin(); iit != target_from_devices_[ tid ].end(); ++iit )
    {
      for ( std::vector< ConnectorBase* >::iterator iiit = iit->begin(); iiit != iit->end(); ++iiit )
      {
        delete *iiit;
      }
    }
  }

  std::vector< std::vector< std::vector< ConnectorBase* > > >().swap( target_to_devices_ );
  std::vector< std::vector< std::vector< ConnectorBase* > > >().swap( target_from_devices_ );
  std::vector< std::vector< size_t > >().swap( sending_devices_node_ids_ );
}

void
TargetTableDevices::resize_to_number_of_neurons()
{
#pragma omp parallel
  {
    const size_t tid = kernel::manager< VPManager >.get_thread_id();
    target_to_devices_[ tid ].resize( kernel::manager< NodeManager >.get_max_num_local_nodes() + 1 );
    target_from_devices_[ tid ].resize( kernel::manager< NodeManager >.get_num_thread_local_devices( tid ) + 1 );
    sending_devices_node_ids_[ tid ].resize( kernel::manager< NodeManager >.get_num_thread_local_devices( tid ) + 1 );
  } // end omp parallel
}

void
TargetTableDevices::resize_to_number_of_synapse_types()
{
  kernel::manager< VPManager >.assert_thread_parallel();

  const size_t tid = kernel::manager< VPManager >.get_thread_id();
  for ( size_t lid = 0; lid < target_to_devices_.at( tid ).size(); ++lid )
  {
    // make sure this device has support for all synapse types
    target_to_devices_.at( tid ).at( lid ).resize(
      kernel::manager< ModelManager >.get_num_connection_models(), nullptr );
  }
  for ( size_t ldid = 0; ldid < target_from_devices_.at( tid ).size(); ++ldid )
  {
    // make sure this device has support for all synapse types
    target_from_devices_.at( tid ).at( ldid ).resize(
      kernel::manager< ModelManager >.get_num_connection_models(), nullptr );
  }
}

void
TargetTableDevices::get_connections_to_devices_( const size_t requested_source_node_id,
  const size_t requested_target_node_id,
  const size_t tid,
  const synindex syn_id,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  if ( requested_source_node_id != 0 )
  {
    const size_t lid = kernel::manager< VPManager >.node_id_to_lid( requested_source_node_id );
    if ( kernel::manager< VPManager >.lid_to_node_id( lid ) != requested_source_node_id )
    {
      return;
    }
    get_connections_to_device_for_lid_( lid, requested_target_node_id, tid, syn_id, synapse_label, conns );
  }
  else
  {
    for ( size_t lid = 0; lid < target_to_devices_[ tid ].size(); ++lid )
    {
      get_connections_to_device_for_lid_( lid, requested_target_node_id, tid, syn_id, synapse_label, conns );
    }
  }
}

void
TargetTableDevices::get_connections_to_device_for_lid_( const size_t lid,
  const size_t requested_target_node_id,
  const size_t tid,
  const synindex syn_id,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  if ( target_to_devices_[ tid ][ lid ].size() > 0 )
  {
    const size_t source_node_id = kernel::manager< VPManager >.lid_to_node_id( lid );
    // not the valid connector
    if ( source_node_id > 0 and target_to_devices_[ tid ][ lid ][ syn_id ] )
    {
      target_to_devices_[ tid ][ lid ][ syn_id ]->get_all_connections(
        source_node_id, requested_target_node_id, tid, synapse_label, conns );
    }
  }
}

void
TargetTableDevices::get_connections_from_devices_( const size_t requested_source_node_id,
  const size_t requested_target_node_id,
  const size_t tid,
  const synindex syn_id,
  const long synapse_label,
  std::deque< ConnectionID >& conns ) const
{
  for ( std::vector< size_t >::const_iterator it = sending_devices_node_ids_[ tid ].begin();
        it != sending_devices_node_ids_[ tid ].end();
        ++it )
  {
    const size_t source_node_id = *it;
    if ( source_node_id > 0 and ( requested_source_node_id == source_node_id or requested_source_node_id == 0 ) )
    {
      const Node* source = kernel::manager< NodeManager >.get_node_or_proxy( source_node_id, tid );
      const size_t ldid = source->get_local_device_id();

      if ( target_from_devices_[ tid ][ ldid ].size() > 0 )
      {
        // not the valid connector
        if ( target_from_devices_[ tid ][ ldid ][ syn_id ] )
        {
          target_from_devices_[ tid ][ ldid ][ syn_id ]->get_all_connections(
            source_node_id, requested_target_node_id, tid, synapse_label, conns );
        }
      }
    }
  }
}

void
TargetTableDevices::get_connections( const size_t requested_source_node_id,
  const size_t requested_target_node_id,
  const size_t tid,
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

void
TargetTableDevices::add_connection_to_device( Node& source,
  Node& target,
  const size_t source_node_id,
  const size_t tid,
  const synindex syn_id,
  const DictionaryDatum& p,
  const double d,
  const double w )
{
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( source_node_id );
  assert( lid < target_to_devices_[ tid ].size() );
  assert( syn_id < target_to_devices_[ tid ][ lid ].size() );

  kernel::manager< ModelManager >.get_connection_model( syn_id, tid ).add_connection( source, target, target_to_devices_[ tid ][ lid ], syn_id, p, d, w );
}

void
TargetTableDevices::add_connection_from_device( Node& source,
  Node& target,
  const size_t tid,
  const synindex syn_id,
  const DictionaryDatum& p,
  const double d,
  const double w )
{
  const size_t ldid = source.get_local_device_id();
  assert( ldid != invalid_index );
  assert( ldid < target_from_devices_[ tid ].size() );
  assert( syn_id < target_from_devices_[ tid ][ ldid ].size() );

  kernel::manager< ModelManager >.get_connection_model( syn_id, tid ).add_connection( source, target, target_from_devices_[ tid ][ ldid ], syn_id, p, d, w );

  // store node ID of sending device
  sending_devices_node_ids_[ tid ][ ldid ] = source.get_node_id();
}


void
TargetTableDevices::get_synapse_status_from_device( const size_t tid,
  const size_t ldid,
  const synindex syn_id,
  DictionaryDatum& dict,
  const size_t lcid ) const
{
  target_from_devices_[ tid ][ ldid ][ syn_id ]->get_synapse_status( tid, lcid, dict );
}

void
TargetTableDevices::set_synapse_status_from_device( const size_t tid,
  const size_t ldid,
  const synindex syn_id,
  ConnectorModel& cm,
  const DictionaryDatum& dict,
  const size_t lcid )
{
  target_from_devices_[ tid ][ ldid ][ syn_id ]->set_synapse_status( lcid, dict, cm );
}

void
TargetTableDevices::send_from_device( const size_t tid,
  const size_t ldid,
  Event& e,
  const std::vector< ConnectorModel* >& cm )
{
  for ( std::vector< ConnectorBase* >::iterator it = target_from_devices_[ tid ][ ldid ].begin();
        it != target_from_devices_[ tid ][ ldid ].end();
        ++it )
  {
    if ( *it )
    {
      ( *it )->send_to_all( tid, cm, e );
    }
  }
}

bool
TargetTableDevices::is_device_connected( const size_t tid, const size_t lcid ) const
{
  for ( auto& synapse : target_from_devices_[ tid ][ lcid ] )
  {
    if ( synapse )
    {
      std::deque< ConnectionID > conns;
      synapse->get_all_connections( lcid, 0, tid, UNLABELED_CONNECTION, conns );
      if ( not conns.empty() )
      {
        return true;
      }
    }
  }
  return false;
}

void
TargetTableDevices::send_to_device( const size_t tid,
  const size_t source_node_id,
  Event& e,
  const std::vector< ConnectorModel* >& cm )
{
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( source_node_id );
  for ( std::vector< ConnectorBase* >::iterator it = target_to_devices_[ tid ][ lid ].begin();
        it != target_to_devices_[ tid ][ lid ].end();
        ++it )
  {
    if ( *it )
    {
      ( *it )->send_to_all( tid, cm, e );
    }
  }
}

void
TargetTableDevices::send_to_device( const size_t tid,
  const size_t source_node_id,
  SecondaryEvent& e,
  const std::vector< ConnectorModel* >& cm )
{
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( source_node_id );
  for ( auto& synid : e.get_supported_syn_ids() )
  {
    if ( target_to_devices_[ tid ][ lid ][ synid ] )
    {
      target_to_devices_[ tid ][ lid ][ synid ]->send_to_all( tid, cm, e );
    }
  }
}

void
TargetTableDevices::get_synapse_status_to_device( const size_t tid,
  const size_t source_node_id,
  const synindex syn_id,
  DictionaryDatum& dict,
  const size_t lcid ) const
{
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( source_node_id );
  if ( target_to_devices_[ tid ][ lid ][ syn_id ] )
  {
    target_to_devices_[ tid ][ lid ][ syn_id ]->get_synapse_status( tid, lcid, dict );
  }
}

void
TargetTableDevices::set_synapse_status_to_device( const size_t tid,
  const size_t source_node_id,
  const synindex syn_id,
  ConnectorModel& cm,
  const DictionaryDatum& dict,
  const size_t lcid )
{
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( source_node_id );
  if ( target_to_devices_[ tid ][ lid ][ syn_id ] )
  {
    target_to_devices_[ tid ][ lid ][ syn_id ]->set_synapse_status( lcid, dict, cm );
  }
}

}
