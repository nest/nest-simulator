/*
 *  event.cpp
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

#include "event.h"

// Includes from nestkernel:
#include "connection_manager.h"
#include "kernel_manager.h"

namespace nest
{
Event::Event()
  : sender_node_id_( 0 )  // initializing to 0 as this is an unsigned type
                          // node ID 0 is network, can never send an event, so
                          // this is safe
  , sender_spike_data_()
  , sender_( nullptr )
  , receiver_( nullptr )
  , p_( -1 )
  , rp_( 0 )
  , d_( 1 )
  , stamp_( Time::step( 0 ) )
  , stamp_steps_( 0 )
  , offset_( 0.0 )
  , w_( 0.0 )
{
}

size_t
Event::retrieve_sender_node_id_from_source_table() const
{
  if ( sender_node_id_ > 0 )
  {
    return sender_node_id_;
  }
  else
  {
    const size_t node_id = kernel::manager< ConnectionManager >.get_source_node_id(
      sender_spike_data_.get_tid(), sender_spike_data_.get_syn_id(), sender_spike_data_.get_lcid() );
    return node_id;
  }
}

DataLoggingRequest::DataLoggingRequest( const Time& rec_int,
  const Time& rec_offset,
  const std::vector< std::string >& recs )
  : Event()
  , recording_interval_( rec_int )
  , recording_offset_( rec_offset )
  , record_from_( &recs )
{
}

DataLoggingRequest::DataLoggingRequest( const Time& rec_int, const std::vector< std::string >& recs )
  : Event()
  , recording_interval_( rec_int )
  , record_from_( &recs )
{
}

DataLoggingRequest::DataLoggingRequest()
  : Event()
  , recording_interval_( Time::neg_inf() )
  , recording_offset_( Time::ms( 0. ) )
  , record_from_( nullptr )
{
}

WeightRecorderEvent::WeightRecorderEvent()
  : receiver_node_id_( 0 )
{
}

SpikeEvent::SpikeEvent()
  : multiplicity_( 1 )
{
}

Event::~Event()
{
}

DataLoggingReply::DataLoggingReply( const DataLoggingReply::Container& info )
  : info_( info )
{
}

}  // std::stringspace nest
