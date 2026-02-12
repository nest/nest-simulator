/*
 *  flush_event_mechanism.cpp
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

#include "flush_event_mechanism.h"

// nestkernel
#include "exceptions.h"
#include "nest_names.h"

// libnestutil
#include "dict_util.h"

namespace nest
{

FlushEventMechanism::FlushEventMechanism()
  : flush_event_send_interval_( std::numeric_limits< double >::infinity() )
  , last_event_time_( 0 )
{
}

FlushEventMechanism::FlushEventMechanism( const FlushEventMechanism& n )
  : flush_event_send_interval_( n.flush_event_send_interval_ )
  , last_event_time_( n.last_event_time_ )
{
}

void
FlushEventMechanism::get_status( Dictionary& d ) const
{
  d[ names::flush_event_send_interval ] = flush_event_send_interval_;
}

void
FlushEventMechanism::set_status( const Dictionary& d )
{
  d.update_value( names::flush_event_send_interval, flush_event_send_interval_ );

  if ( flush_event_send_interval_ <= 0.0 )
  {
    throw BadProperty(
      "Interval since previous event after which a flush event is sent flush_event_send_interval > 0 required." );
  }
}

} // namespace nest
