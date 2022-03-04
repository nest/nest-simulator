/*
 *  adjustentry.cpp
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

#include "adjustentry.h"

namespace nest
{
adjustentry::adjustentry( const double t_lastspike,
  const double old_weight,
  const double t_received,
  const thread tid,
  const synindex syn_id,
  const index lcid )
  : t_lastspike_( t_lastspike )
  , old_weight_( old_weight )
  , t_received_( t_received )
  , tid_( tid )
  , syn_id_( syn_id )
  , lcid_( lcid )
{
}
}
