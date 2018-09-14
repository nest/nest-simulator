/*
 *  modelrange.h
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

#ifndef MODELRANGE_H
#define MODELRANGE_H

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

class modelrange
{
public:
  modelrange( index model, index first_gid, index last_gid );
  bool
  is_in_range( index gid ) const
  {
    return ( ( gid >= first_gid_ ) and ( gid <= last_gid_ ) );
  }
  index
  get_model_id() const
  {
    return model_;
  }
  index
  get_first_gid() const
  {
    return first_gid_;
  }
  index
  get_last_gid() const
  {
    return last_gid_;
  }
  void extend_range( index new_last_gid );

private:
  index model_;
  index first_gid_;
  index last_gid_;
};
}

#endif
