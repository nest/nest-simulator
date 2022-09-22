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

/** @BeginDocumentation
 Name: kernel - Global properties of the simulation kernel.

 Description:
 Global properties of the simulation kernel.

 Parameters:
 The following parameters are available in the kernel status dictionary.

 kernel_status                         dicttype    - Get the complete kernel status (read only).

 Time and resolution
 biological_time                       doubletype  - The current simulation time (in ms).
 max_delay                             doubletype  - The maximum delay in the network, defaults to 0.1 (in ms).
 min_delay                             doubletype  - The minimum delay in the network, defaults to 0.1 (in ms).
 max_update_time                       doubletype  - Longest wall-clock time measured so far for a full update step (in
                                                     s; read only).
 min_update_time                       doubletype  - Shortest wall-clock time measured so far for a full update step (in
                                                     s; read only).
 ms_per_tic                            doubletype  - The number of milliseconds per tic, defaults to 0.001.
 resolution                            doubletype  - The resolution of the simulation (in ms), defaults to 0.1.
 time                                  doubletype  - The current simulation time (in ms).
 tics_per_ms                           doubletype  - The number of tics per millisecond, defaults to 1000.0.
 tics_per_step                         integertype - The number of tics per simulation time step, defaults to 100.
 to_do                                 integertype - The number of steps yet to be simulated (read only).
 T_max                                 doubletype  - The largest representable time value (in ms; read only).
 T_min                                 doubletype  - The smallest representable time value (in ms; read only).
 update_time_limit                     doubletype  - Maximum wall-clock time for one full update step (in s; default
                                                     +∞). This can be used to terminate simulations that slow down
                                                     significantly. Simulations may still get stuck if the slowdown
                                                     occurs within a single update step.

 Parallel processing
 adaptive_spike_buffers                booltype    - Whether MPI buffers for communication of spikes resize on the fly,
                                                     defaults to true.
 adaptive_target_buffers               booltype    - Whether MPI buffers for communication of connections resize on the
                                                     fly, defaults to true.
 buffer_size_secondary_events          integertype - Size of MPI buffers for communicating secondary events (in bytes;
                                                     per MPI rank; for developers; read only).
 buffer_size_spike_data                integertype - Total size of MPI buffer for communication of spikes, defaults to
                                                     2.
 buffer_size_target_data               integertype - Total size of MPI buffer for communication of connections, defaults
                                                     to 2.
 local_num_threads                     integertype - The local number of threads, defaults to 1.
 max_buffer_size_spike_data            integertype - Maximal size of MPI buffers for communication of spikes, defaults
                                                     to 8388608.
 max_buffer_size_target_data           integertype - Maximal size of MPI buffers for communication of connections,
                                                     defaults to 16777216.
 num_processes                         integertype - The number of MPI processes (read only).
 off_grid_spiking                      booltype    - Whether to transmit precise spike times in MPI communication (read
                                                     only).
 sort_connections_by_source            booltype    - Whether to sort connections by their source; increases construction
                                                     time of presynaptic data structures, decreases simulation time if
                                                     the average number of outgoing connections per neuron is smaller
                                                     than the total number of threads, defaults to true.
 total_num_virtual_procs               integertype - The total number of virtual processes, defaults to 1.
 use_compressed_spikes                 booltype    - Whether to use spike compression; if a neuron has targets on
                                                     multiple threads of a process, this switch makes sure that only a
                                                     single packet is sent to the process instead of one packet per
                                                     target thread (requires sort_connections_by_source = true),
                                                     defaults to true.

 Random number generators
 rng_seed                              integertype - Seed value used as basis of seeding of all random number generators
                                                     managed by the kernel (\f$1 leq s \leq 2^{32}-1\f$).
 rng_type                              stringtype  - Name of random number generator type used by NEST, defaults to
                                                     mt19937_64.
 rng_types                             arraytype   - List of available random number generator types (read only).

 Output
 data_path                             stringtype  - A path, where all data is written to, defaults to current
                                                     directory.
 data_prefix                           stringtype  - A common prefix for all data files.
 overwrite_files                       booltype    - Whether to overwrite existing data files, defaults to false.
 print_time                            booltype    - Whether to print progress information during the simulation,
                                                     defaults to false.
 recording_backends                    arraytype   - List of available backends for recording devices (read only).

 Network information
 connection_rules                      arraytype   - The list of available connection rules (read only).
 growth_curves                         arraytype   - The list of the available structural plasticity growth curves (read
                                                     only).
 growth_factor_buffer_spike_data       double      - If MPI buffers for communication of spikes resize on the fly, grow
                                                     them by this factor each round, defaults to 1.5.
 growth_factor_buffer_target_data      double      - If MPI buffers for communication of connections resize on the fly,
                                                     grow them by this factor each round, defaults to 1.5.
 keep_source_table                     booltype    - Whether to keep source table after connection setup is complete,
                                                     defaults to true.
 local_spike_counter                   integertype - Number of spikes fired by neurons on a given MPI rank during the
                                                     most recent call to Simulate(). Only spikes from “normal” neurons
                                                     are counted, not spikes generated by devices such as
                                                     poisson_generator (read only).
 max_num_syn_models                    integertype - Maximal number of synapse models supported (read only).
 network_size                          integertype - The number of nodes in the network (read only).
 node_models                           arraytype   - The list of available node models (neurons and devices; read only).
 num_connections                       integertype - The number of connections in the network (read only; local only).
 stimulation_backends                  arraytype   - List of available backends for stimulation devices (read-only).
 structural_plasticity_synapses        dicttype    - Defines all synapses which are plastic for the structural
                                                     plasticity algorithm. Each entry in the dictionary is composed of a
                                                     synapse model, the presynaptic element and the postsynaptic
                                                     element.
 structural_plasticity_update_interval integertype - Defines the time interval in ms at which the structural plasticity
                                                     manager will make changes in the structure of the network (creation
                                                     and deletion of plastic synapses), defaults to 10000.0.
 synapse_models                        arraytype   - The list of the available synapse models (read only).

 Waveform relaxation method (wfr)
 use_wfr                               booltype    - Whether to use waveform relaxation method, defaults to true.
 wfr_comm_interval                     doubletype  - Desired waveform relaxation communication interval, defaults to
                                                     1.0.
 wfr_interpolation_order               integertype - Interpolation order of polynomial used in wfr iterations, defaults
                                                     to 3.
 wfr_max_iterations                    integertype - Maximal number of iterations used for waveform relaxation, defaults
                                                     to 15.
 wfr_tol                               doubletype  - Convergence tolerance of waveform relaxation method, defaults to
                                                     0.0001.

 Miscellaneous
 dict_miss_is_error                    booltype    - Whether missed dictionary entries are treated as errors.

 SeeAlso: Simulate, Node
*/

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
  void change_number_of_threads( thread new_num_threads );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& );

  void prepare();
  void cleanup();

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
  bool initialized_; //!< true if the kernel is initialized
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
