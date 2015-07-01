/*
 *  communicator.cpp
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

#include "config.h"

/* To avoid problems on BlueGene/L, mpi.h MUST be the
   first included file after config.h.
*/
#ifdef HAVE_MPI
#include <mpi.h>
#endif /* #ifdef HAVE_MPI */

#include <limits>
#include <numeric>
#include "stopwatch.h"
#include "communicator.h"
#include "communicator_impl.h"
#include "network.h"

#include "dictutils.h"
#include "nodelist.h"

nest::Network* nest::Communicator::net_ = 0;
int nest::Communicator::rank_ = 0;
int nest::Communicator::num_processes_ = 1;
int nest::Communicator::n_vps_ = 1;
int nest::Communicator::send_buffer_size_ = 1;
int nest::Communicator::recv_buffer_size_ = 1;
bool nest::Communicator::initialized_ = false;

#ifdef HAVE_MPI

#ifdef HAVE_MUSIC
MUSIC::Setup* nest::Communicator::music_setup = 0;
MUSIC::Runtime* nest::Communicator::music_runtime = 0;
#endif /* #ifdef HAVE_MUSIC */

// Variable to hold the MPI communicator to use.
#ifdef HAVE_MUSIC
MPI::Intracomm comm = 0;
#else  /* #ifdef HAVE_MUSIC */
MPI_Comm comm = 0;
#endif /* #ifdef HAVE_MUSIC */

template <>
MPI_Datatype MPI_Type< nest::int_t >::type = MPI_INT;
template <>
MPI_Datatype MPI_Type< nest::double_t >::type = MPI_DOUBLE;
template <>
MPI_Datatype MPI_Type< nest::long_t >::type = MPI_LONG;
template <>
MPI_Datatype MPI_Type< nest::uint_t >::type = MPI_INT;

MPI_Datatype MPI_OFFGRID_SPIKE = 0;

/* ------------------------------------------------------ */

unsigned int nest::Communicator::COMM_OVERFLOW_ERROR = std::numeric_limits< unsigned int >::max();

std::vector< int > nest::Communicator::comm_step_ = std::vector< int >();

/**
 * Set up MPI and establish number of processes and rank
 */
void
nest::Communicator::init( int* argc, char** argv[] )
{
  /* Initialize MPI

     MPI_Init sets the working directory on all machines to the
     directory from which mpirun was called. This is usually what one
     intends.

     On some machines, eg Linux, executables compiled with MPI can be
     called without mpirun.  MPI_Init will then place NEST in the
     directory containing the NEST binary.  This is a user error: if
     compiled with mpi, nest must be run using mpirun or equivalent.
     Unfortunately, there seems to be no straightforward way to check
     if nest was started through mpirun.

     HEP,MD,AM 2006-08-04
  */

  int init;
  MPI_Initialized( &init );

  int provided_thread_level;
  if ( init == 0 )
  {

#ifdef HAVE_MUSIC
    music_setup = new MUSIC::Setup( *argc, *argv, MPI_THREAD_FUNNELED, &provided_thread_level );
    // get a communicator from MUSIC
    comm = music_setup->communicator();
#else  /* #ifdef HAVE_MUSIC */
    MPI_Init_thread( argc, argv, MPI_THREAD_FUNNELED, &provided_thread_level );
    comm = MPI_COMM_WORLD;
#endif /* #ifdef HAVE_MUSIC */
  }

  MPI_Comm_size( comm, &num_processes_ );
  MPI_Comm_rank( comm, &rank_ );

  recv_buffer_size_ = send_buffer_size_ * num_processes_;

  // create off-grid-spike type for MPI communication
  // creating derived datatype
  OffGridSpike::assert_datatype_compatibility();
  MPI_Datatype source_types[ 2 ];
  int blockcounts[ 2 ];
  MPI_Aint offsets[ 2 ];
  MPI_Aint start_address, address;
  OffGridSpike ogs( 0, 0.0 );

  // OffGridSpike.gid
  offsets[ 0 ] = 0;
  source_types[ 0 ] = MPI_DOUBLE;
  blockcounts[ 0 ] = 1;

  // OffGridSpike.offset
  MPI_Address( &( ogs.gid_ ), &start_address );
  MPI_Address( &( ogs.offset_ ), &address );
  offsets[ 1 ] = address - start_address;
  source_types[ 1 ] = MPI_DOUBLE;
  blockcounts[ 1 ] = 1;

  // generate and commit struct
  MPI_Type_struct( 2, blockcounts, offsets, source_types, &MPI_OFFGRID_SPIKE );
  MPI_Type_commit( &MPI_OFFGRID_SPIKE );

  initialized_ = true;
}

/**
 * Finish off MPI routines
 */
void
nest::Communicator::finalize()
{
  MPI_Type_free( &MPI_OFFGRID_SPIKE );

  int finalized;
  MPI_Finalized( &finalized );

  int initialized;
  MPI_Initialized( &initialized );

  if ( finalized == 0 && initialized == 1 )
  {
    if ( !net_->quit_by_error() )
#ifdef HAVE_MUSIC
    {
      if ( music_runtime == 0 )
      {
        // we need a Runtime object to call finalize(), so we create
        // one, if we don't have one already
        music_runtime = new MUSIC::Runtime( music_setup, 1e-3 );
      }

      music_runtime->finalize();
      delete music_runtime;
    }
#else  /* #ifdef HAVE_MUSIC */
      MPI_Finalize();
#endif /* #ifdef HAVE_MUSIC */
    else
    {
      net_->message( SLIInterpreter::M_INFO,
        "Communicator::finalize()",
        "Calling MPI_Abort() due to errors in the script." );
      MPI_Abort( MPI_COMM_WORLD, net_->get_exitcode() );
    }
  }
}

void
nest::Communicator::mpi_abort( int exitcode )
{
  MPI_Abort( MPI_COMM_WORLD, exitcode );
}


std::string
nest::Communicator::get_processor_name()
{
  char name[ 1024 ];
  int len;
  MPI_Get_processor_name( name, &len );
  name[ len ] = '\0';
  return name;
}

void
nest::Communicator::communicate( std::vector< uint_t >& send_buffer,
  std::vector< uint_t >& recv_buffer,
  std::vector< int >& displacements )
{
  if ( num_processes_ == 1 ) // purely thread-based
  {
    displacements[ 0 ] = 0;
    if ( static_cast< uint_t >( recv_buffer_size_ ) < send_buffer.size() )
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
nest::Communicator::communicate_Allgather( std::vector< uint_t >& send_buffer,
  std::vector< uint_t >& recv_buffer,
  std::vector< int >& displacements )
{
  std::vector< int > recv_counts( num_processes_, send_buffer_size_ );

  // attempt Allgather
  if ( send_buffer.size() == static_cast< uint_t >( send_buffer_size_ ) )
    MPI_Allgather( &send_buffer[ 0 ],
      send_buffer_size_,
      MPI_UNSIGNED,
      &recv_buffer[ 0 ],
      send_buffer_size_,
      MPI_UNSIGNED,
      comm );
  else
  {
    // DEC cxx required 0U literal, HEP 2007-03-26
    std::vector< uint_t > overflow_buffer( send_buffer_size_, 0U );
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
  uint_t max_recv_count = send_buffer_size_;
  bool overflow = false;
  for ( int pid = 0; pid < num_processes_; ++pid )
  {
    uint_t block_disp = pid * send_buffer_size_;
    displacements[ pid ] = disp;
    if ( recv_buffer[ block_disp ] == COMM_OVERFLOW_ERROR )
    {
      overflow = true;
      recv_counts[ pid ] = recv_buffer[ block_disp + 1 ];
      if ( static_cast< uint_t >( recv_counts[ pid ] ) > max_recv_count )
        max_recv_count = recv_counts[ pid ];
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
    recv_buffer_size_ = send_buffer_size_ * num_processes_;
  }
}

template < typename T >
void
nest::Communicator::communicate_Allgather( std::vector< T >& send_buffer,
  std::vector< T >& recv_buffer,
  std::vector< int >& displacements )
{
  std::vector< int > recv_counts( num_processes_, send_buffer_size_ );

  // attempt Allgather
  if ( send_buffer.size() == static_cast< uint_t >( send_buffer_size_ ) )
    MPI_Allgather( &send_buffer[ 0 ],
      send_buffer_size_,
      MPI_Type< T >::type,
      &recv_buffer[ 0 ],
      send_buffer_size_,
      MPI_Type< T >::type,
      comm );
  else
  {
    // DEC cxx required 0U literal, HEP 2007-03-26
    std::vector< uint_t > overflow_buffer( send_buffer_size_, 0U );
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
  uint_t max_recv_count = send_buffer_size_;
  bool overflow = false;
  for ( int pid = 0; pid < num_processes_; ++pid )
  {
    uint_t block_disp = pid * send_buffer_size_;
    displacements[ pid ] = disp;
    if ( recv_buffer[ block_disp ] == COMM_OVERFLOW_ERROR )
    {
      overflow = true;
      recv_counts[ pid ] = recv_buffer[ block_disp + 1 ];
      if ( static_cast< uint_t >( recv_counts[ pid ] ) > max_recv_count )
        max_recv_count = recv_counts[ pid ];
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
    recv_buffer_size_ = send_buffer_size_ * num_processes_;
  }
}

void
nest::Communicator::communicate( std::vector< OffGridSpike >& send_buffer,
  std::vector< OffGridSpike >& recv_buffer,
  std::vector< int >& displacements )
{
  if ( num_processes_ == 1 ) // purely thread-based
  {
    displacements[ 0 ] = 0;
    if ( static_cast< uint_t >( recv_buffer_size_ ) < send_buffer.size() )
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
nest::Communicator::communicate_Allgather( std::vector< OffGridSpike >& send_buffer,
  std::vector< OffGridSpike >& recv_buffer,
  std::vector< int >& displacements )
{
  std::vector< int > recv_counts( num_processes_, send_buffer_size_ );
  // attempt Allgather
  if ( send_buffer.size() == static_cast< uint_t >( send_buffer_size_ ) )
    MPI_Allgather( &send_buffer[ 0 ],
      send_buffer_size_,
      MPI_OFFGRID_SPIKE,
      &recv_buffer[ 0 ],
      send_buffer_size_,
      MPI_OFFGRID_SPIKE,
      comm );
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
  uint_t max_recv_count = send_buffer_size_;
  bool overflow = false;
  for ( int pid = 0; pid < num_processes_; ++pid )
  {
    uint_t block_disp = pid * send_buffer_size_;
    displacements[ pid ] = disp;
    if ( ( recv_buffer[ block_disp ] ).get_gid() == COMM_OVERFLOW_ERROR )
    {
      overflow = true;
      recv_counts[ pid ] = ( recv_buffer[ block_disp + 1 ] ).get_gid();
      if ( static_cast< uint_t >( recv_counts[ pid ] ) > max_recv_count )
        max_recv_count = recv_counts[ pid ];
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
    recv_buffer_size_ = send_buffer_size_ * num_processes_;
  }
}


void
nest::Communicator::communicate( std::vector< double_t >& send_buffer,
  std::vector< double_t >& recv_buffer,
  std::vector< int >& displacements )
{
  // get size of buffers
  std::vector< int > n_nodes( num_processes_ );
  n_nodes[ rank_ ] = send_buffer.size();
  communicate( n_nodes );
  // Set up displacements vector.
  displacements.resize( num_processes_, 0 );
  for ( int i = 1; i < num_processes_; ++i )
    displacements.at( i ) = displacements.at( i - 1 ) + n_nodes.at( i - 1 );

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
nest::Communicator::communicate( double_t send_val, std::vector< double_t >& recv_buffer )
{
  recv_buffer.resize( num_processes_ );
  MPI_Allgather( &send_val, 1, MPI_DOUBLE, &recv_buffer[ 0 ], 1, MPI_DOUBLE, comm );
}


/**
 * communicate function for sending set-up information
 */
void
nest::Communicator::communicate( std::vector< int_t >& buffer )
{
  communicate_Allgather( buffer );
}

void
nest::Communicator::communicate( std::vector< long_t >& buffer )
{
  communicate_Allgather( buffer );
}

void
nest::Communicator::communicate_Allgather( std::vector< int_t >& buffer )
{
  // avoid aliasing, see http://www.mpi-forum.org/docs/mpi-11-html/node10.html
  int_t my_val = buffer[ rank_ ];
  MPI_Allgather( &my_val, 1, MPI_INT, &buffer[ 0 ], 1, MPI_INT, comm );
}

void
nest::Communicator::communicate_Allgather( std::vector< long_t >& buffer )
{
  // avoid aliasing, see http://www.mpi-forum.org/docs/mpi-11-html/node10.html
  long_t my_val = buffer[ rank_ ];
  MPI_Allgather( &my_val, 1, MPI_LONG, &buffer[ 0 ], 1, MPI_LONG, comm );
}

/**
 * Ensure all processes have reached the same stage by waiting until all
 * processes have sent a dummy message to process 0.
 */
void
nest::Communicator::synchronize()
{
  MPI_Barrier( comm );
}

void
nest::Communicator::test_link( int sender, int receiver )
{
  assert( sender < num_processes_ && receiver < num_processes_ );

  if ( num_processes_ > 1 )
  {
    long dummy = 1;
    MPI_Status status;

    if ( rank_ == sender )
      MPI_Ssend( &dummy, 1, MPI_LONG, receiver, 0, comm );
    else if ( rank_ == receiver )
    {
      MPI_Recv( &dummy, 1, MPI_LONG, sender, 0, comm, &status );
      // std::cerr << "link between " << sender << " and " << receiver << " works" << std::endl;
    }
  }
}

void
nest::Communicator::test_links()
{
  for ( int i = 0; i < num_processes_; ++i )
    for ( int j = 0; j < num_processes_; ++j )
      if ( i != j )
        test_link( i, j );
  // std::cerr << "all links are working" << std::endl;
}

// grng_synchrony: called at the beginning of each simulate
bool
nest::Communicator::grng_synchrony( unsigned long process_rnd_number )
{
  if ( num_processes_ > 1 )
  {
    std::vector< unsigned long > rnd_numbers( num_processes_ );
    MPI_Allgather(
      &process_rnd_number, 1, MPI_UNSIGNED_LONG, &rnd_numbers[ 0 ], 1, MPI_UNSIGNED_LONG, comm );
    // compare all rnd numbers
    for ( uint_t i = 1; i < rnd_numbers.size(); ++i )
    {
      if ( rnd_numbers[ i - 1 ] != rnd_numbers[ i ] )
      {
        return false;
      }
    }
  }
  return true;
}

// average communication time for a packet size of num_bytes using Allgather
nest::double_t
nest::Communicator::time_communicate( int num_bytes, int samples )
{
  if ( num_processes_ == 1 )
    return 0.0;
  uint_t packet_length = num_bytes / sizeof( uint_t );
  if ( packet_length < 1 )
    packet_length = 1;
  std::vector< uint_t > test_send_buffer( packet_length );
  std::vector< uint_t > test_recv_buffer( packet_length * num_processes_ );
  // start time measurement here
  Stopwatch foo;
  foo.start();
  for ( int i = 0; i < samples; ++i )
    MPI_Allgather( &test_send_buffer[ 0 ],
      packet_length,
      MPI_UNSIGNED,
      &test_recv_buffer[ 0 ],
      packet_length,
      MPI_UNSIGNED,
      MPI_COMM_WORLD );
  // finish time measurement here
  foo.stop();
  return foo.elapsed() / samples;
}

// average communication time for a packet size of num_bytes using Allgatherv
nest::double_t
nest::Communicator::time_communicatev( int num_bytes, int samples )
{
  if ( num_processes_ == 1 )
    return 0.0;
  uint_t packet_length = num_bytes / sizeof( uint_t );
  if ( packet_length < 1 )
    packet_length = 1;
  std::vector< uint_t > test_send_buffer( packet_length );
  std::vector< uint_t > test_recv_buffer( packet_length * num_processes_ );
  std::vector< int > n_nodes( num_processes_, packet_length );
  std::vector< int > displacements( num_processes_, 0 );

  for ( int i = 1; i < num_processes_; ++i )
    displacements.at( i ) = displacements.at( i - 1 ) + n_nodes.at( i - 1 );

  // start time measurement here
  Stopwatch foo;
  foo.start();
  for ( int i = 0; i < samples; ++i )
    communicate_Allgatherv( test_send_buffer, test_recv_buffer, displacements, n_nodes );

  // finish time measurement here
  foo.stop();
  return foo.elapsed() / samples;
}

// average communication time for a packet size of num_bytes
nest::double_t
nest::Communicator::time_communicate_offgrid( int num_bytes, int samples )
{
  if ( num_processes_ == 1 )
    return 0.0;
  uint_t packet_length = num_bytes / sizeof( OffGridSpike );
  if ( packet_length < 1 )
    packet_length = 1;
  std::vector< OffGridSpike > test_send_buffer( packet_length );
  std::vector< OffGridSpike > test_recv_buffer( packet_length * num_processes_ );
  // start time measurement here
  Stopwatch foo;
  foo.start();
  for ( int i = 0; i < samples; ++i )
    MPI_Allgather( &test_send_buffer[ 0 ],
      packet_length,
      MPI_OFFGRID_SPIKE,
      &test_recv_buffer[ 0 ],
      packet_length,
      MPI_OFFGRID_SPIKE,
      MPI_COMM_WORLD );
  // finish time measurement here
  foo.stop();
  return foo.elapsed() / samples;
}

// average communication time for a packet size of num_bytes using Alltoall
nest::double_t
nest::Communicator::time_communicate_alltoall( int num_bytes, int samples )
{
  if ( num_processes_ == 1 )
    return 0.0;
  uint_t packet_length = num_bytes / sizeof( uint_t ); // this size should be sent to each process
  uint_t total_packet_length =
    packet_length * num_processes_; // this is the total size of send and receive buffers
  if ( total_packet_length < 1 )
    total_packet_length = 1;
  std::vector< uint_t > test_send_buffer( total_packet_length );
  std::vector< uint_t > test_recv_buffer( total_packet_length );
  // start time measurement here
  Stopwatch foo;
  foo.start();
  for ( int i = 0; i < samples; ++i )
  {
    MPI_Alltoall( &test_send_buffer[ 0 ],
      packet_length,
      MPI_UNSIGNED,
      &test_recv_buffer[ 0 ],
      packet_length,
      MPI_UNSIGNED,
      MPI_COMM_WORLD );
  }
  // finish time measurement here
  foo.stop();
  return foo.elapsed() / samples;
}

// average communication time for a packet size of num_bytes using Alltoallv
nest::double_t
nest::Communicator::time_communicate_alltoallv( int num_bytes, int samples )
{
  if ( num_processes_ == 1 )
    return 0.0;
  uint_t packet_length = num_bytes / sizeof( uint_t ); // this size should be sent to each process
  uint_t total_packet_length =
    packet_length * num_processes_; // this is the total size of send and receive buffers
  if ( total_packet_length < 1 )
    total_packet_length = 1;
  std::vector< uint_t > test_send_buffer( total_packet_length );
  std::vector< uint_t > test_recv_buffer( total_packet_length );
  std::vector< int > n_nodes( num_processes_, packet_length );
  std::vector< int > displacements( num_processes_, 0 );

  for ( int i = 1; i < num_processes_; ++i )
    displacements.at( i ) = displacements.at( i - 1 ) + n_nodes.at( i - 1 );

  // start time measurement here
  Stopwatch foo;
  foo.start();
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
      MPI_COMM_WORLD );
  }
  // finish time measurement here
  foo.stop();
  return foo.elapsed() / samples;
}


void
nest::Communicator::communicate_connector_properties( DictionaryDatum& dict )
{
  // Confirm that we're having a MPI process
  if ( num_processes_ > 1 )
  {
    // Move local dictionary values to temporary storage vectors.
    std::vector< nest::long_t > targets =
      getValue< std::vector< nest::long_t > >( dict, "targets" );

    std::vector< nest::double_t > weights =
      getValue< std::vector< nest::double_t > >( dict, "weights" );

    std::vector< nest::double_t > delays =
      getValue< std::vector< nest::double_t > >( dict, "delays" );

    std::vector< nest::long_t > receptors =
      getValue< std::vector< nest::long_t > >( dict, "receptors" );

    // Calculate size of communication buffers (number of connections).
    std::vector< nest::int_t > num_connections( num_processes_ );
    num_connections[ rank_ ] = targets.size();
    communicate( num_connections );

    // Set up displacements vector.
    std::vector< int > displacements( num_processes_, 0 );

    for ( size_t i = 1; i < num_connections.size(); ++i )
      displacements.at( i ) = displacements.at( i - 1 ) + num_connections.at( i - 1 );

    // Calculate sum of global connections.
    nest::int_t num_connections_sum =
      std::accumulate( num_connections.begin(), num_connections.end(), 0 );

    if ( num_connections_sum != 0 )
    {
      // Create global buffers.
      std::vector< nest::long_t > targets_result( num_connections_sum, 0 );

      std::vector< nest::long_t > receptors_result( num_connections_sum, 0 );

      std::vector< nest::double_t > weights_result( num_connections_sum, 0 );

      std::vector< nest::double_t > delays_result( num_connections_sum, 0 );

      // Start communication.
      communicate_Allgatherv< nest::long_t >(
        targets, targets_result, displacements, num_connections );

      communicate_Allgatherv< nest::long_t >(
        receptors, receptors_result, displacements, num_connections );

      communicate_Allgatherv< nest::double_t >(
        weights, weights_result, displacements, num_connections );

      communicate_Allgatherv< nest::double_t >(
        delays, delays_result, displacements, num_connections );

      // Save global values in input dictionary.
      ( *dict )[ "targets" ] = targets_result;
      ( *dict )[ "receptors" ] = receptors_result;
      ( *dict )[ "weights" ] = weights_result;
      ( *dict )[ "delays" ] = delays_result;
    }
  }
}


#ifdef HAVE_MUSIC // functions for interaction with MUSIC library
MUSIC::Setup*
nest::Communicator::get_music_setup()
{
  return music_setup;
}

MUSIC::Runtime*
nest::Communicator::get_music_runtime()
{
  return music_runtime;
}

void
nest::Communicator::enter_runtime( double_t h_min_delay )
{
  // MUSIC needs the step size in seconds
  // std::cout << "nest::Communicator::enter_runtime\n";
  // std::cout << "timestep = " << h_min_delay*1e-3 << std::endl;

  if ( music_runtime == 0 )
    music_runtime = new MUSIC::Runtime( music_setup, h_min_delay * 1e-3 );
}

void
nest::Communicator::advance_music_time( long_t num_steps )
{
  for ( int s = 0; s < num_steps; s++ )
    music_runtime->tick();
}
#endif /* #ifdef HAVE_MUSIC */

#else /* #ifdef HAVE_MPI */

/**
 * communicate (on-grid) if compiled without MPI
 */
void
nest::Communicator::communicate( std::vector< uint_t >& send_buffer,
  std::vector< uint_t >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements[ 0 ] = 0;
  if ( static_cast< size_t >( recv_buffer_size_ ) < send_buffer.size() )
  {
    recv_buffer_size_ = send_buffer_size_ = send_buffer.size();
    recv_buffer.resize( recv_buffer_size_ );
  }
  recv_buffer.swap( send_buffer );
}

/**
 * communicate (off-grid) if compiled without MPI
 */
void
nest::Communicator::communicate( std::vector< OffGridSpike >& send_buffer,
  std::vector< OffGridSpike >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements[ 0 ] = 0;
  if ( static_cast< size_t >( recv_buffer_size_ ) < send_buffer.size() )
  {
    recv_buffer_size_ = send_buffer_size_ = send_buffer.size();
    recv_buffer.resize( recv_buffer_size_ );
  }
  recv_buffer.swap( send_buffer );
}

void
nest::Communicator::communicate( std::vector< double_t >& send_buffer,
  std::vector< double_t >& recv_buffer,
  std::vector< int >& displacements )
{
  displacements.resize( 1 );
  displacements[ 0 ] = 0;
  recv_buffer.swap( send_buffer );
}

void
nest::Communicator::communicate( double_t send_val, std::vector< double_t >& recv_buffer )
{
  recv_buffer.resize( 1 );
  recv_buffer[ 0 ] = send_val;
}

#endif /* #ifdef HAVE_MPI */
