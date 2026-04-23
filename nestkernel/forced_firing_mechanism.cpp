/*
 *  forced_firing_mechanism.cpp
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

#include "forced_firing_mechanism.h"

// nestkernel
#include "exceptions.h"
#include "nest_names.h"

// libnestutil
#include "dict_util.h"

namespace nest
{

ForcedFiringMechanism::ForcedFiringMechanism()
  : forced_firing_( false )
  , forced_firing_offset_( 0.0 )
  , forced_firing_rate_( 10.0 )
{
}

ForcedFiringMechanism::ForcedFiringMechanism( const ForcedFiringMechanism& n )
  : forced_firing_( n.forced_firing_ )
  , forced_firing_offset_( n.forced_firing_offset_ )
  , forced_firing_rate_( n.forced_firing_rate_ )
{
}

bool
ForcedFiringMechanism::emit_spike( bool emit_dynamic_spike )
{
  const bool emit_forced_spike = steps_until_forced_spike_ % forced_firing_interval_steps_ == 0;
  bool emit_spike = forced_firing_ ? emit_forced_spike : emit_dynamic_spike;
  ++steps_until_forced_spike_;
  return emit_spike;
}

void
ForcedFiringMechanism::pre_run_hook()
{
  if ( not forced_firing_ )
  {
    return;
  }

  forced_firing_interval_steps_ = Time( Time::ms( 1000. / forced_firing_rate_ ) ).get_steps();
  steps_until_forced_spike_ = Time( Time::ms( forced_firing_offset_ ) ).get_steps();
}

void
ForcedFiringMechanism::get_status( Dictionary& d ) const
{
  d[ names::forced_firing ] = forced_firing_;
  d[ names::forced_firing_offset ] = forced_firing_offset_;
  d[ names::forced_firing_rate ] = forced_firing_rate_;
}

void
ForcedFiringMechanism::set_status( const Dictionary& d )
{
  d.update_value( names::forced_firing, forced_firing_ );
  d.update_value( names::forced_firing_offset, forced_firing_offset_ );
  d.update_value( names::forced_firing_rate, forced_firing_rate_ );

  if ( forced_firing_offset_ < 0.0 )
  {
    throw BadProperty( "Temporal offset of forced firing offset ≥ 0 required." );
  }

  if ( forced_firing_rate_ <= 0.0 )
  {
    throw BadProperty( "Rate of forced firing rate > 0 required." );
  }
}

}  // namespace nest
