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

#include "config.h"

/* To avoid problems on BlueGene/L, mpi.h MUST be the
 first included file after config.h.
 */
#ifdef HAVE_MPI
// C includes:
#include <mpi.h>
#endif /* #ifdef HAVE_MPI */

#include "mpi_manager.h"

// Includes from nestkernel:
#include "kernel_manager.h"

inline nest::thread
nest::MPIManager::get_process_id_of_vp( const thread vp ) const
{
  return vp % num_processes_;
}

#ifdef HAVE_MPI

// Variable to hold the MPI communicator to use.
#ifdef HAVE_MUSIC
extern MPI::Intracomm comm;
#else  /* #ifdef HAVE_MUSIC */
extern MPI_Comm comm;
#endif /* #ifdef HAVE_MUSIC */


/* ------------------------------------------------------
   The following datatypes are defined here in communicator_impl.h
   file instead of as static class members, to avoid inclusion
   of mpi.h in the .h file. This is necessary, because on
   BlueGene/L mpi.h MUST be included FIRST. Having mpi.h in
   the .h file would lead to requirements on include-order
   throughout the NEST code base and is not acceptable.
   Reported by Mikael Djurfeldt.
   Hans Ekkehard Plesser, 2010-01-28
 */
template < typename T >
struct MPI_Type
{
  static MPI_Datatype type;
};

template < typename T >
void
nest::MPIManager::communicate_Allgatherv( std::vector< T >& send_buffer,
  std::vector< T >& recv_buffer,
  std::vector< int >& displacements,
  std::vector< int >& recv_counts )
{
  // attempt Allgather
  MPI_Allgatherv( &send_buffer[ 0 ],
    send_buffer.size(),
    MPI_Type< T >::type,
    &recv_buffer[ 0 ],
    &recv_counts[ 0 ],
    &displacements[ 0 ],
    MPI_Type< T >::type,
    comm );
}

inline nest::thread
nest::MPIManager::get_process_id_of_node_id( const index node_id ) const
{
  return node_id % kernel().vp_manager.get_num_virtual_processes() % num_processes_;
}

#else // HAVE_MPI


inline nest::thread
nest::MPIManager::get_process_id_of_node_id( const index ) const
{
  return 0;
}

#endif /* HAVE_MPI */

#endif /* MPI_MANAGER_IMPL_H */
