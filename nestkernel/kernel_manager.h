/*
 *  kernel_manager.h
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

#ifndef KERNEL_MANAGER_H
#define KERNEL_MANAGER_H

// Includes from nestkernel:
#include "connection_manager.h"
#include "event_delivery_manager.h"
#include "io_manager.h"
#include "logging_manager.h"
#include "model_manager.h"
#include "modelrange_manager.h"
#include "mpi_manager.h"
#include "music_manager.h"
#include "node_manager.h"
#include "random_manager.h"
#include "simulation_manager.h"
#include "sp_manager.h"
#include "vp_manager.h"

// Includes from sli:
#include "dictdatum.h"

// clang-format off
/** @BeginDocumentation
 Name: kernel - Global properties of the simulation kernel.

 Description:
 Global properties of the simulation kernel.

 Parameters:
 The following parameters are available in the kernel status dictionary.

 Time and resolution
 resolution                    doubletype  - The resolution of the simulation (in ms)
 time                          doubletype  - The current simulation time
 to_do                         integertype - The number of steps yet to be simulated (read only)
 max_delay                     doubletype  - The maximum delay in the network
 min_delay                     doubletype  - The minimum delay in the network
 ms_per_tic                    doubletype  - The number of milliseconds per tic
 tics_per_ms                   doubletype  - The number of tics per millisecond
 tics_per_step                 integertype - The number of tics per simulation time step
 T_max                         doubletype  - The largest representable time value (read only)
 T_min                         doubletype  - The smallest representable time value (read only)

 Parallel processing
 total_num_virtual_procs       integertype - The total number of virtual processes
 local_num_threads             integertype - The local number of threads
 num_processes                 integertype - The number of MPI processes (read only)
 off_grid_spiking              booltype    - Whether to transmit precise spike times in MPI
                                             communication (read only)

 Random number generators
 rng_types                     arraytype   - Names of random number generator types available (read only)
 rng_type                      stringtype  - Name of random number generator type used by kernel
 rng_seed                      integertype - Seed value used as basis of seeding of all random number
                                             generators managed by the kernel (\f$1 leq s \leq 2^{32}-1\f$).

 Output
 data_path                     stringtype  - A path, where all data is written to
                                             (default is the current directory)
 data_prefix                   stringtype  - A common prefix for all data files
 overwrite_files               booltype    - Whether to overwrite existing data files
 print_time                    booltype    - Whether to print progress information during the simulation

 Network information
 network_size                  integertype - The number of nodes in the network (read only)
 num_connections               integertype - The number of connections in the network
                                             (read only, local only)

 Waveform relaxation method (wfr)
 use_wfr                       booltype    - Whether to use waveform relaxation method
 wfr_comm_interval             doubletype  - Desired waveform relaxation communication interval
 wfr_tol                       doubletype  - Convergence tolerance of waveform relaxation method
 wfr_max_iterations            integertype - Maximal number of iterations used for waveform relaxation
 wfr_interpolation_order       integertype - Interpolation order of polynomial used in wfr iterations

 Miscellaneous
 dict_miss_is_error            booltype    - Whether missed dictionary entries are treated as errors

 SeeAlso: Simulate, Node
 */
// clang-format on

namespace nest
{

class KernelManager
{
private:
  KernelManager();
  ~KernelManager();

  unsigned long fingerprint_;

  static KernelManager* kernel_manager_instance_;

  KernelManager( KernelManager const& );  // do not implement
  void operator=( KernelManager const& ); // do not implement

public:
  /**
   * Create/destroy and access the KernelManager singleton.
   */
  static void create_kernel_manager();
  static void destroy_kernel_manager();
  static KernelManager& get_kernel_manager();

  /**
   * Prepare kernel for operation.
   *
   * This method calls the initialization methods of the specific
   * managers in the proper order.
   *
   * @see finalize(), reset()
   */
  void initialize();

  /**
   * Take down kernel after operation.
   *
   * This method calls the finalization methods of the specific managers
   * in the proper order, i.e., inverse to initialize().
   *
   * @see initialize(), reset()
   */
  void finalize();

  /**
   * Reset kernel.
   *
   * Resets kernel by finalizing and initializing.
   *
   * @see initialize(), finalize()
   */
  void reset();

  /**
   * Change number of threads.
   *
   * The kernel first needs to be finalized with the old number of threads
   * and then initialized with the new number of threads.
   *
   * @see initialize(), finalize()
   */
  void change_number_of_threads( thread );

  void prepare();
  void cleanup();

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& );

  //! Returns true if kernel is initialized
  bool is_initialized() const;

  unsigned long get_fingerprint() const;

  LoggingManager logging_manager;
  MPIManager mpi_manager;
  VPManager vp_manager;
  RandomManager random_manager;
  SimulationManager simulation_manager;
  ModelRangeManager modelrange_manager;
  ConnectionManager connection_manager;
  SPManager sp_manager;
  EventDeliveryManager event_delivery_manager;
  ModelManager model_manager;
  MUSICManager music_manager;
  NodeManager node_manager;
  IOManager io_manager;

private:
  std::vector< ManagerInterface* > managers;
  bool initialized_; //!< true if all sub-managers initialized
};

KernelManager& kernel();

} // namespace nest

inline nest::KernelManager&
nest::KernelManager::get_kernel_manager()
{
  assert( kernel_manager_instance_ );
  return *kernel_manager_instance_;
}

inline nest::KernelManager&
nest::kernel()
{
  return KernelManager::get_kernel_manager();
}

inline bool
nest::KernelManager::is_initialized() const
{
  return initialized_;
}

inline unsigned long
nest::KernelManager::get_fingerprint() const
{
  return fingerprint_;
}

#endif /* KERNEL_MANAGER_H */
