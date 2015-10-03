/*
 *  mpi_manager_impl.h
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

#ifndef MPI_MANAGER_IMPL_H
#define MPI_MANAGER_IMPL_H

inline thread
MPIManager::get_process_id( thread vp ) const
{
  if ( vp >= static_cast< thread >( n_sim_procs_
               * kernel().vp_manager.get_num_threads() ) ) // vp belongs to recording VPs
  {
    return ( vp - n_sim_procs_ * kernel().vp_manager.get_num_threads() ) % n_rec_procs_
      + n_sim_procs_;
  }
  else // vp belongs to simulating VPs
  {
    return vp % n_sim_procs_;
  }
}

#endif /* MPI_MANAGER_IMPL_H */
