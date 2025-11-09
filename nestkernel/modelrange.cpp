/*
 *  modelrange.cpp
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

#include "modelrange.h"

nest::modelrange::modelrange( size_t model, size_t first_node_id, size_t last_node_id )
  : model_( model )
  , first_node_id_( first_node_id )
  , last_node_id_( last_node_id )
{
}

void
nest::modelrange::extend_range( size_t new_last_node_id )
{
  last_node_id_ = new_last_node_id;
}
size_t
nest::modelrange::get_last_node_id() const
{

  return last_node_id_;
}

size_t
nest::modelrange::get_first_node_id() const
{

  return first_node_id_;
}

size_t
nest::modelrange::get_model_id() const
{

  return model_;
}

bool
nest::modelrange::is_in_range( size_t node_id ) const
{

  return ( node_id >= first_node_id_ and node_id <= last_node_id_ );
}
