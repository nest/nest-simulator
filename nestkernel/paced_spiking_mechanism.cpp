/*
 *  paced_spiking_mechanism.cpp
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

#include "paced_spiking_mechanism.h"

// nestkernel
#include "exceptions.h"
#include "nest_names.h"

// libnestutil
#include "dict_util.h"

namespace nest
{

PacedSpikingMechanism::PacedSpikingMechanism()
  : paced_spiking_( false )
  , paced_spiking_offset_( 0.0 )
  , paced_spiking_interval_( 100.0 )
{
}

PacedSpikingMechanism::PacedSpikingMechanism( const PacedSpikingMechanism& n )
  : paced_spiking_( n.paced_spiking_ )
  , paced_spiking_offset_( n.paced_spiking_offset_ )
  , paced_spiking_interval_( n.paced_spiking_interval_ )
{
}

void
PacedSpikingMechanism::pre_run_hook()
{
  if ( not paced_spiking_ )
  {
    return;
  }

  paced_spiking_interval_steps_ = Time( Time::ms( paced_spiking_interval_ ) ).get_steps();
  steps_until_paced_spike_ = Time( Time::ms( paced_spiking_offset_ ) ).get_steps();
}

void
PacedSpikingMechanism::get_status( Dictionary& d ) const
{
  d[ names::paced_spiking ] = paced_spiking_;
  d[ names::paced_spiking_offset ] = paced_spiking_offset_;
  d[ names::paced_spiking_interval ] = paced_spiking_interval_;
}

void
PacedSpikingMechanism::set_status( const Dictionary& d )
{
  d.update_value( names::paced_spiking, paced_spiking_ );
  d.update_value( names::paced_spiking_offset, paced_spiking_offset_ );
  d.update_value( names::paced_spiking_interval, paced_spiking_interval_ );

  if ( paced_spiking_offset_ < 0.0 )
  {
    throw BadProperty( "Temporal offset of paced spiking paced_spiking_offset ≥ 0 required." );
  }

  if ( paced_spiking_interval_ <= 0.0 )
  {
    throw BadProperty( "Interval between consecutive paced spikes paced_spiking_interval > 0 required." );
  }
}

}  // namespace nest
