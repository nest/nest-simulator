/*
 *  vp_manager.h
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

#ifndef VP_MANAGER_H
#define VP_MANAGER_H

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from sli:
#include "dictdatum.h"

#include "mpi_manager.h"

#ifdef _OPENMP
// C includes:
#include <omp.h>
#endif

namespace nest
{

struct AssignedRanks
{
  size_t begin;
  size_t end;
  size_t size;
  size_t max_size;
};

class VPManager : public ManagerInterface
{
public:
  VPManager();
  ~VPManager() override
  {
  }

  void initialize( const bool ) override;
  void finalize( const bool ) override;

  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) override;

  /**
   * Gets ID of local thread.
   * Returns thread ID if OpenMP is installed
   * and zero otherwise.
   */
  size_t get_thread_id() const;

  /**
   * Set the number of threads by setting the internal variable
   * n_threads_, the corresponding value in the Communicator, and
   * the OpenMP number of threads.
   */
  void set_num_threads( const size_t n_threads );

  /**
   * Get number of threads.
   *
   * This function returns the total number of threads per process.
   */
  size_t get_num_threads() const;

  /**
   * Get OMP_NUM_THREADS environment variable.
   *
   * @note Returns 0 if OMP_NUM_THREADS is not set.
   */
  size_t get_OMP_NUM_THREADS() const;

  /**
   * Returns true if the given global node exists on this vp.
   */
  bool is_node_id_vp_local( const size_t node_id ) const;

  /**
   * Returns thread local index of a given global node.
   */
  size_t node_id_to_lid( const size_t node_id ) const;

  /**
   * Returns the node ID of a given local index.
   */
  size_t lid_to_node_id( const size_t lid ) const;

  /**
   * Returns virtual process index.
   */
  size_t get_vp() const;

  /**
   * Return a thread number for a given global node id.
   *
   * Each node has a default thread on which it will run.
   * The thread is defined by the relation:
   * t = (node_id div P) mod T, where P is the number of simulation processes and
   * T the number of threads. This may be used by Network::add_node()
   * if the user has not specified anything.
   */
  size_t node_id_to_vp( const size_t node_id ) const;

  /**
   * Convert a given VP ID to the corresponding thread ID
   */
  size_t vp_to_thread( const size_t vp ) const;

  /**
   * Convert a given thread ID to the corresponding VP ID
   */
  size_t thread_to_vp( const size_t tid ) const;

  /**
   * Return true, if the given VP is on the local machine
   */
  bool is_local_vp( const size_t tid ) const;

  /**
   * Returns the number of virtual processes.
   */
  size_t get_num_virtual_processes() const;

  /**
   * Fails if NEST is in thread-parallel section.
   */
  void assert_single_threaded() const;

  /**
   * Fails if NEST is not in thread-parallel section.
   */
  void assert_thread_parallel() const;

  /**
   * Returns the number of processes that are taken care of by a single thread
   * while processing MPI buffers in a multithreaded environment.
   */
  size_t get_num_assigned_ranks_per_thread() const;

  size_t get_start_rank_per_thread( const size_t tid ) const;
  size_t get_end_rank_per_thread( const size_t rank_start, const size_t num_assigned_ranks_per_thread ) const;

  /**
   * Returns assigned ranks per thread to fill MPI buffers.
   *
   * Thread tid is responsible for all ranks in [assigned_ranks.begin,
   * assigned_ranks.end), which are in total assigned_ranks.size and
   * at most assigned_ranks.max_size
   */
  AssignedRanks get_assigned_ranks( const size_t tid );

private:
  const bool force_singlethreading_;
  size_t n_threads_; //!< Number of threads per process.
};

inline size_t
VPManager::get_thread_id() const
{
#ifdef _OPENMP
  return omp_get_thread_num();
#else
  return 0;
#endif
}

inline size_t
VPManager::get_num_threads() const
{
  return n_threads_;
}

inline void
VPManager::assert_single_threaded() const
{
#ifdef _OPENMP
  assert( omp_get_num_threads() == 1 );
#endif
}

inline void
VPManager::assert_thread_parallel() const
{
#ifdef _OPENMP
  // omp_get_num_threads() returns int
  assert( omp_get_num_threads() == static_cast< int >( n_threads_ ) );
#endif
}

inline size_t
VPManager::get_vp() const
{
  return kernel::manager< MPIManager >().get_rank()
    + get_thread_id() * kernel::manager< MPIManager >().get_num_processes();
}

inline size_t
VPManager::node_id_to_vp( const size_t node_id ) const
{
  return node_id % get_num_virtual_processes();
}

inline size_t
VPManager::vp_to_thread( const size_t vp ) const
{
  return vp / kernel::manager< MPIManager >().get_num_processes();
}

inline size_t
VPManager::get_num_virtual_processes() const
{
  return get_num_threads() * kernel::manager< MPIManager >().get_num_processes();
}

inline bool
VPManager::is_local_vp( const size_t vp ) const
{
  return kernel::manager< MPIManager >().get_process_id_of_vp( vp ) == kernel::manager< MPIManager >().get_rank();
}

inline size_t
VPManager::thread_to_vp( const size_t tid ) const
{
  return tid * kernel::manager< MPIManager >().get_num_processes() + kernel::manager< MPIManager >().get_rank();
}

inline bool
VPManager::is_node_id_vp_local( const size_t node_id ) const
{
  return ( node_id % get_num_virtual_processes() == static_cast< size_t >( get_vp() ) );
}

inline size_t
VPManager::node_id_to_lid( const size_t node_id ) const
{
  // starts at lid 0 for node_ids >= 1 (expected value for neurons, excl. node ID 0)
  return std::ceil( static_cast< double >( node_id ) / get_num_virtual_processes() ) - 1;
}

inline size_t
VPManager::lid_to_node_id( const size_t lid ) const
{
  const size_t vp = get_vp();
  return ( lid + static_cast< size_t >( vp == 0 ) ) * get_num_virtual_processes() + vp;
}

inline size_t
VPManager::get_num_assigned_ranks_per_thread() const
{
  return std::ceil( static_cast< double >( kernel::manager< MPIManager >().get_num_processes() ) / n_threads_ );
}

inline size_t
VPManager::get_start_rank_per_thread( const size_t tid ) const
{
  return tid * get_num_assigned_ranks_per_thread();
}

inline size_t
VPManager::get_end_rank_per_thread( const size_t rank_start, const size_t num_assigned_ranks_per_thread ) const
{
  size_t rank_end = rank_start + num_assigned_ranks_per_thread;

  // if we have more threads than ranks, or if ranks can not be
  // distributed evenly on threads, we need to make sure, that all
  // threads care only about existing ranks
  if ( rank_end > kernel::manager< MPIManager >().get_num_processes() )
  {
    rank_end = std::max( rank_start, kernel::manager< MPIManager >().get_num_processes() );
  }

  return rank_end;
}

inline AssignedRanks
VPManager::get_assigned_ranks( const size_t tid )
{
  AssignedRanks assigned_ranks;
  assigned_ranks.begin = get_start_rank_per_thread( tid );
  assigned_ranks.max_size = get_num_assigned_ranks_per_thread();
  assigned_ranks.end = get_end_rank_per_thread( assigned_ranks.begin, assigned_ranks.max_size );
  assigned_ranks.size = assigned_ranks.end - assigned_ranks.begin;
  return assigned_ranks;
}

} // namespace nest

#endif /* #ifndef VP_MANAGER_H */
