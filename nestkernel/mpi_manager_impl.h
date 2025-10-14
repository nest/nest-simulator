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

#include "mpi_manager.h"

namespace nest
{

#ifdef HAVE_MPI
template < class D >
void
MPIManager::communicate_Alltoall( std::vector< D >& send_buffer,
  std::vector< D >& recv_buffer,
  const unsigned int send_recv_count )
{
  void* send_buffer_int = static_cast< void* >( &send_buffer[ 0 ] );
  void* recv_buffer_int = static_cast< void* >( &recv_buffer[ 0 ] );

  communicate_Alltoall_( send_buffer_int, recv_buffer_int, send_recv_count );
}

template < class D >
void
MPIManager::communicate_secondary_events_Alltoallv( std::vector< D >& send_buffer, std::vector< D >& recv_buffer )
{
  void* send_buffer_int = static_cast< void* >( &send_buffer[ 0 ] );
  void* recv_buffer_int = static_cast< void* >( &recv_buffer[ 0 ] );

  communicate_Alltoallv_( send_buffer_int,
    &send_counts_secondary_events_in_int_per_rank_[ 0 ],
    &send_displacements_secondary_events_in_int_per_rank_[ 0 ],
    recv_buffer_int,
    &recv_counts_secondary_events_in_int_per_rank_[ 0 ],
    &recv_displacements_secondary_events_in_int_per_rank_[ 0 ] );
}

#else // HAVE_MPI
template < class D >
void
MPIManager::MPIManager::communicate_Alltoall( std::vector< D >& send_buffer,
  std::vector< D >& recv_buffer,
  const unsigned int )
{
  recv_buffer.swap( send_buffer );
}

template < class D >
void
MPIManager::communicate_secondary_events_Alltoallv( std::vector< D >& send_buffer, std::vector< D >& recv_buffer )
{
  recv_buffer.swap( send_buffer );
}

#endif /* HAVE_MPI */

template < class D >
void
MPIManager::communicate_target_data_Alltoall( std::vector< D >& send_buffer, std::vector< D >& recv_buffer )
{
  const size_t send_recv_count_target_data_in_int_per_rank =
    sizeof( TargetData ) / sizeof( unsigned int ) * send_recv_count_target_data_per_rank_;

  communicate_Alltoall( send_buffer, recv_buffer, send_recv_count_target_data_in_int_per_rank );
}

template < class D >
void
MPIManager::communicate_spike_data_Alltoall( std::vector< D >& send_buffer, std::vector< D >& recv_buffer )
{
  const size_t send_recv_count_spike_data_in_int_per_rank =
    sizeof( SpikeData ) / sizeof( unsigned int ) * send_recv_count_spike_data_per_rank_;

  communicate_Alltoall( send_buffer, recv_buffer, send_recv_count_spike_data_in_int_per_rank );
}

template < class D >
void
MPIManager::communicate_off_grid_spike_data_Alltoall( std::vector< D >& send_buffer, std::vector< D >& recv_buffer )
{
  const size_t send_recv_count_off_grid_spike_data_in_int_per_rank =
    sizeof( OffGridSpikeData ) / sizeof( unsigned int ) * send_recv_count_spike_data_per_rank_;

  communicate_Alltoall( send_buffer, recv_buffer, send_recv_count_off_grid_spike_data_in_int_per_rank );
}

#ifdef HAVE_MPI
// Variable to hold the MPI communicator to use.
#ifdef HAVE_MUSIC
extern MPI::Intracomm comm;
#else  /* #ifdef HAVE_MUSIC */
extern MPI_Comm comm;
#endif /* #ifdef HAVE_MUSIC */

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
  MPI_Allgatherv( &( *send_buffer.begin() ),
    send_buffer.size(),
    MPI_Type< T >::type,
    &recv_buffer[ 0 ],
    &recv_counts[ 0 ],
    &displacements[ 0 ],
    MPI_Type< T >::type,
    comm );
}

#endif /* HAVE_MPI */


}

#endif
