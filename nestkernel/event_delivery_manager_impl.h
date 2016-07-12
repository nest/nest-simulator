/*
 *  event_delivery_manager_impl.h
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

#ifndef EVENT_DELIVERY_MANAGER_IMPL_H
#define EVENT_DELIVERY_MANAGER_IMPL_H

#include "event_delivery_manager.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "connection_manager_impl.h"

namespace nest
{

template< class EventT >
inline void
EventDeliveryManager::send_local_( Node& source, EventT& e, const long_t lag )
{
  assert( not source.has_proxies() );
  e.set_stamp( kernel().simulation_manager.get_slice_origin() + Time::step( lag + 1 ) );
  e.set_sender( source );
  const thread t = source.get_thread();
  const index ldid = source.get_local_device_id();
  kernel().connection_manager.send_from_device( t, ldid, e );
}

template < class EventT >
inline void
EventDeliveryManager::send( Node& source, EventT& e, const long_t lag )
{
  send_local_( source, e, lag );
}

template <>
inline void
EventDeliveryManager::send< SpikeEvent >( Node& source,
  SpikeEvent& e,
  const long_t lag )
{
  const index source_gid = source.get_gid();
  e.set_sender_gid( source_gid );
  if ( source.has_proxies() )
  {
    e.set_stamp(
      kernel().simulation_manager.get_slice_origin() + Time::step( lag + 1 ) );
    e.set_sender( source );
    const thread tid = source.get_thread();

    if ( source.is_off_grid() )
    {
      send_off_grid_remote( tid, e, lag );
    }
    else
    {
      send_remote( tid, e, lag );
    }
    kernel().connection_manager.send_to_devices( tid, source_gid, e );
  }
  else
  {
    send_local_( source, e, lag );
  }
}

template <>
inline void
EventDeliveryManager::send< DSSpikeEvent >( Node& source,
  DSSpikeEvent& e,
  const long_t lag )
{
  e.set_sender_gid( source.get_gid() );
  send_local_( source, e, lag );
}

inline void
EventDeliveryManager::send_remote( thread tid, SpikeEvent& e, const long_t lag )
{
  // Put the spike in a buffer for the remote machines
  const index lid = kernel().vp_manager.gid_to_lid( e.get_sender().get_gid() );
  const std::vector< Target >& targets = kernel().connection_manager.get_targets( tid, lid );

  for ( std::vector< Target >::const_iterator it = targets.begin(); it != targets.end(); ++it )
  {
    const thread assigned_tid = ( *it ).rank / kernel().vp_manager.get_num_assigned_ranks_per_thread();
    for ( int_t i = 0; i < e.get_multiplicity(); ++i )
    {
      ( *spike_register_5g_[ tid ] )[ assigned_tid ][ lag ].push_back( *it );
    }
  }
}

inline void
EventDeliveryManager::send_off_grid_remote( thread tid,
  SpikeEvent& e,
  const long_t lag )
{
  // Put the spike in a buffer for the remote machines
  const index lid = kernel().vp_manager.gid_to_lid( e.get_sender().get_gid() );
  const std::vector< Target >& targets = kernel().connection_manager.get_targets( tid, lid );

  for ( std::vector< Target >::const_iterator it = targets.begin(); it != targets.end(); ++it )
  {
    const thread assigned_tid = ( *it ).rank / kernel().vp_manager.get_num_assigned_ranks_per_thread();
    for ( int_t i = 0; i < e.get_multiplicity(); ++i )
    {
      ( *off_grid_spike_register_5g_[ tid ] )[ assigned_tid ][ lag ].push_back( OffGridTarget( *it, e.get_offset() ) );
    }
  }
}

inline void
EventDeliveryManager::send_secondary( Node& source, SecondaryEvent& e )
{
  e.set_stamp(
    kernel().simulation_manager.get_slice_origin() + Time::step( 1 ) );
  e.set_sender( source );
  e.set_sender_gid( source.get_gid() );
  const thread tid = source.get_thread();
  send_remote( tid, e );
}

inline size_t
EventDeliveryManager::write_toggle() const
{
  return kernel().simulation_manager.get_slice() % 2;
}

inline void
EventDeliveryManager::resize_spike_register_5g_( const thread tid )
{
  for ( std::vector< std::vector< std::vector< Target > > >::iterator it = (*spike_register_5g_[ tid ]).begin();
        it != (*spike_register_5g_[ tid ]).end(); ++it )
  {
    it->resize( kernel().connection_manager.get_min_delay(), std::vector< Target >( 0 ) );
  }

  for ( std::vector< std::vector< std::vector< OffGridTarget > > >::iterator it = (*off_grid_spike_register_5g_[ tid ]).begin();
        it != (*off_grid_spike_register_5g_[ tid ]).end(); ++it )
  {
    it->resize( kernel().connection_manager.get_min_delay(), std::vector< OffGridTarget >( 0 ) );
  }
}


} // of namespace nest

#endif
