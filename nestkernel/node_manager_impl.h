/*
 *  node_manager_impl.h
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

#ifndef NODE_MANAGER_IMPL_H
#define NODE_MANAGER_IMPL_H

#include "node_manager.h"

// Include from nestkernel
#include "kernel_manager.h"

inline nest::thread
nest::NodeManager::get_process_id_of_gid( index gid ) const
{
  return kernel().mpi_manager.get_process_id(
    kernel().vp_manager.suggest_vp( gid ) );
}

#endif
