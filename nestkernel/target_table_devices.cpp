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

#include "target_table_devices.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "connector_base.h"
#include "vp_manager_impl.h"

// Includes from SLI:
// #include "arraydatum.h"

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
  sending_devices_gids_.resize( num_threads );
  for( thread tid = 0; tid < num_threads; ++tid)
  {
    target_to_devices_[ tid ] = new std::vector< HetConnector* >( 0 );
    target_from_devices_[ tid ] = new std::vector< HetConnector* >( 0 );
    sending_devices_gids_[ tid ] = new std::vector< index >( 0 );
  }
}

void
nest::TargetTableDevices::finalize()
{
  for( std::vector< std::vector< HetConnector* >* >::iterator it =
         target_to_devices_.begin(); it != target_to_devices_.end(); ++it )
  {
    for ( std::vector< HetConnector* >::iterator jt = (*it)->begin(); jt != (*it)->end(); ++jt )
    {
      delete *jt;
    }
    delete *it;
  }
  target_to_devices_.clear();
  for( std::vector< std::vector< HetConnector* >* >::iterator it =
         target_from_devices_.begin(); it != target_from_devices_.end(); ++it )
  {
    for ( std::vector< HetConnector* >::iterator jt = (*it)->begin(); jt != (*it)->end(); ++jt )
    {
      delete *jt;
    }
    delete *it;
  }
  target_from_devices_.clear();
  for( std::vector< std::vector< index >* >::iterator it =
         sending_devices_gids_.begin(); it != sending_devices_gids_.end(); ++it )
  {
    delete *it;
  }
  sending_devices_gids_.clear();
}

void
nest::TargetTableDevices::resize()
{
  const thread num_threads = kernel().vp_manager.get_num_threads();
  for( thread tid = 0; tid < num_threads; ++tid)
  {
    const size_t old_size_to_devices = target_to_devices_[ tid ]->size();
    target_to_devices_[ tid ]->resize( kernel().node_manager.get_max_num_local_nodes() );
    for ( size_t i = old_size_to_devices; i < target_to_devices_[ tid ]->size(); ++i )
    {
      (*target_to_devices_[ tid ])[ i ] = new HetConnector();
    }
    const size_t old_size_from_devices = target_from_devices_[ tid ]->size();
    target_from_devices_[ tid ]->resize( kernel().node_manager.get_num_local_devices() );
    sending_devices_gids_[ tid ]->resize( kernel().node_manager.get_num_local_devices(), 0 );
    for ( size_t i = old_size_from_devices; i < target_from_devices_[ tid ]->size(); ++i )
    {
      (*target_from_devices_[ tid ])[ i ] = new HetConnector();
    }
  }
}

size_t
nest::TargetTableDevices::get_num_connections_to_devices_( const thread tid, const synindex synapse_id ) const
{
  size_t num_connections = 0;
  for ( std::vector< HetConnector* >::const_iterator it = target_to_devices_[ tid ]->begin(); it != target_to_devices_[ tid ]->end(); ++it )
  {
    num_connections += (*it)->get_num_connections( synapse_id );
  }
  return num_connections;
}

size_t
nest::TargetTableDevices::get_num_connections_from_devices_( const thread tid, const synindex synapse_id ) const
{
  size_t num_connections = 0;
  for ( std::vector< HetConnector* >::const_iterator it = target_from_devices_[ tid ]->begin(); it != target_from_devices_[ tid ]->end(); ++it )
  {
    num_connections += (*it)->get_num_connections( synapse_id );
  }
  return num_connections;
}

void
nest::TargetTableDevices::get_connections_to_devices_( const index requested_source_gid, const index requested_target_gid, const thread tid, const synindex synapse_id, const long synapse_label, ArrayDatum& conns ) const
{
  for ( size_t lid = 0; lid < target_to_devices_[ tid ]->size(); ++lid )
  {
    const index source_gid = kernel().vp_manager.lid_to_gid( lid );
    if ( requested_source_gid == source_gid || requested_source_gid == 0 )
    {
      if ( source_gid > 0 ) // not the root subnet
      {
        (*target_to_devices_[ tid ])[ lid ]->get_all_connections( source_gid, requested_target_gid, tid, synapse_id, synapse_label, conns );
      }
    }
  }
}

void
nest::TargetTableDevices::get_connections_from_devices_( const index requested_source_gid, const index requested_target_gid, const thread tid, const synindex synapse_id, const long synapse_label, ArrayDatum& conns ) const
{
  for ( std::vector< index >::const_iterator it = sending_devices_gids_[ tid ]->begin();
        it != sending_devices_gids_[ tid ]->end(); ++it )
  {
    const Node* source = kernel().node_manager.get_node( *it, tid );
    const index source_gid = source->get_gid();
    if ( requested_source_gid == source_gid || requested_source_gid == 0 )
    {
      if ( source_gid > 0 ) // not the root subnet
      {
        const index ldid = source->get_local_device_id();
        (*target_from_devices_[ tid ])[ ldid ]->get_all_connections( source_gid, requested_target_gid, tid, synapse_id, synapse_label, conns );
      }
    }
  }
}

void
nest::TargetTableDevices::get_connections( const index requested_source_gid, const index requested_target_gid, const thread tid, const synindex synapse_id, const long synapse_label, ArrayDatum& conns ) const
{
  // collect all connections from neurons to devices
  const size_t num_connections_to_devices_in_thread = get_num_connections_to_devices_( tid, synapse_id );
  conns.reserve( conns.size() + num_connections_to_devices_in_thread );
  get_connections_to_devices_( requested_source_gid, requested_target_gid, tid, synapse_id, synapse_label, conns );

  // collect all connections from devices
  const size_t num_connections_from_devices_in_thread = get_num_connections_from_devices_( tid, synapse_id );
  conns.reserve( conns.size() + num_connections_from_devices_in_thread );
  get_connections_from_devices_( requested_source_gid, requested_target_gid, tid, synapse_id, synapse_label, conns );
}
