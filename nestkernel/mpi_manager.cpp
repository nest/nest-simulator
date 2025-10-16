/*
 *  mpi_manager.cpp
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

#include "mpi_manager_impl.h"

// C++ includes:
#include <cstdlib>

// Includes from nestkernel:
#include "kernel_manager.h"
#include "nest_types.h"

#include "logging.h"
#include "logging_manager.h"
#include "music_manager.h"
#include "nest_names.h"
#include "stopwatch_impl.h"

// Includes from sli:
#include "dictutils.h"

#include <numeric>
#include <unistd.h>

namespace nest
{

#ifdef HAVE_MPI

template <>
MPI_Datatype MPI_Type< int >::type = MPI_INT;
template <>
MPI_Datatype MPI_Type< double >::type = MPI_DOUBLE;
template <>
MPI_Datatype MPI_Type< long >::type = MPI_LONG;
template <>
MPI_Datatype MPI_Type< unsigned int >::type = MPI_INT;
template <>
MPI_Datatype MPI_Type< unsigned long >::type = MPI_UNSIGNED_LONG;

#endif /* #ifdef HAVE_MPI */

MPIManager::MPIManager()
  : num_processes_( 1 )
  , rank_( 0 )
  , send_buffer_size_( 0 )
  , recv_buffer_size_( 0 )
  , use_mpi_( false )
  , buffer_size_target_data_( 1 )
  , buffer_size_spike_data_( 1 )
  , max_buffer_size_target_data_( 16777216 )
  , adaptive_target_buffers_( true )
  , growth_factor_buffer_spike_data_( 1.5 )
  , growth_factor_buffer_target_data_( 1.5 )
  , shrink_factor_buffer_spike_data_( 1.1 )
  , send_recv_count_spike_data_per_rank_( 0 )
  , send_recv_count_target_data_per_rank_( 0 )
#ifdef HAVE_MPI
  , comm_step_( std::vector< int >() )
  , COMM_OVERFLOW_ERROR( std::numeric_limits< unsigned int >::max() )
  , comm( 0 )
  , MPI_OFFGRID_SPIKE( 0 )
#endif
{
}

#ifndef HAVE_MPI

void
MPIManager::init_mpi( int*, char*** )
{
  // if ! HAVE_MPI, initialize process entries for 1 rank
  // use 2 processes entries (need at least two
  // entries per process to use flag of first entry as validity and
  // last entry to communicate end of communication)
  kernel::manager< MPIManager >.set_buffer_size_target_data( 2 );
  kernel::manager< MPIManager >.set_buffer_size_spike_data( 2 );

  recv_counts_secondary_events_in_int_per_rank_.resize( 1, 0 );
  recv_displacements_secondary_events_in_int_per_rank_.resize( 1, 0 );
  send_counts_secondary_events_in_int_per_rank_.resize( 1, 0 );
  send_displacements_secondary_events_in_int_per_rank_.resize( 1, 0 );
}

#else /* HAVE_MPI */

void
MPIManager::set_communicator( MPI_Comm global_comm )
{
  comm = global_comm;
  MPI_Comm_size( comm, &num_processes_ );
  MPI_Comm_rank( comm, &rank_ );
  recv_buffer_size_ = send_buffer_size_ * get_num_processes();

  // use at least 2 * number of processes entries (need at least two
  // entries per process to use flag of first entry as validity and
  // last entry to communicate end of communication)
  kernel::manager< MPIManager >.set_buffer_size_target_data( 2 * kernel::manager< MPIManager >.get_num_processes() );
  kernel::manager< MPIManager >.set_buffer_size_spike_data( 2 * kernel::manager< MPIManager >.get_num_processes() );
}

void
MPIManager::init_mpi( int* argc, char** argv[] )
{
  int init;
  MPI_Initialized( &init );

  if ( init == 0 )
  {
#ifdef HAVE_MUSIC
    kernel::manager< MUSICManager >.init_music( argc, argv );
    // get a communicator from MUSIC
    set_communicator( static_cast< MPI_Comm >( kernel::manager< MUSICManager >.communicator() ) );
#else  /* #ifdef HAVE_MUSIC */
    int provided_thread_level;
    MPI_Init_thread( argc, argv, MPI_THREAD_FUNNELED, &provided_thread_level );
    set_communicator( MPI_COMM_WORLD );
#endif /* #ifdef HAVE_MUSIC */
  }
  else
  {
#ifdef HAVE_MUSIC
    LOG( M_ERROR,
      "MPIManager::init_mpi()",
      "When compiled with MUSIC, NEST must be initialized before any other modules that call MPI_Init(). "
      "Calling MPI_Abort()." );
    comm = MPI_COMM_WORLD;
    mpi_abort( 1 );
#else
    set_communicator( MPI_COMM_WORLD );
#endif
  }

  recv_counts_secondary_events_in_int_per_rank_.resize( get_num_processes(), 0 );
  recv_displacements_secondary_events_in_int_per_rank_.resize( get_num_processes(), 0 );
  send_counts_secondary_events_in_int_per_rank_.resize( get_num_processes(), 0 );
  send_displacements_secondary_events_in_int_per_rank_.resize( get_num_processes(), 0 );

  // create off-grid-spike type for MPI communication
  // creating derived datatype
  OffGridSpike::assert_datatype_compatibility_();
  MPI_Datatype source_types[ 2 ];
  int blockcounts[ 2 ];
  MPI_Aint offsets[ 2 ];
  MPI_Aint start_address, address;
  OffGridSpike ogs( 0, 0.0 );

  // OffGridSpike.node_id
  offsets[ 0 ] = 0;
  source_types[ 0 ] = MPI_DOUBLE;
  blockcounts[ 0 ] = 1;

  // OffGridSpike.offset
  MPI_Get_address( &( ogs.node_id_ ), &start_address );
  MPI_Get_address( &( ogs.offset_ ), &address );
  offsets[ 1 ] = address - start_address;
  source_types[ 1 ] = MPI_DOUBLE;
  blockcounts[ 1 ] = 1;

  // generate and commit struct
  MPI_Type_create_struct( 2, blockcounts, offsets, source_types, &MPI_OFFGRID_SPIKE );
  MPI_Type_commit( &MPI_OFFGRID_SPIKE );

  use_mpi_ = true;
}

#endif /* #ifdef HAVE_MPI */

void
MPIManager::initialize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( adjust_number_of_threads_or_rng_only )
  {
    return;
  }

#ifndef HAVE_MPI
  char* pmix_rank_set = std::getenv( "PMIX_RANK" ); // set by OpenMPI's launcher
  char* pmi_rank_set = std::getenv( "PMI_RANK" );   // set by MPICH's launcher
  const bool mpi_launcher_or_mpi4py_used = pmix_rank_set or pmi_rank_set;

  long mpi_num_procs = 0;
  char* mpi_localnranks = std::getenv( "MPI_LOCALNRANKS" );
  if ( mpi_localnranks )
  {
    mpi_num_procs = std::atoi( mpi_localnranks );
  }

  char* ompi_comm_world_size = std::getenv( "OMPI_COMM_WORLD_SIZE" );
  if ( ompi_comm_world_size )
  {
    mpi_num_procs = std::atoi( ompi_comm_world_size );
  }

  if ( mpi_launcher_or_mpi4py_used and mpi_num_procs > 1 )
  {
    LOG( M_FATAL,
      "MPIManager::initialize()",
      "You seem to be using NEST via an MPI launcher like mpirun, mpiexec or srun "
      "although NEST was not compiled with MPI support. Please see the NEST "
      "documentation about parallel and distributed computing. Exiting." );
    std::exit( 127 );
  }
#endif
}

void
MPIManager::finalize( const bool )
{
}

void
MPIManager::set_status( const DictionaryDatum& dict )
{
  updateValue< bool >( dict, names::adaptive_target_buffers, adaptive_target_buffers_ );

  long new_buffer_size_target_data = buffer_size_target_data_;
  updateValue< long >( dict, names::buffer_size_target_data, new_buffer_size_target_data );
  if ( new_buffer_size_target_data != static_cast< long >( buffer_size_target_data_ )
    and new_buffer_size_target_data < static_cast< long >( max_buffer_size_target_data_ ) )
  {
    set_buffer_size_target_data( new_buffer_size_target_data );
  }

  long new_buffer_size_spike_data = buffer_size_spike_data_;
  updateValue< long >( dict, names::buffer_size_spike_data, new_buffer_size_spike_data );
  if ( new_buffer_size_spike_data != static_cast< long >( buffer_size_spike_data_ ) )
  {
    set_buffer_size_spike_data( new_buffer_size_spike_data );
  }

  updateValue< double >( dict, names::growth_factor_buffer_spike_data, growth_factor_buffer_spike_data_ );
  updateValue< double >( dict, names::growth_factor_buffer_target_data, growth_factor_buffer_target_data_ );

  updateValue< long >( dict, names::max_buffer_size_target_data, max_buffer_size_target_data_ );

  updateValue< double >( dict, names::shrink_factor_buffer_spike_data, shrink_factor_buffer_spike_data_ );
}

void
MPIManager::get_status( DictionaryDatum& dict )
{
  def< long >( dict, names::num_processes, num_processes_ );
  def< bool >( dict, names::adaptive_target_buffers, adaptive_target_buffers_ );
  def< size_t >( dict, names::buffer_size_target_data, buffer_size_target_data_ );
  def< size_t >( dict, names::buffer_size_spike_data, buffer_size_spike_data_ );
  def< size_t >( dict, names::send_buffer_size_secondary_events, get_send_buffer_size_secondary_events_in_int() );
  def< size_t >( dict, names::recv_buffer_size_secondary_events, get_recv_buffer_size_secondary_events_in_int() );
  def< size_t >( dict, names::max_buffer_size_target_data, max_buffer_size_target_data_ );
  def< double >( dict, names::growth_factor_buffer_spike_data, growth_factor_buffer_spike_data_ );
  def< double >( dict, names::growth_factor_buffer_target_data, growth_factor_buffer_target_data_ );
}

#ifdef HAVE_MPI

void
MPIManager::mpi_finalize( int exitcode )
{
  MPI_Type_free( &MPI_OFFGRID_SPIKE );

  int finalized;
  MPI_Finalized( &finalized );

  int initialized;
  MPI_Initialized( &initialized );

  if ( finalized == 0 and initialized == 1 )
  {
    if ( exitcode == 0 )
    {
      kernel::manager< MUSICManager >.music_finalize(); // calls MPI_Finalize()
    }
    else
    {
      LOG( M_INFO, "MPIManager::finalize()", "Calling MPI_Abort() due to errors in the script." );
      mpi_abort( exitcode );
    }
  }
}

#else /* #ifdef HAVE_MPI */

void
MPIManager::mpi_finalize( int )
{
}

#endif /* #ifdef HAVE_MPI */

#ifdef HAVE_MPI

void
MPIManager::mpi_abort( int exitcode ) const
{
  MPI_Abort( comm, exitcode );
}


std::string
MPIManager::get_processor_name()
{
  char name[ 1024 ];
  int len;
  MPI_Get_processor_name( name, &len );
  name[ len ] = '\0';
  return name;
}

void
MPIManager::communicate( std::vector< long >& local_nodes, std::vector< long >& global_nodes )
{
  const size_t num_procs = get_num_processes();

  // We need to work with int in much what follows, because several MPI_Allgatherv() arguments must be int.
  assert( local_nodes.size() <= std::numeric_limits< int >::max() );
  const int num_local_nodes = static_cast< int >( local_nodes.size() );

  // Get number of nodes per rank and total.
  std::vector< int > num_nodes_per_rank( num_procs );
  num_nodes_per_rank[ get_rank() ] = num_local_nodes;
  communicate( num_nodes_per_rank );
  const int num_globals = std::accumulate( num_nodes_per_rank.begin(), num_nodes_per_rank.end(), 0 );
  if ( num_globals == 0 )
  {
    return; // Must return here to avoid passing address to empty global_nodes below
  }

  global_nodes.resize( num_globals, 0L );

  // Set up displacements vector. Entry i specifies the displacement (relative
  // to recv_buffer ) at which to place the incoming data from process i
  std::vector< int > displacements( num_procs, 0 );
  for ( size_t i = 1; i < num_procs; ++i )
  {
    displacements.at( i ) = displacements.at( i - 1 ) + num_nodes_per_rank.at( i - 1 );
  }

  // Avoid dereferencing empty vector. As long as sendcount is 0, we can pass any pointer for sendbuf.
  long dummy = 0;
  MPI_Allgatherv( num_local_nodes > 0 ? &local_nodes[ 0 ] : &dummy,
    num_local_nodes,
    MPI_Type< long >::type,
    &global_nodes[ 0 ],
    &num_nodes_per_rank[ 0 ],
    &displacements[ 0 ],
    MPI_Type< long >::type,
    comm );
}

void
MPIManager::communicate( std::vector< unsigned int >& send_buffer,
  std::vector< unsigned int >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements.resize( num_processes_, 0 );
  if ( get_num_processes() == 1 ) // purely thread-based
  {
    displacements[ 0 ] = 0;
    if ( static_cast< unsigned int >( recv_buffer_size_ ) < send_buffer.size() )
    {
      recv_buffer_size_ = send_buffer_size_ = send_buffer.size();
      recv_buffer.resize( recv_buffer_size_ );
    }
    recv_buffer.swap( send_buffer );
  }
  else
  {
    communicate_Allgather( send_buffer, recv_buffer, displacements );
  }
}

void
MPIManager::communicate_Allgather( std::vector< unsigned int >& send_buffer,
  std::vector< unsigned int >& recv_buffer,
  std::vector< int >& displacements )
{
  std::vector< int > recv_counts( get_num_processes(), send_buffer_size_ );

  // attempt Allgather
  if ( send_buffer.size() == static_cast< unsigned int >( send_buffer_size_ ) )
  {
    MPI_Allgather(
      &send_buffer[ 0 ], send_buffer_size_, MPI_UNSIGNED, &recv_buffer[ 0 ], send_buffer_size_, MPI_UNSIGNED, comm );
  }
  else
  {
    // DEC cxx required 0U literal, HEP 2007-03-26
    std::vector< unsigned int > overflow_buffer( send_buffer_size_, 0U );
    overflow_buffer[ 0 ] = COMM_OVERFLOW_ERROR;
    overflow_buffer[ 1 ] = send_buffer.size();
    MPI_Allgather( &overflow_buffer[ 0 ],
      send_buffer_size_,
      MPI_UNSIGNED,
      &recv_buffer[ 0 ],
      send_buffer_size_,
      MPI_UNSIGNED,
      comm );
  }
  // check for overflow condition
  int disp = 0;
  unsigned int max_recv_count = send_buffer_size_;
  bool overflow = false;
  for ( size_t pid = 0; pid < get_num_processes(); ++pid )
  {
    unsigned int block_disp = pid * send_buffer_size_;
    displacements[ pid ] = disp;
    if ( recv_buffer[ block_disp ] == COMM_OVERFLOW_ERROR )
    {
      overflow = true;
      recv_counts[ pid ] = recv_buffer[ block_disp + 1 ];
      if ( static_cast< unsigned int >( recv_counts[ pid ] ) > max_recv_count )
      {
        max_recv_count = recv_counts[ pid ];
      }
    }
    disp += recv_counts[ pid ];
  }

  // do Allgatherv if necessary
  if ( overflow )
  {
    recv_buffer.resize( disp, 0 );
    MPI_Allgatherv( &send_buffer[ 0 ],
      send_buffer.size(),
      MPI_UNSIGNED,
      &recv_buffer[ 0 ],
      &recv_counts[ 0 ],
      &displacements[ 0 ],
      MPI_UNSIGNED,
      comm );
    send_buffer_size_ = max_recv_count;
    recv_buffer_size_ = send_buffer_size_ * get_num_processes();
  }
}

template < typename T >
void
MPIManager::communicate_Allgather( std::vector< T >& send_buffer,
  std::vector< T >& recv_buffer,
  std::vector< int >& displacements )
{
  std::vector< int > recv_counts( get_num_processes(), send_buffer_size_ );

  // attempt Allgather
  if ( send_buffer.size() == static_cast< unsigned int >( send_buffer_size_ ) )
  {
    MPI_Allgather( &send_buffer[ 0 ],
      send_buffer_size_,
      MPI_Type< T >::type,
      &recv_buffer[ 0 ],
      send_buffer_size_,
      MPI_Type< T >::type,
      comm );
  }
  else
  {
    // DEC cxx required 0U literal, HEP 2007-03-26
    std::vector< unsigned int > overflow_buffer( send_buffer_size_, 0U );
    overflow_buffer[ 0 ] = COMM_OVERFLOW_ERROR;
    overflow_buffer[ 1 ] = send_buffer.size();
    MPI_Allgather( &overflow_buffer[ 0 ],
      send_buffer_size_,
      MPI_Type< T >::type,
      &recv_buffer[ 0 ],
      send_buffer_size_,
      MPI_Type< T >::type,
      comm );
  }
  // check for overflow condition
  int disp = 0;
  unsigned int max_recv_count = send_buffer_size_;
  bool overflow = false;
  for ( size_t pid = 0; pid < get_num_processes(); ++pid )
  {
    unsigned int block_disp = pid * send_buffer_size_;
    displacements[ pid ] = disp;
    if ( recv_buffer[ block_disp ] == COMM_OVERFLOW_ERROR )
    {
      overflow = true;
      recv_counts[ pid ] = recv_buffer[ block_disp + 1 ];
      if ( static_cast< unsigned int >( recv_counts[ pid ] ) > max_recv_count )
      {
        max_recv_count = recv_counts[ pid ];
      }
    }
    disp += recv_counts[ pid ];
  }

  // do Allgatherv if necessary
  if ( overflow )
  {
    recv_buffer.resize( disp, 0 );
    MPI_Allgatherv( &send_buffer[ 0 ],
      send_buffer.size(),
      MPI_Type< T >::type,
      &recv_buffer[ 0 ],
      &recv_counts[ 0 ],
      &displacements[ 0 ],
      MPI_Type< T >::type,
      comm );
    send_buffer_size_ = max_recv_count;
    recv_buffer_size_ = send_buffer_size_ * get_num_processes();
  }
}

void
MPIManager::communicate( std::vector< OffGridSpike >& send_buffer,
  std::vector< OffGridSpike >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements.resize( num_processes_, 0 );
  if ( get_num_processes() == 1 ) // purely thread-based
  {
    displacements[ 0 ] = 0;
    if ( static_cast< unsigned int >( recv_buffer_size_ ) < send_buffer.size() )
    {
      recv_buffer_size_ = send_buffer_size_ = send_buffer.size();
      recv_buffer.resize( recv_buffer_size_ );
    }
    recv_buffer.swap( send_buffer );
  }
  else
  {
    communicate_Allgather( send_buffer, recv_buffer, displacements );
  }
}

void
MPIManager::communicate_Allgather( std::vector< OffGridSpike >& send_buffer,
  std::vector< OffGridSpike >& recv_buffer,
  std::vector< int >& displacements )
{
  std::vector< int > recv_counts( get_num_processes(), send_buffer_size_ );
  // attempt Allgather
  if ( send_buffer.size() == static_cast< unsigned int >( send_buffer_size_ ) )
  {
    MPI_Allgather( &send_buffer[ 0 ],
      send_buffer_size_,
      MPI_OFFGRID_SPIKE,
      &recv_buffer[ 0 ],
      send_buffer_size_,
      MPI_OFFGRID_SPIKE,
      comm );
  }
  else
  {
    std::vector< OffGridSpike > overflow_buffer( send_buffer_size_ );
    overflow_buffer[ 0 ] = OffGridSpike( COMM_OVERFLOW_ERROR, 0.0 );
    overflow_buffer[ 1 ] = OffGridSpike( send_buffer.size(), 0.0 );
    MPI_Allgather( &overflow_buffer[ 0 ],
      send_buffer_size_,
      MPI_OFFGRID_SPIKE,
      &recv_buffer[ 0 ],
      send_buffer_size_,
      MPI_OFFGRID_SPIKE,
      comm );
  }

  // check for overflow condition
  int disp = 0;
  unsigned int max_recv_count = send_buffer_size_;
  bool overflow = false;
  for ( size_t pid = 0; pid < get_num_processes(); ++pid )
  {
    unsigned int block_disp = pid * send_buffer_size_;
    displacements[ pid ] = disp;
    if ( ( recv_buffer[ block_disp ] ).get_node_id() == COMM_OVERFLOW_ERROR )
    {
      overflow = true;
      recv_counts[ pid ] = ( recv_buffer[ block_disp + 1 ] ).get_node_id();
      if ( static_cast< unsigned int >( recv_counts[ pid ] ) > max_recv_count )
      {
        max_recv_count = recv_counts[ pid ];
      }
    }
    disp += recv_counts[ pid ];
  }

  // do Allgatherv if necessary
  if ( overflow )
  {
    recv_buffer.resize( disp );
    MPI_Allgatherv( &send_buffer[ 0 ],
      send_buffer.size(),
      MPI_OFFGRID_SPIKE,
      &recv_buffer[ 0 ],
      &recv_counts[ 0 ],
      &displacements[ 0 ],
      MPI_OFFGRID_SPIKE,
      comm );
    send_buffer_size_ = max_recv_count;
    recv_buffer_size_ = send_buffer_size_ * get_num_processes();
  }
}

void
MPIManager::communicate( std::vector< double >& send_buffer,
  std::vector< double >& recv_buffer,
  std::vector< int >& displacements )
{
  // get size of buffers
  std::vector< int > n_nodes( get_num_processes() );
  n_nodes[ get_rank() ] = send_buffer.size();
  communicate( n_nodes );
  // Set up displacements vector.
  displacements.resize( get_num_processes(), 0 );
  for ( size_t i = 1; i < get_num_processes(); ++i )
  {
    displacements.at( i ) = displacements.at( i - 1 ) + n_nodes.at( i - 1 );
  }

  // Calculate total number of node data items to be gathered.
  size_t n_globals = std::accumulate( n_nodes.begin(), n_nodes.end(), 0 );

  if ( n_globals != 0 )
  {
    recv_buffer.resize( n_globals, 0.0 );
    communicate_Allgatherv( send_buffer, recv_buffer, displacements, n_nodes );
  }
  else
  {
    recv_buffer.clear();
  }
}

void
MPIManager::communicate( std::vector< unsigned long >& send_buffer,
  std::vector< unsigned long >& recv_buffer,
  std::vector< int >& displacements )
{
  // get size of buffers
  std::vector< int > n_nodes( num_processes_ );
  n_nodes[ rank_ ] = send_buffer.size();
  communicate( n_nodes );
  // Set up displacements vector.
  displacements.resize( num_processes_, 0 );
  for ( int i = 1; i < num_processes_; ++i )
  {
    displacements.at( i ) = displacements.at( i - 1 ) + n_nodes.at( i - 1 );
  }

  // Calculate total number of node data items to be gathered.
  size_t n_globals = std::accumulate( n_nodes.begin(), n_nodes.end(), 0 );

  if ( n_globals != 0 )
  {
    recv_buffer.resize( n_globals, 0.0 );
    communicate_Allgatherv( send_buffer, recv_buffer, displacements, n_nodes );
  }
  else
  {
    recv_buffer.clear();
  }
}

void
MPIManager::communicate( std::vector< int >& send_buffer,
  std::vector< int >& recv_buffer,
  std::vector< int >& displacements )
{
  // get size of buffers
  std::vector< int > n_nodes( num_processes_ );
  n_nodes[ rank_ ] = send_buffer.size();
  communicate( n_nodes );
  // Set up displacements vector.
  displacements.resize( num_processes_, 0 );
  for ( int i = 1; i < num_processes_; ++i )
  {
    displacements.at( i ) = displacements.at( i - 1 ) + n_nodes.at( i - 1 );
  }

  // Calculate total number of node data items to be gathered.
  size_t n_globals = std::accumulate( n_nodes.begin(), n_nodes.end(), 0 );

  if ( n_globals != 0 )
  {
    recv_buffer.resize( n_globals, 0.0 );
    communicate_Allgatherv( send_buffer, recv_buffer, displacements, n_nodes );
  }
  else
  {
    recv_buffer.clear();
  }
}

void
MPIManager::communicate( double send_val, std::vector< double >& recv_buffer ) const
{
  recv_buffer.resize( get_num_processes() );
  MPI_Allgather( &send_val, 1, MPI_DOUBLE, &recv_buffer[ 0 ], 1, MPI_DOUBLE, comm );
}


/**
 * communicate function for sending set-up information
 */
void
MPIManager::communicate( std::vector< int >& buffer )
{
  communicate_Allgather( buffer );
}

void
MPIManager::communicate( std::vector< long >& buffer )
{
  communicate_Allgather( buffer );
}

void
MPIManager::communicate_Allgather( std::vector< int >& buffer ) const
{
  // avoid aliasing, see http://www.mpi-forum.org/docs/mpi-11-html/node10.html
  int my_val = buffer[ get_rank() ];
  MPI_Allgather( &my_val, 1, MPI_INT, &buffer[ 0 ], 1, MPI_INT, comm );
}

void
MPIManager::communicate_Allreduce_sum_in_place( double buffer ) const
{
  MPI_Allreduce( MPI_IN_PLACE, &buffer, 1, MPI_Type< double >::type, MPI_SUM, comm );
}

void
MPIManager::communicate_Allreduce_sum_in_place( std::vector< double >& buffer ) const
{
  MPI_Allreduce( MPI_IN_PLACE, &buffer[ 0 ], buffer.size(), MPI_Type< double >::type, MPI_SUM, comm );
}

void
MPIManager::communicate_Allreduce_sum_in_place( std::vector< int >& buffer ) const
{
  MPI_Allreduce( MPI_IN_PLACE, &buffer[ 0 ], buffer.size(), MPI_Type< int >::type, MPI_SUM, comm );
}

void
MPIManager::communicate_Allreduce_sum( std::vector< double >& send_buffer, std::vector< double >& recv_buffer ) const
{
  assert( recv_buffer.size() == send_buffer.size() );
  MPI_Allreduce( &send_buffer[ 0 ], &recv_buffer[ 0 ], send_buffer.size(), MPI_Type< double >::type, MPI_SUM, comm );
}

bool
MPIManager::equal_cross_ranks( const double value ) const
{
  // Flipping the sign of one argument to check both min and max values.
  double values[ 2 ];
  values[ 0 ] = -value;
  values[ 1 ] = value;
  MPI_Allreduce( MPI_IN_PLACE, &values, 2, MPI_DOUBLE, MPI_MIN, comm );
  return values[ 0 ] == -values[ 1 ] and values[ 1 ] != -std::numeric_limits< double >::infinity();
}

void
MPIManager::communicate_Allgather( std::vector< long >& buffer ) const
{
  // avoid aliasing, see http://www.mpi-forum.org/docs/mpi-11-html/node10.html
  long my_val = buffer[ get_rank() ];
  MPI_Allgather( &my_val, 1, MPI_LONG, &buffer[ 0 ], 1, MPI_LONG, comm );
}

void
MPIManager::communicate_Alltoall_( void* send_buffer, void* recv_buffer, const unsigned int send_recv_count ) const
{
  MPI_Alltoall( send_buffer, send_recv_count, MPI_UNSIGNED, recv_buffer, send_recv_count, MPI_UNSIGNED, comm );
}

void
MPIManager::communicate_Alltoallv_( void* send_buffer,
  const int* send_counts,
  const int* send_displacements,
  void* recv_buffer,
  const int* recv_counts,
  const int* recv_displacements ) const
{
  MPI_Alltoallv( send_buffer,
    send_counts,
    send_displacements,
    MPI_UNSIGNED,
    recv_buffer,
    recv_counts,
    recv_displacements,
    MPI_UNSIGNED,
    comm );
}

void
MPIManager::communicate_recv_counts_secondary_events()
{

  communicate_Alltoall(
    recv_counts_secondary_events_in_int_per_rank_, send_counts_secondary_events_in_int_per_rank_, 1 );

  std::partial_sum( send_counts_secondary_events_in_int_per_rank_.begin(),
    send_counts_secondary_events_in_int_per_rank_.end() - 1,
    send_displacements_secondary_events_in_int_per_rank_.begin() + 1 );
}

void
MPIManager::synchronize() const
{
  MPI_Barrier( comm );
}


// any_true: takes a single bool, exchanges with all other processes,
// and returns "true" if one or more processes provide "true"
bool
MPIManager::any_true( const bool my_bool ) const
{
  if ( get_num_processes() == 1 )
  {
    return my_bool;
  }

  const int my_int = my_bool;
  int global_int;
  MPI_Allreduce( &my_int, &global_int, 1, MPI_INT, MPI_LOR, comm );
  return global_int == 1;
}

// average communication time for a packet size of num_bytes using Allgather
double
MPIManager::time_communicate( int num_bytes, int samples ) const
{
  if ( get_num_processes() == 1 )
  {
    return 0.0;
  }
  unsigned int packet_length = num_bytes / sizeof( unsigned int );
  if ( packet_length < 1 )
  {
    packet_length = 1;
  }
  std::vector< unsigned int > test_send_buffer( packet_length );
  std::vector< unsigned int > test_recv_buffer( packet_length * get_num_processes() );
  // start time measurement here
  Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::MasterOnly > stopwatch;
  stopwatch.start();
  for ( int i = 0; i < samples; ++i )
  {
    MPI_Allgather(
      &test_send_buffer[ 0 ], packet_length, MPI_UNSIGNED, &test_recv_buffer[ 0 ], packet_length, MPI_UNSIGNED, comm );
  }
  // finish time measurement here
  stopwatch.stop();
  return stopwatch.elapsed() / samples;
}

// average communication time for a packet size of num_bytes using Allgatherv
double
MPIManager::time_communicatev( int num_bytes, int samples )
{
  if ( get_num_processes() == 1 )
  {
    return 0.0;
  }
  unsigned int packet_length = num_bytes / sizeof( unsigned int );
  if ( packet_length < 1 )
  {
    packet_length = 1;
  }
  std::vector< unsigned int > test_send_buffer( packet_length );
  std::vector< unsigned int > test_recv_buffer( packet_length * get_num_processes() );
  std::vector< int > n_nodes( get_num_processes(), packet_length );
  std::vector< int > displacements( get_num_processes(), 0 );

  for ( size_t i = 1; i < get_num_processes(); ++i )
  {
    displacements.at( i ) = displacements.at( i - 1 ) + n_nodes.at( i - 1 );
  }

  // start time measurement here
  Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::MasterOnly > stopwatch;
  stopwatch.start();
  for ( int i = 0; i < samples; ++i )
  {
    communicate_Allgatherv( test_send_buffer, test_recv_buffer, displacements, n_nodes );
  }

  // finish time measurement here
  stopwatch.stop();
  return stopwatch.elapsed() / samples;
}

// average communication time for a packet size of num_bytes
double
MPIManager::time_communicate_offgrid( int num_bytes, int samples ) const
{
  if ( get_num_processes() == 1 )
  {
    return 0.0;
  }
  unsigned int packet_length = num_bytes / sizeof( OffGridSpike );
  if ( packet_length < 1 )
  {
    packet_length = 1;
  }
  std::vector< OffGridSpike > test_send_buffer( packet_length );
  std::vector< OffGridSpike > test_recv_buffer( packet_length * get_num_processes() );
  // start time measurement here
  Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::MasterOnly > stopwatch;
  stopwatch.start();
  for ( int i = 0; i < samples; ++i )
  {
    MPI_Allgather( &test_send_buffer[ 0 ],
      packet_length,
      MPI_OFFGRID_SPIKE,
      &test_recv_buffer[ 0 ],
      packet_length,
      MPI_OFFGRID_SPIKE,
      comm );
  }
  // finish time measurement here
  stopwatch.stop();
  return stopwatch.elapsed() / samples;
}

// average communication time for a packet size of num_bytes using Alltoall
double
MPIManager::time_communicate_alltoall( int num_bytes, int samples ) const
{
  if ( get_num_processes() == 1 )
  {
    return 0.0;
  }
  unsigned int packet_length = num_bytes / sizeof( unsigned int );        // this size should be sent to each process
  unsigned int total_packet_length = packet_length * get_num_processes(); // total size of send and receive buffers
  if ( total_packet_length < 1 )
  {
    total_packet_length = 1;
  }
  std::vector< unsigned int > test_send_buffer( total_packet_length );
  std::vector< unsigned int > test_recv_buffer( total_packet_length );
  // start time measurement here
  Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::MasterOnly > stopwatch;
  stopwatch.start();
  for ( int i = 0; i < samples; ++i )
  {
    MPI_Alltoall(
      &test_send_buffer[ 0 ], packet_length, MPI_UNSIGNED, &test_recv_buffer[ 0 ], packet_length, MPI_UNSIGNED, comm );
  }
  // finish time measurement here
  stopwatch.stop();
  return stopwatch.elapsed() / samples;
}

// average communication time for a packet size of num_bytes using Alltoallv
double
MPIManager::time_communicate_alltoallv( int num_bytes, int samples ) const
{
  if ( get_num_processes() == 1 )
  {
    return 0.0;
  }
  unsigned int packet_length = num_bytes / sizeof( unsigned int );        // this size should be sent to each process
  unsigned int total_packet_length = packet_length * get_num_processes(); // total size of send and receive buffers
  if ( total_packet_length < 1 )
  {
    total_packet_length = 1;
  }
  std::vector< unsigned int > test_send_buffer( total_packet_length );
  std::vector< unsigned int > test_recv_buffer( total_packet_length );
  std::vector< int > n_nodes( get_num_processes(), packet_length );
  std::vector< int > displacements( get_num_processes(), 0 );

  for ( size_t i = 1; i < get_num_processes(); ++i )
  {
    displacements.at( i ) = displacements.at( i - 1 ) + n_nodes.at( i - 1 );
  }

  // start time measurement here
  Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::MasterOnly > stopwatch;
  stopwatch.start();
  for ( int i = 0; i < samples; ++i )
  {
    MPI_Alltoallv( &test_send_buffer[ 0 ],
      &n_nodes[ 0 ],
      &displacements[ 0 ],
      MPI_UNSIGNED,
      &test_recv_buffer[ 0 ],
      &n_nodes[ 0 ],
      &displacements[ 0 ],
      MPI_UNSIGNED,
      comm );
  }
  // finish time measurement here
  stopwatch.stop();
  return stopwatch.elapsed() / samples;
}

#else /* #ifdef HAVE_MPI */

// communicate (on-grid) if compiled without MPI
void
MPIManager::communicate( std::vector< unsigned int >& send_buffer,
  std::vector< unsigned int >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements.resize( num_processes_, 0 );
  displacements[ 0 ] = 0;
  if ( static_cast< size_t >( recv_buffer_size_ ) < send_buffer.size() )
  {
    recv_buffer_size_ = send_buffer_size_ = send_buffer.size();
    recv_buffer.resize( recv_buffer_size_ );
  }
  recv_buffer.swap( send_buffer );
}

// communicate (off-grid) if compiled without MPI
void
MPIManager::communicate( std::vector< OffGridSpike >& send_buffer,
  std::vector< OffGridSpike >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements.resize( num_processes_, 0 );
  displacements[ 0 ] = 0;
  if ( static_cast< size_t >( recv_buffer_size_ ) < send_buffer.size() )
  {
    recv_buffer_size_ = send_buffer_size_ = send_buffer.size();
    recv_buffer.resize( recv_buffer_size_ );
  }
  recv_buffer.swap( send_buffer );
}

void
MPIManager::communicate( std::vector< double >& send_buffer,
  std::vector< double >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements.resize( num_processes_, 0 );
  displacements[ 0 ] = 0;
  recv_buffer.swap( send_buffer );
}

void
MPIManager::communicate( std::vector< unsigned long >& send_buffer,
  std::vector< unsigned long >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements.resize( num_processes_, 0 );
  displacements[ 0 ] = 0;
  recv_buffer.swap( send_buffer );
}

void
MPIManager::communicate( std::vector< int >& send_buffer,
  std::vector< int >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements.resize( num_processes_, 0 );
  displacements[ 0 ] = 0;
  recv_buffer.swap( send_buffer );
}

void
MPIManager::communicate( double send_val, std::vector< double >& recv_buffer ) const
{
  recv_buffer.resize( 1 );
  recv_buffer[ 0 ] = send_val;
}

void
MPIManager::communicate( std::vector< long >&, std::vector< long >& )
{
}

void
MPIManager::communicate_Allreduce_sum_in_place( double ) const
{
}

void
MPIManager::communicate_Allreduce_sum_in_place( std::vector< double >& ) const
{
}

void
MPIManager::communicate_Allreduce_sum_in_place( std::vector< int >& ) const
{
}

void
MPIManager::communicate_Allreduce_sum( std::vector< double >& send_buffer, std::vector< double >& recv_buffer ) const
{
  recv_buffer.swap( send_buffer );
}

bool
MPIManager::equal_cross_ranks( const double ) const
{
  return true;
}

void
MPIManager::communicate_recv_counts_secondary_events()
{
  // since we only have one process, the send count is equal to the recv count
  send_counts_secondary_events_in_int_per_rank_ = recv_counts_secondary_events_in_int_per_rank_;

  // since we only have one process, the send displacement is zero
  assert( send_displacements_secondary_events_in_int_per_rank_.size() == 1 );
  send_displacements_secondary_events_in_int_per_rank_[ 0 ] = 0;
}

#endif /* #ifdef HAVE_MPI  */

size_t
MPIManager::get_process_id_of_vp( const size_t vp ) const
{
  return vp % num_processes_;
}

#ifdef HAVE_MPI

size_t
MPIManager::get_process_id_of_node_id( const size_t node_id ) const
{
  return node_id % num_processes_;
}

#else

size_t
MPIManager::get_process_id_of_node_id( const size_t ) const
{
  return 0;
}

#endif /* HAVE_MPI */

#ifndef HAVE_MPI

double
MPIManager::time_communicate_alltoallv( int, int ) const
{

  return 0.0;
}

double
MPIManager::time_communicate_alltoall( int, int ) const
{

  return 0.0;
}

double
MPIManager::time_communicate_offgrid( int, int ) const
{

  return 0.0;
}

double
MPIManager::time_communicatev( int, int )
{

  return 0.0;
}

double
MPIManager::time_communicate( int, int ) const
{

  return 0.0;
}

bool
MPIManager::any_true( const bool my_bool ) const
{

  return my_bool;
}

void
MPIManager::synchronize() const
{
}

void
MPIManager::communicate( std::vector< long >& )
{
}

void
MPIManager::communicate( std::vector< int >& )
{
}

void
MPIManager::mpi_abort( int ) const
{
}

std::string
MPIManager::get_processor_name()
{
  char name[ 1024 ];
  name[ 1023 ] = '\0';
  gethostname( name, 1023 );
  return name;
}

#endif /* HAVE_MPI */


bool
MPIManager::adaptive_target_buffers() const
{

  return adaptive_target_buffers_;
}

bool
MPIManager::increase_buffer_size_target_data()
{

  assert( adaptive_target_buffers_ );
  if ( buffer_size_target_data_ >= max_buffer_size_target_data_ )
  {
    return false;
  }
  else
  {
    if ( buffer_size_target_data_ * growth_factor_buffer_target_data_ < max_buffer_size_target_data_ )
    {
      // this also adjusts send_recv_count_target_data_per_rank_
      set_buffer_size_target_data(
        static_cast< size_t >( floor( buffer_size_target_data_ * growth_factor_buffer_target_data_ ) ) );
    }
    else
    {
      // this also adjusts send_recv_count_target_data_per_rank_
      set_buffer_size_target_data( max_buffer_size_target_data_ );
    }
    return true;
  }
}

void
MPIManager::set_buffer_size_spike_data( const size_t buffer_size )
{

  assert( buffer_size >= static_cast< size_t >( 2 * get_num_processes() ) );
  buffer_size_spike_data_ = buffer_size;

  send_recv_count_spike_data_per_rank_ = floor( get_buffer_size_spike_data() / get_num_processes() );

  assert( send_recv_count_spike_data_per_rank_ * get_num_processes() <= get_buffer_size_spike_data() );
}

void
MPIManager::set_buffer_size_target_data( const size_t buffer_size )
{

  assert( buffer_size >= static_cast< size_t >( 2 * get_num_processes() ) );
  if ( buffer_size <= max_buffer_size_target_data_ )
  {
    buffer_size_target_data_ = buffer_size;
  }
  else
  {
    buffer_size_target_data_ = max_buffer_size_target_data_;
  }
  send_recv_count_target_data_per_rank_ = static_cast< size_t >(
    floor( static_cast< double >( get_buffer_size_target_data() ) / static_cast< double >( get_num_processes() ) ) );

  assert( send_recv_count_target_data_per_rank_ * get_num_processes() <= get_buffer_size_target_data() );
}

size_t
MPIManager::get_recv_buffer_size_secondary_events_in_int() const
{

  return recv_displacements_secondary_events_in_int_per_rank_
           [ recv_displacements_secondary_events_in_int_per_rank_.size() - 1 ]
    + recv_counts_secondary_events_in_int_per_rank_[ recv_counts_secondary_events_in_int_per_rank_.size() - 1 ];
}

size_t
MPIManager::get_send_buffer_size_secondary_events_in_int() const
{

  return send_displacements_secondary_events_in_int_per_rank_
           [ send_displacements_secondary_events_in_int_per_rank_.size() - 1 ]
    + send_counts_secondary_events_in_int_per_rank_[ send_counts_secondary_events_in_int_per_rank_.size() - 1 ];
}

unsigned int
MPIManager::get_send_recv_count_spike_data_per_rank() const
{

  return send_recv_count_spike_data_per_rank_;
}

size_t
MPIManager::get_buffer_size_spike_data() const
{

  return buffer_size_spike_data_;
}

unsigned int
MPIManager::get_send_recv_count_target_data_per_rank() const
{

  return send_recv_count_target_data_per_rank_;
}

size_t
MPIManager::get_buffer_size_target_data() const
{

  return buffer_size_target_data_;
}

bool
MPIManager::is_mpi_used() const
{

  return use_mpi_;
}

size_t
MPIManager::get_rank() const
{

  return rank_;
}

size_t
MPIManager::get_num_processes() const
{

  return num_processes_;
}

size_t
MPIManager::get_done_marker_position_in_secondary_events_recv_buffer( const size_t source_rank ) const
{

  return get_recv_displacement_secondary_events_in_int( source_rank )
    + get_recv_count_secondary_events_in_int( source_rank ) - 1;
}

size_t
MPIManager::get_done_marker_position_in_secondary_events_send_buffer( const size_t target_rank ) const
{

  return get_send_displacement_secondary_events_in_int( target_rank )
    + get_send_count_secondary_events_in_int( target_rank ) - 1;
}

size_t
MPIManager::get_send_displacement_secondary_events_in_int( const size_t target_rank ) const
{

  return send_displacements_secondary_events_in_int_per_rank_[ target_rank ];
}

size_t
MPIManager::get_send_count_secondary_events_in_int( const size_t target_rank ) const
{

  return send_counts_secondary_events_in_int_per_rank_[ target_rank ];
}

size_t
MPIManager::get_recv_displacement_secondary_events_in_int( const size_t source_rank ) const
{

  return recv_displacements_secondary_events_in_int_per_rank_[ source_rank ];
}

size_t
MPIManager::get_recv_count_secondary_events_in_int( const size_t source_rank ) const
{

  return recv_counts_secondary_events_in_int_per_rank_[ source_rank ];
}

void
MPIManager::set_recv_counts_secondary_events_in_int_per_rank( const std::vector< int >& recv_counts_in_int_per_rank )
{

  recv_counts_secondary_events_in_int_per_rank_ = recv_counts_in_int_per_rank;

  std::partial_sum( recv_counts_secondary_events_in_int_per_rank_.begin(),
    recv_counts_secondary_events_in_int_per_rank_.end() - 1,
    recv_displacements_secondary_events_in_int_per_rank_.begin() + 1 );
}

#ifndef HAVE_MPI

void
test_link( int, int )
{
}

void
test_links()
{
}

#endif /* HAVE_MPI */


} // namespace nest
