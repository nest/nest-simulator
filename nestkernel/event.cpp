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

/**
 *  @file event.cpp
 *  Implementation of Event::operator() for all event types.
 *  @note Must be isolated here, since it requires full access to
 *  classes Node and Scheduler.
 */

#include "event.h"

// Includes from nestkernel:
#include "node.h"

namespace nest
{
Event::Event()
  : sender_gid_( 0 ) // initializing to 0 as this is an unsigned type
                     // gid 0 is network, can never send an event, so
                     // this is safe
  , sender_( NULL )
  , receiver_( NULL )
  , p_( -1 )
  , rp_( 0 )
  , d_( 1 )
  , stamp_( Time::step( 0 ) )
  , stamp_steps_( 0 )
  , offset_( 0.0 )
  , w_( 0.0 )
{
}


void SpikeEvent::operator()()
{
  receiver_->handle( *this );
}

void WeightRecorderEvent::operator()()
{
  receiver_->handle( *this );
}

void DSSpikeEvent::operator()()
{
  sender_->event_hook( *this );
}

void RateEvent::operator()()
{
  receiver_->handle( *this );
}

void CurrentEvent::operator()()
{
  receiver_->handle( *this );
}

void DSCurrentEvent::operator()()
{
  sender_->event_hook( *this );
}

void ConductanceEvent::operator()()
{
  receiver_->handle( *this );
}

void DoubleDataEvent::operator()()
{
  receiver_->handle( *this );
}

void DataLoggingRequest::operator()()
{
  receiver_->handle( *this );
}

void DataLoggingReply::operator()()
{
  receiver_->handle( *this );
}

void GapJunctionEvent::operator()()
{
  receiver_->handle( *this );
}

void InstantaneousRateConnectionEvent::operator()()
{
  receiver_->handle( *this );
}

void DelayedRateConnectionEvent::operator()()
{
  receiver_->handle( *this );
}

void DiffusionConnectionEvent::operator()()
{
  receiver_->handle( *this );
}
}

nest::index
nest::Event::get_receiver_gid( void ) const
{
  return receiver_->get_gid();
}
