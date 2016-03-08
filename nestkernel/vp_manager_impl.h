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

inline nest::thread
nest::VPManager::get_vp() const
{
  return kernel().mpi_manager.get_rank() + get_thread_id() * kernel().mpi_manager.get_num_sim_processes();
}

inline nest::thread
nest::VPManager::get_num_virtual_processes() const
{
  return get_num_threads() * kernel().mpi_manager.get_num_processes();
}

inline bool
nest::VPManager::is_vp_local( const index gid ) const
{
  return ( gid % ( n_threads_ * kernel().mpi_manager.get_num_sim_processes() ) == get_vp() );
}

inline nest::index
nest::VPManager::gid_to_lid( const index gid ) const
{
  assert( is_vp_local( gid ) );
  return gid / ( n_threads_ * kernel().mpi_manager.get_num_sim_processes() );
}

inline nest::index
nest::VPManager::lid_to_gid( const index lid ) const
{
  return lid * ( get_num_virtual_processes() ) + get_vp();
}

inline unsigned int
nest::VPManager::get_num_assigned_ranks_per_thread() const
{
  return ceil( float( kernel().mpi_manager.get_num_processes() ) / n_threads_ );
}

inline unsigned int
nest::VPManager::get_start_rank_per_thread() const
{
  return get_thread_id() * get_num_assigned_ranks_per_thread();
}

inline unsigned int
nest::VPManager::get_end_rank_per_thread() const
{
  unsigned int rank_start = get_start_rank_per_thread();
  unsigned int rank_end =  rank_start + get_num_assigned_ranks_per_thread();

  // if we have more threads than ranks, or if ranks can not be
  // distributed evenly on threads, we need to make sure, that all
  // threads care only about existing ranks
  while ( rank_end > kernel().mpi_manager.get_num_processes() )
  {
    --rank_end;
    // we use rank_end == rank_start, as a sign, that this thread
    // does not do any work
    if ( rank_end == rank_start )
    {
      break;
    }
  }

  return rank_end;
}

#endif /* VP_MANAGER_IMPL_H */
