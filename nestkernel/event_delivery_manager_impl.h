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
#include "spike_register_table.h"
#include "spike_register_table_impl.h"
#include "connection_builder_manager_impl.h"

namespace nest
{

template< class EventT >
inline void
EventDeliveryManager::send_local_( Node& source, EventT& e, const long_t lag )
{
  assert( !source.has_proxies() );
  e.set_stamp( kernel().simulation_manager.get_slice_origin() + Time::step( lag + 1 ) );
  e.set_sender( source );
  const thread t = source.get_thread();
  const index ldid = source.get_local_device_id();
  kernel().connection_builder_manager.send_from_device( t, ldid, e );
}

template < class EventT >
inline void
EventDeliveryManager::send( Node& source, EventT& e, const long_t lag )
{
  send_local_( source, e, lag );
}

template <>
inline void
EventDeliveryManager::send< SpikeEvent >( Node& source, SpikeEvent& e, const long_t lag )
{
  const index s_gid = source.get_gid();
  e.set_sender_gid( s_gid );
  if ( source.has_proxies() )
  {
    e.set_stamp( kernel().simulation_manager.get_slice_origin() + Time::step( lag + 1 ) );
    e.set_sender( source );
    const thread tid = source.get_thread();

    if ( source.is_off_grid() )
    {
      send_offgrid_remote( tid, e, lag );
    }
    else
    {
      send_remote( tid, e, lag );
      kernel().connection_builder_manager.send_to_devices( tid, s_gid, e );
    }
  }
  else
  {
    send_local_( source, e, lag );
  }
}

template <>
inline void
EventDeliveryManager::send< DSSpikeEvent >( Node& source, DSSpikeEvent& e, const long_t lag )
{
  e.set_sender_gid( source.get_gid() );
  send_local_( source, e, lag );
}

inline void
EventDeliveryManager::send_secondary( Node& source, SecondaryEvent& e )
{
  e.set_stamp( kernel().simulation_manager.get_slice_origin() + Time::step( 1 ) );
  e.set_sender( source );
  e.set_sender_gid( source.get_gid() );
  const thread t = source.get_thread();
  send_remote( t, e );
}

inline size_t
EventDeliveryManager::write_toggle() const
{
  return kernel().simulation_manager.get_slice() % 2;
}

} // of namespace nest

#endif
