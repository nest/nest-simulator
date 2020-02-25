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
  modelrange( index model, index first_node_id, index last_node_id );
  bool
  is_in_range( index node_id ) const
  {
    return ( ( node_id >= first_node_id_ ) and ( node_id <= last_node_id_ ) );
  }
  index
  get_model_id() const
  {
    return model_;
  }
  index
  get_first_node_id() const
  {
    return first_node_id_;
  }
  index
  get_last_node_id() const
  {
    return last_node_id_;
  }
  void extend_range( index new_last_node_id );

private:
  index model_;
  index first_node_id_;
  index last_node_id_;
};
}

#endif
