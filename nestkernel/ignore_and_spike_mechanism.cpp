/*
 *  ignore_and_spike_mechanism.cpp
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

#include "ignore_and_spike_mechanism.h"

// nestkernel
#include "exceptions.h"
#include "nest_names.h"

// libnestutil
#include "dict_util.h"

namespace nest
{

IgnoreAndSpikeMechanism::IgnoreAndSpikeMechanism()
  : ignore_and_spike_( false )
  , offset_( 100.0 )
  , interval_( 100.0 )
{
}

IgnoreAndSpikeMechanism::IgnoreAndSpikeMechanism( const IgnoreAndSpikeMechanism& n )
  : ignore_and_spike_( n.ignore_and_spike_ )
  , offset_( n.offset_ )
  , interval_( n.interval_ )
{
}

void
IgnoreAndSpikeMechanism::pre_run_hook()
{
  if ( not ignore_and_spike_ )
  {
    return;
  }

  interval_steps_ = Time( Time::ms( interval_ ) ).get_steps();
  steps_until_spike_ = Time( Time::ms( offset_ ) ).get_steps();
}

void
IgnoreAndSpikeMechanism::get_status( Dictionary& d ) const
{
  d[ names::ignore_and_spike ] = ignore_and_spike_;
  d[ names::ignore_and_spike_offset ] = offset_;
  d[ names::ignore_and_spike_interval ] = interval_;
}

void
IgnoreAndSpikeMechanism::set_status( const Dictionary& d )
{
  d.update_value( names::ignore_and_spike, ignore_and_spike_ );
  d.update_value( names::ignore_and_spike_offset, offset_ );
  d.update_value( names::ignore_and_spike_interval, interval_ );

  if ( offset_ <= 0.0 )
  {
    throw BadProperty( "Temporal offset of ignore-and-spike mechanism ignore_and_spike_offset > 0 required." );
  }

  if ( interval_ <= 0.0 )
  {
    throw BadProperty(
      "Interval between consecutive spikes in ignore-and-spike mechanism ignore_and_spike_interval > 0 required." );
  }
}

}  // namespace nest
