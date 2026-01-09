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

#include "kernel_manager.h"
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

} // namespace nest

#endif /* #ifndef VP_MANAGER_H */
