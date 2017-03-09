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

namespace nest
{

template < class EventT >
inline void
EventDeliveryManager::send( Node& source, EventT& e, const long lag )
{
  e.set_stamp(
    kernel().simulation_manager.get_slice_origin() + Time::step( lag + 1 ) );
  e.set_sender( source );
  thread t = source.get_thread();
  index gid = source.get_gid();

  assert( not source.has_proxies() );
  kernel().connection_manager.send( t, gid, e );
}

template <>
inline void
EventDeliveryManager::send< SpikeEvent >( Node& source,
  SpikeEvent& e,
  const long lag )
{
  e.set_stamp(
    kernel().simulation_manager.get_slice_origin() + Time::step( lag + 1 ) );
  e.set_sender( source );
  thread t = source.get_thread();

  if ( source.has_proxies() )
  {
    if ( source.is_off_grid() )
    {
      send_offgrid_remote( t, e, lag );
    }
    else
    {
      send_remote( t, e, lag );
    }
  }
  else
  {
    send_local( t, source, e );
  }
}

template <>
inline void
EventDeliveryManager::send< DSSpikeEvent >( Node& source,
  DSSpikeEvent& e,
  const long lag )
{
  e.set_stamp(
    kernel().simulation_manager.get_slice_origin() + Time::step( lag + 1 ) );
  e.set_sender( source );
  thread t = source.get_thread();

  assert( not source.has_proxies() );
  send_local( t, source, e );
}

inline void
EventDeliveryManager::send_secondary( Node& source, SecondaryEvent& e )
{
  e.set_stamp(
    kernel().simulation_manager.get_slice_origin() + Time::step( 1 ) );
  e.set_sender( source );
  e.set_sender_gid( source.get_gid() );
  thread t = source.get_thread();
  send_remote( t, e );
}

inline size_t
EventDeliveryManager::write_toggle() const
{
  return kernel().simulation_manager.get_slice() % 2;
}

inline void
EventDeliveryManager::send_local( thread t, Node& source, Event& e )
{
  index sgid = source.get_gid();
  e.set_sender_gid( sgid );
  kernel().connection_manager.send( t, sgid, e );
}
}

#endif
