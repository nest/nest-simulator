/*
 *  simulation_manager.h
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

#ifndef SIMULATION_MANAGER_H
#define SIMULATION_MANAGER_H

// C includes:
#include <sys/time.h>

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"
#include "stopwatch_impl.h"

// Includes from nestkernel:
#include "nest_time.h"
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{
class Node;

class SimulationManager : public ManagerInterface
{
public:
  SimulationManager();

  void initialize( const bool ) override;
  void finalize( const bool ) override;
  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) override;

  /**
   *  Check for errors in time before run
   *
   *   @throws KernelException if illegal time passed
   */
  void assert_valid_simtime( Time const& );

  /**
   * Initialize simulation for a set of run calls.
   *
   * Must be called before a sequence of runs, and again after cleanup.
   */
  void prepare() override;

  /**
   * Run a simulation for another `Time`.
   *
   * Can be repeated ad infinitum with
   * calls to get_status(), but any changes to the network are undefined,
   * leading serious risk of incorrect results.
   */
  void run( Time const& );

  /**
   * Closes a set of runs, doing finalizations such as file closures.
   *
   * After cleanup() is called, no more run()s can be called before another
   * prepare() call.
   */
  void cleanup() override;

  /**
   * Returns true if waveform relaxation is used.
   */
  bool use_wfr() const;

  /**
   * Get the desired communication interval for the waveform relaxation
   */
  double get_wfr_comm_interval() const;

  /**
   * Get the convergence tolerance of the waveform relaxation method
   */
  double get_wfr_tol() const;

  /**
   * Get the interpolation order of the waveform relaxation method
   */
  size_t get_wfr_interpolation_order() const;

  /**
   * Get the time at the beginning of the current time slice.
   */
  Time const& get_slice_origin() const;

  /**
   * Get the time at the beginning of the previous time slice.
   */
  Time const get_previous_slice_origin() const;

  /**
   * Precise time of simulation.
   *
   * @note The precise time of the simulation is defined only
   *       while the simulation is not in progress.
   */
  Time const get_time() const;

  /**
   * Return true, if the SimulationManager has already been simulated for some
   * time.
   *
   * This does NOT indicate that simulate has been called (i.e. if
   * Simulate is called with 0 as argument, the flag is still set to false.)
   */
  bool has_been_simulated() const;

  /**
   * Return true, if the SimulationManager has been prepared for simulation.
   * This is the case from the time when the Prepare function is called until
   * the simulation context is left by a call to Cleanup.
   */
  bool has_been_prepared() const;

  /**
   * Get slice number. Increased by one for each slice. Can be used
   * to choose alternating buffers.
   */
  size_t get_slice() const;

  //! Return current simulation time.
  // TODO: Precisely how defined? Rename!
  Time const& get_clock() const;

  /**
   * Get the simulation duration in the current call to run().
   */
  Time run_duration() const;

  /**
   * Get the start time of the current call to run().
   */
  Time run_start_time() const;

  /**
   * Get the simulation's time at the end of the current call to run().
   */
  Time run_end_time() const;

  //! Return start of current time slice, in steps.
  // TODO: rename / precisely how defined?
  long get_from_step() const;

  //! Return end of current time slice, in steps.
  // TODO: rename / precisely how defined?
  long get_to_step() const;

  //! Sorts source table and connections and create new target table.
  void update_connection_infrastructure( const size_t tid );

  /**
   * Set time measurements for internal profiling to zero (reg. prep.)
   */
  virtual void reset_timers_for_preparation();

  /**
   * Set time measurements for internal profiling to zero (reg. sim. dyn.)
   */
  virtual void reset_timers_for_dynamics();

  Time get_eprop_update_interval() const;
  Time get_eprop_learning_window() const;
  bool get_eprop_reset_neurons_on_update() const;

  //! Get the stopwatch to measure the time each thread is idle during network construction.
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded >&
  get_omp_synchronization_construction_stopwatch();

  //! Get the stopwatch to measure the time each thread is idle during simulation.
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded >&
  get_omp_synchronization_simulation_stopwatch();

  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::MasterOnly >& get_mpi_synchronization_stopwatch();

private:
  void call_update_(); //!< actually run simulation, aka wrap update_
  void update_();      //! actually perform simulation
  bool wfr_update_( Node* );
  void advance_time_();   //!< Update time to next time step
  void print_progress_(); //!< TODO: Remove, replace by logging!

  Time clock_;                     //!< SimulationManager clock, updated once per slice
  long slice_;                     //!< current update slice
  long to_do_;                     //!< number of pending steps
  long to_do_total_;               //!< number of requested steps in current simulation
  long from_step_;                 //!< update clock_+from_step<=T<clock_+to_step_
  long to_step_;                   //!< update clock_+from_step<=T<clock_+to_step_
  timeval t_slice_begin_;          //!< Wall-clock time at the begin of a time slice
  timeval t_slice_end_;            //!< Wall-clock time at the end of time slice
  long t_real_;                    //!< Accumulated wall-clock time spent simulating (in us)
  bool prepared_;                  //!< Indicates whether the SimulationManager is in a prepared
                                   //!< state
  bool simulating_;                //!< true if simulation in progress
  bool simulated_;                 //!< indicates whether the SimulationManager has already been
                                   //!< simulated for sometime
  bool inconsistent_state_;        //!< true after exception during update_
                                   //!< simulation must not be resumed
  bool print_time_;                //!< Indicates whether time should be printed during
                                   //!< simulations (or not)
  bool use_wfr_;                   //!< Indicates wheter waveform relaxation is used
  double wfr_comm_interval_;       //!< Desired waveform relaxation communication
                                   //!< interval (in ms)
  double wfr_tol_;                 //!< Convergence tolerance of waveform relaxation method
  long wfr_max_iterations_;        //!< maximal number of iterations used for waveform
                                   //!< relaxation
  size_t wfr_interpolation_order_; //!< interpolation order for waveform
                                   //!< relaxation method
  double update_time_limit_;       //!< throw exception if single update cycle takes longer
                                   //!< than update_time_limit_ (seconds, default inf)
  double min_update_time_;         //!< shortest update time seen so far (seconds)
  double max_update_time_;         //!< longest update time seen so far (seconds)

  // private stop watches for benchmarking purposes
  Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::MasterOnly > sw_simulate_;
  Stopwatch< StopwatchGranularity::Normal, StopwatchParallelism::Threaded > sw_communicate_prepare_;
  // intended for internal core developers, not for use in the public API
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::MasterOnly > sw_gather_spike_data_;
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::MasterOnly > sw_gather_secondary_data_;
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded > sw_update_;
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded > sw_gather_target_data_;
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded > sw_deliver_spike_data_;
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded > sw_deliver_secondary_data_;

  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded > sw_omp_synchronization_construction_;
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::Threaded > sw_omp_synchronization_simulation_;
  Stopwatch< StopwatchGranularity::Detailed, StopwatchParallelism::MasterOnly > sw_mpi_synchronization_;

  double eprop_update_interval_;
  double eprop_learning_window_;
  bool eprop_reset_neurons_on_update_;
};


}


#endif /* #ifndef SIMULATION_MANAGER_H */
