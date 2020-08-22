/*
 *  nest_timeconverter.cpp
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

#include "nest_timeconverter.h"

// Includes from nestkernel:
#include "nest_time.h"

namespace nest
{

TimeConverter::TimeConverter()
{
  OLD_TICS_PER_STEP = Time::get_tics_per_step();
  OLD_TICS_PER_MS = Time::get_tics_per_ms();
}

Time
TimeConverter::from_old_steps( long s_old ) const
{
  if ( s_old == Time::LIM_NEG_INF.steps or s_old == Time::LIM_POS_INF.steps )
  {
    return Time( Time::step( s_old ) );
  }
  double ms = s_old * OLD_TICS_PER_STEP / OLD_TICS_PER_MS;
  return Time::ms( ms );
}

Time
TimeConverter::from_old_tics( tic_t t_old ) const
{
  if ( t_old == Time::LIM_NEG_INF.tics or t_old == Time::LIM_POS_INF.tics )
  {
    return Time( Time::tic( t_old ) );
  }
  double ms = t_old / OLD_TICS_PER_MS;
  return Time::ms( ms );
}
}
