/*
 *  syn_id_delay.cpp
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

#include "syn_id_delay.h"

namespace nest
{

SynIdDelay::SynIdDelay( double d )
  : delay( 0 )
  , syn_id( invalid_synindex )
  , more_targets( false )
  , disabled( false )
{
  set_delay_ms( d );
}

SynIdDelay::SynIdDelay( const SynIdDelay& s ) = default;
SynIdDelay& SynIdDelay::operator=( const SynIdDelay& s ) = default;

double
SynIdDelay::get_delay_ms() const
{
  return Time::delay_steps_to_ms( delay );
}

void
SynIdDelay::set_delay_ms( const double d )
{
  delay = Time::delay_ms_to_steps( d );
}

void
SynIdDelay::set_source_has_more_targets( const bool more )
{
  more_targets = more;
}

bool
SynIdDelay::source_has_more_targets() const
{
  return more_targets;
}

void
SynIdDelay::disable()
{
  disabled = true;
}

bool
SynIdDelay::is_disabled() const
{
  return disabled;
}

} // namespace nest
