/*
 *  nest_timeconverter.h
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

/*
 * first version
 * date: april 2008
 * author: Moritz Helias
 *
 */

#ifndef NEST_TIME_CONVERTER_H
#define NEST_TIME_CONVERTER_H

#include "nest_types.h"

namespace nest
{
class Time;
/**
 * Class to convert times from one representation to another.
 * Creating an object of TimeConverter at a current time representation
 * saves the current values of TICS_PER_MS and TICS_PER_STEP.
 * After having changed the time representation,
 * the members from_old_steps and from_old_tics can be used
 * to convert steps or tics given with respect to the old representation
 * in the new representation.
 */
class TimeConverter
{

private:
  double OLD_TICS_PER_MS;
  double OLD_TICS_PER_STEP;

public:
  /**
   * Constructor saves current TICS_PER_MS and TICS_PER_STEP in
   * members OLD_TICS_PER_MS, OLD_TICS_PER_STEP.
   */
  TimeConverter();


  /**
   * Converts a given number of steps with respect to old representation
   * into a time object in current representation.
   *
   * Be careful not to call Time::get_steps() from an old Time object, as
   * it will use the new TICS_PER_STEP constant. Use
   * TimeConverter::from_old_tics instead.
   */
  Time from_old_steps( long s_old ) const;

  /**
   * Converts a given number of tics with respect to old representation
   * into a time object in current representation.
   */
  Time from_old_tics( tic_t t_old ) const;
};

} // of namespace nest

#endif
