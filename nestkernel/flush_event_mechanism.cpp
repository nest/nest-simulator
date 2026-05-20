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
  , flush_event_send_interval_steps_( Time( Time::ms( flush_event_send_interval_ ) ).get_steps() )
  , last_event_time_( 0 )
{
}

FlushEventMechanism::FlushEventMechanism( const FlushEventMechanism& n )
  : flush_event_send_interval_( n.flush_event_send_interval_ )
  , flush_event_send_interval_steps_( n.flush_event_send_interval_steps_ )
  , last_event_time_( n.last_event_time_ )
{
}

void
FlushEventMechanism::pre_run_hook()
{
  flush_event_send_interval_steps_ = Time( Time::ms( flush_event_send_interval_ ) ).get_steps();
}

void
FlushEventMechanism::get_status( Dictionary& d ) const
{
  d[ names::flush_event_send_interval ] = flush_event_send_interval_;
}

void
FlushEventMechanism::set_status( const Dictionary& d, const bool check_eprop_constraint )
{
  double flush_event_send_interval_tmp = flush_event_send_interval_;

  d.update_value( names::flush_event_send_interval, flush_event_send_interval_tmp );

  if ( flush_event_send_interval_tmp <= 0.0 )
  {
    throw BadProperty( "flush_event_send_interval > 0 required." );
  }

  if ( check_eprop_constraint )
  {
    const double eprop_update_interval = kernel().simulation_manager.get_eprop_update_interval().get_ms();
    if ( flush_event_send_interval_tmp < eprop_update_interval )
    {
      throw BadProperty( "flush_event_send_interval ≥ eprop_update_interval required." );
    }
  }

  flush_event_send_interval_ = flush_event_send_interval_tmp;
}

}  // namespace nest
