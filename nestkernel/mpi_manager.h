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

#include "nest_types.h"
#include "manager_interface.h"
#include "dictdatum.h"
#include "communicator.h"

namespace nest
{

class MPIManager : ManagerInterface
{
public:
  MPIManager();
  ~MPIManager()
  {
  }

  virtual void init_mpi( int* argc, char** argv[] );
  virtual void init();
  virtual void reset();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

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
   * @note The `called_by_reset` parameter is a cludge to avoid a chicken-and-egg
   *       problem when resetting the kernel. It surpresses a test for existing
   *       nodes, trusting that the kernel will immediately afterwards delete all
   *       existing nodes.
   */
  void set_num_rec_processes( int nrp, bool called_by_reset );

  /**
   * Return the process id for a given virtual process. The real process' id
   * of a virtual process is defined by the relation: p = (vp mod P), where
   * P is the total number of processes.
   */
  thread get_process_id( thread vp ) const;

private:
  int num_processes_; //!< number of MPI processes
  int rank_;          //!< rank of the MPI process
  index n_rec_procs_; //!< MPI processes dedicated for recording devices
  index n_sim_procs_; //!< MPI processes used for simulation
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
}

#endif /* MPI_MANAGER_H */
