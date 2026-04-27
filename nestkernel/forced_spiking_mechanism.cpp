/*
 *  forced_spiking_mechanism.cpp
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

#include "forced_spiking_mechanism.h"

// nestkernel
#include "exceptions.h"
#include "nest_names.h"

// libnestutil
#include "dict_util.h"

namespace nest
{

ForcedSpikingMechanism::ForcedSpikingMechanism()
  : forced_spiking_( false )
  , forced_spiking_offset_( 0.0 )
  , forced_spiking_interval_( 100.0 )
{
}

ForcedSpikingMechanism::ForcedSpikingMechanism( const ForcedSpikingMechanism& n )
  : forced_spiking_( n.forced_spiking_ )
  , forced_spiking_offset_( n.forced_spiking_offset_ )
  , forced_spiking_interval_( n.forced_spiking_interval_ )
{
}

void
ForcedSpikingMechanism::pre_run_hook()
{
  if ( not forced_spiking_ )
  {
    return;
  }

  forced_spiking_interval_steps_ = Time( Time::ms( forced_spiking_interval_ ) ).get_steps();
  steps_until_forced_spike_ = Time( Time::ms( forced_spiking_offset_ ) ).get_steps();
}

void
ForcedSpikingMechanism::get_status( Dictionary& d ) const
{
  d[ names::forced_spiking ] = forced_spiking_;
  d[ names::forced_spiking_offset ] = forced_spiking_offset_;
  d[ names::forced_spiking_interval ] = forced_spiking_interval_;
}

void
ForcedSpikingMechanism::set_status( const Dictionary& d )
{
  d.update_value( names::forced_spiking, forced_spiking_ );
  d.update_value( names::forced_spiking_offset, forced_spiking_offset_ );
  d.update_value( names::forced_spiking_interval, forced_spiking_interval_ );

  if ( forced_spiking_offset_ < 0.0 )
  {
    throw BadProperty( "Temporal offset of forced spiking forced_spiking_offset ≥ 0 required." );
  }

  if ( forced_spiking_interval_ <= 0.0 )
  {
    throw BadProperty( "Interval between consecutive forced spikes forced_spiking_interval > 0 required." );
  }
}

}  // namespace nest
