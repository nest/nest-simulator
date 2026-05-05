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

// Includes from libnestutil
#include "config.h"

// Includes from nestkernel:
#include "connection_manager.h"
#include "event_delivery_manager.h"
#include "io_manager.h"
#include "logging_manager.h"
#include "model_manager.h"
#include "modelrange_manager.h"
#include "module_manager.h"
#include "mpi_manager.h"
#include "music_manager.h"
#include "node_manager.h"
#include "random_manager.h"
#include "simulation_manager.h"
#include "sp_manager.h"
#include "vp_manager.h"

#include "compose.hpp"
#include <fstream>

/**
 * Wrap debugging output code for easy enabling/disabling via -Dwith-full-logging=ON/OFF.
 */
#ifdef ENABLE_FULL_LOGGING
#define FULL_LOGGING_ONLY( code ) code
#else
#define FULL_LOGGING_ONLY( code )
#endif

namespace nest
{

/**
 * @brief Global properties of the simulation kernel.
 *
 * This class provides access to global properties of the simulation kernel.
 *
 * The following parameters are available in the kernel status dictionary:
 *
 * @par Time and resolution
 * @param biological_time The current simulation time (in ms).
 * @param max_delay The maximum delay in the network, defaults to 0.1 (in ms).
 * @param min_delay The minimum delay in the network, defaults to 0.1 (in ms).
 * @param max_update_time Longest wall-clock time measured so far for a full update step (in s; read only).
 * @param min_update_time Shortest wall-clock time measured so far for a full update step (in s; read only).
 * @param ms_per_tic The number of milliseconds per tic, calculated by ms_per_tic = 1 / tics_per_ms (read only).
 * @param resolution The resolution of the simulation (in ms), defaults to 0.1.
 * @param tics_per_ms The number of tics per millisecond, defaults to 1000.0.
 * @param tics_per_step The number of tics per simulation time step, calculated by tics_per_step = resolution *
 * tics_per_ms (read only).
 * @param to_do The number of steps yet to be simulated (read only).
 * @param T_max The largest representable time value (in ms; read only).
 * @param T_min The smallest representable time value (in ms; read only).
 * @param update_time_limit Maximum wall-clock time for one full update step (in s; default +infinity).
 *
 * @par Parallel processing
 * @param adaptive_target_buffers Whether MPI buffers for communication of connections resize on the fly, defaults to
 * true.
 * @param buffer_size_spike_data Total size of MPI buffer for communication of spikes, defaults to 2.
 * @param buffer_size_target_data Total size of MPI buffer for communication of connections, defaults to 2.
 * @param local_num_threads The local number of threads, defaults to 1.
 * @param num_processes The number of MPI processes (read only).
 * @param off_grid_spiking Whether to transmit precise spike times in MPI communication (read only).
 * @param total_num_virtual_procs The total number of virtual processes, defaults to 1.
 * @param use_compressed_spikes Whether to use spike compression, defaults to true.
 *
 * @par Random number generators
 * @param rng_seed Seed value used as basis of seeding of all random number generators managed by the kernel.
 * @param rng_type Name of random number generator type used by NEST, defaults to mt19937_64.
 * @param rng_types List of available random number generator types (read only).
 *
 * @par Output
 * @param data_path A path where all data is written to, defaults to current directory.
 * @param data_prefix A common prefix for all data files.
 * @param overwrite_files Whether to overwrite existing data files, defaults to false.
 * @param print_time Whether to print progress information during the simulation, defaults to false.
 * @param recording_backends List of available backends for recording devices (read only).
 *
 * @par Network information
 * @param connection_rules The list of available connection rules (read only).
 * @param growth_curves The list of the available structural plasticity growth curves (read only).
 * @param growth_factor_buffer_target_data If MPI buffers for communication of connections resize on the fly, grow them
 * by this factor each round, defaults to 1.5.
 * @param spike_buffer_grow_extra When spike exchange buffer is expanded, resize it to (1 + spike_buffer_grow_extra) *
 * required_buffer_size, defaults to 0.5.
 * @param spike_buffer_shrink_limit If largest number of spikes sent from any rank to any rank is less than
 * spike_buffer_shrink_limit * buffer_size, then reduce buffer size. Defaults to 0.2.
 * @param spike_buffer_resize_log Information on spike buffer resizing as dictionary (read only).
 * @param keep_source_table Whether to keep source table after connection setup is complete, defaults to true.
 * @param local_spike_counter Number of spikes fired by neurons on a given MPI rank during the most recent call to
 * Simulate() (read only).
 * @param max_num_syn_models Maximal number of synapse models supported (read only).
 * @param network_size The number of nodes in the network (read only).
 * @param node_models The list of available node models (neurons and devices; read only).
 * @param num_connections The number of connections in the network (read only; local only).
 * @param stimulation_backends List of available backends for stimulation devices (read only).
 * @param structural_plasticity_synapses Defines all synapses which are plastic for the structural plasticity algorithm.
 * @param structural_plasticity_update_interval Defines the time interval in ms at which the structural plasticity
 * manager will make changes, defaults to 10000.
 * @param synapse_models The list of the available synapse models (read only).
 *
 * @par Waveform relaxation method (wfr)
 * @param use_wfr Whether to use waveform relaxation method, defaults to true.
 * @param wfr_comm_interval Desired waveform relaxation communication interval, defaults to 1.0.
 * @param wfr_interpolation_order Interpolation order of polynomial used in wfr iterations, defaults to 3.
 * @param wfr_max_iterations Maximal number of iterations used for waveform relaxation, defaults to 15.
 * @param wfr_tol Convergence tolerance of waveform relaxation method, defaults to 0.0001.
 *
 * @par Miscellaneous
 * @param dict_miss_is_error Whether missed dictionary entries are treated as errors.
 * @param build_info Various information about the NEST build.
 * @param memory_size Memory occupied by NEST process in kB (-1 if not available for OS).
 *
 * @see Simulate, Node
 */
class KernelManager
{
private:
  KernelManager();
  ~KernelManager();

  unsigned long fingerprint_;

  static KernelManager* kernel_manager_instance_;

  KernelManager( KernelManager const& );   // do not implement
  void operator=( KernelManager const& );  // do not implement

  Dictionary get_build_info_();

public:
  /**
   * Create/destroy and access the KernelManager singleton.
   */
  static void create_kernel_manager();
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
   * Resets the kernel by finalizing and initializing all managers.
   *
   * @see initialize(), finalize()
   */
  void reset();

  /**
   * Change number of threads.
   *
   * Set the new number of threads on all managers by calling
   * change_number_of_threads() on each of them.
   */
  void change_number_of_threads( size_t new_num_threads );

  void set_status( const Dictionary& );
  void get_status( Dictionary& );

  void prepare();
  void cleanup();

  //! Returns true if kernel is initialized
  bool is_initialized() const;

  unsigned long get_fingerprint() const;

  /**
   * Write data to file per rank and thread. For use with FULL_LOGGING.
   *
   * @note This method has a `omp critical` section to avoid write-collisions from threads.
   */
  void write_to_dump( const std::string& msg );

  /**
   * \defgroup Manager components in NEST kernel
   *
   * The managers are defined below in the order in which they need to be initialized.
   *
   * NodeManager is last to ensure all model structures are in place before it is initialized.
   * @{
   */
  LoggingManager logging_manager;
  MPIManager mpi_manager;
  VPManager vp_manager;
  ModuleManager module_manager;
  RandomManager random_manager;
  SimulationManager simulation_manager;
  ModelRangeManager modelrange_manager;
  ConnectionManager connection_manager;
  SPManager sp_manager;
  EventDeliveryManager event_delivery_manager;
  IOManager io_manager;
  ModelManager model_manager;
  MUSICManager music_manager;
  NodeManager node_manager;
  /**@}*/

  //! Get the stopwatch to measure the time each thread is idle during network construction.
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded >&
  get_omp_synchronization_construction_stopwatch()
  {
    return sw_omp_synchronization_construction_;
  }

  //! Get the stopwatch to measure the time each thread is idle during simulation.
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded >&
  get_omp_synchronization_simulation_stopwatch()
  {
    return sw_omp_synchronization_simulation_;
  }

  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::MasterOnly >&
  get_mpi_synchronization_stopwatch()
  {
    return sw_mpi_synchronization_;
  }

private:
  size_t get_memsize_linux_() const;   //!< return VmSize in kB
  size_t get_memsize_darwin_() const;  //!< return resident_size in kB

  //! All managers, order determines initialization and finalization order (latter backwards)
  std::vector< ManagerInterface* > managers;

  bool initialized_;    //!< true if the kernel is initialized
  std::ofstream dump_;  //!< for FULL_LOGGING output

  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded > sw_omp_synchronization_construction_;
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded > sw_omp_synchronization_simulation_;
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::MasterOnly > sw_mpi_synchronization_;
};

KernelManager& kernel();

inline RngPtr
get_rank_synced_rng()
{
  return kernel().random_manager.get_rank_synced_rng();
}

inline RngPtr
get_vp_synced_rng( size_t tid )
{
  return kernel().random_manager.get_vp_synced_rng( tid );
}

inline RngPtr
get_vp_specific_rng( size_t tid )
{
  return kernel().random_manager.get_vp_specific_rng( tid );
}

}  // namespace nest

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
