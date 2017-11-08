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

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

class MPIManager : public ManagerInterface
{
public:
  // forward declaration of internal classes
  class OffGridSpike;
  class NodeAddressingData;

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
   * Get number of recording processes.
   */
  thread get_num_rec_processes() const;

  /**
   * Get number of simulating processes.
   */
  thread get_num_sim_processes() const;

  /**
   * Set number of recording processes, switches NEST to global
   * spike detection mode.
   *
   * @param nrp  number of recording processes
   * @param called_by_reset   pass true when calling from Scheduler::reset()
   *
   * @note The `called_by_reset` parameter is a cludge to avoid a
   *       chicken-and-egg problem when resetting the kernel. It surpresses a
   *       test for existing nodes, trusting that the kernel will immediately
   *       afterwards delete all existing nodes.
   */
  void set_num_rec_processes( int nrp, bool called_by_reset );

  /**
   * Return the process id for a given virtual process. The real process' id
   * of a virtual process is defined by the relation: p = (vp mod P), where
   * P is the total number of processes.
   */
  thread get_process_id( const thread vp ) const;

  /*
   * Return the process id of the node with gid.
   */
  thread get_process_id_of_gid( const index gid ) const;

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

  /*
   * Maximum across all ranks
   */
  void communicate_Allreduce_max_in_place( std::vector< long >& buffer );

  /**
   * Collect GIDs for all nodes in a given node list across processes.
   * The NodeListType should be one of LocalNodeList, LocalLeafList,
   * LocalChildList.
   */
  template < typename NodeListType >
  void communicate( const NodeListType& local_nodes,
    std::vector< NodeAddressingData >& all_nodes,
    bool remote = false );

  template < typename NodeListType >
  void communicate( const NodeListType& local_nodes,
    std::vector< NodeAddressingData >& all_nodes,
    DictionaryDatum params,
    bool remote = false );

  // TODO: not used...
  void communicate_connector_properties( DictionaryDatum& dict );

  std::string get_processor_name();

  // int get_send_buffer_size();
  // int get_recv_buffer_size();
  bool is_mpi_used();
  size_t get_buffer_size_target_data() const;
  size_t get_buffer_size_spike_data() const;
  size_t get_chunk_size_secondary_events() const;
  size_t get_buffer_size_secondary_events() const;

  void communicate_Alltoall( unsigned int* send_buffer,
    unsigned int* recv_buffer,
    const unsigned int send_recv_count );
  void communicate_secondary_events_Alltoall( unsigned int* send_buffer,
    unsigned int* recv_buffer );

  void synchronize();

  // TODO: not used...
  void test_link( int, int );
  void test_links();

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

  // void set_buffer_sizes( int send_buffer_size, int recv_buffer_size );
  void set_buffer_size_target_data( size_t buffer_size );
  void set_buffer_size_spike_data( size_t buffer_size );

  void set_chunk_size_secondary_events( const size_t chunk_size );

  bool increase_buffer_size_target_data();
  bool increase_buffer_size_spike_data();

  bool adaptive_target_buffers() const;
  bool adaptive_spike_buffers() const;

private:
  int num_processes_;    //!< number of MPI processes
  int rank_;             //!< rank of the MPI process
  index n_rec_procs_;    //!< MPI processes dedicated for recording devices
  index n_sim_procs_;    //!< MPI processes used for simulation
  int send_buffer_size_; //!< expected size of send buffer
  int recv_buffer_size_; //!< size of receive buffer
  bool use_mpi_;         //!< whether MPI is used
  size_t buffer_size_target_data_;     //!< total size of MPI buffer for
                                       //communication of connections
  size_t buffer_size_spike_data_;      //!< total size of MPI buffer for
                                       //communication of spikes
  size_t chunk_size_secondary_events_; //!< total size of MPI buffer for
                                       //communication of secondary events
  size_t max_buffer_size_target_data_; //!< maximal size of MPI buffer for
                                       //communication of connections
  size_t max_buffer_size_spike_data_;  //!< maximal size of MPI buffer for
                                       //communication of spikes
  bool adaptive_target_buffers_; //!< whether MPI buffers for communication of
                                 //connections resize on the fly
  bool adaptive_spike_buffers_;  //!< whether MPI buffers for communication of
                                 //spikes resize on the fly
  double growth_factor_buffer_spike_data_;
  double growth_factor_buffer_target_data_;

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

  class NodeAddressingData
  {
  public:
    NodeAddressingData()
      : gid_( 0 )
      , parent_gid_( 0 )
      , vp_( 0 )
    {
    }
    NodeAddressingData( unsigned int gid,
      unsigned int parent_gid,
      unsigned int vp )
      : gid_( gid )
      , parent_gid_( parent_gid )
      , vp_( vp )
    {
    }

    unsigned int
    get_gid() const
    {
      return gid_;
    }
    unsigned int
    get_parent_gid() const
    {
      return parent_gid_;
    }
    unsigned int
    get_vp() const
    {
      return vp_;
    }
    bool operator<( const NodeAddressingData& other ) const
    {
      return this->gid_ < other.gid_;
    }
    bool operator==( const NodeAddressingData& other ) const
    {
      return this->gid_ == other.gid_;
    }

  private:
    unsigned int gid_;        //!< GID of neuron
    unsigned int parent_gid_; //!< GID of neuron's parent
    unsigned int vp_;         //!< virtual process of neuron
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

inline thread
MPIManager::get_num_rec_processes() const
{
  return n_rec_procs_;
}

inline thread
MPIManager::get_num_sim_processes() const
{
  return n_sim_procs_;
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

inline size_t
MPIManager::get_buffer_size_spike_data() const
{
  return buffer_size_spike_data_;
}

inline size_t
MPIManager::get_chunk_size_secondary_events() const
{
  return chunk_size_secondary_events_;
}

inline size_t
MPIManager::get_buffer_size_secondary_events() const
{
  return chunk_size_secondary_events_ * get_num_processes();
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
}

inline void
MPIManager::set_chunk_size_secondary_events( const size_t chunk_size )
{
  chunk_size_secondary_events_ = chunk_size;
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
      buffer_size_target_data_ = static_cast< size_t >( floor( buffer_size_target_data_ * growth_factor_buffer_target_data_ ) );
    }
    else
    {
      buffer_size_target_data_ = max_buffer_size_target_data_;
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
      buffer_size_spike_data_ = static_cast< size_t >( floor( buffer_size_spike_data_ * growth_factor_buffer_spike_data_ ) );
    }
    else
    {
      buffer_size_spike_data_ = max_buffer_size_spike_data_;
    }
    return true;
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
MPIManager::communicate_connector_properties( DictionaryDatum& )
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
