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
#include "nest.h"
#include "nest_time.h"
#include "nest_timeconverter.h"

namespace nest {

  TimeConverter::TimeConverter()
  {
    OLD_TICS_PER_STEP = Time::get_tics_per_step();
    OLD_TICS_PER_MS = Time::get_tics_per_ms();
  }

  Time TimeConverter::from_old_steps(long_t s_old) const
  {
    double ms = s_old * OLD_TICS_PER_STEP / OLD_TICS_PER_MS; 
    return Time::ms(ms);
  }

  Time TimeConverter::from_old_tics(tic_t t_old) const
  {
    double ms = t_old / OLD_TICS_PER_MS;
    return Time::ms(ms);
  }

}
