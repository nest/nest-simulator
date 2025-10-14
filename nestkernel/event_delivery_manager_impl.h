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

namespace nest
{

template < class EventT >
inline void
EventDeliveryManager::send_local_( Node& source, EventT& e, const long lag )
{
  assert( not source.has_proxies() );
  e.set_stamp( kernel::manager< SimulationManager >.get_slice_origin() + Time::step( lag + 1 ) );
  e.set_sender( source );
  const size_t t = source.get_thread();
  const size_t ldid = source.get_local_device_id();
  kernel::manager< ConnectionManager >.send_from_device( t, ldid, e );
}


template < class EventT >
inline void
EventDeliveryManager::send( Node& source, EventT& e, const long lag )
{
  send_local_( source, e, lag );
}

template <>
inline void
EventDeliveryManager::send< SpikeEvent >( Node& source, SpikeEvent& e, const long lag )
{
  const size_t tid = source.get_thread();
  const size_t source_node_id = source.get_node_id();
  e.set_sender_node_id( source_node_id );
  if ( source.has_proxies() )
  {
    local_spike_counter_[ tid ] += e.get_multiplicity();

    e.set_stamp( kernel::manager< SimulationManager >.get_slice_origin() + Time::step( lag + 1 ) );
    e.set_sender( source );

    if ( source.is_off_grid() )
    {
      send_off_grid_remote( tid, e, lag );
    }
    else
    {
      send_remote( tid, e, lag );
    }
    kernel::manager< ConnectionManager >.send_to_devices( tid, source_node_id, e );
  }
  else
  {
    send_local_( source, e, lag );
  }
}

template <>
inline void
EventDeliveryManager::send< DSSpikeEvent >( Node& source, DSSpikeEvent& e, const long lag )
{
  e.set_sender_node_id( source.get_node_id() );
  send_local_( source, e, lag );
}


} // namespace nest

#endif /* EVENT_DELIVERY_MANAGER_IMPL_H */
