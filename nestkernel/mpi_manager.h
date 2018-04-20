/*
 *  mpi_manager.h
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

#ifndef MPI_MANAGER_H
#define MPI_MANAGER_H

// Generated includes:
#include "config.h"

// C includes:
#include <unistd.h>
#ifdef HAVE_MPI
#include <mpi.h>
#endif

// C++ includes:
#include <cassert>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

class MPIManager : public ManagerInterface
{
public:
  // forward declaration of internal classes
  class OffGridSpike;

  MPIManager();
  ~MPIManager()
  {
  }

  virtual void initialize();
  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  void init_mpi( int* argc, char** argv[] );

  /**
   * Return the number of processes used during simulation.
   * This functions returns the number of processes.
   * Since each process has the same number of threads, the total number
   * of threads is given by get_num_threads()*get_num_processes().
   */
  thread get_num_processes() const;

  /**
   * Set the number of processes state variable.
   * This is used by dryrun_mode.
   */
  void set_num_processes( thread n_procs );

  /**
   * Get rank of MPI process
   */
  thread get_rank() const;

  /**
   * Return the process id for a given virtual process. The real process' id
   * of a virtual process is defined by the relation: p = (vp mod P), where
   * P is the total number of processes.
   */
  thread get_process_id( thread vp ) const;

  /**
   * Finalize MPI communication (needs to be separate from MPIManager::finalize
   * when compiled with MUSIC since spikes can arrive and handlers called here)
   */
  void mpi_finalize( int exitcode );

  /**
   * If MPI is available, this method calls MPI_Abort with the exitcode.
   */
  void mpi_abort( int exitcode );


  void communicate( std::vector< unsigned int >& send_buffer,
    std::vector< unsigned int >& recv_buffer,
    std::vector< int >& displacements );

  void communicate( std::vector< OffGridSpike >& send_buffer,
    std::vector< OffGridSpike >& recv_buffer,
    std::vector< int >& displacements );

  void communicate( std::vector< double >& send_buffer,
    std::vector< double >& recv_buffer,
    std::vector< int >& displacements );

  void communicate( std::vector< unsigned long >& send_buffer,
    std::vector< unsigned long >& recv_buffer,
    std::vector< int >& displacements );

  void communicate( std::vector< int >& send_buffer,
    std::vector< int >& recv_buffer,
    std::vector< int >& displacements );

  void communicate( double, std::vector< double >& );
  void communicate( std::vector< int >& );
  void communicate( std::vector< long >& );

  /*
   * Sum across all rank
   */
  void communicate_Allreduce_sum_in_place( double buffer );
  void communicate_Allreduce_sum_in_place( std::vector< double >& buffer );
  void communicate_Allreduce_sum_in_place( std::vector< int >& buffer );
  void communicate_Allreduce_sum( std::vector< double >& send_buffer,
    std::vector< double >& recv_buffer );

  std::string get_processor_name();

  int get_send_buffer_size();
  int get_recv_buffer_size();
  bool is_mpi_used();

  void synchronize();

  bool grng_synchrony( unsigned long );
  bool any_true( const bool );

  /** Benchmark communication time of different MPI methods
   *
   *  The methods `time_communicate*` can be used to benchmark the timing
   *  of different MPI communication methods.
   */
  double time_communicate( int num_bytes, int samples = 1000 );
  double time_communicatev( int num_bytes, int samples = 1000 );
  double time_communicate_offgrid( int num_bytes, int samples = 1000 );
  double time_communicate_alltoall( int num_bytes, int samples = 1000 );
  double time_communicate_alltoallv( int num_bytes, int samples = 1000 );

  void set_buffer_sizes( int send_buffer_size, int recv_buffer_size );

private:
  int num_processes_;    //!< number of MPI processes
  int rank_;             //!< rank of the MPI process
  int send_buffer_size_; //!< expected size of send buffer
  int recv_buffer_size_; //!< size of receive buffer
  bool use_mpi_;         //!< whether MPI is used

#ifdef HAVE_MPI
  //! array containing communication partner for each step.
  std::vector< int > comm_step_;
  unsigned int COMM_OVERFLOW_ERROR;

//! Variable to hold the MPI communicator to use (the datatype matters).
#ifdef HAVE_MUSIC
  MPI::Intracomm comm;
#else  /* #ifdef HAVE_MUSIC */
  MPI_Comm comm;
#endif /* #ifdef HAVE_MUSIC */
  MPI_Datatype MPI_OFFGRID_SPIKE;

  void communicate_Allgather( std::vector< unsigned int >& send_buffer,
    std::vector< unsigned int >& recv_buffer,
    std::vector< int >& displacements );

  void communicate_Allgather( std::vector< OffGridSpike >& send_buffer,
    std::vector< OffGridSpike >& recv_buffer,
    std::vector< int >& displacements );

  void communicate_Allgather( std::vector< int >& );
  void communicate_Allgather( std::vector< long >& );

  template < typename T >
  void communicate_Allgatherv( std::vector< T >& send_buffer,
    std::vector< T >& recv_buffer,
    std::vector< int >& displacements,
    std::vector< int >& recv_counts );

  template < typename T >
  void communicate_Allgather( std::vector< T >& send_buffer,
    std::vector< T >& recv_buffer,
    std::vector< int >& displacements );

#endif /* #ifdef HAVE_MPI */

public:
  /**
   * Combined storage of GID and offset information for off-grid spikes.
   *
   * @note This class actually stores the GID as @c double internally.
   *       This is done so that the user-defined MPI type MPI_OFFGRID_SPIKE,
   *       which we use to communicate off-grid spikes, is homogeneous.
   *       Otherwise, OpenMPI spends extreme amounts of time on packing
   *       and unpacking the data, see #458.
   */
  class OffGridSpike
  {
    friend void MPIManager::init_mpi( int*, char*** );

  public:
    //! We defined this type explicitly, so that the assert function below
    //! always tests the correct type.
    typedef unsigned int gid_external_type;

    OffGridSpike()
      : gid_( 0 )
      , offset_( 0.0 )
    {
    }
    OffGridSpike( gid_external_type gidv, double offsetv )
      : gid_( gidv )
      , offset_( offsetv )
    {
    }

    unsigned int
    get_gid() const
    {
      return static_cast< gid_external_type >( gid_ );
    }
    void
    set_gid( gid_external_type gid )
    {
      gid_ = static_cast< double >( gid );
    }
    double
    get_offset() const
    {
      return offset_;
    }

  private:
    double gid_;    //!< GID of neuron that spiked
    double offset_; //!< offset of spike from grid

    //! This function asserts that doubles can hold GIDs without loss
    static void
    assert_datatype_compatibility_()
    {
      assert( std::numeric_limits< double >::digits
        > std::numeric_limits< gid_external_type >::digits );

      // the next one is doubling up, better be safe than sorry
      const gid_external_type maxgid =
        std::numeric_limits< gid_external_type >::max();
      OffGridSpike ogs( maxgid, 0.0 );
      assert( maxgid == ogs.get_gid() );
    }
  };
};

inline thread
MPIManager::get_num_processes() const
{
  return num_processes_;
}

inline void
MPIManager::set_num_processes( thread n_procs )
{
  num_processes_ = n_procs;
}

inline thread
MPIManager::get_rank() const
{
  return rank_;
}

inline int
MPIManager::get_send_buffer_size()
{
  return send_buffer_size_;
}

inline int
MPIManager::get_recv_buffer_size()
{
  return recv_buffer_size_;
}

inline bool
MPIManager::is_mpi_used()
{
  return use_mpi_;
}


inline void
MPIManager::set_buffer_sizes( int send_buffer_size, int recv_buffer_size )
{
  send_buffer_size_ = send_buffer_size;
  recv_buffer_size_ = recv_buffer_size;
}

#ifndef HAVE_MPI
inline std::string
MPIManager::get_processor_name()
{
  char name[ 1024 ];
  name[ 1023 ] = '\0';
  gethostname( name, 1023 );
  return name;
}

inline void
MPIManager::mpi_abort( int )
{
}

inline void
MPIManager::communicate( std::vector< int >& )
{
}

inline void
MPIManager::communicate( std::vector< long >& )
{
}

inline void
MPIManager::synchronize()
{
}

inline void
test_link( int, int )
{
}

inline void
test_links()
{
}

/* replaced u_long with unsigned long since u_long is not known when
 mpi.h is not available. This is a rather ugly fix.
 HEP 2007-03-09
 */
inline bool
MPIManager::grng_synchrony( unsigned long )
{
  return true;
}

inline bool
MPIManager::any_true( const bool my_bool )
{
  return my_bool;
}

inline double
MPIManager::time_communicate( int, int )
{
  return 0.0;
}

inline double
MPIManager::time_communicatev( int, int )
{
  return 0.0;
}

inline double
MPIManager::time_communicate_offgrid( int, int )
{
  return 0.0;
}

inline double
MPIManager::time_communicate_alltoall( int, int )
{
  return 0.0;
}

inline double
MPIManager::time_communicate_alltoallv( int, int )
{
  return 0.0;
}
#endif
}

#endif /* MPI_MANAGER_H */
