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
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from nestkernel:
#include "nest_types.h"
#include "spike_data.h"
#include "target_data.h"

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
  ~MPIManager() override
  {
  }

  void initialize() override;
  void finalize() override;
  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) override;

  void init_mpi( int* argc, char** argv[] );
#ifdef HAVE_MPI
  void set_communicator( MPI_Comm );

  MPI_Comm
  get_communicator()
  {
    return comm;
  };
#endif

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
  thread get_process_id_of_vp( const thread vp ) const;

  /*
   * Return the process id of the node with the specified node ID.
   */
  thread get_process_id_of_node_id( const index node_id ) const;

  /**
   * Finalize MPI communication (needs to be separate from MPIManager::finalize
   * when compiled with MUSIC since spikes can arrive and handlers called here)
   */
  void mpi_finalize( int exitcode );

  /**
   * If MPI is available, this method calls MPI_Abort with the exitcode.
   */
  void mpi_abort( int exitcode );

  /*
   * gather all send_buffer vectors on other mpi process to recv_buffer
   * vector
   */
  void communicate( std::vector< long >& send_buffer, std::vector< long >& recv_buffer );

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

  void
  communicate( std::vector< int >& send_buffer, std::vector< int >& recv_buffer, std::vector< int >& displacements );

  void communicate( double, std::vector< double >& );
  void communicate( std::vector< int >& );
  void communicate( std::vector< long >& );

  /*
   * Sum across all rank
   */
  void communicate_Allreduce_sum_in_place( double buffer );
  void communicate_Allreduce_sum_in_place( std::vector< double >& buffer );
  void communicate_Allreduce_sum_in_place( std::vector< int >& buffer );
  void communicate_Allreduce_sum( std::vector< double >& send_buffer, std::vector< double >& recv_buffer );

  /**
   * Equal across all ranks.
   *
   * @param value value on calling rank
   * @return true if values across all ranks are equal, false otherwise or if
   *         any rank passes -inf as value
   */
  bool equal_cross_ranks( const double value );

  std::string get_processor_name();

  bool is_mpi_used();

  /**
   * Returns total size of MPI buffer for communication of connections.
   */
  size_t get_buffer_size_target_data() const;

  /**
   * Returns size of MPI buffer for connections divided by number of processes.
   */
  unsigned int get_send_recv_count_target_data_per_rank() const;

  /**
   * Returns total size of MPI buffer for communication of spikes.
   */
  size_t get_buffer_size_spike_data() const;

  /**
   * Returns size of MPI buffer for spikes divided by number of processes.
   */
  unsigned int get_send_recv_count_spike_data_per_rank() const;

  /**
   * Returns total size of MPI send buffer for communication of secondary events.
   */
  size_t get_send_buffer_size_secondary_events_in_int() const;

  /**
   * Returns total size of MPI recv buffer for communication of secondary events.
   */
  size_t get_recv_buffer_size_secondary_events_in_int() const;

#ifdef HAVE_MPI

  void communicate_Alltoall_( void* send_buffer, void* recv_buffer, const unsigned int send_recv_count );

  void communicate_Alltoallv_( void* send_buffer,
    const int* send_counts,
    const int* send_displacements,
    void* recv_buffer,
    const int* recv_counts,
    const int* recv_displacements );

#endif // HAVE_MPI

  template < class D >
  void communicate_Alltoall( std::vector< D >& send_buffer,
    std::vector< D >& recv_buffer,
    const unsigned int send_recv_count );
  template < class D >
  void communicate_target_data_Alltoall( std::vector< D >& send_buffer, std::vector< D >& recv_buffer );
  template < class D >
  void communicate_spike_data_Alltoall( std::vector< D >& send_buffer, std::vector< D >& recv_buffer );
  template < class D >
  void communicate_off_grid_spike_data_Alltoall( std::vector< D >& send_buffer, std::vector< D >& recv_buffer );
  template < class D >
  void communicate_secondary_events_Alltoallv( std::vector< D >& send_buffer, std::vector< D >& recv_buffer );

  void synchronize();

  bool any_true( const bool );

  /**
   * Benchmark communication time of different MPI methods
   *
   *  The methods `time_communicate*` can be used to benchmark the timing
   *  of different MPI communication methods.
   */
  double time_communicate( int num_bytes, int samples = 1000 );
  double time_communicatev( int num_bytes, int samples = 1000 );
  double time_communicate_offgrid( int num_bytes, int samples = 1000 );
  double time_communicate_alltoall( int num_bytes, int samples = 1000 );
  double time_communicate_alltoallv( int num_bytes, int samples = 1000 );

  void set_buffer_size_target_data( size_t buffer_size );
  void set_buffer_size_spike_data( size_t buffer_size );

  /**
   * Increases the size of the MPI buffer for communication of connections if it
   * needs to be increased. Returns whether the size was changed.
   */
  bool increase_buffer_size_target_data();

  /**
   * Increases the size of the MPI buffer for communication of spikes if it
   * needs to be increased. Returns whether the size was changed.
   */
  bool increase_buffer_size_spike_data();

  /**
   * Decreases the size of the MPI buffer for communication of spikes if it
   * can be decreased.
   */
  void decrease_buffer_size_spike_data();

  /**
   * Returns whether MPI buffers for communication of connections are adaptive.
   */
  bool adaptive_target_buffers() const;

  /**
   * Returns whether MPI buffers for communication of spikes are adaptive.
   */
  bool adaptive_spike_buffers() const;

  /**
   * Sets the recvcounts parameter of Alltoallv for communication of
   * secondary events, i.e., the number of elements (in ints) to recv
   * from the corresponding rank.
   */
  void set_recv_counts_secondary_events_in_int_per_rank( const std::vector< int >& recv_counts_in_int_per_rank );

  /**
   * Returns the recvcounts parameter of Alltoallv for communication of
   * secondary events, i.e., the number of elements (in ints) to recv
   * from `source_rank`.
   */
  size_t get_recv_count_secondary_events_in_int( const size_t source_rank ) const;

  /**
   * Returns the rdispls parameter of Alltoallv for communication of
   * secondary events, i.e., the offset in the MPI buffer where
   * elements from `source_rank` are written.
   */
  size_t get_recv_displacement_secondary_events_in_int( const size_t source_rank ) const;

  /**
   * Returns the number of elements (in ints) to be sent to `target_rank`.
   */
  size_t get_send_count_secondary_events_in_int( const size_t target_rank ) const;

  /**
   * Returns the send displacement of elements (in ints) to be sent to rank `target_rank`.
   */
  size_t get_send_displacement_secondary_events_in_int( const size_t target_rank ) const;

  /**
   * Returns where the done marker is located in the MPI send buffer for `target_rank`.
   */
  size_t get_done_marker_position_in_secondary_events_send_buffer( const size_t target_rank ) const;

  /**
   * Returns where the done marker is located in the MPI recv buffer for `source_rank`.
   */
  size_t get_done_marker_position_in_secondary_events_recv_buffer( const size_t source_rank ) const;

  void communicate_recv_counts_secondary_events();

private:
  int num_processes_;              //!< number of MPI processes
  int rank_;                       //!< rank of the MPI process
  int send_buffer_size_;           //!< expected size of send buffer
  int recv_buffer_size_;           //!< size of receive buffer
  bool use_mpi_;                   //!< whether MPI is used
  size_t buffer_size_target_data_; //!< total size of MPI buffer for
  // communication of connections

  size_t buffer_size_spike_data_; //!< total size of MPI buffer for
  // communication of spikes

  size_t max_buffer_size_target_data_; //!< maximal size of MPI buffer for
  // communication of connections

  size_t max_buffer_size_spike_data_; //!< maximal size of MPI buffer for
  // communication of spikes

  bool adaptive_target_buffers_; //!< whether MPI buffers for communication of
  // connections resize on the fly

  bool adaptive_spike_buffers_; //!< whether MPI buffers for communication of
  // spikes resize on the fly

  double growth_factor_buffer_spike_data_;
  double growth_factor_buffer_target_data_;

  double shrink_factor_buffer_spike_data_;

  unsigned int send_recv_count_spike_data_per_rank_;
  unsigned int send_recv_count_target_data_per_rank_;

  std::vector< int > recv_counts_secondary_events_in_int_per_rank_; //!< how many secondary elements (in ints) will be
                                                                    //!< received from each rank
  std::vector< int >
    send_counts_secondary_events_in_int_per_rank_; //!< how many secondary elements (in ints) will be sent to each rank

  std::vector< int > recv_displacements_secondary_events_in_int_per_rank_; //!< offset in the MPI receive buffer (in
  //!< ints) at which elements received from each
  //!< rank will be written

  std::vector< int > send_displacements_secondary_events_in_int_per_rank_; //!< offset in the MPI send buffer (in ints)
  //!< from which elements send to each rank will
  //!< be read

#ifdef HAVE_MPI
  //! array containing communication partner for each step.
  std::vector< int > comm_step_;
  unsigned int COMM_OVERFLOW_ERROR;

  //! Variable to hold the MPI communicator to use (the datatype matters).
  MPI_Comm comm;
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
   * Combined storage of node ID and offset information for off-grid spikes.
   *
   * @note This class actually stores the node ID as @c double internally.
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
    typedef unsigned int node_id_external_type;

    OffGridSpike()
      : node_id_( 0 )
      , offset_( 0.0 )
    {
    }
    OffGridSpike( node_id_external_type node_idv, double offsetv )
      : node_id_( node_idv )
      , offset_( offsetv )
    {
    }

    unsigned int
    get_node_id() const
    {
      return static_cast< node_id_external_type >( node_id_ );
    }
    void
    set_node_id( node_id_external_type node_id )
    {
      node_id_ = static_cast< double >( node_id );
    }
    double
    get_offset() const
    {
      return offset_;
    }

  private:
    double node_id_; //!< node ID of neuron that spiked
    double offset_;  //!< offset of spike from grid

    //! This function asserts that doubles can hold node IDs without loss
    static void
    assert_datatype_compatibility_()
    {
      assert( std::numeric_limits< double >::digits > std::numeric_limits< node_id_external_type >::digits );

      // the next one is doubling up, better be safe than sorry
      const node_id_external_type maxnode_id = std::numeric_limits< node_id_external_type >::max();
      OffGridSpike ogs( maxnode_id, 0.0 );
      assert( maxnode_id == ogs.get_node_id() );
    }
  };
};

inline void
MPIManager::set_recv_counts_secondary_events_in_int_per_rank( const std::vector< int >& recv_counts_in_int_per_rank )
{
  recv_counts_secondary_events_in_int_per_rank_ = recv_counts_in_int_per_rank;

  std::partial_sum( recv_counts_secondary_events_in_int_per_rank_.begin(),
    recv_counts_secondary_events_in_int_per_rank_.end() - 1,
    recv_displacements_secondary_events_in_int_per_rank_.begin() + 1 );
}

inline size_t
MPIManager::get_recv_count_secondary_events_in_int( const size_t source_rank ) const
{
  return recv_counts_secondary_events_in_int_per_rank_[ source_rank ];
}

inline size_t
MPIManager::get_recv_displacement_secondary_events_in_int( const size_t source_rank ) const
{
  return recv_displacements_secondary_events_in_int_per_rank_[ source_rank ];
}

inline size_t
MPIManager::get_send_count_secondary_events_in_int( const size_t target_rank ) const
{
  return send_counts_secondary_events_in_int_per_rank_[ target_rank ];
}

inline size_t
MPIManager::get_send_displacement_secondary_events_in_int( const size_t target_rank ) const
{
  return send_displacements_secondary_events_in_int_per_rank_[ target_rank ];
}

inline size_t
MPIManager::get_done_marker_position_in_secondary_events_send_buffer( const size_t target_rank ) const
{
  return get_send_displacement_secondary_events_in_int( target_rank )
    + get_send_count_secondary_events_in_int( target_rank ) - 1;
}

inline size_t
MPIManager::get_done_marker_position_in_secondary_events_recv_buffer( const size_t source_rank ) const
{
  return get_recv_displacement_secondary_events_in_int( source_rank )
    + get_recv_count_secondary_events_in_int( source_rank ) - 1;
}

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

inline bool
MPIManager::is_mpi_used()
{
  return use_mpi_;
}

inline size_t
MPIManager::get_buffer_size_target_data() const
{
  return buffer_size_target_data_;
}

inline unsigned int
MPIManager::get_send_recv_count_target_data_per_rank() const
{
  return send_recv_count_target_data_per_rank_;
}

inline size_t
MPIManager::get_buffer_size_spike_data() const
{
  return buffer_size_spike_data_;
}

inline unsigned int
MPIManager::get_send_recv_count_spike_data_per_rank() const
{
  return send_recv_count_spike_data_per_rank_;
}

inline size_t
MPIManager::get_send_buffer_size_secondary_events_in_int() const
{
  return send_displacements_secondary_events_in_int_per_rank_
           [ send_displacements_secondary_events_in_int_per_rank_.size() - 1 ]
    + send_counts_secondary_events_in_int_per_rank_[ send_counts_secondary_events_in_int_per_rank_.size() - 1 ];
}

inline size_t
MPIManager::get_recv_buffer_size_secondary_events_in_int() const
{
  return recv_displacements_secondary_events_in_int_per_rank_
           [ recv_displacements_secondary_events_in_int_per_rank_.size() - 1 ]
    + recv_counts_secondary_events_in_int_per_rank_[ recv_counts_secondary_events_in_int_per_rank_.size() - 1 ];
}

inline void
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

inline void
MPIManager::set_buffer_size_spike_data( const size_t buffer_size )
{
  assert( buffer_size >= static_cast< size_t >( 2 * get_num_processes() ) );
  if ( buffer_size <= max_buffer_size_spike_data_ )
  {
    buffer_size_spike_data_ = buffer_size;
  }
  else
  {
    buffer_size_spike_data_ = max_buffer_size_spike_data_;
  }

  send_recv_count_spike_data_per_rank_ = floor( get_buffer_size_spike_data() / get_num_processes() );

  assert( send_recv_count_spike_data_per_rank_ * get_num_processes() <= get_buffer_size_spike_data() );
}

inline bool
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

inline bool
MPIManager::increase_buffer_size_spike_data()
{
  assert( adaptive_spike_buffers_ );
  if ( buffer_size_spike_data_ >= max_buffer_size_spike_data_ )
  {
    return false;
  }
  else
  {
    if ( buffer_size_spike_data_ * growth_factor_buffer_spike_data_ < max_buffer_size_spike_data_ )
    {
      set_buffer_size_spike_data( floor( buffer_size_spike_data_ * growth_factor_buffer_spike_data_ ) );
    }
    else
    {
      set_buffer_size_spike_data( max_buffer_size_spike_data_ );
    }
    return true;
  }
}

inline void
MPIManager::decrease_buffer_size_spike_data()
{
  assert( adaptive_spike_buffers_ );
  // the minimum is set to 4.0 * get_num_processes() to differentiate the initial size
  if ( buffer_size_spike_data_ / shrink_factor_buffer_spike_data_ > 4.0 * get_num_processes() )
  {
    set_buffer_size_spike_data( floor( buffer_size_spike_data_ / shrink_factor_buffer_spike_data_ ) );
  }
}

inline bool
MPIManager::adaptive_target_buffers() const
{
  return adaptive_target_buffers_;
}

inline bool
MPIManager::adaptive_spike_buffers() const
{
  return adaptive_spike_buffers_;
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

#endif /* HAVE_MPI */

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

#endif // HAVE_MPI

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
}

#endif /* MPI_MANAGER_H */
