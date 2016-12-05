/*
 *  event_impl.h
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

#ifndef EVENT_IMPL_H
#define EVENT_IMPL_H

#include "event.h"

// Includes from nestkernel:
#include "kernel_manager.h"

namespace nest
{

inline size_t
GapJunctionEvent::prototype_size() const
{
  // size_t s = number_of_uints_covered< synindex >(); // first entry is syn_id
  // to identify type of event
  // s += number_of_uints_covered< index >(); // second entry is gid of sender
  return number_of_uints_covered< double >()
    * kernel().connection_manager.get_min_delay()
    * ( kernel().simulation_manager.get_wfr_interpolation_order()
           + 1 ); // payload
  // return s;
}

} // namespace nest

#endif // EVENT_IMPL_H
