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
#ifdef HAVE_MPI
#include <mpi.h>
#endif

// C++ includes:
#include <cassert>
#include <limits>
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from nestkernel:
#include "kernel_manager.h"
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

  void initialize( const bool ) override;
  void finalize( const bool ) override;
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
   *
   * This functions returns the number of processes.
   * Since each process has the same number of threads, the total number
   * of threads is given by get_num_threads()*get_num_processes().
   */
  size_t get_num_processes() const;

  /**
   * Get rank of MPI process
   */
  size_t get_rank() const;

  /**
   * Return the process id for a given virtual process.
   * The real process' id of a virtual process is defined
   * by the relation: p = (vp mod P), where
   * P is the total number of processes.
   */
  size_t get_process_id_of_vp( const size_t vp ) const;

  /*
   * Return the process id of the node with the specified node ID.
   */
  size_t get_process_id_of_node_id( const size_t node_id ) const;

  /**
   * Finalize MPI communication (needs to be separate from MPIManager::finalize
   * when compiled with MUSIC since spikes can arrive and handlers called here)
   */
  void mpi_finalize( int exitcode );

  /**
   * If MPI is available, this method calls MPI_Abort with the exitcode.
   */
  void mpi_abort( int exitcode ) const;

  // gather all send_buffer vectors on other mpi process to recv_buffer
  // vector
  void communicate( std::vector< long >& send_buffer, std::vector< long >& recv_buffer );

  /**
   * communicate (on-grid) if compiled without MPI
   */
  void communicate( std::vector< unsigned int >& send_buffer,
    std::vector< unsigned int >& recv_buffer,
    std::vector< int >& displacements );

  /**
   * communicate (off-grid) if compiled without MPI
   */
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

  void communicate( double, std::vector< double >& ) const;
  void communicate( std::vector< int >& );
  void communicate( std::vector< long >& );

  //! Sum across all ranks
  void communicate_Allreduce_sum_in_place( double buffer ) const;
  void communicate_Allreduce_sum_in_place( std::vector< double >& buffer ) const;
  void communicate_Allreduce_sum_in_place( std::vector< int >& buffer ) const;
  void communicate_Allreduce_sum( std::vector< double >& send_buffer, std::vector< double >& recv_buffer ) const;

  /**
   * Equal across all ranks.
   *
   * @param value value on calling rank
   * @return true if values across all ranks are equal, false otherwise or if
   *         any rank passes -inf as value
   */
  bool equal_cross_ranks( const double value ) const;

  std::string get_processor_name();

  bool is_mpi_used() const;

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

  void communicate_Alltoall_( void* send_buffer, void* recv_buffer, const unsigned int send_recv_count ) const;

  void communicate_Alltoallv_( void* send_buffer,
    const int* send_counts,
    const int* send_displacements,
    void* recv_buffer,
    const int* recv_counts,
    const int* recv_displacements ) const;

#endif /* HAVE_MPI */

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

  /**
   * Ensure all processes have reached the same stage by waiting until all
   * processes have sent a dummy message to process 0.
   */
  void synchronize() const;

  bool any_true( const bool ) const;

  /**
   * Benchmark communication time of different MPI methods
   *
   * The methods `time_communicate*` can be used to benchmark the timing
   * of different MPI communication methods.
   */
  double time_communicate( int num_bytes, int samples = 1000 ) const;
  double time_communicatev( int num_bytes, int samples = 1000 );
  double time_communicate_offgrid( int num_bytes, int samples = 1000 ) const;
  double time_communicate_alltoall( int num_bytes, int samples = 1000 ) const;
  double time_communicate_alltoallv( int num_bytes, int samples = 1000 ) const;

  void set_buffer_size_target_data( size_t buffer_size );
  void set_buffer_size_spike_data( size_t buffer_size );

  /**
   * Increases the size of the MPI buffer for communication of connections if it
   * needs to be increased. Returns whether the size was changed.
   */
  bool increase_buffer_size_target_data();

  /**
   * Returns whether MPI buffers for communication of connections are adaptive.
   */
  bool adaptive_target_buffers() const;

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

  bool adaptive_target_buffers_; //!< whether MPI buffers for communication of
  // connections resize on the fly

  double growth_factor_buffer_spike_data_;
  double growth_factor_buffer_target_data_;

  double shrink_factor_buffer_spike_data_;

  unsigned int send_recv_count_spike_data_per_rank_;
  unsigned int send_recv_count_target_data_per_rank_;

  //! How many secondary elements (in ints) will be received from each rank
  std::vector< int > recv_counts_secondary_events_in_int_per_rank_;

  std::vector< int >
    send_counts_secondary_events_in_int_per_rank_; //!< how many secondary elements (in ints) will be sent to each rank

  //! Offset in the MPI receive buffer (in ints) at which elements received from each rank will be written
  std::vector< int > recv_displacements_secondary_events_in_int_per_rank_;

  //! Offset in the MPI send buffer (in ints) from which elements send to each rank will be read
  std::vector< int > send_displacements_secondary_events_in_int_per_rank_;

#ifdef HAVE_MPI

  std::vector< int > comm_step_;

  unsigned int COMM_OVERFLOW_ERROR; //<! array containing communication partner for each step.


  //! Variable to hold the MPI communicator to use (the datatype matters).
  MPI_Comm comm;
  MPI_Datatype MPI_OFFGRID_SPIKE;

  void communicate_Allgather( std::vector< unsigned int >& send_buffer,
    std::vector< unsigned int >& recv_buffer,
    std::vector< int >& displacements );

  void communicate_Allgather( std::vector< OffGridSpike >& send_buffer,
    std::vector< OffGridSpike >& recv_buffer,
    std::vector< int >& displacements );

  void communicate_Allgather( std::vector< int >& ) const;
  void communicate_Allgather( std::vector< long >& ) const;

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

}

#endif /* MPI_MANAGER_H */
