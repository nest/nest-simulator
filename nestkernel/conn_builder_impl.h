/*
 *  conn_builder_impl.h
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

#ifndef CONN_BUILDER_IMPL_H
#define CONN_BUILDER_IMPL_H

#include "conn_builder.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_names.h"

namespace nest
{

inline void
ConnBuilder::single_disconnect_( index snode_id, Node& target, thread target_thread )
{
  // index tnode_id = target.get_node_id();
  // This is the most simple case in which only the synapse_model_ has been
  // defined. TODO: Add functionality to delete synapses with a given weight
  // or a given delay
  kernel().sp_manager.disconnect( snode_id, &target, target_thread, synapse_model_id_ );
}

} // namespace nest

#endif /* CONN_BUILDER_IMPL_H */
