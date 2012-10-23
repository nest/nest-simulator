/*
 *  event_priority.h
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

#ifndef EVENT_PRIORITY_H
#define EVENT_PRIORITY_H

#include "event.h"

namespace nest
{
  bool operator<(const Event &, const Event &);

  class EventPTRPriority
  {
  public:
    typedef Event const * value_type;
    typedef bool return_type;
    bool operator()(Event const *e1, Event const*e2) const
    {
      return !(*e1 < *e2);
    }
  };

  inline
  bool operator<(const Event &e1, const Event &e2)
  {
   return 
     (e1.get_stamp().get_steps()+e1.get_delay()) < (e2.get_stamp().get_steps()+e2.get_delay());
  }

}
#endif
