/*
 *  adjustentry.h
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

#ifndef ADJUSTENTRY_H
#define ADJUSTENTRY_H

#include "nest_types.h"

namespace nest
{
struct adjustentry
{
  adjustentry()
  {
  } // TODO: Should be safely removed

  adjustentry( double t_lastspike,
    double old_weight,
    double t_received,
    const thread tid,
    const synindex syn_id,
    const index lcid );

  double t_lastspike_;
  double old_weight_;
  double t_received_;
  thread tid_;
  synindex syn_id_;
  index lcid_;
};
}

#endif
