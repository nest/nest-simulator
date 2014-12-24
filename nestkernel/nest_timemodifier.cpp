/*
 *  nest_timemodifier.cpp
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

#include "nest_timemodifier.h"
#include "time.h"


void nest::TimeModifier::set_time_representation(nest::double_t tics_per_ms, double_t ms_per_step)
{
  nest::Time::set_resolution(tics_per_ms, ms_per_step); // set TICS_PER_STEP
}

void nest::TimeModifier::reset_to_defaults()
{
  nest::Time::reset_to_defaults();
}

