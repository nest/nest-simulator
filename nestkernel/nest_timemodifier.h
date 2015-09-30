/*
 *  nest_timemodifier.h
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

#ifndef NEST_TIMEMODIFIER_H
#define NEST_TIMEMODIFIER_H

#include "network.h"

/*
TimeModifier is an interface class which defines the networks's
ability to modify the representation of time. It is only safe
to change the number of tics representing a millisecond if no
Time objects exist or it is guaranteed that all Time objects are
reinitialized before usage. Only the network can do this.
Therefore the functions
  set_tics_per_ms
and
  set_tics_per_step_default
are only accessible by specific members of the Network
 (reset, set_status)

Diesmann
*/


namespace nest
{

class TimeModifier
{
  // allow Network::set_status to change Time representation
  friend void NodeManager::set_status( index, const DictionaryDatum& );

  // allow Network::rest to change Time representation
  friend void Network::reset();

private:
  /**
   * Set the rime represeantation (TICS_PER_MS_, MS_PER_TICS_ and TICS_PER_STEP)
   * \param tics_per_ms number of TICS per ms
   * \param ms_per_step amount of ms per simulation step
   */
  static void set_time_representation( double_t tics_per_ms, double_t ms_per_step );

  /**
   * Reset TICS_PER_MS_, MS_PER_TICS_ and TICS_PER_STEP_ to compiled in default values
   */
  static void reset_to_defaults();
};


} // Namespace


#endif
