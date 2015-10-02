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

#include "kernel_manager.h"
#include "vp_manager.h"

inline int
nest::VPManager::get_num_virtual_processes() const
{
  return n_threads_ * kernel ().mpi_manager.get_num_processes();
}

#endif /* VP_MANAGER_IMPL_H */
