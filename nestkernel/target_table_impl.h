/*
 *  target_table_impl.h
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

#ifndef TARGET_TABLE_IMPL_H
#define TARGET_TABLE_IMPL_H

// Includes from nestkernel:
#include "kernel_manager.h"
#include "target_table.h"
#include "vp_manager_impl.h"

inline void
nest::TargetTable::add_target( thread tid, const TargetData& target_data )
{
  index lid = kernel().vp_manager.gid_to_lid( target_data.gid );
  (*targets_[ tid ])[ lid ].push_back( target_data.target );
}

#endif /* TARGET_TABLE_IMPL_H */
