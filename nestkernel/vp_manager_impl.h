/*
 *  vp_manager_impl.h
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

#ifndef VP_MANAGER_IMPL_H
#define VP_MANAGER_IMPL_H

#include "vp_manager.h"

// Includes from nestkernel:
#include "kernel_manager.h"
#include "mpi_manager.h"
#include "mpi_manager_impl.h"

namespace nest
{

inline thread
VPManager::get_num_virtual_processes() const
{
  return get_num_threads() * kernel().mpi_manager.get_num_processes();
}

inline bool
VPManager::is_local_vp( thread vp ) const
{
  return kernel().mpi_manager.get_process_id( vp )
    == kernel().mpi_manager.get_rank();
}

inline thread
VPManager::suggest_vp( index gid ) const
{
  return gid % ( get_num_virtual_processes() );
}

inline thread
VPManager::vp_to_thread( thread vp ) const
{
  return vp / kernel().mpi_manager.get_num_processes();
}

inline thread
VPManager::thread_to_vp( thread t ) const
{
  return t * kernel().mpi_manager.get_num_processes()
    + kernel().mpi_manager.get_rank();
}

} // namespace nest

#endif /* VP_MANAGER_IMPL_H */
