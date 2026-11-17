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

#include "connection_manager.h"

#include <cassert>
#include <cstddef>
#include <vector>

#include "event.h"
#include "event_delivery_manager.h"
#include "kernel_manager.h"
#include "nest_time.h"
#include "node.h"
#include "simulation_manager.h"
#include "vp_manager.h"

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

template <>
inline void
EventDeliveryManager::send< DSSpikeEvent >( Node& source, DSSpikeEvent& e, const long lag )
{
  e.set_sender_node_id( source.get_node_id() );
  send_local_( source, e, lag );
}

template < class EventT >
inline void
EventDeliveryManager::send( Node& source, EventT& e, const long lag )
{
  send_local_( source, e, lag );
}

inline void
EventDeliveryManager::send_off_grid_remote( size_t tid, SpikeEvent& e, const long lag )
{
  // Put the spike in a buffer for the remote machines
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( e.get_sender().get_node_id() );
  const auto& targets = kernel::manager< ConnectionManager >.get_remote_targets_of_local_node( tid, lid );

  for ( const auto& target : targets )
  {
    // Unroll spike multiplicity as plastic synapses only handle individual spikes.
    for ( size_t i = 0; i < e.get_multiplicity(); ++i )
    {
      off_grid_emitted_spikes_register_[ tid ]->emplace_back( target, lag, e.get_offset() );
    }
  }
}

inline void
EventDeliveryManager::send_remote( size_t tid, SpikeEvent& e, const long lag )
{
  // Put the spike in a buffer for the remote machines
  const size_t lid = kernel::manager< VPManager >.node_id_to_lid( e.get_sender().get_node_id() );
  const auto& targets = kernel::manager< ConnectionManager >.get_remote_targets_of_local_node( tid, lid );

  for ( const auto& target : targets )
  {
    // Unroll spike multiplicity as plastic synapses only handle individual spikes.
    for ( size_t i = 0; i < e.get_multiplicity(); ++i )
    {
      emitted_spikes_register_[ tid ]->emplace_back( target, lag, e.is_flush_event() );
    }
  }
}

}  // namespace nest

#endif /* EVENT_DELIVERY_MANAGER_IMPL_H */
